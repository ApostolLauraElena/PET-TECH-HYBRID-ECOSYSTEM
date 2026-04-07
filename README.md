# Sistem de Hranire si Hidratare

Membrii echipei:
 - Apostol Laura Elena
 - Gheorghiu Adelina Ioana

## Descriere proiect

Ne dorim sa implementam un sistem hibrid (Auto-Feeder & Auto-Waterer) conceput pentru a usura procesul de hranire si hidratare al animalelor de companie, oferind o interfata de monitorizare prin Bluetooth.

## Obiective:
 + Dezvoltarea unui sistem de hrana autonom
 + Integrarea coerenta Hard si Soft folosind Pico C SDK
 + Replicarea unui sistem de dozare industriala


## Componente Hardware

 + Raspberry Pi Pico 2W
 + Modul HX711
 + Celula sarcina (1kg)
 + Servomotor SG90
 + Mini pompa de apa
 + Modul releu cu 1 canal
 + Senzor HC-SR04
 + Termistor NTC 10k
 + Buzzer Pasiv
 + Tranzistor 2N2222
 + Dioda 1N4007
 + Rezistoare
 + Butoane Tactile
 + LED-uri colorate

### Cerinte
+ Dozare programata: eliberarea hranei la ore fixe.
+ Monitorizare resurse: cantarirea in timp real a bolurilor folosind senzori de greutate si module HX711.
+ Calitatea apei: masurarea temperaturii apei si alertarea utilizatorului daca apa este prea calda.
+ Alerte sononore: buzzer-ul anunta finalizarea dozarii sau erori critice(rezervor gol).


