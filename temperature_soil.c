/* ========================================
 *
 * @author  Pavel Arefyev
 * @name    Temperature sensors interface abstraction
 * @company Metropolia University of Applied Sciences
 * @date    26.05.2022
 *
 * This abstraction is built on top of OneWire interface.
 * It is meant for DS18B20 temperature sensor devices, however, it can smoothly
 * operate with any OneWire enabled devices added to the bus.
 * Datasheet: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 *
 * During initialization, all the sensor found on the bus are stored in
 * devices_on_bus structure that is later accessed by public interface functions.
 * To alter number of sensor present on OneWire bus, change NUMBER_OF_SOIL_TEMP_SENSORS parameter.
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
    set_speed();
    
    /* Read devices' ROM codes */
    /* After initialization ROM codes can be accessed from devices_on_bus in private interface */
    onewire_first(&devices_on_bus.rom_codes[0]);
    for (uint8_t i = 1; i < NUMBER_OF_SOIL_TEMP_SENSORS; i++) {
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
    // Sanity check
    if (sensor_index > NUMBER_OF_SOIL_TEMP_SENSORS - 1) return 0;
    
    set_speed();
    
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
    set_speed();
    
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
