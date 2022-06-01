/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
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
