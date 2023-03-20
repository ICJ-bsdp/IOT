//indentured definations
#include <Arduino.h>
#include <U8g2lib.h> //install this

#include <bits/stdc++.h>
#include <vector>
#include <SPI.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

using namespace std;

//GND plugged to GND
//DIN to 23
//CLK to 18
#define OLED_DC  17 //DC plugged on port 17
#define OLED_CS  5 //CS plugged on port 5
#define OLED_RST 15 //RST plugged on 15

//driver api don't touch
U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI u8g2(/* rotation=*/U8G2_R1, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC,/* reset=*/OLED_RST);

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

//make sure this also matches the android applet so it writes to right characteristic & service
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setUpBLE(string);

//homemaid engines
class Word {
  public: 
    double lifetime = 5000; //ms
    double createdTick;
    string s;

    Word(string inStr)
    {
      createdTick = millis();
      s = inStr;
    }

    Word (string inStr, int inLifetime)
    {
      createdTick = millis();
      s = inStr;
      lifetime = inLifetime;
    }

    void update()
    {
      //negative lifetime = permenant
      if (lifetime < 0)
      {
        return;
      }

      double timeNow = millis();
      if (createdTick + lifetime < timeNow)
      {
        s = "";
      }
    }
};

//8 words per row
class SubtitleEngine {
  public:
    vector<Word> log;
    SubtitleEngine() {}

    void addWord(string word, int lifetimeOverride)
    {
      Word typed(word, lifetimeOverride);
      addWord(typed);
    }

    void addWord(string word)
    {
      Word typed(word);
      addWord(typed);
    }

    void addWord(Word word)
    {
      if (log.size() >= 8)
      {
        log.erase(log.begin());
      }
      log.push_back(word);
    }

    void printToScreen() {
      //centering algorithm
      const int height = 128;
      const int starting = 128 / 2 + (-1 * (15 * log.size()) / 2);
      for (int i = 0; i < log.size(); ++i)
      {
        u8g2.drawStr(0, starting + 15 * i, log[i].s.c_str());
      }
    }

    void clear() {
      log.clear();
    }

    void cleanUp(){
      int i = 0;
      while (i < log.size())
      {
        log[i].update();
        if (log[i].s == "")
        {
          log.erase(log.begin() + i);
        }
        i++;
      }
    }
};

SubtitleEngine engine;
bool deviceConnected = false;
bool lastTickDeviceConnected = false;

class CommandEngine {
  public:

  string getParameters(string unfiltered, string command) {
    return unfiltered.substr(command.length());
  }

  CommandEngine(string cmd)
  {
    string processed = cmd.substr(5);
    Serial.print("CMD Engine INI");

    if (processed.rfind("CHANGE_NAME", 0) == 0)
    {
      changeDeviceName(getParameters(processed, "CHANGE_NAME"));
      return;
    } 
  }

  void changeDeviceName(string newName)
  {
    //
  }
};

//Bluetooth Low Energy callback on written to
class BLEWriteCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.rfind("[CMD]") == 0)
      {
        CommandEngine c(value);
        return;
      }

      if (value.length() > 0) {
        Serial.println(value.c_str());
        engine.addWord(value);
      }
    }
};

class ServerConnectionCallback: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setUpBLE(string name)
{
  BLEDevice::init(name);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->setCallbacks(new BLEWriteCallback());
  pServer->setCallbacks(new ServerConnectionCallback());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void setup(void) {
  //setup console (make sure telemetry is also set to 115200 baud)
  Serial.begin(115200);

  //setup display
  u8g2.setFontPosTop();
  u8g2.begin();  
  u8g2.setFont(u8g2_font_10x20_tf);

  engine.addWord("Starting", -1);

  //setup BLE service 
  setUpBLE("SLATE Glass");

  //writing to engine
  engine.clear();
  engine.addWord("Open", -1);
  engine.addWord("To", -1);
  engine.addWord("Pair", -1);
}

void loop()
{
  //new connection
  if (deviceConnected && !lastTickDeviceConnected)
  {
    engine.clear();
    engine.addWord("Paired");
    BLEDevice::stopAdvertising();
    lastTickDeviceConnected = deviceConnected;
  }
  //connection dropped
  else if (!deviceConnected && lastTickDeviceConnected)
  {
    engine.clear();
    engine.addWord("Open", -1);
    engine.addWord("To", -1);
    engine.addWord("Pair", -1);
    BLEDevice::startAdvertising();
    lastTickDeviceConnected = deviceConnected;
  }

  u8g2.firstPage();   
  do
  {
    engine.cleanUp(); //kills all old words that expired lifetimes
    engine.printToScreen(); //prints to screen words in top down based on recentness (most recent at bottom)
  } while(u8g2.nextPage());
  delay(100);
}