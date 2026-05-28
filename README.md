# Sistem de Hranire si Hidratare

Proiect embedded pentru Raspberry Pi Pico 2 W, realizat in C cu Pico SDK. Sistemul monitorizeaza nivelul de mancare, nivelul de apa si temperatura apei, apoi actioneaza automat un servomotor pentru dozarea mancarii si o pompa comandata prin releu pentru completarea apei.

Dispozitivul transmite periodic starea prin Bluetooth Classic SPP, sub numele `Pico2W_Dozator`, astfel incat valorile curente sa poata fi citite de pe telefon sau dintr-o aplicatie terminal Bluetooth.

## Functii principale

- calibrare automata a senzorilor de greutate la pornire;
- citirea a doua module HX711: unul pentru mancare si unul pentru apa;
- citirea temperaturii apei prin ADC, folosind un senzor NTC;
- dozare automata a mancarii cu servomotor;
- completare automata a apei cu pompa;
- control manual pentru clapeta de mancare;
- avertizare LED cand temperatura apei depaseste pragul configurat;
- transmitere status prin Bluetooth RFCOMM/SPP.

## Structura proiectului

| Fisier | Rol |
| --- | --- |
| `proiect.c` | Punctul de intrare al aplicatiei si bucla principala. |
| `config.h` | Pini, praguri, factori de calibrare si valori pentru servo. |
| `hardware.c` / `hardware.h` | Initializare pini, PWM, ADC, pompa, citire HX711 si temperatura. |
| `logic.c` / `logic.h` | Logica de calibrare, monitorizare si dozare automata. |
| `bluetooth.c` / `app_bluetooth.h` | Initializare Bluetooth Classic si transmitere status prin SPP. |
| `btstack_config.h` | Configuratia BTstack pentru Bluetooth Classic. |
| `lwipopts.h` | Configuratie lwIP necesara pentru Pico W/Pico 2 W. |
| `CMakeLists.txt` | Configuratia de build pentru Pico SDK. |

## Documentatie completa

Documentatia tehnica detaliata este in [DOCUMENTATIE.md](DOCUMENTATIE.md).

## Build rapid

Proiectul este configurat pentru placa `pico2_w` si Pico SDK 2.2.0.

```powershell
cmake -S . -B build
cmake --build build
```

Dupa build, fisierul `.uf2` generat in directorul `build` se copiaza pe placa aflata in modul BOOTSEL.
