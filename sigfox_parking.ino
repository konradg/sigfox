/*
    Parking spot demo
    -----------------
    
    A slightly modified example using code from SmeIoT Library demos (DataModeEu for SIGFOX module, VL6180 proximity demo)
    
    Covering the proximity sensor will indicate the parking spot is taken. A SIGFOX ENTER message is sent and the LED changes color. Removing the cover simulates the car leaving - an EXIT message is sent and the program terminates. Requires board restart to repeat.
 */

#include <Wire.h>
#include <SmeSFX.h>
#include <VL6180.h>
#include <Arduino.h>

#define STATE_FREE 0
#define STATE_TAKEN 1

#define LED_OFF 0
#define LED_RED 1
#define LED_GREEN 2
#define LED_BLUE 3

char enterMsg[5]= {'E','n', 't', 'e', 'r'};
char exitMsg[4]= {'E','x', 'i', 't'};
int state;
int ledstate;
short entryBuffer;

// the setup function runs once when you press reset or power the board
void setup() {
    Wire.begin();
    if (!smeProximity.begin()) {
        while(1){
            ; // endless loop due to error on VL6180 initialization
        }
    }
    SerialUSB.begin(115200);
    sfxAntenna.begin();
    int initFinish=1;

    SerialUSB.println("SFX in Command mode");
    sfxAntenna.setSfxConfigurationMode(); // enter in configuration Mode

    do {
        uint8_t answerReady = sfxAntenna.hasSfxAnswer();
        if (answerReady){
            switch (initFinish){
            case 1:                                
                SerialUSB.println("SFX in Data mode");
                sfxAntenna.setSfxDataMode();
                initFinish++;
                break;

            case 2:
                initFinish++; // exit
                break;
            }
        }
    } while (initFinish!=3);
    state = STATE_FREE;
    ledstate = 0;
    entryBuffer = 0;
}

// the loop function runs over and over again forever
void loop() {
    bool isClose = smeProximity.rangePollingRead() < 50;
    switch (state) {
        case STATE_FREE:
            if (isClose) {
                entryBuffer ++;
            }
            if (entryBuffer >= 8) {
                setLed(LED_BLUE);
                sendMsg(enterMsg);
                state = STATE_TAKEN;
            } else {
                setLed(LED_GREEN);
            }
            break;
        case STATE_TAKEN:
            if (!isClose) {
                setLed(LED_BLUE);
                sendMsg(exitMsg);
                state = STATE_FREE;
            } else {
                setLed(LED_RED);
            }
            break;
    }
    delay(500);
}

void setLed(int color) {
    if (ledstate == color) {
        color = LED_OFF; // blink
    }
    switch (color) {
        case LED_OFF:
            ledRedLight(LOW);
            ledGreenLight(LOW);
            ledBlueLight(LOW);
            break;
        case LED_RED:
            ledRedLight(HIGH);
            ledGreenLight(LOW);
            ledBlueLight(LOW);
            break;
        case LED_GREEN:
            ledRedLight(LOW);
            ledGreenLight(HIGH);
            ledBlueLight(LOW);
            break;
        case LED_BLUE:
            ledRedLight(LOW);
            ledGreenLight(LOW);
            ledBlueLight(HIGH);
            break;
    }
    ledstate = color;
}

void sendMsg(char* msg) {
  
    sfxAntenna.sfxSendData(msg, strlen((char*)msg));
    
    bool answerReady = sfxAntenna.hasSfxAnswer();
    while (!answerReady) {
        answerReady = sfxAntenna.hasSfxAnswer();
        if (sfxAntenna.getSfxMode() == sfxDataMode) {
            switch (sfxAntenna.sfxDataAcknoledge()) {
            case SFX_DATA_ACK_START:
                SerialUSB.println("Waiting Answer");
                break;

            case SFX_DATA_ACK_PROCESSING:
                SerialUSB.print('.');
                break;

            case SFX_DATA_ACK_OK:
                SerialUSB.println(' ');
                SerialUSB.println("Answer OK :) :) :) :)");
                break;

            case SFX_DATA_ACK_KO:
                SerialUSB.println(' ');
                SerialUSB.println("Answer KO :( :( :( :(");
                break;
            }
        }
    }
    delay(1000);
}


