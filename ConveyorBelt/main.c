#include <msp430g2553.h>
#include "PCD8544.h"

#define LCD5110_SCLK_PIN            BIT5
#define LCD5110_DN_PIN              BIT7
#define LCD5110_SCE_PIN             BIT0
#define LCD5110_DC_PIN              BIT1
#define LCD5110_SELECT              P1OUT &= ~LCD5110_SCE_PIN
#define LCD5110_DESELECT            P1OUT |= LCD5110_SCE_PIN
#define LCD5110_SET_COMMAND         P1OUT &= ~LCD5110_DC_PIN
#define LCD5110_SET_DATA            P1OUT |= LCD5110_DC_PIN
#define LCD5110_COMMAND             0
#define LCD5110_DATA                1
#define PERIOD 1000
#define MIN_DUTY_CYCLE 0
#define MAX_DUTY_CYCLE 1000

volatile unsigned int dutyCycle = MIN_DUTY_CYCLE;

void setupPWM(void)
{
    P1DIR |= BIT2;              
    P1SEL |= BIT2;              

    TA0CCR0 = PERIOD - 1;       
    TA0CCTL1 = OUTMOD_7;        
    TA0CCR1 = dutyCycle;        

    TA0CTL = TASSEL_2 + MC_1;
}

void setupButtons(void)
{
    P2DIR &= ~(BIT0 + BIT3 + BIT4 + BIT5);
    P2REN |= (BIT0 + BIT3 + BIT4 + BIT5);
    P2OUT &= ~(BIT0 + BIT3 + BIT4 + BIT5);  
    P2IE |= (BIT0 + BIT3 + BIT4 + BIT5);   
    P2IES |= (BIT0 + BIT3 + BIT4 + BIT5);  
    P2IFG &= ~(BIT0 + BIT3 + BIT4 + BIT5); 
    P2DIR |= BIT1 + BIT2;
    P2OUT |= BIT1;
    P2OUT &= ~BIT2;
    P1DIR &= ~BIT4;
    P1REN !=BIT4;
    P1OUT |= BIT4;

}

void setAddr(unsigned char xAddr, unsigned char yAddr);
void writeToLCD(unsigned char dataCommand, unsigned char data);
void writeCharToLCD(char c);
void writeStringToLCD(const char *string);
void initLCD();
void clearLCD();
void clearBank(unsigned char bank);

volatile unsigned int objectCount = 0;

void main(void) {


    WDTCTL = WDTPW | WDTHOLD;



    setupPWM();
    setupButtons();
    __enable_interrupt();

    P1OUT |= LCD5110_SCE_PIN | LCD5110_DC_PIN;
    P1DIR |= LCD5110_SCE_PIN | LCD5110_DC_PIN;



    P1SEL |= LCD5110_SCLK_PIN | LCD5110_DN_PIN;
    P1SEL2 |= LCD5110_SCLK_PIN | LCD5110_DN_PIN;

    UCB0CTL0 |= UCCKPH | UCMSB | UCMST | UCSYNC;
    UCB0CTL1 |= UCSSEL_2;
    UCB0BR0 |= 0x01;
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;

    __delay_cycles(500000);
    initLCD();
    clearLCD();

    setAddr(0, 1);
            writeStringToLCD("Conveyor Belt");

    setAddr(5, 5);
           writeStringToLCD("Ibrahim Ozer");




    while(1) {
        setAddr(5, 3);  //
        writeStringToLCD("Count: ");
        setAddr(2, 3);


        writeCharToLCD((char) ('0' + objectCount / 100));

        writeCharToLCD((char) ('0' + (objectCount / 10) % 10));

        writeCharToLCD((char) ('0' + objectCount % 10));




        }
            __delay_cycles(100000);

            setAddr(5, 4);

                   if (dutyCycle == 300) {
                       writeStringToLCD("Slow Mode");
                   } else if (dutyCycle == 600) {
                       writeStringToLCD("Normal Mode");
                   } else if (dutyCycle == 900) {
                       writeStringToLCD("Fast Mode");
                   } else if (dutyCycle == 0) {
                       writeStringToLCD("Stop");
                   } else {
                       writeStringToLCD("Unknown");
                   }



    }
}


#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{

    if (P1IFG & BIT4){

        objectcount++;
    }

    P1IFG &= ~BIT4;
}

#pragma vector=PORT2_VECTOR
__interrupt void Port2_ISR(void)
{
    if (P2IFG & BIT0)
    {
        dutyCycle = 300;
    }
    if (P2IFG & BIT3)
    {
        dutyCycle = 600;
    }
    if (P2IFG & BIT4)
    {
        dutyCycle = 900;
    }
    if (P2IFG & BIT5)
    {
        dutyCycle = 0;
    }

    TA0CCR1 = dutyCycle;

    P2IFG &= ~(BIT0 + BIT3 + BIT4 + BIT5);
}




void setAddr(unsigned char xAddr, unsigned char yAddr) {
    writeToLCD(LCD5110_COMMAND, PCD8544_SETXADDR | xAddr);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETYADDR | yAddr);
}

void writeToLCD(unsigned char dataCommand, unsigned char data) {
    LCD5110_SELECT;

    if(dataCommand) {
        LCD5110_SET_DATA;
    } else {
        LCD5110_SET_COMMAND;
    }

    UCB0TXBUF = data;
    while(!(IFG2 & UCB0TXIFG));
    LCD5110_DESELECT;
}

void initLCD() {
    writeToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETVOP | 0x3F);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETTEMP | 0x02);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETBIAS | 0x03);
    writeToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET);
    writeToLCD(LCD5110_COMMAND, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
}

void writeCharToLCD(char c) {
    unsigned char i;
    for(i = 0; i < 5; i++) {
        writeToLCD(LCD5110_DATA, font[c - 0x20][i]);
    }
    writeToLCD(LCD5110_DATA, 0);
}

void writeStringToLCD(const char *string) {
    while(*string) {
        writeCharToLCD(*string++);
    }
}

void clearLCD() {
    setAddr(0, 0);
    int i = 0;
    while(i < PCD8544_MAXBYTES) {
        writeToLCD(LCD5110_DATA, 0);
        i++;
    }
    setAddr(0, 0);
}

void clearBank(unsigned char bank) {
    setAddr(0, bank);
    int i = 0;
    while(i < PCD8544_HPIXELS) {
        writeToLCD(LCD5110_DATA, 0);
        i++;
    }
    setAddr(0, bank);
}
