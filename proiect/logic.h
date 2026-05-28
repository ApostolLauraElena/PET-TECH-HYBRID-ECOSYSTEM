#ifndef LOGIC_H
#define LOGIC_H

void calibrare_sistem(void);
void update_control_manual(void);
void update_senzori(void);
void update_dozator_mancare(void);
void update_dozator_apa(void);

float get_temperatura(void);
float get_grame_mancare(void);
float get_ml_apa(void);
#endif // LOGIC_H