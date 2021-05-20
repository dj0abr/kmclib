int init_adc();
uint16_t read_adc16(int channel);
float calcVoltage(uint16_t dv);
float cf_calc_temp(float Umess);
float calc_voltage(float f);
float calc_current(float f);

// I2C adress: MAX11615EEE: 0110011x
#define MAX11615 (0x66 / 2)

#define VREF    2.5         // ADCs VREF voltage
#define R_SHUNT 0.001	    // Ohms
