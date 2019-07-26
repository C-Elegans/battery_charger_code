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
#ifdef __FRAMAC__
#define __bit char
#endif

#include <xc.h>
#include <stdint.h>

#include "pps.h"
#include "pwm.h"
#define BOOST_SENSE_ADC 5
#define BATT_SENSE_ADC 6
#define ISENSE_ADC 7

#define BOOST_THRESHOLD 170 // 17V * 1k/(1k+20k) * 1024/5V
#define BOOST_DCL_MAX 80 

#define BUCK_DCL_MAX 110

#define STAGE_1_CURRENT_THRESHOLD 243 // 2.5A * 0.05 ohm * 9x gain, * 1024/5V
#define STAGE_1_VOLTAGE_THRESHOLD 268 // 14.4V * 1k/(1k+10k) * 1024/5V

#define STAGE_2_CURRENT_THRESHOLD 97 // 3A * 0.05 ohm * 9x gain, * 1024/5V
#define STAGE_2_CURRENT_MINIMUM 10   // .1A * 0.05 ohm * 9x gain, * 1024/5V
#define STAGE_2_VOLTAGE_THRESHOLD 256 // 13.8V * 1k/(1k+10k) * 1024/5V

uint16_t current_threshold = STAGE_1_CURRENT_THRESHOLD;
uint16_t voltage_threshold = STAGE_1_VOLTAGE_THRESHOLD;

//@ assigns ADCON1bits, ADCON0bits;
void init_adc(void){
    ADCON1bits.ADFM = 1;
    ADCON1bits.ADCS = 0x2; // FOSC/32 - TAd = 1us
    ADCON1bits.ADPREF = 0;
    
    ADCON0bits.ADON = 1;
}

/*@
assigns ADCON0bits;
*/
uint16_t read_adc(uint8_t pin){
    ADCON0bits.CHS = pin;
    uint8_t i=0;
    /*@ loop invariant 0 <= i <= 32;
      loop assigns i;
      loop variant 32 - i;
     */
    for(; i<32; i++){
      __asm__ volatile("nop":::);
    }
    ADCON0bits.GO = 1;
    uint16_t j = 0;
    /*@ loop invariant 0 <= j <= 1024;
      loop assigns j;
      loop variant 1024 - j;
     */
#ifdef __FRAMAC__
    while(ADCON0bits.GO == 1 && j < 1024){
      j++;
    }
#else
    while(ADCON0bits.GO == 1) {}
#endif
    return (ADRESH & 0x03) << 8 | ADRESL;
}

/*@
  ensures (boost_voltage < BOOST_THRESHOLD && PWM1DCL < BOOST_DCL_MAX) ==>
                PWM1DCL >= \old(PWM1DCL);
  ensures (boost_voltage > BOOST_THRESHOLD && PWM1DCL > 0) ==>
                PWM1DCL <= \old(PWM1DCL);
  ensures 5 < \old(PWM1DCL) < BOOST_DCL_MAX ==>
          0 <= PWM1DCL <= BOOST_DCL_MAX;
  assigns PWM1DCL, PWM1LDCONbits; 
*/
void update_boost(uint16_t boost_voltage){
    if(boost_voltage < BOOST_THRESHOLD && PWM1DCL < BOOST_DCL_MAX){
       PWM1DCL += 1;
       
    }
    if(boost_voltage > BOOST_THRESHOLD && PWM1DCL >= 2){
        PWM1DCL -= 2;
    }
    PWM1LDCONbits.LDA = 1;
}

uint16_t buck_counter = 63;

/*@ 
     
  assigns PWM2DCL, PWM2LDCONbits, ADCON0bits;
  ensures (buck_voltage < voltage_threshold && current_sense < current_threshold) ==> PWM2DCL >= \old(PWM2DCL);
  ensures (buck_voltage > voltage_threshold || current_sense > current_threshold) ==> PWM2DCL <= \old(PWM2DCL);
*/
void update_buck(uint16_t buck_voltage, uint16_t current_sense){
    if(buck_voltage < voltage_threshold && PWM2DCL < BUCK_DCL_MAX && current_sense < current_threshold){
        PWM2DCL += 1;
    }
    else if((buck_voltage > voltage_threshold || current_sense > current_threshold) && PWM2DCL > 0){
        PWM2DCL -= 1;
    }
    PWM2LDCONbits.LDA = 1;
}

enum state{
  CONSTANT_CURRENT,
  CONSTANT_VOLTAGE,
  DONE
};
enum state state = CONSTANT_CURRENT;

/*@ assigns current_threshold, voltage_threshold, state;
  behavior cc:
    assumes state == CONSTANT_CURRENT;
    ensures voltage_threshold == STAGE_1_VOLTAGE_THRESHOLD;
    ensures current_threshold == STAGE_1_CURRENT_THRESHOLD;
    ensures buck_voltage > STAGE_1_VOLTAGE_THRESHOLD - 2 ==> state == CONSTANT_VOLTAGE;
  behavior cv:
    assumes state == CONSTANT_VOLTAGE;
    ensures voltage_threshold == STAGE_2_VOLTAGE_THRESHOLD;
    ensures current_threshold == STAGE_2_CURRENT_THRESHOLD;
    ensures current_sense < STAGE_2_CURRENT_MINIMUM ==> state == DONE;
  behavior done:
    assumes state != CONSTANT_CURRENT && state != CONSTANT_VOLTAGE;
    ensures voltage_threshold == 0;
    ensures current_threshold == 0;
  complete behaviors;
  disjoint behaviors;
 */
void state_machine(uint16_t buck_voltage, uint16_t current_sense){
  switch(state){
  case CONSTANT_CURRENT:
    voltage_threshold = STAGE_1_VOLTAGE_THRESHOLD;
    current_threshold = STAGE_1_CURRENT_THRESHOLD;
    if(buck_voltage > STAGE_1_VOLTAGE_THRESHOLD - 2){
      state = CONSTANT_VOLTAGE;
    }
    break;
  case CONSTANT_VOLTAGE:
    voltage_threshold = STAGE_2_VOLTAGE_THRESHOLD;
    current_threshold = STAGE_2_CURRENT_THRESHOLD;
    if(current_sense < STAGE_2_CURRENT_MINIMUM){
      state = DONE;
    }
    break;
  default:
    voltage_threshold = 0;
    current_threshold = 0;
    break;
  }
}

unsigned char dummy = 5;

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
        uint16_t boost_voltage = read_adc(BOOST_SENSE_ADC);
        uint16_t buck_voltage = read_adc(BATT_SENSE_ADC);
        uint16_t current_sense = read_adc(ISENSE_ADC);
        update_boost(boost_voltage);
        update_buck(buck_voltage, current_sense);
    }
    return;
}
