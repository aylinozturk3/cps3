/*
 * File:     uart.c
 * Author:   cps-update-hiwis
 *
 * Created on June 1, 2026, 2:16 PM
 */
#define FCY 12800000UL
#include <libpic30.h>


#include "uart.h"
#include <xc.h>
#include <stdint.h>
#include "types.h"


// Note: Ensure RX_BUFFER_SIZE is defined in your uart.h file
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_index = 0;
volatile uint8_t data_ready_flag = 0;

// Force microcontroller to always read values from RAM instead of optimizing via registers
void uart1_init(unsigned long baud, unsigned int byteformat, unsigned int mode)
{
    

    U1MODEbits.UARTEN = 0;
    
  
    IEC0bits.U1RXIE = 0;  // Kept off while initialization routine is executing
    IEC0bits.U1TXIE = 0;
        
   
    IFS0bits.U1RXIF = 0;
    IFS0bits.U1TXIF = 0;
        
   
    TRISFbits.TRISF2 = 1;    // RX Pin set as input to listen to external incoming signals
    TRISFbits.TRISF3 = 0;    // TX Pin set as output to drive the line for transmissions
    
    if (mode) {
        TRISDbits.TRISD14 = 1;    // CTS Pin set as input to check if external device is ready
        TRISDbits.TRISD15 = 0;    // RTS Pin set as output to signal readiness to external device
    }
    
    
    // UxBRG acts as a clock prescaler countdown timer
    U1BRG = (uint32_t)800000 / baud - 1; // Formula applies when BRGH = 0
    
    
    U1MODE |= 0x00;
    U1MODEbits.BRGH = 0; // Standard speed mode configuration
    
    U1MODEbits.UARTEN = 1; // Enable UART peripheral module
    
    if (mode) {    // Hardware Flow Control Mode
        U1MODEbits.RTSMD = 1;
        U1MODEbits.UEN = 2;
    } else {    // Simple Mode
        U1MODEbits.RTSMD = 0;
        U1MODEbits.UEN = 0;    
    }
    U1MODE |= byteformat & 0x07; // Map number of data bits, parity, and stop bits

    
    U1STA = 0;
    U1STAbits.UTXEN = 1;
}

void uart2_init(unsigned long baud, unsigned int byteformat, unsigned int mode)
{
    
    U2MODEbits.UARTEN = 0; // FIXED: Added missing semicolon here
    
    
    IEC1bits.U2RXIE = 0;
    IEC1bits.U2TXIE = 0;
    
    
    IFS1bits.U2RXIF = 0;
    IFS1bits.U2TXIF = 0;
        
    
    TRISFbits.TRISF4 = 1;    // RX Pin - input
    TRISFbits.TRISF5 = 0;    // TX Pin - output
    
    if (mode) {
        TRISFbits.TRISF12 = 1;    // CTS Pin
        TRISFbits.TRISF13 = 0;    // RTS Pin
    }
    U2MODEbits.BRGH = 0;
    
   
    U2BRG = (uint32_t)800000 / baud - 1; // Formula applies when BRGH = 0
    
    
    U2MODE |= 0x00;
    
    if (mode) {    // Hardware Flow Control Mode
        U2MODEbits.RTSMD = 1;
        U2MODEbits.UEN = 2;
    } else {    // Simple Mode
        U2MODEbits.RTSMD = 0;
        U2MODEbits.UEN = 0;    
    }
    U2MODE |= byteformat & 0x0007; // Map number of data bits, parity, and stop bits

    
    U2STA = 0;
    U2STAbits.UTXEN = 1; // Enable UART TX module driver hardware
    
    // Configure interrupt vector parameters
    IFS1bits.U2RXIF = 0;    // Flush any leftover historical flag latches clear
    IPC7bits.U2RXIP = 4;    // Establish baseline priority processing at Level 4
    IEC1bits.U2RXIE = 1;    // Turn on the UART2 Receiver Peripheral Interrupt vector
    
    
    U2MODEbits.STSEL = 0;   // 1 Stop bit
    U2MODEbits.PDSEL = 0;   // 8-bit data, no parity

    // E?er sinyal terslenmi?se bunu d�zeltir:
    U2STAbits.UTXINV = 1;   // TX pinini tersle 
    U2MODEbits.UARTEN = 1;
}

void uart2_send_8(int8_t data){
    while (U2STAbits.UTXBF); // Block execution if hardware transmit register is full
    U2TXREG = data; 
    while(!U2STAbits.TRMT);  // Wait until transmission shift register is empty
}

// Low-level Vector Interrupt Routine for UART2 Receive operations
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void)
{
    if (U2STAbits.OERR) {
        U2STAbits.OERR = 0; // Clear software overrun errors immediately to unlock FIFO buffer
    }

    while (U2STAbits.URXDA) { // Process characters while data is available in the hardware FIFO
        char received_char = U2RXREG & 0x00FF; 
        
        if (received_char == '\n' || received_char == '\r') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0';  // Append null terminator string delimiter
                data_ready_flag = 1;         // Raise flag alerting the scheduler task loop
            }
        } 
        else if (rx_index < (RX_BUFFER_SIZE - 1) && !data_ready_flag) {
            rx_buffer[rx_index++] = received_char; // Store value and increment index tracker
        }
    }

    IFS1bits.U2RXIF = 0; // Reset peripheral interrupt flag vector
} // FIXED: Removed the stray trailing curly bracket that was breaking compilation scope here

 
