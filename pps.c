/*
 * File:   pps.c
 * Author: mnolan
 *
 * Created on July 17, 2019, 10:24 PM
 */


#include "pps.h"


void unlock_pps(void){
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCK = 0x00;
}
void lock_pps(void){
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCK = 0x01;
}
void setup_pps(void){
    unlock_pps();
    RA2PPS = 0x03; //PWM1
    RC0PPS = 0x04; //PWM2
    lock_pps();
}
void setup_clock(void){
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0x0e; // 32MHz oscillator
}
