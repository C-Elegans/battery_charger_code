#ifndef __XC_H
#define __XC_H
typedef union {
    struct {
        unsigned ADPREF0                :1;
        unsigned ADPREF1                :1;
        unsigned pad1                       :2;
        unsigned ADCS0                  :1;
        unsigned ADCS1                  :1;
        unsigned ADCS2                  :1;
        unsigned ADFM                   :1;
    };
    struct {
        unsigned ADPREF                 :2;
        unsigned pad2                       :2;
        unsigned ADCS                   :3;
    };
} ADCON1bits_t;

typedef union {
    struct {
        unsigned ADON                   :1;
        unsigned GO_nDONE               :1;
        unsigned CHS0                   :1;
        unsigned CHS1                   :1;

        unsigned CHS2                   :1;
        unsigned CHS3                   :1;
        unsigned CHS4                   :1;
    };
    struct {
        unsigned                        :1;
        unsigned ADGO                   :1;
        unsigned CHS                    :5;
    };
    struct {
        unsigned                        :1;
        unsigned GO                     :1;
    };
    struct {
        unsigned                        :1;
        unsigned nDONE                  :1;
    };
} ADCON0bits_t;


extern ADCON1bits_t ADCON1bits;
extern ADCON0bits_t ADCON0bits;
extern unsigned char ADRESH;
extern unsigned char ADRESL;
extern unsigned char PWM1DCL;
extern unsigned char PWM1DCH;
extern unsigned char PWM2DCL;
extern unsigned char PWM2DCH;

typedef union {
    struct {
        unsigned LDS                    :2;
        unsigned                        :4;
        unsigned LDT                    :1;
        unsigned LDA                    :1;
    };
    struct {
        unsigned PWM1LDS0               :1;
        unsigned PWM1LDS1               :1;
    };
    struct {
        unsigned PWM1LDS                :2;
        unsigned                        :4;
        unsigned PWM1LDM                :1;
        unsigned PWM1LD                 :1;
    };
    struct {
        unsigned LDS0                   :1;
        unsigned LDS1                   :1;
    };
} PWM1LDCONbits_t;

typedef union {
    struct {
        unsigned TRISA0                 :1;
        unsigned TRISA1                 :1;
        unsigned TRISA2                 :1;
        unsigned TRISA3                 :1;
        unsigned TRISA4                 :1;
        unsigned TRISA5                 :1;
    };
} TRISAbits_t;
typedef union {
    struct {
        unsigned TRISC0                 :1;
        unsigned TRISC1                 :1;
        unsigned TRISC2                 :1;
        unsigned TRISC3                 :1;
        unsigned TRISC4                 :1;
        unsigned TRISC5                 :1;
    };
} TRISCbits_t;
typedef union {
    struct {
        unsigned LATC0                  :1;
        unsigned LATC1                  :1;
        unsigned LATC2                  :1;
        unsigned LATC3                  :1;
        unsigned LATC4                  :1;
        unsigned LATC5                  :1;
    };
} LATCbits_t;

extern PWM1LDCONbits_t PWM1LDCONbits;
extern PWM1LDCONbits_t PWM2LDCONbits;
extern TRISAbits_t TRISAbits;
extern TRISCbits_t TRISCbits;
extern LATCbits_t LATCbits;
#endif
