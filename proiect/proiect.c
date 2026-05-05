#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// --- CONFIGURARE PINI ---
#define LED_ROSU    16
#define LED_VERDE   17
#define BTN_1       14
#define BTN_2       15
#define ADC_TEMP_PIN 28 // MUTATI FIRUL DE LA 13 LA 26!

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
    // gpio_pull_up(BTN_1); // Folosim daca nu avem rezistente externe

    gpio_init(BTN_2);
    gpio_set_dir(BTN_2, GPIO_IN);
    // gpio_pull_up(BTN_2);

    // 3. Initializare ADC (Senzor Grog)
    adc_init();
    adc_gpio_init(ADC_TEMP_PIN);
    adc_select_input(2); // Corespunde GP28

    printf("🏴‍☠️ Sistemul Black Pearl este online!\n");
    printf("Monitorizare: Butoane (Manual) si Temperatura (ADC)\n");

    while (true) {
        // --- LOGICA BUTOANE ---
        // Citim starea (0 = apasat, 1 = liber datorita pull-up)
        if (gpio_get(BTN_1) == 0) {
            gpio_put(LED_ROSU, 1);
        } else {
            gpio_put(LED_ROSU, 0);
        }

        if (gpio_get(BTN_2) == 0) {
            gpio_put(LED_VERDE, 1);
        } else {
            gpio_put(LED_VERDE, 0);
        }

        // --- LOGICA TERMISTOR (ADC) ---
        uint16_t raw_val = adc_read();
        const float conversion_factor = 3.3f / (1 << 12);
        float voltage = raw_val * conversion_factor;

        // Afisare date in consola pentru Etapa 5
        printf("🌡️ Temperatura Grog: Raw=%u | Voltaj=%.2f V\n", raw_val, voltage);

        // Exemplu alerta: daca voltajul scade sub un prag (apa se incalzeste)
        if (voltage < 1.5f) {
            printf("⚠️ ALERTA: Apa este prea calda pentru echipaj!\n");
        }

        sleep_ms(200); // Rata de update a consolei
    }
}