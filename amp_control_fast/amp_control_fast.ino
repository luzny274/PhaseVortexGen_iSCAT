#include "image1.h"

void setup() {
  // put your setup code here, to run once:

  for(int i = 2; i < 14; i++) pinMode(i, OUTPUT);
  for(int i = 21; i < 53; i++) pinMode(i, OUTPUT);

  REG_PIOC_OWER = 0xFFFFFFFF;
  REG_PIOC_OER =  0xFFFFFFFF;
}


void loop() {
  while(true){
    drawImage1();
  }
}
