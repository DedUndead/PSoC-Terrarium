/******************************************************************************
* 
* @author  Pavel Arefyev
* @name    TX00DB04 - Terrarium Automation project
* @company Metropolia University of Applied Sciences
* @date    14.05.2022
*
* This project implements simple Terrarium Automation as PSoC based device.
* More information is available at: https://github.com/DedUndead/PSoC-Terrarium
*
* Project idea is an intellectual property of Antti Piironen, Principal Lecturer in Smart Systems Engineering.
* Metropolia University of Applied Sciences, Finland, Uusimaa
*
*******************************************************************************/

#include "project.h"
#include <stdio.h>

#define false             0
#define true              1
#define I2C_ERROR         0xc8     // An arbitary I2C error code
#define MAX_BUFFER_LENGTH 50       // Maximum length of transmit buffer
#define TC74_ADDRESS      0x4a     // I2C address of TC74 sensor
#define TC74_TEMP_REG     0x00     // Temperature register of the sensor

/* Function declarations */
static int16 read_i2c_data(uint8 slave_address, uint8 register_address);

int main()
{
    /* Enable global interrupts. */
    CyGlobalIntEnable; 
    
    /* Start hardware components */
    I2C_Start();
    UART_Start();
    Clock_1MHz_Start(); 
    PWM_Start();

    /* main Variable block */
    char transmit_buffer[MAX_BUFFER_LENGTH];
    int16 sample = 0;
    
    while (true) {
        sample = read_i2c_data(TC74_ADDRESS, TC74_TEMP_REG);
        sprintf(transmit_buffer, "%d\r\n", sample);
        UART_PutString(transmit_buffer);
        
        PWM_WriteCompare(1000);
        CyDelay(1000);
        PWM_WriteCompare(2000);
        CyDelay(1000);
    }
}

/*
 * @brief Read i2c data from the slave
 * @param slave_address    Address of target device on the bus
 * @param register_address Address to read from
 * @return                 Integer actual reading
 */
static int16 read_i2c_data(uint8 slave_address, uint8 register_address)
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
