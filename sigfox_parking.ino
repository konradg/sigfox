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



char enterMsg[5]= {'E','n', 't', 'e', 'r'};
char exitMsg[4]= {'E','x', 'i', 't'};
bool finished;
bool entered;
bool ledon;

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
    finished = 0;
    entered = 0;
    ledon = 0;
}

// the loop function runs over and over again forever
void loop() {
    while(!finished) {
        char light = smeProximity.rangePollingRead();
        if (!entered) {
            if (light > 0 && light < 50) {
              sendMsg(enterMsg);
              entered = 1;
            }
        } else {
            if (light >= 50) {
                sendMsg(exitMsg);
                finished = 1;
            }
        }
        if (ledon) {
          if (entered){
            ledGreenLight(LOW);
          } else {
            ledRedLight(LOW);
          }
            ledon = 0;
        } else {
            if (entered){
              ledGreenLight(HIGH);
            } else {
              ledRedLight(HIGH);
            }
            ledon = 1;
        }
        delay(1000);
    }
    ledRedLight(LOW);
    ledGreenLight(LOW);
    ledBlueLight(HIGH);
    delay(10000);
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


