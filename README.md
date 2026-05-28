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

## Cerințe Funcționale
1.	Sistemul trebuie să inițializeze componentele hardware: servo, pompă/releu, buton manual, senzori HX711, senzor de temperatură ADC și LED-uri.
2.	Sistemul trebuie să calibreze automat bolurile de mâncare și apă la pornire, timp de aproximativ 3 secunde.
3.	Sistemul trebuie să monitorizeze periodic cantitatea de mâncare, cantitatea de apă și temperatura apei.
4.	Sistemul trebuie să permită alimentarea manuală cu mâncare prin apăsarea unui buton, deschizând clapeta servo cât timp butonul este apăsat.
5.	Sistemul trebuie să dozeze automat mâncare când nivelul scade sub 15 g, deschizând clapeta până la pragul anticipat de 43 g sau până la timeout.

##	Cerințe Non-Funcționale
1.	Sistemul trebuie să folosească Pico SDK, BTStack, ADC și PWM pentru controlul componentelor.
2.	Sistemul trebuie să actualizeze monitorizarea periodic, la aproximativ 500 ms.
3.	Sistemul trebuie să evite valori negative pentru cantitățile măsurate, setându-le la 0.


## Build rapid

Proiectul este configurat pentru placa `pico2_w` si Pico SDK 2.2.0.

```powershell
cmake -S . -B build
cmake --build build
```

Dupa build, fisierul `.uf2` generat in directorul `build` se copiaza pe placa aflata in modul BOOTSEL.
