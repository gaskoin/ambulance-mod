#include <msp430.h>
#include <stdint.h>

#define LEFT BIT0
#define RIGHT BIT1
#define POWER BIT2
#define SIRENE BIT6

#define TONE 150

#define LIGHT_SWITCH_INTERVAL 8
#define SIRENE_FEQUENCY_SWITCH_INTERVAL 16
#define POWER_INTERVAL 150

#define LOW_FREQUENCY 1500
#define HIGH_FREQUENCY 2000

#define rightOn() (P1OUT |= RIGHT)
#define leftOff() (P1OUT &= ~LEFT)
#define powerOn() (P1OUT |= POWER)
#define powerOff() (P1OUT &= ~POWER)

void configureClocks();
void configureIO();
void configureWatchdog();
void configureTimer();

void handlePowerOff();
void handleLightSwitch();
void handleSireneFrequencySwitch();

volatile uint16_t frequency = LOW_FREQUENCY;

// This is very low frequency interruption so I do not care with handling all of these inside
__attribute__((__interrupt__(WDT_VECTOR))) void Watchdog()
{
    handlePowerOff();
    handleLightSwitch();
    handleSireneFrequencySwitch();
}

inline void handlePowerOff()
{
    static volatile uint16_t powerOffInterval = 0;
    if (++powerOffInterval == POWER_INTERVAL)
    {
        powerOff();
    }
}

inline void handleLightSwitch()
{
    static volatile uint8_t lightSwitchInterval = 0;
    if (++lightSwitchInterval == LIGHT_SWITCH_INTERVAL)
    {
        P1OUT ^= LEFT | RIGHT;
        lightSwitchInterval = 0;
    }
}

inline void handleSireneFrequencySwitch()
{
    static volatile uint8_t sireneSwitchInterval = 0;
    if (++sireneSwitchInterval == SIRENE_FEQUENCY_SWITCH_INTERVAL)
    {
        frequency = (frequency == LOW_FREQUENCY) ? HIGH_FREQUENCY : LOW_FREQUENCY;
        TACCR0 = frequency;
        sireneSwitchInterval = 0;
    }
}

void main()
{
    configureWatchdog();
    configureClocks();
    configureIO();

    powerOn();
    rightOn();
    leftOff();

    configureTimer();
    __enable_interrupt();
    __low_power_mode_1();
}

void configureTimer()
{
    TACTL |= TASSEL1 | MC0;
    TACCR0 = frequency;
    TACCR1 = TONE;
    TA0CCTL1 |= OUTMOD_7;
}

void configureClocks()
{
    // ACLK = 6khz, SMCLK = 1MHz
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;
    BCSCTL3 = LFXT1S1;
}

void configureIO()
{
    P1SEL |= SIRENE;
    P1DIR |= 0xff;
}

void configureWatchdog()
{
    WDTCTL = WDTPW | WDTTMSEL | WDTSSEL | WDTIS1;
    IE1 |= WDTIE;
}
