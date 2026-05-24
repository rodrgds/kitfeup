#include "gpio.h"
#include <stdio.h>

MapeamentoPino mapa_placa[27] = { //apenas pinos do Header J3 (pinos do lado do flashcard)
    [3]  = { BANCO_B, 20 },
    [5]  = { BANCO_B, 21 },
    [7]  = { BANCO_B, 18 },
    [8]  = { BANCO_A, 16 },
    [10] = { BANCO_A, 17 },
    [11] = { BANCO_B, 11 },
    [12] = { BANCO_B, 19 },
    [13] = { BANCO_B, 12 },
    [15] = { BANCO_B, 22 },
    [16] = { BANCO_A, 20 },
    [18] = { BANCO_A, 19 },
    [19] = { BANCO_B, 13 },
    [21] = { BANCO_B, 14 },
    [22] = { BANCO_A, 18 },
    [23] = { BANCO_B, 15 },
    [24] = { BANCO_B, 16 },
    [26] = { BANCO_A, 28 }
};

static volatile uint32_t *gpioa_virt_base = NULL;
static volatile uint32_t *gpiob_virt_base = NULL;

// Inicializa o acesso aos GPIOs mapeando as regiões físicas para memória virtual
int gpio_init(umdp_connection *conn) {
    
    if (conn == NULL) return -1;

    // Mapear o Banco A
    int ret = umdp_mmap_physical(conn, GPIOA_FISICO, GPIO_SIZE, (void **)&gpioa_virt_base);
    if (ret != 0) {
        fprintf(stderr, "Failed to map GPIOA: %s\n", umdp_strerror(ret));
        return ret;
    }

    // Mapear o Banco B
    ret = umdp_mmap_physical(conn, GPIOB_FISICO, GPIO_SIZE, (void **)&gpiob_virt_base);
    if (ret != 0) {
        fprintf(stderr, "Failed to map GPIOB: %s\n", umdp_strerror(ret));
        return ret;
    }

    return 0; 
}

// Configura o modo de um pino (INPUT ou OUTPUT)
int pinMode(int numero_do_pino, int modo) {
    
    MapeamentoPino pino = mapa_placa[numero_do_pino];

    if (pino.bit == 0 && pino.banco == 0) {
        fprintf(stderr, "Pino %d não mapeado. Verifique o mapa de pinos.\n", numero_do_pino);
        return 1;
    }
    
    uint32_t *gpio2_mem = pino.banco == BANCO_A ? (uint32_t *)gpioa_virt_base : (uint32_t *)gpiob_virt_base;
    
    if (modo == OUTPUT) {
        gpio2_mem[1] |= (1 << pino.bit);  
    } else {
        gpio2_mem[1] &= ~(1 << pino.bit); 
    }
    
    return 0;   
}

// Escreve um valor digital (HIGH ou LOW) em um pino configurado como OUTPUT    
int digitalWrite(int numero_do_pino, int estado) {
    
    MapeamentoPino pino = mapa_placa[numero_do_pino];

    if (pino.bit == 0 && pino.banco == 0) {
        fprintf(stderr, "Pino %d não mapeado. Verifique o mapa de pinos.\n", numero_do_pino);
        return 1;
    }
    
    uint32_t *gpio2_mem = pino.banco == BANCO_A ? (uint32_t *)gpioa_virt_base : (uint32_t *)gpiob_virt_base;
    
    if (estado == HIGH) {
        gpio2_mem[0] |= (1 << pino.bit);  
    } else {
        gpio2_mem[0] &= ~(1 << pino.bit); 
    }
    
    return 0;
}