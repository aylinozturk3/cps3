#ifndef LAB05_H
#define LAB05_H

#include <xc.h>

// Fonksiyon Prototipleri
void uart2_send(char* str);
void adc_touchscreen_init(void);
void servo_timer_init(char axis);
unsigned int servo_ms_to_counts(float ms, unsigned int prescaler);
void servo_set_ms(char axis, float ms);
void touchscreen_direction_select(char axis);

// ADC okuma için yapı (struct) tanımı
typedef struct {
   unsigned int x; 
   unsigned int y; 
} Pos;

Pos ADC_touchscreen_read(void);
void mainloop(void);

#endif /* LAB05_H */