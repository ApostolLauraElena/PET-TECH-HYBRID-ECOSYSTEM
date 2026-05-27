#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h> 
#include "pico/stdlib.h"
#include "hardware/pwm.h" 

#define LED_MANCARE 0
#define TRIG_MANCARE 2
#define ECHO_MANCARE 3
#define MANCARE_SCK  16  
#define MANCARE_DOUT 17

#define LED_APA 1
//#define TRIG_APA 4
//#define ECHO_APA 5
#define APA_SCK    14  
#define APA_DOUT   15  

#define SERVO_PIN 13       
#define RELEU_POMPA_PIN 18 

#define FACTOR_MANCARE 818.82f
#define FACTOR_APA     818.82f 
#define TINTA_MANCARE  50.0f
#define TINTA_APA      50.0f
#define ANTICIPARE_MANCARE 43.0f 
#define PRAG_RECIPIENT_MANCARE_CM 16.0f

#define TIMEOUT_POMPA_MS 15000 
#define TIMEOUT_ULTRASONIC_US 30000
// =========================================================================
// MODIFICĂ AICI DACĂ LOGICA POMPEI ESTE INVERSATĂ:
// Dacă pompa pornește singură la boot, schimbă PONRIT cu 1 și OPRIT cu 0
#define POMPA_PORNIT 0
#define POMPA_OPRIT  1
// =========================================================================

// VALORI PWM CONFIGURATE PENTRU CURSĂ MAXIMĂ (Ajustabile)
#define SERVO_INCHIS   1000  // Poziția de repaus / clapetă închisă (1ms)
#define SERVO_DESCHIS  2000  // Poziția de deschidere maximă (2ms)
long tara_mancare = 0; 
long tara_apa = 0; 

void init_senzor_mancare() {
    gpio_init(TRIG_MANCARE);
    gpio_set_dir(TRIG_MANCARE, GPIO_OUT);
    gpio_put(TRIG_MANCARE, 0);

    gpio_init(ECHO_MANCARE);
    gpio_set_dir(ECHO_MANCARE, GPIO_IN);
}

float citeste_distanta_mancare_cm() {
    gpio_put(TRIG_MANCARE, 0);
    sleep_us(2);
    gpio_put(TRIG_MANCARE, 1);
    sleep_us(10);
    gpio_put(TRIG_MANCARE, 0);

    absolute_time_t timeout_start = get_absolute_time();
    while (!gpio_get(ECHO_MANCARE)) {
        if (absolute_time_diff_us(timeout_start, get_absolute_time()) > TIMEOUT_ULTRASONIC_US) {
            return -1.0f;
        }
    }

    absolute_time_t echo_start = get_absolute_time();
    while (gpio_get(ECHO_MANCARE)) {
        if (absolute_time_diff_us(echo_start, get_absolute_time()) > TIMEOUT_ULTRASONIC_US) {
            return -1.0f;
        }
    }

    int64_t durata_us = absolute_time_diff_us(echo_start, get_absolute_time());
    return (float)durata_us / 58.0f;
}

void set_servo_position(uint pin, uint pulse_width_us) {
    pwm_set_gpio_level(pin, pulse_width_us);
}

void porneste_pompa() {
    gpio_put(RELEU_POMPA_PIN, POMPA_PORNIT); 
}

void opreste_pompa() {
    gpio_put(RELEU_POMPA_PIN, POMPA_OPRIT); 
}

long read_hx711(uint pin_sck, uint pin_dout) {
    int timeout = 50000; 
    while (gpio_get(pin_dout) && timeout > 0) {
        sleep_us(1);
        timeout--;
    } 
    if (timeout <= 0) return -999999; 

    long value = 0;
    for (int i = 0; i < 24; i++) {
        gpio_put(pin_sck, 1);
        sleep_us(1);
        value = (value << 1) | gpio_get(pin_dout);
        gpio_put(pin_sck, 0);
        sleep_us(1);
    }
    gpio_put(pin_sck, 1); sleep_us(1);
    gpio_put(pin_sck, 0); sleep_us(1);

    if (value & 0x800000) value |= 0xFF000000;
    return value;
}

