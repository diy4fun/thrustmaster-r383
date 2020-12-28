/*
 * This sketch is based on code provided at https://www.noelmccullagh.com
 * Special thanks to Taras Ivaniukovich (original author) who help me to fix Noel's version
 *
 * WIRING INFO
 * ===========
 * On Arduino Pro Micro side it must be connected as follows:
 *
 * Blue   = NOT USED (or can be connected to Arduino MOSI pin 16)
 * Green  = GND  -> Arduino pin GND
 * Orange = MISO -> Arduino pin 14
 * White  = SS   -> Arduino pin 7
 * Grey   = SCK  -> Arduino pin 15
 * Red    = +VCC -> Arduino pin +5V (or RAW if USB current is +5V already)
 *
 * R383 BUTTON MAPPING
 * ===================
 * BYTE 1
 * 7
 * 6
 * 5
 * 4
 * 3
 * 2
 * 1
 * 0 - bottom right right
 *
 * BYTE 2
 * 7 - righ paddle
 * 6 - top right right
 * 5 - top right left
 * 4
 * 3 - bottom left right
 * 2 - bottom right middle
 * 1 - bottom right left
 * 0
 *
 * BYTE 3
 * 7 - bottom left middle
 * 6
 * 5 - joy left
 * 4 - joy up
 * 3 - joy right
 * 2 - joy down
 * 1 - bottom left left
 * 0 - left paddle
*/

#include <SPI.h>
#include <Joystick.h>

const int buttonCount = 14;
const int slaveSelectPin = 7;

byte pos[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
byte currBytes[] = {0x00, 0x00, 0x00, 0x00};
byte prevBytes[] = {0x00, 0x00, 0x00};
bool btnState, joyBtnState, prevJoyBtnState;

 // numbers the buttons from "bottom right right" => "bottom left" => "top right" to "top left"
 // NOTE: for paddles => remove minus (-) in numbers -14 and -15 (also do not forget to increase $buttonCount)
int bit2btn[] = {
    -1, -1, -1, -1, -1, -1, -1,  0, // BYTE 1
   -14,  6,  7, -1,  3,  1,  2, -1, // BYTE 2
    4,  8, 11,  12, 9,  10,  5, -15 // BYTE 3
};

Joystick_ Joystick(
    JOYSTICK_DEFAULT_REPORT_ID,
    JOYSTICK_TYPE_GAMEPAD,
    buttonCount,
    0,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false
);

void setup() {
    // input from wheel
    Serial.begin(9600);

    SPCR |= _BV(CPHA);
    SPCR |= _BV(CPOL);

    SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));
    SPI.begin();

    pinMode(slaveSelectPin, OUTPUT);

    // output to joystick
    Joystick.begin();
}

void loop() {
    // tell the wheel, that we are ready to read the data now
    digitalWrite(slaveSelectPin, LOW);

    //read the next 4 bytes (get 2's compliment)
    for(int i=0; i<4; i++) {
        currBytes[i] = ~SPI.transfer(0x00);
    }

    // deal with the buttons first
    for(int i=0; i<3; i++) { // process the three bytes
        for(int b=0; b<8; b++) { // one bit at a time
            // if the bit has changed send the update
            if((btnState=currBytes[i] & pos[b])!=(prevBytes[i] & pos[b])) {
                Joystick.setButton(bit2btn[(i*8)+b], btnState);
            }
        }
    }

    joyBtnState = (currBytes[3] & pos[0]) && !(currBytes[2] & 0x3c);

    if(joyBtnState != prevJoyBtnState) {
        Joystick.setButton(13, joyBtnState);
    }

    for(int i=0;i<3;i++) {
        prevBytes[i] = currBytes[i]; // finally update the just read input to the the previous input for the next cycle
    }

    prevJoyBtnState = joyBtnState;

    // release the wheel
    digitalWrite(slaveSelectPin, HIGH);
}
