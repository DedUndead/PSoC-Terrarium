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
* Note there are desing de  viations comparing to original description.
* All logic is properely described in documentation linked above.
* Refer to it for any clarifications in design.
*
*******************************************************************************/

/* Standard includes */
#include "project.h"
#include <stdio.h>
#include <time.h>

/* Custom includes */
#include "hatch.h"
#include "heater.h"
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

#define DEVICE_INFO_PROMPT "PSoC Terrarium V1. Developed by Pavel Arefyev.\r\n"

/* Types and structures */
// Provides easy read/write programming logic for saving
typedef struct msr_packed {
    uint32 timestamp;
    int16  air_temperature;
    int16  soil_moisture;
    float  soil_temperature;  
} packed_samples;

/* Global variables */
uint8 static volatile adc_conversion_ready = false;
uint8 static volatile ready_to_measure     = false;
uint8 static volatile ready_to_save        = false;
uint8 static volatile minute_passed        = false;

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

CY_ISR(isr_Timer_DeviceClock)
{
    Timer_DeviceClock_ReadStatusRegister(); // Acknowledge interrupt
    minute_passed = true;
}

/* Function declarations */
void   save_samples_to_eeprom(packed_samples samples);
uint16 print_samples_from_eeprom();
void   print_sample(packed_samples* sample);
void   init_eeprom_layout();
void   erase_samples_from_eeprom();
void   print_help();
uint8  set_date(uint day, uint month, uint year);
uint8  set_time(uint hour, uint minute);
struct tm get_time_from_eeprom();
void   print_current_time();
void   save_time_to_eeprom(uint32 timestamp);
uint32 get_time_from_eeprom_unix();

/* ==================== */
/*  MAIN FUNCTION BODY  */
/* ==================== */

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
    Timer_DeviceClock_Start();
    
    /* Enable interrupt sources */
    isr_ADC_StartEx(isr_ADC_conversion);
    isr_Timer_Measure_StartEx(isr_Timer_measure); 
    isr_Timer_Save_StartEx(isr_Timer_save);
    isr_Timer_DeviceClock_StartEx(isr_Timer_DeviceClock);
    
    /* Start abstract hardware components */
    inititialize_hatch();
    initialize_soil_moisture_sensor();
    //initialize_soil_temp_sensors();
    initialize_i2c();
    init_eeprom_layout();

    /* main Variable block */
    char transmit_buffer[DEF_BUFFER_LENGTH];
    char receive_buffer[DEF_BUFFER_LENGTH];
    uint8 rx_index = 0;  // Index that points to next write index in rx buffer
    uint8 input;         // User input
    
    int air_temperature = 0;
    int soil_moisture   = 0;
    packed_samples measurements;

    /* Initialize filters as empty */
    AverageFilter       adc_moist_filter    = { ADC_FILTER_LENGTH, 0, 0 };
    /* Moving average filters are used for data logging */
    MovingAverageFilter soil_moisute_filter = { {0}, 0, 0 };
    MovingAverageFilter air_temp_filter     = { {0}, 0, 0 };
    
    print_help();  // Print user help information 
    while (true) {
        /* HANDLE INTERRUPTS */    
        
        /* ADC conversion for soil moisture sensor ready */
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
            
            //sprintf(transmit_buffer, "%d%%, %d C\r\n", soil_moisture, air_temperature);
            //UART_PutString(transmit_buffer);
            
            /* Adjust actuators accoring to sample measurements */
            adjust_hatch(air_temperature);
            adjust_heater(air_temperature);
            
            ready_to_measure = false;
        }
        
        /* Ready to save, write the measurement to next EEPROM block */
        if (ready_to_save) {
            /* Prepare timestamp */
            measurements.timestamp = get_time_from_eeprom_unix();
            
            /* Filter collected samples using box average */
            measurements.air_temperature = get_MA_filtered_result(&air_temp_filter);
            measurements.soil_moisture = get_MA_filtered_result(&soil_moisute_filter);
            measurements.soil_temperature = 0;
            
            /* Save samples to EEPROM */
            save_samples_to_eeprom(measurements);
            
            ready_to_save = false;
        }
        
        /* Minute passed, adjust clock and save current time to EEPROM */
        if (minute_passed) {
            // Adjust timestamp by 60 seconds
            uint32 current_time = get_time_from_eeprom_unix();
            current_time += 60;
            save_time_to_eeprom(current_time);
            
            minute_passed = false;
        }
        
        /* HANDLE USER INPUT */
        
        /* Non-blocking call to get the menu option from the user */
        input = UART_GetChar();
        if (input != 0) {
            receive_buffer[rx_index++] = input;
            
            /* Echo input back to terminal */
            UART_PutChar(input);
            if (input == '\r') UART_PutChar('\n'); // CRLF
            if (input == '\n') UART_PutChar('\r'); // CRLF
        }
        /* Reset index if user overflows buffer */
        if (rx_index == DEF_BUFFER_LENGTH) {
            rx_index = 0;
        }
        /* Sent the command when CR/LF issued */
        if (input == '\r' || input == '\n') {
            receive_buffer[--rx_index] = '\0';  // Replace CR/LF with terminator
            rx_index = 0;
            
            /* Parse received command */
            uint day, month, year, hour, minute;
            if (strcmp(receive_buffer, "?") == 0) {
                UART_PutString(DEVICE_INFO_PROMPT);  
            }
            else if (strcmp(receive_buffer, "A") == 0) {
                print_samples_from_eeprom();
            }
            else if (strcmp(receive_buffer, "C") == 0) {
                erase_samples_from_eeprom();
            }
            else if (sscanf(receive_buffer, "D %u/%u/%u", &day, &month, &year) == 3) {
                uint8 success = set_date(day, month, year);
                success ? UART_PutString("New date set.\r\n") : UART_PutString("Invalid values.\r\n");
            }
            else if (sscanf(receive_buffer, "T %u:%u", &hour, &minute) == 2) {
                uint8 success = set_time(hour, minute);
                success ? UART_PutString("New time set.\r\n") : UART_PutString("Invalid values.\r\n");
            }
            else if (strcmp(receive_buffer, "D") == 0) {
                print_current_time();
            }
            
            print_help(); 
        }
    }
}

