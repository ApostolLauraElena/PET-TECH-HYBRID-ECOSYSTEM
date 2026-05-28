#ifndef APP_BLUETOOTH_H
#define APP_BLUETOOTH_H

// Pornește cipul intern și configurează numele Bluetooth
void bluetooth_init(void);

// Pune datele în coada de transmisie către telefon
void bluetooth_trimite_status(float temperatura, float grame_mancare, float ml_apa);

#endif // APP_BLUETOOTH_H
