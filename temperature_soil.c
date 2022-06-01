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

#include "temperature_soil.h"

static slaves_info devices_on_bus = { .rom_codes = { 0 } };

/*
 * @brief Identify sensors present on the bus
 * NUMBER_OF_SENSORS configures number of sensors on the device
 */
void initialize_soil_temp_sensors()
{
    /* Read devices' ROM codes */
    onewire_first(&devices_on_bus.rom_codes[0]);
    for (uint8_t i = i; i < NUMBER_OF_SENSORS; i++) {
        onewire_next(&devices_on_bus.rom_codes[i]);    
    }
}

/*
 * @brief  Get reading of temperature
 * @param  sensor_index Sensor to read from
 * @return              Temperature value
 */
float get_soil_temperature(uint8 sensor_index)
{
    set_speed(STANDARD);
    
    // Detect presence
    if (onewire_touch_reset()) return 0;
    
    /* Issue match ROM command and write the device address */
    onewire_write_byte(CMD_ROM_MATCH);
    /* LSB->MSB device address */
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t to_send = devices_on_bus.rom_codes[sensor_index] >> (BYTE_LEN * i);
        onewire_write_byte(to_send);
    }
    
    // Initiate reading
    onewire_write_byte(CMD_READ_SCRATCHPAD);
    
    // Read LSB and MSB from the scratchpad
    uint8 lsb = onewire_read_byte();
    uint8 msb = onewire_read_byte();
    
    // Initiate reset to terminate reading
    onewire_touch_reset();
    
    // Get the result as floating point number
    int   decimal  = ((msb & 0x07) << 4) | (lsb >> 4);
    float floating = ((lsb & 0x01) >> 0) * (1 / 16.0) +
                     ((lsb & 0x02) >> 1) * (1 / 8.0)  +
                     ((lsb & 0x04) >> 2) * (1 / 4.0)  +
                     ((lsb & 0x08) >> 3) * (1 / 2.0);
    float result = decimal + floating;
    if (msb & 0x80) result *= -1; // Determine sign
                                      
    return result;
}

/*
 * @brief Initiate DS1822 conversion
 * @param sensor_index Sensor to read from
 */
void start_conversion_soil_temp_sensor(uint8 sensor_index)
{
    set_speed(STANDARD);
    
    // Detect presence
    if (onewire_touch_reset()) return;
    
    /* Issue match ROM command and write the device address */
    onewire_write_byte(CMD_ROM_MATCH);
    /* LSB->MSB device address */
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t to_send = devices_on_bus.rom_codes[sensor_index] >> (8 * i);
        onewire_write_byte(to_send);
    }
    
    // Issue conversion command to sample the temperature
    onewire_write_byte(CMD_CONVERT_TEMP);
}

/* [] END OF FILE */
