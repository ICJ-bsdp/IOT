
/*!
 * @file Cube.ino
 * @brief Rotating 3D stereoscopic graphics
 * @n This is a simple rotating tetrahexon
 * 
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author [Ivey](Ivey.lu@dfrobot.com)
 * @maintainer [Fary](feng.yang@dfrobot.com)
 * @version  V1.0
 * @maintainer [Fary](feng.yang@dfrobot.com)
 * @version  V1.0
 * @date  2019-10-15
 * @url https://github.com/DFRobot/U8g2_Arduino
*/

#include <Arduino.h>
#include <U8g2lib.h>

#include <bits/stdc++.h>
#include <vector>
#include <SPI.h>

using namespace std;


/*
 * Display hardware IIC interface constructor
 *@param rotation：U8G2_R0 Not rotate, horizontally, draw direction from left to right
           U8G2_R1 Rotate clockwise 90 degrees, drawing direction from top to bottom
           U8G2_R2 Rotate 180 degrees clockwise, drawing in right-to-left directions
           U8G2_R3 Rotate clockwise 270 degrees, drawing direction from bottom to top
           U8G2_MIRROR Normal display of mirror content (v2.6.x version used above)
           Note: U8G2_MIRROR need to be used with setFlipMode().
 *@param reset：U8x8_PIN_NONE Indicates that the pin is empty and no reset pin is used
 * Display hardware SPI interface constructor
 *@param  Just connect the CS pin (pins are optional)
 *@param  Just connect the DC pin (pins are optional)
 *
*/
#define OLED_DC  17
#define OLED_CS  5
#define OLED_RST 15
U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI u8g2(/* rotation=*/U8G2_R1, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC,/* reset=*/OLED_RST);

class SubtitleEngine {
  public:
    vector<string> log;
    // U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI oledDisplay;

    // SubtitleEngine(const U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI &oledDisplay) : oledDisplay(oledDisplay) {}
    SubtitleEngine() {}

    void addWord(string word)
    {
      log.push_back(word);
    }

    void printToScreen() {
      for (int i = 0; i < log.size(); ++i)
      {
        u8g2.drawStr(0, 15 * i, log[i].c_str());
      }
    }

    void clear() {
      log.clear();
    }
};

SubtitleEngine engine;

void setup(void) {
  u8g2.setFontPosTop();
  u8g2.begin();  
  u8g2.setFont(u8g2_font_10x20_tf);
  engine.addWord("Penis");
  engine.addWord("Penis");
  engine.addWord("Penis");
  engine.addWord("Penis");
  engine.addWord("Penis");
}

void loop()
{
  u8g2.firstPage();
  do
  {
    engine.printToScreen();
  } while( u8g2.nextPage());
  // delay(1000);
}