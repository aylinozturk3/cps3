/*
 * server.c - Raspberry Pi 5
 * Lab05 UART Server
 *
 * Derleme:
 *   gcc -o server server.c -lm
 *
 * Calistirma:
 *   ./server
 *
 * Protokol:
 *   dsPIC -> Pi:  "XY 512 384\n"    (ADC ham deger, 0-1023)
 *   Pi -> dsPIC:  "XY 1430 1442\n"  (servo pulse ms * 1000, integer)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <math.h>

#define PORT  "/dev/ttyUSB0"
#define BAUD  B9600

/* --- Setpoint (merkez ADC degeri, 0-1023) --- */
#define CENTER_X    390
#define CENTER_Y    360

/* --- Servo merkez PWM (ms) --- */
#define PWM_CENTER_X  1.430f
#define PWM_CENTER_Y  1.442f

/* --- PWM Limitleri (ms) --- */
#define PWM_MIN  1.0f
#define PWM_MAX  2.0f

/* --- PD Parametreleri (lab06.c'den) --- */
#define KP_X  0.0006f
#define KD_X  0.014f
#define KP_Y  0.0008f
#define KD_Y  0.017f
#define DT    0.02f

/* --- Butterworth (N=1, fc=3Hz, fs=50Hz) --- */
#define B0   0.160200350887737f
#define B1   0.160200350887737f
#define A1  -0.679599298224527f

typedef struct { float prev_error; } pd_state_t;
typedef struct { float x_prev, y_prev; } filter_state_t;

/* -------------------------------------------------------
 * Butterworth filtre
 * ------------------------------------------------------- */
static float butterworth(float x, filter_state_t *s)
{
    float y = B0 * x + B1 * s->x_prev - A1 * s->y_prev;
    s->x_prev = x;
    s->y_prev = y;
    return y;
}

/* -------------------------------------------------------
 * PD kontrolcu
 * ------------------------------------------------------- */
static float pd_update(float setpoint, float current,
                       pd_state_t *s, float kp, float kd)
{
    float error   = setpoint - current;
    float d_error = (error - s->prev_error) / DT;
    s->prev_error = error;
    return kp * error + kd * d_error;
}

/* -------------------------------------------------------
 * Clamp
 * ------------------------------------------------------- */
static float clamp(float val, float min, float max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/* -------------------------------------------------------
 * Seri Port Ac
 * ------------------------------------------------------- */
static int open_serial(const char *port, speed_t baud)
{
    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0) { perror("open"); return -1; }

    struct termios tio;
    tcgetattr(fd, &tio);
    cfsetispeed(&tio, baud);
    cfsetospeed(&tio, baud);
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8 | CLOCAL | CREAD;
    tio.c_iflag  = IGNPAR;
    tio.c_oflag  = 0;
    tio.c_lflag  = 0;
    tio.c_cc[VMIN]  = 1;
    tio.c_cc[VTIME] = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tio);
    return fd;
}

/* -------------------------------------------------------
 * Satir Oku
 * ------------------------------------------------------- */
static int read_line(int fd, char *buf, int max_len)
{
    int i = 0;
    char c;
    while (i < max_len - 1) {
        if (read(fd, &c, 1) <= 0) return -1;
        if (c == '\n' || c == '\r') { if (i > 0) break; }
        else buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

/* -------------------------------------------------------
 * Main
 * ------------------------------------------------------- */
int main(void)
{
    int fd = open_serial(PORT, BAUD);
    if (fd < 0) { fprintf(stderr, "Port acilamadi: %s\n", PORT); return -1; }

    printf("[OK] Server baslatildi: %s @ 9600 baud\n", PORT);
    printf("     Ctrl+C ile durdur\n\n");

    pd_state_t     pd_x  = {0.0f}, pd_y  = {0.0f};
    filter_state_t flt_x = {0.0f, 0.0f}, flt_y = {0.0f, 0.0f};

    char line[64], cmd[64];

    while (1) {
        /* 1. dsPIC'ten veri al */
        if (read_line(fd, line, sizeof(line)) <= 0) continue;

        /* 2. Parse et: "XY 512 384" */
        int x_raw, y_raw;
        if (sscanf(line, "XY %d %d", &x_raw, &y_raw) != 2) {
            fprintf(stderr, "[WARN] Parse hatasi: %s\n", line);
            continue;
        }
        printf("RX: X=%4d Y=%4d", x_raw, y_raw);

        /* 3. Butterworth filtre */
        float x_filt = butterworth((float)x_raw, &flt_x);
        float y_filt = butterworth((float)y_raw, &flt_y);

        /* 4. PD kontrolcu */
        float ctrl_x = clamp(PWM_CENTER_X + pd_update(CENTER_X, x_filt, &pd_x, KP_X, KD_X), PWM_MIN, PWM_MAX);
        float ctrl_y = clamp(PWM_CENTER_Y + pd_update(CENTER_Y, y_filt, &pd_y, KP_Y, KD_Y), PWM_MIN, PWM_MAX);

        printf("   TX: X=%.3fms Y=%.3fms\n", ctrl_x, ctrl_y);

        /* 5. dsPIC'e gonder: "XY 1430 1442\n" */
        snprintf(cmd, sizeof(cmd), "XY %d %d\n",
                 (int)(ctrl_x * 1000.0f),
                 (int)(ctrl_y * 1000.0f));
        write(fd, cmd, strlen(cmd));
    }

    close(fd);
    return 0;
}
