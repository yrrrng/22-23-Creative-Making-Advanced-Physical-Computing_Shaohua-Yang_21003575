

#include "DHT.h" //Shaohua Yang
#include <CapacitiveSensor.h>
#define DHTPIN 4                                    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11                               // DHT 11
CapacitiveSensor cs_4_2 = CapacitiveSensor(9, A1);  // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
DHT dht(DHTPIN, DHTTYPE);
#include <FastLED.h>
#include <DFRobot_DF1201S.h>
#include <SoftwareSerial.h>
#include "OLED_Driver.h"
#include "GUI_paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include "ImageData.h"

SoftwareSerial DF1201SSerial(2, 3);  //RX  TX

DFRobot_DF1201S DF1201S;

// How many leds in your strip?  
#define NUM_LEDS 1  //Jin Yu
#define DATA_PIN 5
CRGB leds[NUM_LEDS];
void setup() {
  // put your setup code here, to run once:
  //set Serial
  Serial.begin(115200);

  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);  // turn off autocalibrate on channel 1 - just as an example
  //Serial.begin(9600);   //Jin Yu
  Serial.println(F("DHTxx test!"));
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);  // heat panel work
  delay(1000);
  digitalWrite(A2, LOW);
  dht.begin();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed  //Jin Yu


  DF1201SSerial.begin(115200);     //Jin Yu
  while (!DF1201S.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }
  /*Set volume to 20*/
  DF1201S.setVol(/*VOL = */ 20);  //Jin Yu
  Serial.print("VOL:");
  /*Get volume*/
  Serial.println(DF1201S.getVol());
  /*Enter music mode*/
  DF1201S.switchFunction(DF1201S.MUSIC);
  /*Wait for the end of the prompt tone */
  delay(2000);
  /*Set playback mode to "repeat all"*/
  DF1201S.setPlayMode(DF1201S.SINGLE);
  Serial.print("PlayMode:");
  /*Get playback mode*/
  Serial.println(DF1201S.getPlayMode());

  DF1201S.playFileNum(/*File Number = */ 1);

  System_Init();
  if (USE_IIC) {
    Serial.print("Only USE_SPI_4W, Please revise DEV_config.h !!!");
    return 0;
  }

  Serial.print(F("OLED_Init()...\r\n"));
  OLED_1in5_rgb_Init();
  Driver_Delay_ms(500);
  OLED_1in5_rgb_Clear();      //Jin Yu

  //1.Create a new image size  //Shaohua Yang
  UBYTE *BlackImage;
  Serial.print("Paint_NewImage\r\n");
  Paint_NewImage(BlackImage, OLED_1in5_RGB_WIDTH, OLED_1in5_RGB_HEIGHT, 270, BLACK);
  Paint_SetScale(65);
  Paint_SetRotate(0);
  Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, BLUE);
  Paint_DrawNum(10, 30, "123.4567", &Font12, 2, RED, BLACK);
  Paint_DrawString_CN(10, 50, "你好Ab", &Font12CN, BLACK, BROWN);
  Paint_DrawString_CN(0, 80, "微雪电子", &Font24CN, BLACK, BRED);
  Driver_Delay_ms(2000);
  OLED_1in5_rgb_Clear();   //Shaohua Yang
}

long readUltrasonicDistance(int triggerPin, int echoPin) {         //Shaohua Yang
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);   //Shaohua Yang
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}
int flag = 0;   //Shaohua Yang
long counterMillisDist = 0;
long counterMillisHeat = 0;
int breathRed = 255;
int breathRedDir = -1;
long resetTimer = 0;

int cloeFlag = 0;
long millTouch=0;
void loop() {    //Shaohua Yang

  int soil =100- analogRead(A0) / 1024.0 * 100;
  int ultra = 0.006783 * readUltrasonicDistance(6, A3);  //distance
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  long touch = cs_4_2.capacitiveSensor(30);  //Shaohua Yang
  if (t < 20 || t > 32) {
    cloeFlag = 0;
    if (ultra < 30) {
      if (millis() - counterMillisDist > 20000) {
        DF1201S.playFileNum(/*File Number = */ 1);
        counterMillisDist = millis();
      }
    }
    if (millis() - counterMillisHeat <= 60000) {
      digitalWrite(A2, HIGH);
    } else {
      digitalWrite(A2, LOW);
    }
    FastLED.setBrightness(breathRed);
    leds[0] = CRGB::Red;
    FastLED.show();
    breathRed = breathRed + breathRedDir * 6;
    if (breathRed <= 0 && breathRedDir == -1) {
      breathRedDir = 1;
    } else if (breathRed >= 255 && breathRedDir == 1) {
      breathRedDir = -1;
    }
  } else if (soil < 40) {
    cloeFlag = 0;
    if (ultra < 30) {
      if (millis() - counterMillisDist > 20000) {
        DF1201S.playFileNum(/*File Number = */ 3);
        counterMillisDist = millis();
      }
    }
    if (millis() - counterMillisHeat <= 60000) {
      digitalWrite(A2, HIGH);
    } else {
      digitalWrite(A2, LOW);
    }
    FastLED.setBrightness(breathRed); //Shaohua Yang
    leds[0] = CRGB::Red;
    FastLED.show();
    breathRed = breathRed + breathRedDir * 6;
    if (breathRed <= 0 && breathRedDir == -1) {
      breathRedDir = 1;
    } else if (breathRed >= 255 && breathRedDir == 1) {
      breathRedDir = -1;
    }
  } else {
    counterMillisHeat = millis(); //Shaohua Yang
    if (ultra < 30) {
      if (cloeFlag == 0) {
        cloeFlag = 1;
        DF1201S.playFileNum(/*File Number = */ 4);
      }
    }else{
      cloeFlag=0;
    }
    if(touch>200 && millis()-millTouch>10000){
      DF1201S.playFileNum(/*File Number = */ 2);
      millTouch=millis();
    }
    FastLED.setBrightness(breathRed);  //Jin Yu
    leds[0] = CRGB::Green;
    FastLED.show();
    breathRed = breathRed + breathRedDir * 6;
    if (breathRed <= 0 && breathRedDir == -1) {
      breathRedDir = 1;
    } else if (breathRed >= 255 && breathRedDir == 1) {
      breathRedDir = -1;
    }
  }  
  // put your main code here, to run repeatedly:
  Serial.print("Soil Humidity: ");//Jin Yu
  Serial.print(soil);
  Serial.print("%  Distance: ");
  Serial.print(ultra);
  Serial.print("cm  Temperature: ");
  Serial.print(t);
  Serial.print("C  Humidity: ");
  Serial.print(h);
  Serial.print("%  Touch: ");
  Serial.println(touch);

  if (millis() - resetTimer > 2000) {  //Shaohua Yang
    OLED_1in5_rgb_Clear();
    Paint_DrawString_EN(10, 0, ("Soil:" + String(soil) + "%").c_str(), &Font12, BLACK, BLUE);

    Paint_DrawString_EN(10, 16, ("Distance:" + String(ultra) + "cm").c_str(), &Font12, BLACK, BLUE);
    Paint_DrawString_EN(10, 32, ("Temperature:" + String(int(t)) + "C").c_str(), &Font12, BLACK, BLUE);

    Paint_DrawString_EN(10, 48, ("Humidity:" + String(int(h)) + "%").c_str(), &Font12, BLACK, BLUE);
    Paint_DrawString_EN(10, 64, ("Touch:" + String(touch)).c_str(), &Font12, BLACK, BLUE);
    resetTimer = millis();
  }
}
