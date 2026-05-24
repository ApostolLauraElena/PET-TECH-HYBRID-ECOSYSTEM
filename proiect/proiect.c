#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h" 

#define MANCARE_SCK  16  
#define MANCARE_DOUT 17  

#define APA_SCK  14  
#define APA_DOUT 15  

#define SERVO_PIN 13       
#define RELEU_POMPA_PIN 18 

#define FACTOR_MANCARE 818.82f
#define FACTOR_APA     818.82f 

long tara_mancare = 0; 
long tara_apa = 0; 

void set_servo_position(uint pin, uint pulse_width_us) {
    pwm_set_gpio_level(pin, pulse_width_us);
}

void porneste_pompa() {
    gpio_set_dir(RELEU_POMPA_PIN, GPIO_OUT);
    gpio_put(RELEU_POMPA_PIN, 0); 
}

void opreste_pompa() {
    gpio_set_dir(RELEU_POMPA_PIN, GPIO_IN); 
    gpio_disable_pulls(RELEU_POMPA_PIN);
}

long read_hx711(uint pin_sck, uint pin_dout) {
    while (gpio_get(pin_dout)); 

    long value = 0;
    for (int i = 0; i < 24; i++) {
        gpio_put(pin_sck, 1);
        sleep_us(1);
        value = (value << 1) | gpio_get(pin_dout);
        gpio_put(pin_sck, 0);
        sleep_us(1);
    }

    gpio_put(pin_sck, 1);
    sleep_us(1);
    gpio_put(pin_sck, 0);
    sleep_us(1);

    if (value & 0x800000) value |= 0xFF000000;
    
    return value;
}

int main() {
    stdio_init_all();

    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 150.0f); 
    pwm_config_set_wrap(&config, 20000);   
    pwm_init(slice_num, &config, true);
    set_servo_position(SERVO_PIN, 1500); 

    gpio_init(RELEU_POMPA_PIN);
    opreste_pompa(); 

    gpio_init(MANCARE_SCK); gpio_set_dir(MANCARE_SCK, GPIO_OUT); gpio_put(MANCARE_SCK, 0); 
    gpio_init(MANCARE_DOUT); gpio_set_dir(MANCARE_DOUT, GPIO_IN);

    gpio_init(APA_SCK); gpio_set_dir(APA_SCK, GPIO_OUT); gpio_put(APA_SCK, 0); 
    gpio_init(APA_DOUT); gpio_set_dir(APA_DOUT, GPIO_IN);

    sleep_ms(3000); 
    printf("\nSe face Auto-Tara pentru AMBELE boluri. NU le atingeți!\n");

    long suma_mancare = 0;
    long suma_apa = 0;
    for(int i = 0; i < 10; i++) {
        suma_mancare += read_hx711(MANCARE_SCK, MANCARE_DOUT);
        suma_apa += read_hx711(APA_SCK, APA_DOUT);
        sleep_ms(50);
    }
    tara_mancare = suma_mancare / 10;
    tara_apa = suma_apa / 10;
    
    while (true) {
        long val_mancare = read_hx711(MANCARE_SCK, MANCARE_DOUT);
        float grame_mancare = (float)labs(val_mancare - tara_mancare) / FACTOR_MANCARE;
        if (grame_mancare < 1.5f) grame_mancare = 0.0f; 

        long val_apa = read_hx711(APA_SCK, APA_DOUT);
        float ml_apa = (float)labs(val_apa - tara_apa) / FACTOR_APA;
        if (ml_apa < 1.5f) ml_apa = 0.0f; 

        printf("Mâncare: %.1f g | Apă: %.1f ml\n", grame_mancare, ml_apa); 

        if (grame_mancare < 15.0f) {
            printf("Se deschide recipientul...\n");
            set_servo_position(SERVO_PIN, 1800); 
            
            while (true) {
                long val = read_hx711(MANCARE_SCK, MANCARE_DOUT);
                grame_mancare = (float)labs(val - tara_mancare) / FACTOR_MANCARE;
                if (grame_mancare >= 50.0f) {
                    printf("Ținta de 50g mâncare atinsă!\n");
                    break; 
                }
                sleep_ms(100); 
            }
            printf("Se închide recipientul de mâncare.\n");
            set_servo_position(SERVO_PIN, 1500); 
            sleep_ms(2000); 
        }

        if (ml_apa < 20.0f) {
            printf("Pornim pompa...\n");
            porneste_pompa(); 
            
            while (true) {
                long val = read_hx711(APA_SCK, APA_DOUT);
                ml_apa = (float)labs(val - tara_apa) / FACTOR_APA;
                if (ml_apa < 1.5f) ml_apa = 0.0f;

                printf("Apa: %.1f ml\n", ml_apa);

                if (ml_apa >= 50.0f) { 
                    printf("Ținta de 50ml apă atinsă!\n");
                    break; 
                }
                sleep_ms(100); 
            }
            
            printf("Se oprește pompa de apă.\n");
            opreste_pompa(); 
            sleep_ms(2000); 
        }

        sleep_ms(500); 
    }
}