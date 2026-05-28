#include "hardware.h"
#include "config.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>
void hardware_init(void) {
    stdio_init_all();

    // Configurare Servo
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 150.0f); 
    pwm_config_set_wrap(&config, 20000);   
    pwm_init(slice_num, &config, true);
    set_servo_position(SERVO_PIN, SERVO_INCHIS); 

    // Configurare Buton Manual Mancare
    gpio_init(BUTON_MANCARE_PIN);
    gpio_set_dir(BUTON_MANCARE_PIN, GPIO_IN);
    gpio_pull_up(BUTON_MANCARE_PIN); 

    // Configurare Releu Pompă
    gpio_init(RELEU_POMPA_PIN);
    gpio_set_dir(RELEU_POMPA_PIN, GPIO_OUT);
    opreste_pompa(); 

    // Configurare Senzor Temperatura (ADC)
    adc_init();
    adc_gpio_init(SENZOR_TEMP); 

    // Configurare pini HX711 și LED-uri
    gpio_init(MANCARE_SCK); gpio_set_dir(MANCARE_SCK, GPIO_OUT); gpio_put(MANCARE_SCK, 0); 
    gpio_init(MANCARE_DOUT); gpio_set_dir(MANCARE_DOUT, GPIO_IN);
    
    gpio_init(APA_SCK); gpio_set_dir(APA_SCK, GPIO_OUT); gpio_put(APA_SCK, 0); 
    gpio_init(APA_DOUT); gpio_set_dir(APA_DOUT, GPIO_IN);

    gpio_init(LED_MANCARE); gpio_set_dir(LED_MANCARE, GPIO_OUT);
    gpio_init(LED_APA); gpio_set_dir(LED_APA, GPIO_OUT);
}

void set_servo_position(uint pin, uint pulse_width_us) {
    pwm_set_gpio_level(pin, pulse_width_us);
}

void porneste_pompa(void) {
    gpio_put(RELEU_POMPA_PIN, 1); 
    printf("[APA] Semnal HIGH trimis la tranzistor. Pompa porneste.\n");
}

void opreste_pompa(void) {
    gpio_put(RELEU_POMPA_PIN, 0); 
    printf("[APA] Semnal LOW trimis la tranzistor. Pompa se opreste.\n");
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
        gpio_put(pin_sck, 1); sleep_us(1);
        value = (value << 1) | gpio_get(pin_dout);
        gpio_put(pin_sck, 0); sleep_us(1);
    }
    gpio_put(pin_sck, 1); sleep_us(1);
    gpio_put(pin_sck, 0); sleep_us(1);

    if (value & 0x800000) value |= 0xFF000000;
    return value;
}

float citeste_temperatura(void) {
    adc_select_input(0); 
    uint16_t raw_adc = adc_read();
    
    if (raw_adc == 0) raw_adc = 1;
    if (raw_adc == 4095) raw_adc = 4094;

    float tensiune = raw_adc * (3.3f / 4095.0f);
    float rezistenta_ntc = 10000.0f * ((3.3f / tensiune) - 1.0f); 

    float temperatura = rezistenta_ntc / 10000.0f;     
    temperatura = log(temperatura);                    
    temperatura /= 3950.0f;                            
    temperatura += 1.0f / (25.0f + 273.15f);           
    temperatura = 1.0f / temperatura;                  
    temperatura -= 273.15f;                            
    
    return temperatura;
}