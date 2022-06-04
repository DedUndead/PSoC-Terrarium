/* ========================================
 *
 * @author  Pavel Arefyev
 * @name    I2C interface abstraction
 * @company Metropolia University of Applied Sciences
 * @date    14.05.2022
 *
 * Simple interface for interacting with I2C bus.
 * This interface only allows to read one byte from specified slave->register
 *
 * The communication template:
 * S | SLAVEADDR | W | REGADDR | ACK | RS | R | DATA | NAK | ST
 *
 * ========================================
*/

#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H
    
    
#include "project.h"
    
#define I2C_ERROR 0xc8 // An arbitary I2C error code

void  initialize_i2c();
int16 read_i2c_data(uint8 slave_address, uint8 register_address);
  

#endif

/* [] END OF FILE */