int main() {
    stdio_init_all();

    // Configurare Servo
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 150.0f); 
    pwm_config_set_wrap(&config, 20000);   
    pwm_init(slice_num, &config, true);
    
    // Forțăm servo în poziția închis chiar de la boot
    set_servo_position(SERVO_PIN, SERVO_INCHIS); 

    // Configurare Releu Pompă
    gpio_init(RELEU_POMPA_PIN);
    gpio_set_dir(RELEU_POMPA_PIN, GPIO_OUT);
    opreste_pompa(); // Oprim pompa imediat la boot

    // Configurare HX711 pini
    gpio_init(MANCARE_SCK); gpio_set_dir(MANCARE_SCK, GPIO_OUT); gpio_put(MANCARE_SCK, 0); 
    gpio_init(MANCARE_DOUT); gpio_set_dir(MANCARE_DOUT, GPIO_IN);
     gpio_init(APA_SCK); gpio_set_dir(APA_SCK, GPIO_OUT); gpio_put(APA_SCK, 0); 
    gpio_init(APA_DOUT); gpio_set_dir(APA_DOUT, GPIO_IN);

    init_senzor_mancare();

    gpio_init(LED_MANCARE);
    gpio_set_dir(LED_MANCARE, GPIO_OUT);
    gpio_put(LED_MANCARE, false);

    gpio_init(LED_APA);
    gpio_set_dir(LED_APA, GPIO_OUT);

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
    gpio_put(LED_APA, true);
    while (true) {
        // Citiri senzori
        long val_mancare = read_hx711(MANCARE_SCK, MANCARE_DOUT);
        float grame_mancare = (val_mancare != -999999) ? (float)(val_mancare - tara_mancare) / FACTOR_MANCARE : 0.0f;
        if (grame_mancare < 0.0f) grame_mancare = 0.0f; 

        long val_apa = read_hx711(APA_SCK, APA_DOUT);
        float ml_apa = (val_apa != -999999) ? (float)(val_apa - tara_apa) / FACTOR_APA : 0.0f;
        if (ml_apa < 0.0f) ml_apa = 0.0f; 

        float distanta_recipient_cm = citeste_distanta_mancare_cm();
        if (distanta_recipient_cm >= 0.0f) {
            bool recipient_gol = distanta_recipient_cm > PRAG_RECIPIENT_MANCARE_CM;
            gpio_put(LED_MANCARE, recipient_gol);
            printf("Recipient mancare: %.1f cm | LED_MANCARE: %s\n",
                   distanta_recipient_cm,
                   recipient_gol ? "APRINS" : "STINS");
        } else {
            printf("Recipient mancare: citire invalida ultrasonic\n");
        }

        printf("Monitorizare -> Mâncare: %.1f g | Apă: %.1f ml\n", grame_mancare, ml_apa); 

        // EXECUȚIE CONTROL MÂNCARE
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
                if (to_ms_since_boot(get_absolute_time()) - servo_start > 8000) { // Siguranță 8 secunde
                    printf("[MANCARE] Timeout de siguranță clapetă!\n");
                    break;
                }
                sleep_ms(50); 
            }
            set_servo_position(SERVO_PIN, SERVO_INCHIS); 
            sleep_ms(2000); 
        }

        // EXECUȚIE CONTROL APĂ
        if (ml_apa < 20.0f) {
            printf("[APA] Nivel scăzut. Pornim pompa...\n");
            porneste_pompa(); 
            
            uint32_t pompa_start_time = to_ms_since_boot(get_absolute_time());
            int citiri_consecutive_plin = 0;
            
            while (true) {
                long val = read_hx711(APA_SCK, APA_DOUT);
                if (val != -999999) {
                    ml_apa = (float)(val - tara_apa) / FACTOR_APA;
                }

                if (ml_apa >= TINTA_APA) { 
                    citiri_consecutive_plin++;
                } else {
                    citiri_consecutive_plin = 0; 
                }

                if (citiri_consecutive_plin >= 3) {
                    printf("[APA] Greutate țintă atinsă în mod stabil.\n");
                    break; 
                }
                if (to_ms_since_boot(get_absolute_time()) - pompa_start_time > TIMEOUT_POMPA_MS) {
                    printf("[APA] TIMEOUT! Pompa s-a oprit pentru siguranță.\n");
                    break;
                }
                sleep_ms(100); 
            }
            opreste_pompa(); 
            sleep_ms(2000); 
        }

        sleep_ms(500); 
    }
}