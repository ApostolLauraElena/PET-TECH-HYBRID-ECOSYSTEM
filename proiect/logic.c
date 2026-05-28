#include "logic.h"
#include "hardware.h"
#include "config.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdint.h>

static long tara_mancare = 0; 
static long tara_apa = 0; 
static float grame_mancare = 0.0f;
static float ml_apa = 0.0f;

// Adaugă astea undeva în logic.c
static float temperatura_curenta = 0.0f; // Salvează temperatura aici în update_senzori()

float get_temperatura(void) { return temperatura_curenta; }
float get_grame_mancare(void) { return grame_mancare; }
float get_ml_apa(void) { return ml_apa; }

void calibrare_sistem(void) {
    sleep_ms(2000); 
    printf("\n[SISTEM] Calibrare automată (3 secunde). Nu atingeți bolurile!\n");
    
    int64_t suma_mancare = 0;
    int64_t suma_apa = 0;
    int numar_mostre = 0;

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - start_time < 3000) {
        long m = read_hx711(MANCARE_SCK, MANCARE_DOUT);
        long a = read_hx711(APA_SCK, APA_DOUT);
        if (m != -999999) suma_mancare += m;
        if (a != -999999) suma_apa += a;
        numar_mostre++;
        sleep_ms(100); 
    }

    if (numar_mostre > 0) {
        tara_mancare = (long)(suma_mancare / numar_mostre);
        tara_apa = (long)(suma_apa / numar_mostre);
    }
    printf("[SISTEM] Calibrare finalizată cu succes.\n");

    gpio_put(LED_MANCARE, false);
    gpio_put(LED_APA, false);
}

void update_control_manual(void) {
    if (gpio_get(BUTON_MANCARE_PIN) == 0) { 
        printf("[MANCARE] Buton apăsat! Deschidem clapeta manual...\n");
        set_servo_position(SERVO_PIN, SERVO_DESCHIS); 
        
        while(gpio_get(BUTON_MANCARE_PIN) == 0) {
            sleep_ms(50); 
        }
        
        printf("[MANCARE] Buton eliberat. Închidem clapeta.\n");
        set_servo_position(SERVO_PIN, SERVO_INCHIS);
        sleep_ms(1000); 
    }
}

void update_senzori(void) {
    float temperatura = citeste_temperatura();
    temperatura_curenta = temperatura;
    // Alerta pe LED dacă apa e prea caldă
    gpio_put(LED_APA, (temperatura > TEMP_LIMITA));

    long val_mancare = read_hx711(MANCARE_SCK, MANCARE_DOUT);
    grame_mancare = (val_mancare != -999999) ? (float)(val_mancare - tara_mancare) / FACTOR_MANCARE : 0.0f;
    if (grame_mancare < 0.0f) grame_mancare = 0.0f; 

    long val_apa = read_hx711(APA_SCK, APA_DOUT);
    ml_apa = (val_apa != -999999) ? (float)(val_apa - tara_apa) / FACTOR_APA : 0.0f;
    if (ml_apa < 0.0f) ml_apa = 0.0f; 

    printf("Monitorizare -> Mâncare: %.1f g | Apă: %.1f ml | Temp apă: %.1f C\n", 
           grame_mancare, ml_apa, temperatura); 
}

void update_dozator_mancare(void) {
    if (grame_mancare < 15.0f) {
        printf("[MANCARE] Nivel scăzut. Deschidem clapeta la impuls %d...\n", SERVO_DESCHIS);
        set_servo_position(SERVO_PIN, SERVO_DESCHIS); 
        
        uint32_t servo_start = to_ms_since_boot(get_absolute_time());
        while (true) {
            long val = read_hx711(MANCARE_SCK, MANCARE_DOUT);
            if (val != -999999) {
                grame_mancare = (float)(val - tara_mancare) / FACTOR_MANCARE;
            }
            
            if (grame_mancare >= ANTICIPARE_MANCARE) { 
                printf("[MANCARE] Prag anticipat atins: %.1f g. Închidem.\n", grame_mancare);
                break; 
            }
            if (to_ms_since_boot(get_absolute_time()) - servo_start > 8000) { 
                printf("[MANCARE] Timeout de siguranță clapetă!\n");
                break;
            }
            sleep_ms(50); 
        }
        set_servo_position(SERVO_PIN, SERVO_INCHIS); 
        sleep_ms(2000); 
    }
}

void update_dozator_apa(void) {
    if (ml_apa < 20.0f) {
        printf("[APA] Nivel scăzut. Pompăm o rafală scurtă...\n");
        porneste_pompa(); 
        sleep_ms(1000); 
        
        opreste_pompa();
        printf("[APA] Așteptăm stabilizarea apei în bol...\n");
        sleep_ms(2000); 
    } 
}