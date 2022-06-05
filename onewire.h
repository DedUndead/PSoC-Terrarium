/* ========================================
 *
 * @name    TX00DB04 - Programmable System on Chip Design. OneWire interface
 * @company Metropolia University of Applied Sciences
 *
 * Interface to access OneWire. Partially adopted from Maxim Integrated.
 * URL: https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
 *
 * Binary search algorithm is partially adopted from Maxim Integrated.
 * URL: https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/187.html
 *
 * ========================================
 */

#ifndef ONEWIRE_H
#define ONEWIRE_H 
    
    
#include "project.h"
    
#define OVERDRIVE  0
#define STANDARD   1
#define BYTE_LEN   8
#define CODE_LEN   64
#define LOW        0x00
#define HIGH       0x01
    
#define CMD_ROM_MATCH       0x55
#define CMD_ROM_SEARCH      0xf0
#define CMD_READ_SCRATCHPAD 0xbe
#define CMD_CONVERT_TEMP    0x44
    
#define DEVICES_ON_BUS      2
    
/* Functions declarations */
/* Basic */
void  set_speed();
uint8 onewire_touch_reset(void);
void  onewire_write_bit(uint8 bit);
uint8 onewire_read_bit();
void  onewire_write_byte(uint8 data);
uint8 onewire_read_byte(void);
uint8 onewire_touch_byte(uint8 data);
void  onewire_block(unsigned char *data, int data_len);
uint8 onewire_overdrive_skip();

/* Binary search */
int           onewire_first(uint64* buf);
int           onewire_next(uint64* buf);
int           onewire_search(uint64* buf);
unsigned char docrc8(unsigned char value);
    
    
#endif
