// main.c
// for NerdKits with ATmega168
// Main Program for the ENS6155 Project Assignment
//
// author: asatria; masaaed 
// 21 October 2018
//
// 

#define F_CPU 14745600
#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

#include "libnerdkits/delay.h"
#include "libnerdkits/lcd.h"
#include "libnerdkits/uart.h"

#include "common/sevensegment.h"
#include "common/commonkeypad.h"
#include "common/timeddelay.h"

void clearLcd() {
    lcd_home();
    lcd_line_one();
    lcd_write_string(PSTR("                    "));
    lcd_line_two();
    lcd_write_string(PSTR("                    "));
    lcd_line_three();
    lcd_write_string(PSTR("                    "));
    lcd_line_four();
    lcd_write_string(PSTR("                    "));
}

void lcdPrintArray(uint8_t seq[5]) {
    int i;
    lcd_home();
    lcd_line_four();
    for (i = 0; i < 5; i++) {
        lcd_write_int16(seq[i]);
        lcd_write_string(PSTR(" "));
    }
}

int main() {
    lcd_init();
    sevenSegmentInit();
    keypadInit();
    timedDelayMs(1000);

    /**
     *** Level One State definition
     *** 0 => Main Menu
     *** 1 => Counting up (funtion 1)
     *** 2 => Counting down (function 2)
     *** 3 => Sequency Entry (funtion 3)
     *** 4 => Speed Test (function 4)
     **/
    uint8_t levelOneState = 0;

    uint32_t lastUpdate = 0;
    uint8_t countDown = 3;
    uint8_t ledDownCount = 9;
    uint8_t ledCountUp = 0;


    while (1) {

        if (levelOneState == 0) {
            //main menu
            lcd_home();
            lcd_line_one();
            lcd_write_string(PSTR("Menu: 1. Count Up"));
            lcd_line_two();
            lcd_write_string(PSTR("      2. Count Down"));
            lcd_line_three();
            lcd_write_string(PSTR("      3. Seq Input"));
            lcd_line_four();
            lcd_write_string(PSTR("      4. Speed Test"));

            char key = getKeypadChar();
            if (key != '$') {
                int keyValue = key - '0';
                if (keyValue <= 4) {
                    levelOneState = keyValue;
                    lastUpdate = getMillis();
                    clearLcd();
                }
            }
        }

        if (levelOneState == 1) {
            segmentBlanking(1);
            bcdPortB(ledCountUp);
            lcd_home();
            lcd_line_one();
            lcd_write_string(PSTR("Counting Up..."));
            lcd_line_two();
            lcd_write_int16(ledCountUp);
            lcd_write_string(PSTR("  "));
            //debug
            //lcd_line_four();
            //lcd_write_string(PSTR("Debug:"));
            //lcd_write_int16(getMillis());

            //watch current millliss
            if ((getMillis() - lastUpdate) >= 1000) {
                ledCountUp++;
                if (ledCountUp >= 10) {
                    ledCountUp = 0;
                }
                lastUpdate = getMillis();
            }


            char key = getKeypadChar();
            if (key != '$') {
                segmentBlanking(0);
                levelOneState = 0;
                ledCountUp = 0;
                clearLcd();
            }
        }

        if (levelOneState == 2) {
            segmentBlanking(1);
            bcdPortB(ledDownCount);
            lcd_home();
            lcd_line_one();
            lcd_write_string(PSTR("Counting Down..."));
            lcd_line_two();
            lcd_write_int16(ledDownCount);
            lcd_write_string(PSTR("  "));
            //debug
            //lcd_line_four();
            //lcd_write_string(PSTR("Debug:"));
            //lcd_write_int16(getMillis());

            //watch current millliss
            if ((getMillis() - lastUpdate) >= 1000) {
                ledDownCount--;
                if (ledDownCount >= 10) {
                    ledDownCount = 9;
                }
                lastUpdate = getMillis();
            }


            char key = getKeypadChar();
            if (key != '$') {
                segmentBlanking(0);
                levelOneState = 0;
                ledDownCount = 9;
                clearLcd();
            }

        }

        if (levelOneState == 3) {
            /**
             * Level two state definition
             * 0 - entry of the 5 sequence
             * 1 - display of 5 entry sequence
             */
            static uint8_t levelTwoState = 0;
            static uint8_t seq[5];
            static uint8_t index = 0;

            if (levelTwoState == 0) {
                lcd_home();
                lcd_line_one();
                lcd_write_string(PSTR("Enter Number Seq"));
                lcdPrintArray(seq);

                char key = getKeypadChar();
                if (key != '$') {
                    int keyValue = key - '0';
                    seq[index] = keyValue;
                    index++;
                }

                if (index >= 5) {
                    //all the value has been entered
                    clearLcd();
                    index = 0;
                    levelTwoState = 1;
                    lastUpdate = getMillis();
                }
            }

            if (levelTwoState == 1) {
                lcd_home();
                lcd_line_one();
                lcd_write_string(PSTR("Displaying Sequence"));
                lcdPrintArray(seq);
                bcdPortB(seq[index]);
                segmentBlanking(1);

                //watch current milllis
                if ((getMillis() - lastUpdate) >= 1500) {
                    index++;
                    if (index >= 5) {
                        index = 0;
                    }
                    lastUpdate = getMillis();
                }

                char key = getKeypadChar();
                if (key != '$') {
                    segmentBlanking(0);
                    levelOneState = 0;
                    levelTwoState = 0;
                    index = 0;
                    int i;
                    for (i = 0; i < 5; i++) {
                        seq[i] = 0;
                    }

                }

            }

        }

        if (levelOneState == 4) {
            /**
             * Level Two State Declaration
             * 0 => just entered the menu preparation method should be displayed
             * 1 => ready 3..2..1.. is displayed
             * 2 => random number is displayed (waiting for key pressed)
             * 3 => result is displayed
             * 
             */
            static uint8_t levelTwoState = 0;
            static uint8_t randomNumber = 0;
            static uint8_t reactionTime = 0;
            if (levelTwoState == 0) {
                lcd_home();
                lcd_line_one();
                lcd_write_string(PSTR("Reaction speed Test!"));
                lcd_line_two();
                lcd_write_string(PSTR("Press any key.."));

                char key = getKeypadChar();
                if (key != '$') {
                    levelTwoState = 1;
                    lastUpdate = getMillis();
                    clearLcd();
                }
            }

            if (levelTwoState == 1) {
                bcdPortB(countDown);
                segmentBlanking(1);
                lcd_home();
                lcd_line_one();
                lcd_write_string(PSTR("Get Ready ! "));
                lcd_line_two();
                lcd_write_int16(countDown);
                lcd_write_string(PSTR(".."));
                //debug
                //lcd_line_four();
                //lcd_write_int16(getMillis());

                //watch current milllis
                if ((getMillis() - lastUpdate) >= 1000) {
                    countDown--;
                    lastUpdate = getMillis();
                }

                if (countDown == 0 || countDown >= 4) {
                    //segmentBlanking(0);
                    levelTwoState = 2;
                    countDown = 3;
                    clearLcd();
                    srand(getMillis);
                    randomNumber = rand() % 10;
                    lastUpdate = getMillis();
                }
            }

            if (levelTwoState == 2) {
                lcd_home();
                lcd_line_one();
                lcd_write_string(PSTR("press any key "));
                lcd_line_two();
                lcd_write_string(PSTR("when 0 is displayed"));
                bcdPortB(randomNumber);
                segmentBlanking(1);

                if ((getMillis() - lastUpdate) >= 1000) {
                    randomNumber = rand() % 10;
                    lastUpdate = getMillis();
                }

                char key = getKeypadChar();
                if (key != '$') {
                    levelTwoState = 3;
                    reactionTime = getMillis() - lastUpdate;
                    lastUpdate = getMillis();
                    clearLcd();
                    segmentBlanking(0);
                }
            }

            if (levelTwoState == 3) {
                if (randomNumber == 0) {
                    //pressed key on the right digit
                    lcd_home();
                    lcd_line_one();
                    lcd_write_string(PSTR("Congratulation !!"));
                    lcd_line_two();
                    lcd_write_string(PSTR("Reaction time:"));
                    lcd_line_three();
                    lcd_write_int16(reactionTime);
                    lcd_write_string(PSTR("ms  "));
                } else {
                    //pressed key on the wrong digit
                    lcd_home();
                    lcd_line_one();
                    lcd_write_string(PSTR("Wrong Digit !!"));
                    lcd_line_two();
                    lcd_write_string(PSTR("(*_*)"));
                    lcd_line_three();
                    lcd_write_int16(reactionTime);
                    lcd_write_string(PSTR("ms  "));
                }
                char key = getKeypadChar();
                if (key != '$') {
                    levelTwoState = 0;
                    reactionTime = 0;
                    lastUpdate = getMillis();
                    clearLcd();
                    levelOneState = 0;
                }

            }

        }
    }
    return 0;
}









