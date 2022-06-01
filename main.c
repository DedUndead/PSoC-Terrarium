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

/* Standard includes */
#include "project.h"
#include <stdio.h>

/* Custom includes */
#include "hatch.h"
#include "temperature_soil.h"
#include "moisture_sensor.h"
#include "i2c_driver.h"
#include "average_filter.h"
#include "moving_average_filter.h"

#define false             0
#define true              1
#define DEF_BUFFER_LENGTH 50       // Maximum length of transmit buffer
#define ADC_FILTER_LENGTH 100000

#define TC74_ADDRESS      0x4a     // I2C address of TC74 sensor
#define TC74_TEMP_REG     0x00     // Temperature register of the sensor

/* Refer to memory layout for more information */
#define EEPROM_WRITE_ADDR_MSB   0x00
#define EEPROM_WRITE_ADDR_LSB   0x01
#define EEPROM_INFO_ADDR_MSB    0x02
#define EEPROM_DATA_START_ADDR  0x06

/* Types and structures */
// Provides easy read/write programming logic for saving
typedef struct msr_packed {
    uint  timestamp;
    int16 air_temperature;
    int16 soil_moisture;
    float soil_temperature;  
} packed_samples;

/* Global variables */
uint8 static volatile adc_conversion_ready = false;
uint8 static volatile ready_to_measure     = false;
uint8 static volatile ready_to_save        = false;

/* Interrupt handlers */
CY_ISR(isr_ADC_conversion)
{
    // No need to acknowledge interrupt
    adc_conversion_ready = true;
}

CY_ISR(isr_Timer_measure)
{
    Timer_Measure_ReadStatusRegister(); // Acknowledge interrupt
    ready_to_measure = true;
}

CY_ISR(isr_Timer_save)
{
    Timer_Save_ReadStatusRegister(); // Acknowledge interrupt
    ready_to_save = true;
}

/* Function declarations */
void   save_samples_to_eeprom(packed_samples samples);
uint16 print_samples_from_eeprom();
void   print_sample(packed_samples* sample);
void   init_eeprom_layout();

int main()
{
    /* Enable global interrupts. */
    CyGlobalIntEnable; 
    
    /* Start hardware components */
    UART_Start();
    EEPROM_Start();
    Clock_1MHz_Start();
    Timer_Measure_Start();
    Timer_Save_Start();
    
    /* Enable interrupt sources */
    isr_ADC_StartEx(isr_ADC_conversion);
    isr_Timer_Measure_StartEx(isr_Timer_measure); 
    isr_Timer_Save_StartEx(isr_Timer_save);
    
    /* Start abstract hardware components */
    inititialize_hatch();
    initialize_soil_moisture_sensor();
    //initialize_soil_temp_sensors();
    initialize_i2c();
    init_eeprom_layout();

    /* main Variable block */
    char transmit_buffer[DEF_BUFFER_LENGTH];
    int air_temperature = 0;
    int soil_moisture   = 0;
    packed_samples measurements;
    
    /* Initialize filters as empty */
    AverageFilter       adc_moist_filter    = { ADC_FILTER_LENGTH, 0, 0 };
    /* Moving average filters are used for data logging */
    MovingAverageFilter soil_moisute_filter = { {0}, 0, 0 };
    MovingAverageFilter air_temp_filter     = { {0}, 0, 0 };
    
    while (true) {
        if (adc_conversion_ready) {
            int16 adc_sample = get_soil_moisture();
            add_sample_to_filter(&adc_moist_filter, adc_sample);
            
            adc_conversion_ready = false;
        }
        
        /* Ready to measure, update sensor values */
        if (ready_to_measure) {
            // Update soil moisture and save to moving average filter
            soil_moisture = get_filtered_result(&adc_moist_filter);
            add_sample_to_MA_filter(&soil_moisute_filter, soil_moisture);
            
            // Update air temperature and save to moving average filter
            air_temperature = read_i2c_data(TC74_ADDRESS, TC74_TEMP_REG);
            add_sample_to_MA_filter(&air_temp_filter, air_temperature);
            
            sprintf(transmit_buffer, "%d%%, %d C\r\n", soil_moisture, air_temperature);
            //UART_PutString(transmit_buffer);
            
            ready_to_measure = false;
        }
        
        /* Ready to save, write the measurement to next EEPROM block */
        if (ready_to_save) {
            /* Prepare timestamp */
            measurements.timestamp = 0;
            
            /* Filter collected samples using box average */
            measurements.air_temperature = get_MA_filtered_result(&air_temp_filter);
            measurements.soil_moisture = get_MA_filtered_result(&soil_moisute_filter);
            measurements.soil_temperature = 0;
            
            /* Save samples to EEPROM */
            save_samples_to_eeprom(measurements);
            print_samples_from_eeprom();
            
            ready_to_save = false;
        }

        CyDelay(100);
    }
}

