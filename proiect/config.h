#ifndef CONFIG_H
#define CONFIG_H

// --- PINI ---
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

// --- CONSTANTE CALIBRARE ---
#define TEMP_LIMITA 25.0f // Pragul la care se va aprinde LED_APA
#define FACTOR_MANCARE 818.82f
#define FACTOR_APA     818.82f 
#define TINTA_MANCARE  50.0f
#define TINTA_APA      50.0f
#define ANTICIPARE_MANCARE 43.0f 

// --- VALORI SERVO ---
#define SERVO_INCHIS   1000  // Poziția de repaus / clapetă închisă (1ms)
#define SERVO_DESCHIS  2000  // Poziția de deschidere maximă (2ms)

#endif // CONFIG_H