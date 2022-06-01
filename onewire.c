/* ========================================
 *
 * @name    TX00DB04 - Programmable System on Chip Design. OneWire interface
 * @company Metropolia University of Applied Sciences
 * @date    26.04.2022
 *
 * Interface to access OneWire. Partially adopted from Maxim Integrated.
 * URL: https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
 *
 * ========================================
 */

#include "onewire.h"

/* ============================= */
/* Private interface definitions */
/* ============================= */

/* Keep timings static to avoid namespace polution */
static int A, B, C, D, E, F, G, H, I, J;

// One tick is 0.25 us according to standards
static void tick_delay(int ticks)
{
    CyDelayUs(ticks / 4);
}

/* =============================*/
/* Public interface definitions */
/* =============================*/

/* 
 * @brief Configure timings.
 * Only available for standard mode due to delay precision issues.
 */
void set_speed()
{
    // Standard Speed
    A = 6 * 4;
    B = 64 * 4;
    C = 60 * 4;
    D = 10 * 4;
    E = 9 * 4;
    F = 55 * 4;
    G = 0;
    H = 480 * 4;
    I = 70 * 4; /* Pretty weird. Had to increase the time to get the response from slave */
    J = 410 * 4;
}

/* 
 * @brief  Generate a 1-Wire reset, return 1 if no presence detected.
 * @return Presence in boolean format, reverse
 */
uint8 onewire_touch_reset()
{
    int result;
    
    /* GHIJ */
    tick_delay(G);
    OneWire_Pin_Write(LOW);
    tick_delay(H);
    OneWire_Pin_Write(HIGH);     // Releases the bus
    tick_delay(I);
    result = OneWire_Pin_Read() ^ 0x01;      // Sample for presence pulse from slave
    tick_delay(J);               // Complete the reset sequence recovery
    
    return result; // Return sample presence pulse result
}

/*
 * @brief  Send a OneWire write bit. Provide 10 us recovery time
 * @param  bit  Bit of data
 */
void onewire_write_bit(uint8 bit)
{
    if (bit) {
        // Write '1' bit - AB
        OneWire_Pin_Write(LOW); // Drives DQ low
        tick_delay(A);
        OneWire_Pin_Write(HIGH); // Releases the bus
        tick_delay(B);       // Complete the time slot and 10us recovery
    }
    else {
        // Write '0' bit - CD
        OneWire_Pin_Write(LOW); // Drives DQ low
        tick_delay(C);
        OneWire_Pin_Write(HIGH); // Releases the bus
        tick_delay(D);
    }
}

/*
 * @brief  Read a OneWire bit. Provide 10 us recovery time
 * @return Bit of data
 */
uint8 onewire_read_bit()
{
    int result;
    
    /* AEF */
    OneWire_Pin_Write(LOW);             // Drives DQ low
    tick_delay(A);
    OneWire_Pin_Write(HIGH);            // Releases the bus
    tick_delay(E);
    result = OneWire_Pin_Read() & 0x01; // Sample the bit value from the slave
    tick_delay(F);                      // Complete the time slot and 10us recovery

    return result;
}

/*
 * @brief  Write OneWire data byte
 * @param  data  Target byte
 */
void onewire_write_byte(uint8 data)
{
    int loop;

    // Loop to write each bit in the byte, LS-bit first
    for (loop = 0; loop < BYTE_LEN; loop++)
    {
        onewire_write_bit(data & 0x01);

        // Shift the data byte for the next bit
        data >>= 1;
    }
}

/*
 * @brief  Read OneWire data byte and return it
 * @return Sampled byte
 */
uint8 onewire_read_byte(void)
{
    int loop, result=0;

    for (loop = 0; loop < BYTE_LEN; loop++)
    {
        // shift the result to get it ready for the next bit
        result >>= 1;

        // if result is one, then set MS bit
        if (onewire_read_bit()) result |= 0x80;
    }
    return result;
}

/*
 * @brief  Write a OneWire data byte and return the sampled result
 * @param  data  Target byte
 * @return       Sampled result
 */
uint8 onewire_touch_byte(uint8 data)
{
    int loop, result = 0;

    for (loop = 0; loop < BYTE_LEN; loop++)
    {
        // Shift the result to get it ready for the next bit
        result >>= 1;

        // If sending a '1' then read a bit else write a '0'
        if (data & 0x01)
        {
            if (onewire_read_bit()) result |= 0x80;
        }
        else
            onewire_write_bit(0);

        // Shift the data byte for the next bit
        data >>= 1;
    }
    return result;
}

/*
 * @brief  Write a block OneWire data bytes and return
 * the sampled result in the same buffer
 * @param  data     Target data
 * @param  data_len Number of bytes
 */
void onewire_block(unsigned char *data, int data_len)
{
    int loop;

    for (loop = 0; loop < data_len; loop++)
    {
        data[loop] = onewire_touch_byte(data[loop]);
    }
}

/*
 * @brief  Find the 'first' devices on the 1-Wire bus
 * @return TRUE  : device found, ROM number in ROM_NO buffer
 *         FALSE : no device present
 */
int onewire_first(uint64* buf)
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = 0;
   LastFamilyDiscrepancy = 0;

   return onewire_search(buf);
}

/*
 * @brief  Find the 'next' devices on the 1-Wire bus
 * @return TRUE  : device found, ROM number in ROM_NO buffer
 *         FALSE : no device present
 */
int onewire_next(uint64* buf)
{
   // leave the search state alone
   return onewire_search(buf);
}

/*
 * @brief  Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
 * search state.
 * @return TRUE  : device found, ROM number in ROM_NO buffer
 *         FALSE : device not found, end of search
 */
int onewire_search(uint64* buf)
{
   int id_bit_number;
   int last_zero, rom_byte_number, search_result;
   int id_bit, cmp_id_bit;
   unsigned char rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
   crc8 = 0;

   // if the last call was not the last one
    if (!LastDeviceFlag)
    {
        // 1-Wire reset
        if (onewire_touch_reset())
        {
            // reset the search
            LastDiscrepancy = 0;
            LastDeviceFlag = 0;
            LastFamilyDiscrepancy = 0;
            return 0;
        }

        // issue the search command 
        onewire_write_byte(0xF0);  

        // loop to do the search
        do
        {
            // read a bit and its complement
            id_bit = onewire_read_bit();
            cmp_id_bit = onewire_read_bit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else
            {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                   search_direction = id_bit;  // bit write value for search
                else
                {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0)
                    {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                onewire_write_bit(search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0)
                {
                    docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        }
        while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (!((id_bit_number < 65) || (crc8 != 0)))
        {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)
                LastDeviceFlag = 1;
             
            search_result = 1;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0])
    {
        LastDiscrepancy = 0;
        LastDeviceFlag = 0;
        LastFamilyDiscrepancy = 0;
        search_result = 0;
    }
    else memcpy(buf, ROM_NO, 8);

    return search_result;
}

static unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

/*
 * @brief  Calculate the CRC8 of the byte value provided with the current 
 * global 'crc8' value. 
 * @return Current global crc8 value
 */
unsigned char docrc8(unsigned char value)
{
   // See Application Note 27
    
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}

