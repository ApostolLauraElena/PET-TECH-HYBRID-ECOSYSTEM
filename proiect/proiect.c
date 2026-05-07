#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// --- CONFIGURARE PINI ---
#define LED_ROSU    16
#define LED_VERDE   17
#define BTN_1       14
#define BTN_2       15
#define ADC_TEMP_PIN 28 
#define TRIG_PIN    18
#define ECHO_PIN    19

#define BETA 3950
#define R0 10000
#define T0 298.15

float citeste_temperatura(uint16_t raw_value) {
    float rezistenta = 10000.0f * (4095.0f / (float)raw_value - 1.0f);
    float temperatura_k;
    temperatura_k = rezistenta / 10000.0f;
    temperatura_k = log(temperatura_k);
    temperatura_k /= 3950.0f;
    temperatura_k += 1.0f / (25.0f + 273.15f);
    temperatura_k = 1.0f / temperatura_k;
    return temperatura_k - 273.15f;
}

int main() {
    stdio_init_all();

    gpio_init(LED_ROSU);
    gpio_set_dir(LED_ROSU, GPIO_OUT);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);

    gpio_init(BTN_1);
    gpio_set_dir(BTN_1, GPIO_IN);
    gpio_init(BTN_2);
    gpio_set_dir(BTN_2, GPIO_IN);

    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    adc_init();
    adc_gpio_init(ADC_TEMP_PIN);
    adc_select_input(2); 

    while (true) {
        uint16_t raw = adc_read();
        float resistance = R0 * (4095.0f / (float)raw - 1.0f);
        float steinhart;
        steinhart = resistance / R0;
        steinhart = log(steinhart);
        steinhart /= BETA;
        steinhart += 1.0f / T0;
        steinhart = 1.0f / steinhart;
        float celsius = steinhart - 273.15f;

        gpio_put(TRIG_PIN, 1);
        sleep_us(10);
        gpio_put(TRIG_PIN, 0);

        while (gpio_get(ECHO_PIN) == 0); // Asteptam ecoul
        absolute_time_t start = get_absolute_time();
        while (gpio_get(ECHO_PIN) == 1); // Masuram durata
        absolute_time_t end = get_absolute_time();

        uint64_t diff = absolute_time_diff_us(start, end);
        float distanta = (float)diff * 0.0343f / 2.0f;

        printf("\rTEMP: %.2f C | NIVEL: %.2f cm    ", celsius, distanta);
        
        if (celsius > 30.0f || distanta > 15.0f) {
            if (celsius > 30.0f) printf(" 🔥 CALD!");
            if (distanta > 15.0f) printf(" 💧 GOL!");
            
            gpio_put(LED_ROSU, 1); 
            gpio_put(LED_VERDE, 0);
        }
        else {
            gpio_put(LED_ROSU, 0);
            gpio_put(LED_VERDE, 1); 
        }

        sleep_ms(200); 
    }
}