#include "pico/stdlib.h"
#include "hardware.h"
#include "logic.h"
#include "app_bluetooth.h"

int main() {
    hardware_init();
    bluetooth_init(); 
    
    calibrare_sistem();

    while (true) {
        update_control_manual();
        update_senzori();
        update_dozator_mancare();
        update_dozator_apa();

        bluetooth_trimite_status(get_temperatura(), get_grame_mancare(), get_ml_apa());

        sleep_ms(500); 
    }
    return 0;
}
