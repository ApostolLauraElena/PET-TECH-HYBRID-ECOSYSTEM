#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED_ROSU    16
#define LED_VERDE   17
#define BTN_1       14
#define BTN_2       15
#define ADC_TEMP_PIN 28 
#define BETA 3950
#define R0 10000
#define T0 298.15

float citeste_temperatura(uint16_t raw_value) {
    // 1. Calculam rezistenta termistorului
    // R_term = R_fixa * (4095 / raw - 1)
    float rezistenta = 10000.0f * (4095.0f / (float)raw_value - 1.0f);

    // 2. Aplicam ecuatia Beta
    float temperatura_k;
    temperatura_k = rezistenta / 10000.0f;     // R / R0
    temperatura_k = log(temperatura_k);         // ln(R / R0)
    temperatura_k /= 3950.0f;                   // 1/Beta * ln(R / R0)
    temperatura_k += 1.0f / (25.0f + 273.15f);  // + 1/T0
    temperatura_k = 1.0f / temperatura_k;       // Inversam pentru a afla T

    // 3. Convertim din Kelvin in Celsius
    return temperatura_k - 273.15f;
}

int main() {
    stdio_init_all();

    // 1. Initializare LED-uri
    gpio_init(LED_ROSU);
    gpio_set_dir(LED_ROSU, GPIO_OUT);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);

    // 2. Initializare Butoane
    gpio_init(BTN_1);
    gpio_set_dir(BTN_1, GPIO_IN);

    gpio_init(BTN_2);
    gpio_set_dir(BTN_2, GPIO_IN);

    // 3. Initializare ADC (Senzor Grog)
    adc_init();
    adc_gpio_init(ADC_TEMP_PIN);
    adc_select_input(2); // Corespunde GP28

    printf("Monitorizare: Butoane (Manual) si Temperatura (ADC)\n");

    while (true) {
        uint16_t raw = adc_read();
        
        // 1. Calculăm rezistența termistorului
        float resistance = R0 * (4095.0f / (float)raw - 1.0f);

        // 2. Calculăm temperatura folosind Ecuația Beta
        // Formula: 1/T = 1/T0 + 1/B * ln(R/R0)
        float steinhart;
        steinhart = resistance / R0;           // (R/R0)
        steinhart = log(steinhart);            // ln(R/R0)
        steinhart /= BETA;                     // 1/B * ln(R/R0)
        steinhart += 1.0f / T0;                // + (1/T0)
        steinhart = 1.0f / steinhart;          // Invert
        float celsius = steinhart - 273.15f;    // Convert în Celsius

        printf("TEMP: %.2f °C    /n", celsius);
        
        // Dacă e prea cald, tragem un semnal de alarmă în consolă
        if (celsius > 30.0f) {
            printf(" 🔥 APA PREA CALDA!");
            gpio_put(LED_ROSU, 1);
            gpio_put(LED_VERDE, 0);
        }
        else
        {
            gpio_put(LED_ROSU, 0);
            gpio_put(LED_VERDE, 1);
        }

        sleep_ms(200); 
    }
}