/*
 * sketch heavily based on code provided at:
 * April 2015 © blog@rr-m.org
 * Author: Noel McCullagh
*/

/*
Also see comments from Ignacio Hernandez-Ros & mispeaced under https://www.youtube.com/watch?v=CJjLUAgAFnQ
* On Arduino Pro Micro side it must be connected as follows:
 *
 * Blue -> not used (or can be connected to arduino MOSI pin 16)
 * Green - GND -> arduino uno GND pin
 * Orange - MISO -> arduino uno pin 14
 * White – SS -> arduino uno pin 7
 * Grey – SCK -> arduino uno pin 15
 * Red +VCC -> arduino +5V pin (or RAW if USB current is +5V already)
 *
* Byte 1
 * 7
 * 6
 * 5
 * 4
 * 3
 * 2
 * 1
 * 0 - bottom right right
 *
 * Byte 2
 * 7 - righ paddle
 * 6 - top right right
 * 5 - top right left
 * 4
 * 3 - bottom left right
 * 2 - bottom right middle
 * 1 - bottom right left
 * 0
 *
 * Byte 3
 * 7 - bottom left middle
 * 6
 * 5 - joy left
 * 4 - joy up
 * 3 - joy right
 * 2 - joy down
 * 1 - bottom left left
 * 0 - paddle left
*/

#include <SPI.h>
#include <Joystick.h>

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, 16, 0,
  false, false, false, false, false, false,
  false, false, false, false, false);

const int slaveSelectPin = 7;
byte pos[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
byte currBytes[] = {0x00, 0x00, 0x00, 0x00};
byte prevBytes[] = {0x00, 0x00, 0x00};
bool btnState, joyBtnState, prevJoyBtnState;
int bit2btn[] = {-1,-1,-1,-1,-1,-1,-1,8,  1,2,13,-1,5,7,6,5,  4,14,11,9,12,10,3,0};              //numbers the buttons from top left to bottom right (main is D,U,press)

void setup() {
  //input from wheel
  Serial.begin(9600);
  SPCR |= _BV(CPHA);
  SPCR |= _BV(CPOL);
  SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
  pinMode(slaveSelectPin, OUTPUT);

  //output to joystick
  Joystick.begin();
}

void loop() {
  // tell the wheel, that we are ready to read the data now
  digitalWrite(slaveSelectPin, LOW);

  //read the next 4 bytes (get 2's compliment)
  for(int i=0; i<4; i++)
    currBytes[i] = ~SPI.transfer(0x00);

  //deal with the buttons first
  for(int i=0; i<3; i++)      //process the three bytes
    for(int b=0; b<8; b++)     //one bit at a time
      if((btnState=currBytes[i] & pos[b])!=(prevBytes[i] & pos[b]))
        Joystick.setButton(bit2btn[(i*8)+b], btnState);      //if the bit has changed send the update

  joyBtnState = (currBytes[3] & pos[0]) && !(currBytes[2] & 0x3c);

  if(joyBtnState != prevJoyBtnState)
    Joystick.setButton(13, joyBtnState);

  for(int i=0;i<3;i++)
    prevBytes[i] = currBytes[i];   //finally update the just read input to the the previous input for the next cycle

  prevJoyBtnState = joyBtnState;

  // release the wheel
  digitalWrite(slaveSelectPin, HIGH);
}
