#define FCY 12800000UL
#include <libpic30.h>
#include <p33FJ256MC710.h>


#include <stdio.h>

// Kendi yazdığın kütüphaneler (Makefile bunları aynı klasörde bulacak)
#include "led.h"
#include "lab05.h"
#include "uart.h"
#include "types.h"

/* FCY Tanımı */



/* Configuration of the Chip */
// Initial Oscillator Source Selection = Primary (XT, HS, EC) Oscillator with PLL
#pragma config FNOSC = PRIPLL
// Primary Oscillator Mode Select = XT Crystal Oscillator mode
#pragma config POSCMD = XT
// OSC2 Pin Function: OSC2 is Clock Output
#pragma config OSCIOFNC = ON
// Watchdog Timer Enable = Watchdog Timer enabled/disabled by user software
// (LPRC can be disabled by clearing the SWDTEN bit in the RCON register)
#pragma config FWDTEN = OFF

// KÜTÜPHANEDEN ARAMASIN DİYE İSMİNİ DEĞİŞTİRİYORUZ



int main(){
    
    
    mainloop();
    // Stop
    while(1)
        ;
      
}