/* ==================== */
/* FUNCTION DEFINITIONS */
/* ==================== */

/*
 * @brief Set new date up to year 2050
 * @param day   New day
 * @param month New month
 * @param year  New year
 * @return      True on success
 */
uint8 set_date(uint day, uint month, uint year)
{
    // Series of sanity checks
    if (year < 1900 || year > 2050) return false;
    if (month > 12 || month < 1)    return false;
    if (day > 31 || day < 1)        return false;
    
    struct tm current_time = get_time_from_eeprom();
    current_time.tm_year = year - 1900;
    current_time.tm_mon = month - 1;
    current_time.tm_mday = day;
    
    uint32 timestamp = (uint32)mktime(&current_time);
    save_time_to_eeprom(timestamp);
    
    return true;
}

/*
 * @brief Set new time
 * @param hour   New hour
 * @param minute New minute
 * @return       True on success
 */
uint8 set_time(uint hour, uint minute)
{
    // Series of sanity checks
    if (minute > 59) return false;
    if (hour > 23)   return false;

    struct tm current_time = get_time_from_eeprom();
    current_time.tm_hour = hour;
    current_time.tm_min = minute;
    
    uint32 timestamp = (uint32)mktime(&current_time);
    save_time_to_eeprom(timestamp);
    
    return true;
}

/*
 * @brief  Print current time to eeprom
 * @return TM structure containing current device time information
 */
void print_current_time()
{
    struct tm current_time = get_time_from_eeprom();
    char transmit_buffer[DEF_BUFFER_LENGTH * 2];
    
    sprintf(
        transmit_buffer,
        "Current time: %02d/%02d/%d %02d:%02d\r\n",
        current_time.tm_mday, current_time.tm_mon + 1, current_time.tm_year + 1900,
        current_time.tm_hour, current_time.tm_min
    );
    
    UART_PutString(transmit_buffer);
}

/*
 * @brief  Get current device time information from eeprom
 * @return TM structure containing current device time information
 */
struct tm get_time_from_eeprom()
{
    /* Obtain current timestamp */
    time_t timestamp = (time_t)get_time_from_eeprom_unix();
    struct tm device_time;
    (void)localtime_r(&timestamp, &device_time);  // Breakdown unix timestamp
    
    return device_time;
}

/*
 * @brief Get current device time information from eeprom
 * @param Unix timestamp
 */
uint32 get_time_from_eeprom_unix()
{
    /* Obtain current timestamp */
    uint32 timestamp = ((EEPROM_ReadByte(EEPROM_INFO_ADDR_MSB)     << 24)  | 
                        (EEPROM_ReadByte(EEPROM_INFO_ADDR_MSB + 1) << 16) |
                        (EEPROM_ReadByte(EEPROM_INFO_ADDR_MSB + 2) << 8 ) |
                        (EEPROM_ReadByte(EEPROM_INFO_ADDR_MSB + 3))); 
    
    return timestamp;
}

/*
 * @brief Save time to EEPROM
 * @param Unix timestamp to be saved
 */
void save_time_to_eeprom(uint32 timestamp)
{
    for (int i = 3; i >= 0; i--) {
        EEPROM_WriteByte((timestamp >> (8 * i)), EEPROM_INFO_ADDR_MSB + (3 - i));   
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

/*
 * @brief Erase all the samples stored in EEPROM
 */
void erase_samples_from_eeprom()
{
    /* Erasing samples from eeprom just means resetting writing index */
    uint16 address = EEPROM_DATA_START_ADDR;
    EEPROM_WriteByte(address >> 8 , EEPROM_WRITE_ADDR_MSB);
    EEPROM_WriteByte(address      , EEPROM_WRITE_ADDR_LSB);
}

/*
 * @brief Save new sample to EERPOM
 * @param samples New samples to save
 */
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

/*
 * @brief Print all samples saved in EEPROM
 */
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

/*
 * @brief Print single sample in JSON format
 * @param sample Sample to print
 */
void print_sample(packed_samples* sample)
{
    char transmit_buffer[DEF_BUFFER_LENGTH * 3];
    
    struct tm dtime;
    time_t ts = (time_t)(sample->timestamp);
    (void)localtime_r(&ts, &dtime);  // Breakdown unix timestamp
    
    sprintf(
        transmit_buffer,
        "{\r\n"
        "\tDate:   %02d.%02d.%d %02d:%02d\r\n"
        "\tTair:   %d dC\r\n"
        "\tTsoil:  %.4f dC\r\n"
        "\tHsoild  %d %%\r\n"
        "}\r\n",
        dtime.tm_mday, dtime.tm_mon + 1, dtime.tm_year + 1900,
        dtime.tm_hour, dtime.tm_min,
        sample->air_temperature,
        sample->soil_temperature,
        sample->soil_moisture
    );
    
    UART_PutString(transmit_buffer);
}

/*
 * @brief Print small reference on how to communicate with device
 */
void print_help()
{
    UART_PutString(
        "\r\n"
        "?            - Device manufacturer information\n\r"
        "A            - Read all samples saved on the device\n\r"
        "C            - Clear device memory\n\r"
        "T hh:mm      - Set current hours and minutes\n\r"
        "D dd/mm/yyyy - Set current date\n\r"
        "D            - Print current device time\n\r"
        "\r\n"
    );   
}
