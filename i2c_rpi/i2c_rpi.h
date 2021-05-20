
int i2c_init();
int i2c_seldev(int addr);
int i2c_write(int addr, uint8_t *pdata, int len);
int i2c_read(int addr, uint8_t *pdata, int len);
int i2c_write_register(uint8_t slave_addr, uint8_t reg, uint8_t data);
int i2c_read_register(uint8_t addr, uint8_t reg);

void setPort(int port, int onoff);
int getOutPort(int port);
int getPort(int port);
float getADC(int channel);
float getADCvoltage(int channel);