void init_eeprom_layout()
{
    /* Obtain next writing address */
    uint16 address = (EEPROM_ReadByte(EEPROM_WRITE_ADDR_MSB) << 8) | EEPROM_ReadByte(EEPROM_WRITE_ADDR_LSB);
    
    /* Write address validity check */
    if (address < EEPROM_DATA_START_ADDR) {
        address = EEPROM_DATA_START_ADDR;
        EEPROM_WriteByte(address >> 8 , EEPROM_WRITE_ADDR_MSB);
        EEPROM_WriteByte(address      , EEPROM_WRITE_ADDR_LSB);
    }
}

void save_samples_to_eeprom(packed_samples samples)
{
    /* Represent structure as byte array */
    uint8 out_buffer[sizeof(packed_samples)];
    memcpy(out_buffer, &samples, sizeof(packed_samples));
    
    /* Obtain next writing address */
    uint16 address = (EEPROM_ReadByte(EEPROM_WRITE_ADDR_MSB) << 8) | EEPROM_ReadByte(EEPROM_WRITE_ADDR_LSB);
    /* If no space left, reset the address */
    if (address + sizeof(packed_samples) >= CYDEV_EE_SIZE) address = EEPROM_DATA_START_ADDR;
    
    /* Save previously obtained byte array */
    for (uint8 i = 0; i < (uint8)sizeof(packed_samples); i++) {
        EEPROM_WriteByte(out_buffer[i], address++);
    }
    
    /* Update next writing address */
    EEPROM_WriteByte(address >> 8 , EEPROM_WRITE_ADDR_MSB);
    EEPROM_WriteByte(address      , EEPROM_WRITE_ADDR_LSB);
}

uint16 print_samples_from_eeprom()
{
    /* Obtain next writing address */
    uint16 last_address = (EEPROM_ReadByte(EEPROM_WRITE_ADDR_MSB) << 8) | EEPROM_ReadByte(EEPROM_WRITE_ADDR_LSB);
    uint16 num_samples_read = 0;
    
    /* Print all the samples */
    uint16 i = EEPROM_DATA_START_ADDR;
    while (i < last_address) {
        uint8 in_buffer[sizeof(packed_samples)];
        
        /* Read single sample from EEPROM */
        for (uint8 j = 0; j < (uint8)sizeof(packed_samples); j++) {
            in_buffer[j] = EEPROM_ReadByte(i++);
        }
        
        /* Print newly read sample */
        packed_samples* sample = (packed_samples*)in_buffer;
        print_sample(sample);
        
        num_samples_read++;  // Increment number of read samples
    }
    
    return num_samples_read;
}

void print_sample(packed_samples* sample)
{
    char transmit_buffer[DEF_BUFFER_LENGTH * 3];
    sprintf(
        transmit_buffer,
        "{\r\n"
        "\tDate:   %d.%d.%d %d:%d\r\n"
        "\tTair:   %d dC\r\n"
        "\tTsoil:  %f dC\r\n"
        "\tHsoild  %d%%\r\n"
        "}\r\n",
        0, 0, 0, 0, 0,
        sample->air_temperature,
        sample->soil_temperature,
        sample->soil_moisture
    );
    
    UART_PutString(transmit_buffer);
}

/* [] END OF FILE */
