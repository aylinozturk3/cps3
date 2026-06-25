#ifndef __UART_H
#define __UART_H

#include <xc.h>
#include <stdint.h>

#define UART_ERR -1
#define UART_SUC 0

#define BIT9		0x06
#define BIT8_ODD	0x04
#define BIT8_EVEN	0x02
#define BIT8_NO		0x00

#define BIT_STOP_1	0x00
#define BIT_STOP_2	0x01

#define CTRL_SIMPLE	0x00
#define CTRL_FLOW	0x01

#define TX_INT_SINGLE	0x8000
#define TX_INT_EMPTY	0x2000
#define TX_INT_LAST	0x0000

#define RX_INT_FULL	0x00C0
#define RX_INT_3OF4	0x0080
#define RX_INT_SINGLE	0x0040

#define BAUD_9600 9600
#define BAUD_115200 115200


#include <xc.h>
#include <stdint.h>

// Makroyu burada tan?ml?yoruz ki hem uart.c hem de aylin1.c g�rebilsin
#define RX_BUFFER_SIZE 50 

// Global de?i?kenlerin harici bildirimleri (extern)
extern volatile char rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16_t rx_index;
extern volatile uint8_t data_ready_flag;

// Fonksiyon prototipleri
void uart1_init(unsigned long baud, unsigned int byteformat, unsigned int mode);
void uart2_init(unsigned long baud, unsigned int byteformat, unsigned int mode);
void uart2_send_8(int8_t data);
/*
inline void uart1_init(unsigned long baud, unsigned int byteformat, unsigned int mode);

inline signed char uart1_putc(unsigned char data);

inline signed char uart1_getc(unsigned char* data);


inline void uart2_init(unsigned long baud, unsigned int byteformat, unsigned int mode);

inline signed char uart2_putc(unsigned char data);

inline signed char uart2_getc(unsigned char *data);*/


#endif
