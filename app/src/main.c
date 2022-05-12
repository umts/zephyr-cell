#include <zephyr.h>
#include <stdlib.h>
#include <https_client.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/i2c.h>
#include <sys/printk.h>
#include <data/json.h>
//#include <parse_json.h>

// 1000 msec = 1 sec
#define SLEEP_TIME_MS 1000

#define MY_I2C "I2C_1"
const struct device *i2c_dev;

void main(void) {
  k_msleep(SLEEP_TIME_MS);
  printk("\n>\t %s\n\n", http_get_request());
  //parse_json();
  
	i2c_dev = device_get_binding(MY_I2C);
	if (i2c_dev == NULL) {
		printk("Can't bind I2C device %s\n", MY_I2C);
		return;
	}

	// Write to i2c
	uint8_t i2c_addr = 0x41;
  unsigned char i2c_tx_buffer[] = {'3', '0'};
  i2c_write(i2c_dev, &i2c_tx_buffer, sizeof(i2c_tx_buffer), i2c_addr);
}