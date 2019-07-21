/*
 * File:   main.c
 * Author: mnolan
 *
 * Created on July 17, 2019, 8:54 PM
 */

#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator; I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PPS1WAY = OFF    // PPSLOCK bit One-Way Set Enable bit (PPSLOCKED Bit Can Be Cleared & Set Repeatedly)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOREN = OFF    // Low Power Brown-out Reset enable bit (LPBOR is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

#include "pps.h"
#include "pwm.h"
#define BOOST_SENSE_ADC 5
#define BATT_SENSE_ADC 6
#define ISENSE_ADC 7

#define BOOST_THRESHOLD 170 // 17V * 1k/(1k+20k) * 1024/5V
#define BOOST_DCL_MAX 80 

#define BATT_THRESHOLD 223 // 12V * 1k/(1k+10k) * 1024/5V
#define BUCK_DCL_MAX 110

#define CURRENT_THRESHOLD 280 // .2A * 0.05 ohm * 9x gain, * 1024/5V

void init_adc(void){
    ADCON1bits.ADFM = 1;
    ADCON1bits.ADCS = 0x2; // FOSC/32 - TAd = 1us
    ADCON1bits.ADPREF = 0;
    
    ADCON0bits.ADON = 1;
}

uint16_t read_adc(uint8_t pin){
    ADCON0bits.CHS = pin;
    for(uint8_t i=0; i<32; i++){
        __asm__ volatile("nop");
    }
    ADCON0bits.GO = 1;
    while(ADCON0bits.GO == 1){}
    return ADRESH << 8 | ADRESL;
}

void update_boost(void){
    uint16_t boost_voltage = read_adc(BOOST_SENSE_ADC);
    if(boost_voltage < BOOST_THRESHOLD && PWM1DCL < BOOST_DCL_MAX){
       PWM1DCL += 1;
       
    }
    if(boost_voltage > BOOST_THRESHOLD && PWM1DCL > 5){
        PWM1DCL = 0;
    }
    PWM1LDCONbits.LDA = 1;
}
uint16_t buck_counter = 63;
void update_buck(void){
    uint16_t buck_voltage = read_adc(BATT_SENSE_ADC);
    uint16_t current_sense = read_adc(ISENSE_ADC);
    if(buck_voltage < BATT_THRESHOLD && PWM2DCL < BUCK_DCL_MAX && current_sense < CURRENT_THRESHOLD){
        buck_counter += 1;
    }
    if((buck_voltage > BATT_THRESHOLD || current_sense > CURRENT_THRESHOLD) && PWM2DCL > 0){
        buck_counter -= 1;
    }
    PWM2DCL = buck_counter;
    PWM2LDCONbits.LDA = 1;
    
}

void setup_clock(void){
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0x0e; // 32MHz oscillator
}
void main(void) {
    setup_clock();
    init_adc();
    TRISCbits.TRISC5 = 0;
    LATCbits.LATC5 = 1;
    TRISAbits.TRISA2 = 0;
    TRISCbits.TRISC0 = 0;
    init_pwm1();
    init_pwm2();
    setup_pps();
    while(1) {
        update_boost();
        update_buck();
    }
    return;
}
