#include "app_bluetooth.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include <stdio.h>
#include <string.h>

#define RFCOMM_SERVER_CHANNEL 1

static uint16_t current_rfcomm_cid = 0;
static uint8_t  spp_service_buffer[150];
static int client_connected = 0;

// Buffere pentru transmiterea asincronă (cerută de BTstack)
static char date_de_trimis[128];
static bool date_noi = false;
static uint16_t len_de_trimis = 0;

// Handler-ul care ascultă evenimentele Bluetooth (conectare, deconectare, permisiune de trimitere)
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    if (packet_type != HCI_EVENT_PACKET) return;
    uint8_t event_type = hci_event_packet_get_type(packet);
    
    switch (event_type) {
        case RFCOMM_EVENT_INCOMING_CONNECTION:
            // Telefonul încearcă să se conecteze
            current_rfcomm_cid = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
            rfcomm_accept_connection(current_rfcomm_cid);
            break;
            
        case RFCOMM_EVENT_CHANNEL_OPENED:
            if (rfcomm_event_channel_opened_get_status(packet)) {
                printf("[BLUETOOTH] Eroare la deschiderea canalului!\n");
                break;
            }
            current_rfcomm_cid = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
            client_connected = 1;
            printf("[BLUETOOTH] Telefon conectat!\n");
            break;
            
        case RFCOMM_EVENT_CHANNEL_CLOSED:
            client_connected = 0;
            current_rfcomm_cid = 0;
            printf("[BLUETOOTH] Telefon deconectat!\n");
            break;
            
        case RFCOMM_EVENT_CAN_SEND_NOW:
            // BTstack ne anunță că radioul e liber și putem trimite datele
            if (date_noi && client_connected) {
                rfcomm_send(current_rfcomm_cid, (uint8_t*)date_de_trimis, len_de_trimis);
                date_noi = false;
            }
            break;
            
        default:
            break;
    }
}

void bluetooth_init(void) {
    // 1. Pornim cipul wireless de pe Pico 2 W
    if (cyw43_arch_init()) {
        printf("[EROARE] Cipul wireless intern nu a putut fi pornit!\n");
        return;
    }
    
    // 2. Inițializăm stiva Bluetooth (L2CAP și RFCOMM)
    l2cap_init();
    rfcomm_init();
    rfcomm_register_service(packet_handler, RFCOMM_SERVER_CHANNEL, 100);
    
    // 3. Înregistrăm serviciul pentru a fi recunoscut ca terminal serial
    sdp_init();
    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    spp_create_sdp_record(spp_service_buffer, 0x10001, RFCOMM_SERVER_CHANNEL, "Dozator Smart");
    sdp_register_service(spp_service_buffer);
    
    // 4. Setăm numele care va apărea pe telefon
    gap_set_local_name("Pico2W_Dozator");
    gap_discoverable_control(1); // Îl facem vizibil
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT); // Fără PIN complex
    
    // 5. Pornim antena
    hci_power_control(HCI_POWER_ON);
    printf("[BLUETOOTH] Antenă integrată inițializată. Caută numele: Pico2W_Dozator\n");
}

void bluetooth_trimite_status(float temperatura, float grame_mancare, float ml_apa) {
    if (!client_connected) return; // Dacă telefonul nu e conectat, ignorăm pentru a nu consuma resurse
    
    // Pregătim string-ul
    len_de_trimis = snprintf(date_de_trimis, sizeof(date_de_trimis), 
                             "Temp: %.1f C | Mancare: %.1f g | Apa: %.1f ml\r\n", 
                             temperatura, grame_mancare, ml_apa);
    date_noi = true;
    
    // Îi spunem antenei: "Când ai timp, am niște date de trimis"
    rfcomm_request_can_send_now_event(current_rfcomm_cid);
}
