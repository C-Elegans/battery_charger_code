#include "pwm.h"

void init_pwm1(void){
    PWM1CONbits.EN = 1;
    PWM1CONbits.OUT = 1;
    PWM1CONbits.POL = 0;
    PWM1CONbits.MODE = 0;
    
    PWM1CLKCONbits.PS = 0;
    PWM1CLKCONbits.CS = 0;
    
    PWM1PHH = 0;
    PWM1PHL = 0;
    
    PWM1DCH = 0;
    PWM1DCL = 0;
    PWM1PRH = 0;
    PWM1PRL = 128;
    
    PWM1LDCONbits.LDT = 0;
    PWM1LDCONbits.LDA = 1;
}
void init_pwm2(void){
    PWM2CONbits.EN = 1;
    PWM2CONbits.OUT = 1;
    PWM2CONbits.POL = 0;
    PWM2CONbits.MODE = 0;
    
    PWM2CLKCONbits.PS = 0;
    PWM2CLKCONbits.CS = 0;
    
    PWM2PHH = 0;
    PWM2PHL = 0;
    
    PWM2DCH = 0;
    PWM2DCL = 63;
    PWM2PRH = 0;
    PWM2PRL = 128;
    
    PWM2LDCONbits.LDT = 0;
    PWM2LDCONbits.LDA = 1;
}