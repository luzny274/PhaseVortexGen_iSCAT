
#include "image1.h"   
#include "image2.h"   
#include "image3.h"   
#include "image4.h"   
#include "image5.h"   
#include "image6.h"   
#include "image7.h"   
#include "image8.h"   
#include "image9.h"
 
#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 3; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {14, 15, 16, 17}; 
byte colPins[COLS] = {18, 19, 20}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  for(int i = 2; i < 14; i++) pinMode(i, OUTPUT);
  for(int i = 21; i < 53; i++) pinMode(i, OUTPUT);

  REG_PIOC_OWER = 0xFFFFFFFF;
  REG_PIOC_OER =  0xFFFFFFFF;
}

short my_cursor = 0;

short cur_img = 1;

short imgSz(){
  switch (cur_img) {
      case 1:
        return image1_size;
        break;
      case 2:
        return image2_size;
        break;
      case 3:
        return image3_size;
        break;
      case 4:
        return image4_size;
        break;
      case 5:
        return image5_size;
        break;
      case 6:
        return image6_size;
        break;
      case 7:
        return image7_size;
        break;
      case 8:
        return image8_size;
        break;
      case 9:
        return image9_size;
        break;
    }
    return 1;

}
short imgW(){
  switch (cur_img) {
      case 1:
        return image1_width;
        break;
      case 2:
        return image2_width;
        break;
      case 3:
        return image3_width;
        break;
      case 4:
        return image4_width;
        break;
      case 5:
        return image5_width;
        break;
      case 6:
        return image6_width;
        break;
      case 7:
        return image7_width;
        break;
      case 8:
        return image8_width;
        break;
      case 9:
        return image9_width;
        break;
    }
    return 1;

}


inline void printImgSz(){
  Serial.print("Current image size : ");
  Serial.print(imgSz());
  Serial.print(" = ");
  Serial.print(imgW());
  Serial.print(" x ");
  Serial.println(imgW());
}



inline void drawImg(){
  switch (cur_img) {
    case 1:
      drawImage1();
      break;
    case 2:
      drawImage2();
      break;
    case 3:
      drawImage3();
      break;
    case 4:
      drawImage4();
      break;
    case 5:
      drawImage5();
      break;
    case 6:
      drawImage6();
      break;
    case 7:
      drawImage7();
      break;
    case 8:
      drawImage8();
      break;
    case 9:
      drawImage9();
      break;
    
  }
}


void loop() {
  char customKey = 0;
  long before = 0;
  long timing = 0;
  while(true){
    if(customKey)
      before = micros();
    drawImg();
    REG_PIOC_ODSR = 0b00000000000000000000000000000000;
    if(customKey)
      timing = micros() - before;
    
    if (customKey){
      Serial.print("Key:");
      Serial.println(customKey);
      if(customKey == '#'){
        Serial.print("Current image : ");
        Serial.println(cur_img);
        printImgSz();
        Serial.print("Time to draw the image : ");
        Serial.print(timing);
        Serial.print("us ~ ");
        Serial.print(1000000.0 / (float)timing);
        Serial.println("Hz");
        Serial.print("Time to draw a pixel : ");
        Serial.print((float)timing / (float) imgSz());
        Serial.print("us ~ ");
        Serial.print(1.0 / ((float)timing / (float) imgSz()));
        Serial.println("MHz");
      }
      if(customKey >= '1' && customKey <= '9')
        cur_img = (short)(customKey - '0');
    }
    customKey = customKeypad.getKey();
  }
}
