//indentured definations
#include <Arduino.h>
#include <U8g2lib.h>

#include <bits/stdc++.h>
#include <vector>
#include <SPI.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

using namespace std;

#define OLED_DC  17
#define OLED_CS  5
#define OLED_RST 15
U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI u8g2(/* rotation=*/U8G2_R1, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC,/* reset=*/OLED_RST);

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

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

    void update()
    {
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
    // U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI oledDisplay;

    // SubtitleEngine(const U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI &oledDisplay) : oledDisplay(oledDisplay) {}
    SubtitleEngine() {}

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

//Bluetooth Low Energy callback
class BLECallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println(value.c_str());
        engine.addWord(value);
      }
    }
};
void setup(void) {
  //setup console (make sure telemetry is also set to 115200 baud)
  Serial.begin(115200);

  //setup display
  u8g2.setFontPosTop();
  u8g2.begin();  
  u8g2.setFont(u8g2_font_10x20_tf);

  //setup BLE service 
  BLEDevice::init("BSDP Network");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->setCallbacks(new BLECallback());

  // pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop()
{
  u8g2.firstPage();   
  do
  {
    engine.cleanUp(); //kills all old words that expired lifetimes
    engine.printToScreen(); //prints to screen words in top down based on recentness (most recent at bottom)
  } while(u8g2.nextPage());
  delay(100);
}