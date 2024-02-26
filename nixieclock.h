#ifndef __NIXIE_CLOCK__
#define __NIXIE_CLOCK__

#include <avr/io.h>

#define sbi(port, bit) (port) |= _BV(bit)
#define cbi(port, bit) (port) &= ~_BV(bit)

#define bcd2dec(x) ((((x&0xf0) >> 4)*10) + (x&0x0f))
#define dec2bcd(x) (((x/10) << 4) + (x % 10))

/* ASM shortcut*/
#define NOP __asm__ __volatile__ ("nop")

/* Define tube roles */
#define NIXIE_TUBES_NUM 4

#define NIXIE_MODE_CLOCK 0
#define NIXIE_MODE_SETTINGS 1

#define TUBE_EMPTY 0x0A

static void NIXIE_Init();
static void NIXIE_SetTubeVal(uint8_t u8Tube, uint8_t u8Value);
static void NIXIE_DisplayUpdate(void);
static void NIXIE_DisplayLeft(uint8_t u8);
static void NIXIE_DisplayRight(uint8_t u8);
static void NIXIE_BacklightOn(void);
static void NIXIE_BacklightOff(void);
static void NIXIE_BacklightToggle(void);
static void NIXIE_ReadTime(void);
static void NIXIE_SaveTime(uint8_t u8Hrs, uint8_t u8Min, uint8_t u8Sec);
static void NIXIE_SelfTest(void);
static void NIXIE_RunClockMode(void);
static void NIXIE_RunSettingsMode(void);

/* TWI */
#define TWI_DDR DDRC
#define TWI_PORT PORTC
#define TWI_SCL_PIN 5
#define TWI_SDA_PIN 4

static void TWI_Init(void);
static void TWI_Start(void);
static void TWI_Stop();
static void TWI_Write(uint8_t u8);
static void TWI_Read(uint8_t* u8);

/* BTN */
#define BTN_P0_PIN 5

#define BTN_P0 ((unsigned char)_BV(BTN_P0_PIN))

static void BTN_Init(void);

/* DS3231SO */
#define DS3231_ADR_W 0xD0
#define DS3231_ADR_R 0xD1

#define DS3231_REG_TIME_SEC 0x00
#define DS3231_REG_TIME_MIN 0x01
#define DS3231_REG_TIME_HRS 0x02
#define DS3231_REG_TIME_DAY 0x03
#define DS3231_REG_TIME_DATE 0x04
#define DS3231_REG_TIME_MONTH 0x05
#define DS3231_REG_TIME_YEAR 0x06
#define DS3231_REG_ALARM1_SEC 0x07
#define DS3231_REG_ALARM1_MIN 0x08
#define DS3231_REG_ALARM1_HRS 0x09
#define DS3231_REG_ALARM1_DAY 0x0a
#define DS3231_REG_ALARM2_MIN 0x0b
#define DS3231_REG_ALARM2_HRS 0x0c
#define DS3231_REG_ALARM2_DAY 0x0d
#define DS3231_REG_CONTROL 0x0e
#define DS3231_REG_STATUS 0x0f
#define DS3231_REG_AGING_OFFSET 0x10
#define DS3231_REG_MSB_TEMP 0x11
#define DS3231_REG_LSB_TEMP 0x12

/* Control Register (0Eh) */
#define EOSC 7
#define BBSQW 6
#define CONV 5
#define RS2 4
#define RS1 3
#define INTCN 2
#define A2IE 1
#define A1IE 0

/* Status Register (0Fh) */
#define OSF 7
#define EN32KHZ 3
#define BSY 2
#define A2F 1
#define A1F 0

void DS3231_Write(uint8_t u8Address, uint8_t u8Value);
uint8_t DS3231_Read(uint8_t u8Address);

/* IRQ */
static void IRQ_Init(void);

#endif // __NIXIE_CLOCK__