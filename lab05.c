#define FCY 12800000UL
#include <libpic30.h>

#include <xc.h>
#include <p33FJ256MC710.h>

#include <stdint.h>
#include <stdio.h>



#include "lab05.h"
#include "lcd.h"
#include "led.h"

#include "uart.h" // uart2_send_8 
#include "common.h"



/*
 * PWM code
 */

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

#define PWM_MIN_US 1000 // macrosecond
#define PWM_MID_US 1500
#define PWM_MAX_US 2000
#define PWM_CYC_US 20000


//#define TMR2_PRESCALER 256UL
//#define TMR2_TICK_US ((TMR2_PRESCALER*1000000UL)/FCY) // 20ms full period 

//0.9 ms for servo - 0 degree
// 1.5ms for servo - 90 degree
//2.1ms for serco - 180 degree

//SERVO X-OC7, SERVO Y -OC8




 void uart2_send(char* str) {

    while(*str) {

        uart2_send_8(*str);

        str++;

    }
} 


void adc_touchscreen_init(void){
    CLEARBIT(AD1CON1bits.ADON);
    
    SETBIT(TRISBbits.TRISB15);     // RB15 input (AN15)
    SETBIT(TRISBbits.TRISB9);      // RB9 input (AN9)
    
    CLEARBIT(AD1PCFGLbits.PCFG15); // AN15 analog
    CLEARBIT(AD1PCFGLbits.PCFG9);  // AN9 analog
    
    CLEARBIT(AD1CON1bits.AD12B);   // 10-bit ADC
    AD1CON1bits.FORM = 0;          // Integer output
    
  
    AD1CON1bits.SSRC = 0x7;        
    
    AD1CON2 = 0;                   // No scan

    CLEARBIT(AD1CON3bits.ADRC);    // Internal clock
    AD1CON3bits.SAMC = 0x1F;       // 31 Tad
    AD1CON3bits.ADCS = 0x02;       // Tad = 3Tcy
    
   
    CLEARBIT(TRISEbits.TRISE1); 
    CLEARBIT(TRISEbits.TRISE2); 
    CLEARBIT(TRISEbits.TRISE3); 
    
    SETBIT(AD1CON1bits.ADON);
}

void servo_timer_init(char axis)
{   
        if(axis == 'X') {
        // Timer2 + OC8
        CLEARBIT(T2CONbits.TON);
        CLEARBIT(T2CONbits.TCS);
        CLEARBIT(T2CONbits.TGATE);
        TMR2 = 0x00;
        T2CONbits.TCKPS = TCKPS_256;
        PR2 = 1000;
        CLEARBIT(IFS0bits.T2IF);
        CLEARBIT(TRISDbits.TRISD7);  // OC8 output
        OC8R  = 0;
        OC8RS = 0;
        OC8CON = 0x0006;  // PWM, Timer2
    }
    else if(axis == 'Y') {
        // Timer3 + OC7
        CLEARBIT(T3CONbits.TON);
        CLEARBIT(T3CONbits.TCS);
        CLEARBIT(T3CONbits.TGATE);
        TMR3 = 0x00;
        T3CONbits.TCKPS = TCKPS_256;
        PR3 = 1000;
        CLEARBIT(IFS0bits.T3IF);
        CLEARBIT(TRISDbits.TRISD6);  // OC7 output
        OC7R  = 0;
        OC7RS = 0;
        OC7CON = 0x000E;  // PWM, Timer3
    }
}



// Servo PWM hesaplaması
unsigned int servo_ms_to_counts(float ms, unsigned int prescaler)
{
    // FCY = 12.8MHz, 1 tick = prescaler/FCY sec = 1 tick is 20 us
    float ticks_per_us = (float)FCY / prescaler / 1e6f; 
    return (unsigned int)(ms * 1000.0f * ticks_per_us); // ms -> us -> ticks
}

