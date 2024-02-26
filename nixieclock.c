/**
 * NIXIE CLOCK
 * Hardware designed by Dawid Placha
 * Software implemented by Dariusz Iwanoczko
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nixieclock.h"

volatile uint8_t au8NixieTube[NIXIE_TUBES_NUM];

volatile uint8_t u8HrsBCD = 0;
volatile uint8_t u8MinBCD = 0;
volatile uint8_t u8SecBCD = 0;

static volatile uint8_t NIXIE_Mode = NIXIE_MODE_CLOCK;

static volatile uint8_t BTN_P0_Short = 0;
static volatile uint8_t BTN_P0_Long = 0;
static volatile uint8_t BTN_P0_Cntr = 0;

/**
 * TWI
 */
static void TWI_Init(void) {
    /* Set SCL and SDA output */
    TWI_PORT |= _BV(TWI_SCL_PIN) | _BV(TWI_SDA_PIN);
    /* TWI Bit Rate Register */
    TWBR = 0x02;
}

static void TWI_Start(void) {
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    while (!(TWCR & _BV(TWINT)));
}

static void TWI_Stop() {
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

static void TWI_Write(uint8_t u8) {
    TWDR = u8;
    TWCR = _BV(TWINT) | _BV(TWEN);
    while (!(TWCR & _BV(TWINT)));
}

static void TWI_Read(uint8_t* u8) {
    TWCR = _BV(TWINT) | _BV(TWEN);
    while (!(TWCR & _BV(TWINT)));
    *u8 = TWDR;
}

/**
 * BTN
 */
static void BTN_Init(void) {
    DDRB &= ~_BV(BTN_P0_PIN);
    PORTB |= _BV(BTN_P0_PIN);
}

/**
 * DS3231SO
 */
void DS3231_Write(uint8_t u8Address, uint8_t u8Value) {
    TWI_Start();
    TWI_Write(DS3231_ADR_W);
    TWI_Write(u8Address);
    TWI_Write(u8Value);
    TWI_Stop();
}

uint8_t DS3231_Read(uint8_t u8Address) {
    uint8_t u8rcvd;

    TWI_Start();
    TWI_Write(DS3231_ADR_W);
    TWI_Write(u8Address);
    TWI_Start();
    TWI_Write(DS3231_ADR_R);
    TWI_Read(&u8rcvd);
    TWI_Stop();

    return u8rcvd;
}

/**
 * NIXIE
 */
static void NIXIE_Init() {
    DDRB |= _BV(DD0) | _BV(DD1) | _BV(DD2) | _BV(DD3) | _BV(DD4);
    DDRC |= _BV(DD0) | _BV(DD1) | _BV(DD2) | _BV(DD3);
    DDRD |= _BV(DD0) | _BV(DD1) | _BV(DD2) | _BV(DD3) | _BV(DD4) | _BV(DD5) | _BV(DD6) | _BV(DD7);
}

static void NIXIE_SetTubeVal(uint8_t u8Tube, uint8_t u8Value) {
    uint8_t A, B, C, D;

    A = ((u8Value >> 0) & 0x01);
    B = ((u8Value >> 1) & 0x01);
    C = ((u8Value >> 2) & 0x01);
    D = ((u8Value >> 3) & 0x01);

    if (u8Tube == 0) {
        if (A) sbi(PORTC, 0); else cbi(PORTC, 0);
        if (B) sbi(PORTC, 2); else cbi(PORTC, 2);
        if (C) sbi(PORTC, 3); else cbi(PORTC, 3);
        if (D) sbi(PORTC, 1); else cbi(PORTC, 1);
    } else if (u8Tube == 1) {
        if (A) sbi(PORTD, 4); else cbi(PORTD, 4);
        if (B) sbi(PORTD, 1); else cbi(PORTD, 1);
        if (C) sbi(PORTD, 0); else cbi(PORTD, 0);
        if (D) sbi(PORTD, 2); else cbi(PORTD, 2);
    } else if (u8Tube == 2) {
        if (A) sbi(PORTB, 0); else cbi(PORTB, 0);
        if (B) sbi(PORTD, 6); else cbi(PORTD, 6);
        if (C) sbi(PORTD, 5); else cbi(PORTD, 5);
        if (D) sbi(PORTD, 7); else cbi(PORTD, 7);
    } else if (u8Tube == 3) {
        if (A) sbi(PORTB, 1); else cbi(PORTB, 1);
        if (B) sbi(PORTB, 3); else cbi(PORTB, 3);
        if (C) sbi(PORTB, 4); else cbi(PORTB, 4);
        if (D) sbi(PORTB, 2); else cbi(PORTB, 2);
    }
}

static void NIXIE_DisplayUpdate(void) {
    for (int i=0; i<NIXIE_TUBES_NUM; ++i)
        NIXIE_SetTubeVal(i, au8NixieTube[i]);
}

static void NIXIE_DisplayLeft(uint8_t u8) {
    au8NixieTube[0] = (u8/10)%10;
    au8NixieTube[1] = u8%10;
    NIXIE_DisplayUpdate();
}

static void NIXIE_DisplayRight(uint8_t u8) {
    au8NixieTube[2] = (u8/10)%10;
    au8NixieTube[3] = u8%10;
    NIXIE_DisplayUpdate();
}

static void NIXIE_BacklightOn(void) {
    sbi(PORTD, 3);
}

static void NIXIE_BacklightOff(void) {
    cbi(PORTD, 3);
}

static void NIXIE_BacklightToggle(void) {
    if (PORTD & _BV(3)) {
        NIXIE_BacklightOff();
    } else {
        NIXIE_BacklightOn();
    }
}

static void NIXIE_ReadTime(void) {
    u8SecBCD = DS3231_Read(DS3231_REG_TIME_SEC);
    u8MinBCD = DS3231_Read(DS3231_REG_TIME_MIN);
    u8HrsBCD = DS3231_Read(DS3231_REG_TIME_HRS);
}

static void NIXIE_SaveTime(uint8_t u8Hrs, uint8_t u8Min, uint8_t u8Sec) {
    u8SecBCD = dec2bcd(u8Sec);
    u8MinBCD = dec2bcd(u8Min);
    u8HrsBCD = dec2bcd(u8Hrs);

    DS3231_Write(DS3231_REG_TIME_SEC, u8SecBCD);
    DS3231_Write(DS3231_REG_TIME_MIN, u8MinBCD);
    DS3231_Write(DS3231_REG_TIME_HRS, u8HrsBCD);
}

static void NIXIE_SelfTest(void) {
    for (int i=0; i<10; ++i) {
        for (int j=0; j<NIXIE_TUBES_NUM; ++j)
            NIXIE_SetTubeVal(j, i);
        _delay_ms(300);
    }
}

static void NIXIE_RunClockMode(void) {
    while(1) {
        NIXIE_ReadTime();

        au8NixieTube[0] = (u8HrsBCD & 0xf0) >> 4;
        au8NixieTube[1] = u8HrsBCD & 0x0f;
        au8NixieTube[2] = (u8MinBCD & 0xf0) >> 4;
        au8NixieTube[3] = u8MinBCD & 0x0f;

        NIXIE_DisplayUpdate();

        if (BTN_P0_Short) {
            BTN_P0_Short = 0;
            NIXIE_BacklightToggle();
        }

        if (BTN_P0_Long) {
            BTN_P0_Long = 0;
            break;
        }
    }

    NIXIE_Mode = NIXIE_MODE_SETTINGS;
}

static void NIXIE_RunSettingsMode(void) {
    uint8_t u8CurrentSetting = 0;
    uint8_t u8TimeChanged = 0;

    uint8_t u8Hrs = bcd2dec(u8HrsBCD);
    uint8_t u8Min = bcd2dec(u8MinBCD);

    while (1) {
        if (u8CurrentSetting == 0) {
            NIXIE_DisplayLeft(u8Hrs);
            au8NixieTube[2] = TUBE_EMPTY;
            au8NixieTube[3] = TUBE_EMPTY;

            if (BTN_P0_Short) {
                BTN_P0_Short = 0;
                if (++u8Hrs > 23)
                    u8Hrs = 0;

                u8TimeChanged = 1;
            }
        } else if (u8CurrentSetting == 1) {
            au8NixieTube[0] = TUBE_EMPTY;
            au8NixieTube[1] = TUBE_EMPTY;
            NIXIE_DisplayRight(u8Min);

            if (BTN_P0_Short) {
                BTN_P0_Short = 0;
                if (++u8Min > 59)
                    u8Min = 0;

                u8TimeChanged = 1;
            }
        }

        if (BTN_P0_Long) {
            BTN_P0_Long = 0;
            if (++u8CurrentSetting > 1)
                break;
        }
    }

    if (u8TimeChanged) {
        NIXIE_SaveTime(u8Hrs, u8Min, 0x00);
    }

    NIXIE_Mode = NIXIE_MODE_CLOCK;
}

/**
 * IRQ
 */
static void IRQ_Init(void) {
    /* clkio with 1024 prescaling */
#if atmega328p
    TCCR0B = _BV(CS00) | _BV(CS02);
    TIMSK0 = _BV(TOIE0);
#elif atmega8
    TCCR0 = _BV(CS00) | _BV(CS02);
    TIMSK = _BV(TOIE0);
#else
    #error "Unsupported MCU device!"
#endif
}

__attribute__((OS_main))
int main(void) {
    NIXIE_Init();
    TWI_Init();
    BTN_Init();

    NIXIE_SelfTest();
    IRQ_Init();

    /* Enable Interrupt */
    sei();

    while(1) {
        switch (NIXIE_Mode) {
        case NIXIE_MODE_CLOCK:
            NIXIE_RunClockMode();
            break;
        case NIXIE_MODE_SETTINGS:
            NIXIE_RunSettingsMode();
            break;
        default:
            break;
        }
    }

    return 0;
}

/**
 * ISR Vectors
 */
ISR(TIMER0_OVF_vect) {
    // One interrupt per 16,32 ms
    if (!(PINB & BTN_P0)) {
        if (++BTN_P0_Cntr > 100)
            BTN_P0_Cntr = 100;
    } else {
        if (BTN_P0_Cntr) {
            if (BTN_P0_Cntr < 2) {
                // Debouncing
            }  else if (BTN_P0_Cntr < 20) {
                BTN_P0_Short = 1;
            }  else if (BTN_P0_Cntr > 30) {
                BTN_P0_Long = 1;
            }

            BTN_P0_Cntr = 0;
        }
    }
}
