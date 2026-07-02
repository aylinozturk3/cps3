/*
 * server.c - Raspberry Pi 5
 * Lab05 UART Server
 *
 * Derleme:
 *   gcc -o server server.c
 *
 * Calistirma:
 *   ./server
 *
 * Protokol:
 *   dsPIC -> Pi:  "XY 512 384\n"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define PORT  "/dev/ttyUSB0"
#define BAUD  B9600

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

int main(void)
{
    int fd = open_serial(PORT, BAUD);
    if (fd < 0) { fprintf(stderr, "Port acilamadi: %s\n", PORT); return -1; }

    printf("[OK] Server baslatildi: %s @ 9600 baud\n", PORT);
    printf("     Ctrl+C ile durdur\n\n");

    char line[64];

    while (1) {
        if (read_line(fd, line, sizeof(line)) <= 0) continue;

        int x, y;
        if (sscanf(line, "XY %d %d", &x, &y) != 2) {
            fprintf(stderr, "[WARN] Parse hatasi: %s\n", line);
            continue;
        }

        printf("X=%4d  Y=%4d\n", x, y);
    }

    close(fd);
    return 0;
}