void servo_set_ms(char axis, float ms)
{
    // Clamp pulse width
    if(ms < 1.0f) ms = 1.0f;
    if(ms > 2.0f) ms = 2.0f;

    // Invert pulse 
    ms = 20.0f - ms; // ex. 2ms pulse istiyoruz ama ters calisiyor 2ms 0 verecek cunku 0 da calisiyor PWM, 18 ms de output 1 olacak PWM calismayacak

    // Hesapla counts
    unsigned int counts = servo_ms_to_counts(ms, 256);

    if(axis == 'X') {
        CLEARBIT(T2CONbits.TON);  // Timer kapat
        OC8RS = counts;            // PWM değeri güncelle
        SETBIT(T2CONbits.TON);    // Timer aç
    } 
    else if(axis == 'Y') {
        CLEARBIT(T3CONbits.TON);  // Timer kapat (Y için de Timer2 kullanıyoruz)
        OC7RS = counts;            // PWM değeri güncelle
        SETBIT(T3CONbits.TON);    // Timer aç
    }
}


/*
 * touch screen code
 */


void touchscreen_direction_select(char axis)
{   
    
    if(axis == 'X') {
    	
        CLEARBIT(PORTEbits.RE1);
        Nop();
        SETBIT(PORTEbits.RE2);
        Nop();
        SETBIT(PORTEbits.RE3);
        Nop();
        
        SETBIT(TRISBbits.TRISB15); //input
        CLEARBIT(AD1PCFGLbits.PCFG15);//as analog
    } 
   
    else if (axis == 'Y') {
        SETBIT(PORTEbits.RE1);
        Nop();
        CLEARBIT(PORTEbits.RE2);
        Nop();
        CLEARBIT(PORTEbits.RE3);
        Nop();
        
        SETBIT(TRISBbits.TRISB9); //input
        CLEARBIT(AD1PCFGLbits.PCFG9); // set as analog pin
    }

}



Pos ADC_touchscreen_read(void)
{   
    Pos position;
    //char uart_buffer[32]; // Buffer overflow'u önlemek için 32 byte yapıldı
    
    // --- X okuma ---
    touchscreen_direction_select('X');
    __delay_ms(10);               // Sinyalin oturması için döngüsel 10ms bekleme
    AD1CHS0bits.CH0SA = 15;        
    SETBIT(AD1CON1bits.SAMP);     // Örneklemeyi başlat
    CLEARBIT(AD1CON1bits.SAMP);   // Çevrimi el ile başlat (Manuel Mode)
    while(!AD1CON1bits.DONE);     
    position.x = ADC1BUF0;
    
    
    
    
    // --- Y okuma ---
    touchscreen_direction_select('Y');
    __delay_ms(10);               // Sinyalin oturması için döngüsel 10ms bekleme
    AD1CHS0bits.CH0SA = 9;        
    SETBIT(AD1CON1bits.SAMP);     
    __delay_ms(2);                
    CLEARBIT(AD1CON1bits.SAMP);   
    while(!AD1CON1bits.DONE);     
    position.y = ADC1BUF0;
    
    
    

    
    // Raspberry Pi UART Çıktısı

    //
    // sprintf(uart_buffer, "X:%u Y:%u\n", position.x, position.y);
    //uart2_send(uart_buffer);
    ////
               
    
    return position;
}




void mainloop(void)
{
    
    led_initialize(); 
 
    adc_touchscreen_init();
    servo_timer_init('X');
    servo_timer_init('Y');
    
    SETLED(LED1_PORT);          // Buraya kadar geldi = init tamam
    __delay_ms(500);
    CLEARLED(LED1_PORT);
    
    while(1)
    {
        
        
	    servo_set_ms('X', 1.2); // in microsec
	    servo_set_ms('Y', 1.2); 
	    __delay_ms(1000);
         SETLED(LED2_PORT);
	    ADC_touchscreen_read();
        CLEARLED(LED2_PORT);        // ADC BİTTİYSE söner
        SETLED(LED3_PORT);          // ADC başarıyla bitti kanıtı
        __delay_ms(1000);
        CLEARLED(LED3_PORT);
        
        __delay_ms(1000);
       
      
            
	    servo_set_ms('X', 1.2); 
		servo_set_ms('Y', 2.0); 
		__delay_ms(1000);
	    ADC_touchscreen_read();
        __delay_ms(2000);
        
		   
      
        
		servo_set_ms('X', 2.0); 
		servo_set_ms('Y', 2.0); 
		__delay_ms(1000);
	    ADC_touchscreen_read();
        __delay_ms(2000);
			 
            
           
		servo_set_ms( 'X', 2.0); 
		servo_set_ms('Y', 1.2); 
	    __delay_ms(1000);
		ADC_touchscreen_read();
		__delay_ms(2000);
       
            
        
    }
    
}
