#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "umdp.h"
#include "gpio.h"

#define BUZZER_PIN 26

//Frequências das Notas Musicais (Hz)
#define NOTE_C4  261
#define NOTE_D4  294
#define NOTE_E4  329
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

void play_note_software(int frequency, int duration_ms) {
    if (frequency == 0) {
        digitalWrite(BUZZER_PIN, LOW);  // Desliga o buzzer
        usleep(duration_ms * 1000);
        return;
    }

    //1 segundo = 1 000 000 us
    uint32_t period_us = 1000000u / frequency;
    uint32_t half_period_us = period_us / 2u;

    uint32_t total_cycles = (duration_ms * 1000u) / period_us;

    for (uint32_t i = 0; i < total_cycles; i++) {
        digitalWrite(BUZZER_PIN, HIGH); // Liga o buzzer
        usleep(half_period_us);       // Espera metade do período
        
        digitalWrite(BUZZER_PIN, LOW);  // Desliga o buzzer
        usleep(half_period_us);       // Espera a outra metade
    }
}

int main() {
    umdp_connection *conn;
    int ret;

    //Conectar ao UMDP
    conn = umdp_connect();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to UMDP. Run: insmod umdp.ko\n");
        return 1;
    }

    //Configurar GPIOs
    if (gpio_init(conn) != 0) {
        fprintf(stderr, "Failed to initialize GPIO: %s\n", umdp_strerror(-1));
        umdp_disconnect(conn);
        return 1;
    }
    
    if (pinMode(BUZZER_PIN, OUTPUT) != 0) {
        fprintf(stderr, "Failed to set pin mode \n");
        umdp_disconnect(conn);
        return 1;
    }

    printf("A tocar melodia via controlo direto de MMIO (Software Polling)...\n");

    int melody[] = { NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4 };
    int durations[] = { 350, 350, 350, 350, 150, 400 };

    for (int i = 0; i < 6; i++) {
        play_note_software(melody[i], durations[i]);
        play_note_software(0, 40); // Pausa entre notas
    }

    digitalWrite(BUZZER_PIN, LOW); // Garantir que o buzzer está desligado
    umdp_disconnect(conn);
    
    printf("Melodia terminada!\n");
    return 0;
}



