#ifndef GPIO_H
#define GPIO_H

#include "umdp.h"

#define BANCO_A 0
#define BANCO_B 1

#define GPIO_SIZE             0x1000
#define GPIOA_FISICO          0x03020000
#define GPIOB_FISICO          0x03021000

#define INPUT  0
#define OUTPUT 1
#define HIGH   1
#define LOW    0

typedef struct {
    int banco;    // BANCO_A ou BANCO_B
    int bit;      // O bit de 0 a 31
} MapeamentoPino;


int pinMode(int numero_do_pino, int modo);
int digitalWrite(int numero_do_pino, int estado);
int gpio_init(umdp_connection *conn);

#endif