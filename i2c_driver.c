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

#include "i2c_driver.h"

void initialize_i2c()
{
    I2C_Start();   
}

/*
 * @brief Read i2c data from the slave
 * @param slave_address    Address of target device on the bus
 * @param register_address Address to read from
 * @return                 Integer actual reading
 */
int16 read_i2c_data(uint8 slave_address, uint8 register_address)
{
    uint8 status;
    
    // Send start bit and write bit
    status = I2C_MasterSendStart(slave_address, I2C_WRITE_XFER_MODE);
    // Write register address to the bus
    status = I2C_MasterWriteByte(register_address);
    // Send restart condition with read bit
    status = I2C_MasterSendRestart(slave_address, I2C_READ_XFER_MODE);
    // Check if any errors occured during the operation
    if (status != I2C_MSTR_NO_ERROR) return I2C_ERROR;
    
    // Read one byte and send NAK
    uint8 reading = I2C_MasterReadByte(I2C_NAK_DATA);
    I2C_MasterSendStop();  // Send stop bit
    
    /* Two's complenment -> integer cast */
    if (reading & 0x80) reading = -~(reading - 1);
    
    return reading;   
}

/* [] END OF FILE */
