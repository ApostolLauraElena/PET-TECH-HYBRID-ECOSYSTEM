#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h> 
#include "pico/stdlib.h"
#include "hardware/pwm.h" 
#include "hardware/adc.h" 

#define LED_MANCARE 0
#define MANCARE_SCK  16  
#define MANCARE_DOUT 17
#define BUTON_MANCARE_PIN 20

#define LED_APA 1
#define APA_SCK    14  
#define APA_DOUT   15  

#define SERVO_PIN 13       
#define RELEU_POMPA_PIN 19 

#define SENZOR_TEMP 26 
#define TEMP_LIMITA 25.0f // Pragul la care se va aprinde LED_APA

#define FACTOR_MANCARE 818.82f
#define FACTOR_APA     818.82f 

#define TINTA_MANCARE  50.0f
#define TINTA_APA      50.0f
#define ANTICIPARE_MANCARE 43.0f 

// VALORI PWM CONFIGURATE PENTRU CURSĂ MAXIMĂ (Ajustabile)
#define SERVO_INCHIS   1000  // Poziția de repaus / clapetă închisă (1ms)
#define SERVO_DESCHIS  2000  // Poziția de deschidere maximă (2ms)

long tara_mancare = 0; 
long tara_apa = 0; 

void set_servo_position(uint pin, uint pulse_width_us) {
    pwm_set_gpio_level(pin, pulse_width_us);
}

void porneste_pompa() {
    // Pico trimite 3.3V -> Tranzistorul se deschide -> Releul primeste GND -> Pompa PORNEȘTE
    gpio_put(RELEU_POMPA_PIN, 1); 
    printf("[APA] Semnal HIGH trimis la tranzistor. Pompa porneste.\n");
}

void opreste_pompa() {
    // Pico trimite 0V -> Tranzistorul se inchide -> Releul se deconecteaza -> Pompa SE OPREȘTE
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

    // Configurare Buton Manual Mancare
    gpio_init(BUTON_MANCARE_PIN);
    gpio_set_dir(BUTON_MANCARE_PIN, GPIO_IN);
    gpio_pull_up(BUTON_MANCARE_PIN); // Activăm rezistența internă (citește 1 când e liber)

    // Configurare Releu Pompă cu Tranzistor
    gpio_init(RELEU_POMPA_PIN);
    gpio_set_dir(RELEU_POMPA_PIN, GPIO_OUT);
    opreste_pompa(); // Oprim pompa imediat la boot (trimite 0V)

    // Configurare Senzor Temperatura (ADC)
    adc_init();
    adc_gpio_init(SENZOR_TEMP); // Inițializează pinul 26 pentru citire analogică

    // Configurare HX711 pini
    gpio_init(MANCARE_SCK); gpio_set_dir(MANCARE_SCK, GPIO_OUT); gpio_put(MANCARE_SCK, 0); 
    gpio_init(MANCARE_DOUT); gpio_set_dir(MANCARE_DOUT, GPIO_IN);
    gpio_init(APA_SCK); gpio_set_dir(APA_SCK, GPIO_OUT); gpio_put(APA_SCK, 0); 
    gpio_init(APA_DOUT); gpio_set_dir(APA_DOUT, GPIO_IN);

    gpio_init(LED_MANCARE);
    gpio_set_dir(LED_MANCARE, GPIO_OUT);

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
    gpio_put(LED_APA, false);

    while (true) {
        // --- 0. CONTROL MANUAL MÂNCARE (BUTON) ---
        if (gpio_get(BUTON_MANCARE_PIN) == 0) { // Dacă citește 0, înseamnă că butonul e apăsat (legat la GND)
            printf("[MANCARE] Buton apăsat! Deschidem clapeta manual...\n");
            set_servo_position(SERVO_PIN, SERVO_DESCHIS); 
            
            // Ținem sistemul pe pauză (clapeta deschisă) CÂT TIMP butonul rămâne apăsat
            while(gpio_get(BUTON_MANCARE_PIN) == 0) {
                sleep_ms(50); // Așteptăm scurt, ca să nu blocăm procesorul
            }
            
            // Imediat ce degetul a fost ridicat, ieșim din while-ul mic și închidem clapeta
            printf("[MANCARE] Buton eliberat. Închidem clapeta.\n");
            set_servo_position(SERVO_PIN, SERVO_INCHIS);
            
            sleep_ms(1000); // Pauză 1 secundă ca să se așeze mâncarea în bol înainte de a reciti cântarul
        }
        // --- 1. CITIRE TEMPERATURĂ (Termistor NTC) ---
        adc_select_input(0); // Pinul 26
        uint16_t raw_adc = adc_read();
        
        // Protecție: Evităm erorile de calcul (împărțirea la zero)
        if (raw_adc == 0) raw_adc = 1;
        if (raw_adc == 4095) raw_adc = 4094;

        // Convertim citirea brută în tensiune
        float tensiune = raw_adc * (3.3f / 4095.0f);
        
        // Calculăm rezistența senzorului în acel moment (presupunând rezistență de 10k pe modul)
        // ATENȚIE: Dacă observi că temperatura afișată SCADE când ții senzorul în mână, 
        // comentează linia de mai jos și decomenteaz-o pe următoarea:
        float rezistenta_ntc = 10000.0f * ((3.3f / tensiune) - 1.0f); 
        //float rezistenta_ntc = 10000.0f * (tensiune / (3.3f - tensiune));

        // Ecuația Steinhart-Hart pentru conversia în grade Celsius
        float temperatura = rezistenta_ntc / 10000.0f;     // R/Ro 
        temperatura = log(temperatura);                    // ln(R/Ro)
        temperatura /= 3950.0f;                            // 1/Beta (Valoare Beta comună: 3950)
        temperatura += 1.0f / (25.0f + 273.15f);           // + (1/To)
        temperatura = 1.0f / temperatura;                  // Inversăm
        temperatura -= 273.15f;                            // Convertim din Kelvin în Celsius

        // Alerta pe LED dacă apa e prea caldă
        if (temperatura > TEMP_LIMITA) {
            gpio_put(LED_APA, true); 
        } else {
            gpio_put(LED_APA, false);
        }

        // --- 2. CITIRI CÂNTAR ---
        long val_mancare = read_hx711(MANCARE_SCK, MANCARE_DOUT);
        float grame_mancare = (val_mancare != -999999) ? (float)(val_mancare - tara_mancare) / FACTOR_MANCARE : 0.0f;
        if (grame_mancare < 0.0f) grame_mancare = 0.0f; 

        long val_apa = read_hx711(APA_SCK, APA_DOUT);
        float ml_apa = (val_apa != -999999) ? (float)(val_apa - tara_apa) / FACTOR_APA : 0.0f;
        if (ml_apa < 0.0f) ml_apa = 0.0f; 

        printf("Monitorizare -> Mâncare: %.1f g | Apă: %.1f ml | Temp apă: %.1f C\n", grame_mancare, ml_apa, temperatura); 

        // --- 3. EXECUȚIE CONTROL MÂNCARE ---
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

        // --- 4. EXECUȚIE CONTROL APĂ (Runde scurte / Spells) ---
        if (ml_apa < 20.0f) {
            printf("[APA] Nivel scăzut. Pompăm o rafală scurtă...\n");
            porneste_pompa(); 
            sleep_ms(1000); // Ține pompa pornită 1 secundă
            
            opreste_pompa();
            printf("[APA] Așteptăm stabilizarea apei în bol...\n");
            sleep_ms(2000); // Pauză 2 secunde ca senzorul să detecteze noua greutate
        } 

        sleep_ms(500); 
    }
}