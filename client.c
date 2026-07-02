/*
 * client.c - dsPIC33FJ256MC710
 *
 * Ana dongu:
 *   1. ADC touchscreen X,Y oku
 *   2. Pi'ye "XY x y\n" gonder
 *   3. Pi'den "XY x_ms y_ms\n" gelirse servo'ya uygula
 */

#define FCY 12800000UL
#include <libpic30.h>
#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lab05.h"
#include "uart.h"
#include "led.h"
#include "common.h"

/* uart.c'deki global degiskenler */
extern volatile char     rx_buffer[];
extern volatile uint16_t rx_index;
extern volatile uint8_t  data_ready_flag;

/* -------------------------------------------------------
 * Pi'den gelen servo komutunu isle
 * Format: "XY 1450 1320"  (ms * 1000, integer)
 * ------------------------------------------------------- */
static void process_command(void)
{
    int x_val, y_val;

    if (sscanf((char*)rx_buffer, "XY %d %d", &x_val, &y_val) == 2) {
        float x_ms = x_val / 1000.0f;
        float y_ms = y_val / 1000.0f;

        servo_set_ms('X', x_ms);
        servo_set_ms('Y', y_ms);

        /* LED3 - komut alindi */
        SETLED(LED3_PORT);
        __delay_ms(50);
        CLEARLED(LED3_PORT);
    }

    /* Buffer temizle */
    rx_index        = 0;
    data_ready_flag = 0;
}

/* -------------------------------------------------------
 * Ana Dongu
 * ------------------------------------------------------- */
void client_loop(void)
{
    char buf[32];
    Pos pos;

    while (1) {
        /* 1. ADC oku */
        pos = ADC_touchscreen_read();

        /* 2. Pi'ye gonder: "XY 512 384\n" */
        sprintf(buf, "XY %u %u\n", pos.x, pos.y);
        uart2_send(buf);

        SETLED(LED5_PORT);
        __delay_ms(50);
        CLEARLED(LED5_PORT);

        /* 3. Pi'den komut geldi mi? */
        if (data_ready_flag) {
            process_command();
        }

        __delay_ms(20);   /* ~50 Hz */
    }
}
