# PSoC Terrarium

Despite terrarium, greenhouse and plant growing automation projects are overhyped, this little development is meant for the exploration of Programmable System on Chip capabilities. As long as my portfolio consists of Embedded projects that inherit classical principles of emb-dev, this work is focused on showing the flexibility of PSoC microcontrollers, which will be described well enough in the documentation below.

Terrarium is designed to be scalable. On top of existing platform and codebase, many additional sensors and actuators can be added without altering the core API.

#### Table of contents

1. [General description](#general-description)
2. [List of components](#list-of-components)
    1. [Justification for components choice](#justification-for-components-choice)
3. [Hardware architecture](#hardware-architecture)
    1. [Internal hardware architecture](internal-hardware-architecture)
    2. [External hardware architecture](#external-hardware-architecture)
4. [Software architecture](#software-architecture)
    1. [Overall software architecture logic](#overall-software-architecture-logic)
    2. [EEPROM layout](#eeprom-layout)
    3. [Custom interfaces](#custom-interfaces)
5. [User guide](#user-guide)
6. [Future desing consideration](#future-design-consideration)
7. [Credit](#credit)

# General description

PSoC Terrarium provides a multisensing device solution to track the state of the environment, where the Terrarium system is placed.<br>
The general system description with the interfaces is presented in Figure 1.<br>

<p align="center"><img src="https://i.imgur.com/mARAkWj.png" alt="General system description"></p>
<p align="center">Figure 1. General system description</p>

The system is equipped with four three sensor for different purposes, soil moisture sensor, UART interface for user interactions as well as LED indicator and 9g servo m otor, that represents hatch.
The details about the interfaces, components, hardware and software architectures are discussed in later sections.

The prototype of Terrarium does not have a case, all the components are laid out on the breadboard. Thus, case description is not present in this documentation.

# List of components

<p>Table 1. List of components for Terrarium.</p>

| Component         | Name            | Datasheet                                                                                              |  
|-------------------|-----------------|--------------------------------------------------------------------------------------------------------|
| MCU               | PSoC 5LP        | [Link](https://www.infineon.com/dgdl/Infineon-Component_PSoC_3_PSoC_5LP_System_Reference_Guide_V5.90-Software+Module+Datasheets-v06_01-EN.pdf?fileId=8ac78c8c7d0d8da4017d0ea0ae2524ad&utm_source=cypress&utm_medium=referral&utm_campaign=202110_globe_en_all_integration-files)                                                                                                                                         |
| Soil temp sensor  | DS18B20         | [Link](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)                                       |
| Air temp sensor   | TC74            | [Link](https://ww1.microchip.com/downloads/en/DeviceDoc/21462D.pdf)                                    |
| Soil moist sensor | DFRobot         | [Link](https://media.digikey.com/pdf/Data%20Sheets/DFRobot%20PDFs/SEN0193_Web.pdf)                     |
| LED               | Any             | N/A                                                                                                    |
| 9g Servo          | Any             | N/A                                                                                                    |

## Justification for components choice

### MCU - PSoC 5LP

Programmable System of Chip microcontrollers provide extensive functionality for quick prototyping of embedded devices. The ability to quickly map the internal hardware components in PSoC Creator allows to generate and utilize SDK through API prepared by the IDE. Widely used ARM Cortex M3 architecture makes this choice even more solid.

### Soil temperature sensors - DS18B20

DS18B20 sensors are interfaced using OneWire bus. OneWire devices have unique IDs assigned and stored in ROM during manufacturing stage, thus solving the problem of having similar devices on one bus without a need for additional request to a manufacturer. The simplicity of hardware connections make OneWire based sensors an obvious choice when the application reqiures _up to N_ devices to be placed. One of such applications is used in the project - measuring soil temperature in different areas of terrarium. It is worth noting, that DS18B20 offers a high and configurable precision.

### Air temperature sensors - TC74

TC74 sensor is a simple I2C device. It cannot provide as accurate measurement as DS18B20, however, its triviality is a big bonus when measuring temperature of the air. It is not a requirement to be aware of exact air temperature and, therefore, resolution up to one centigrade is reasonable.

### Soil moisture sensor - DFRobot

DFRobot's soil temperature sensor is an analog plug-and-play solution for measuring soil temperature. Originally designed DFRobot's extension shields, it is still a decent device to use in a prototype project including PSoC Terrarium.

### Actuators

In the current prototype, simple red LED and 9g servo motor are used for demonstration purposes.<br>
Some details on 9g related software are provided in respective section.

# Hardware architecture

## Internal hardware architecture

## External hardware architecture

# Software architecture

## Overall software architecture logic

Overall software architecture is based on modularity.
All the logic is performed inside modules, that are called based on the internal hardware timers.
Figure 2 showcases module names and their periodicity.

<p align="center"><img src="https://i.imgur.com/boOiVY4.png" alt="General system description"></p>
<p align="center">Figure 2. Software architecture modules</p>

You can view this modules controlled by the flags with similar names from the main function body.

### Ready to Save
**Responsible timer**: Timer_Save<br>
Ready so save module obtains filtered samples from boxcar average filters.<br>
It then creates a new samples structure and saves it to EEPROM.<br>

### Minute Passed
**Responsible timer**: Timer_DeviceClock<br>
Minute passed module adjusts device's time by one minute.<br>
It then saves adjusted time into EEPROM.

### Ready to Measure
**Responsible timer**: Timer_Measure<br>
Ready to measure modules gets raw samples from the samples.<br>
It then appends those samples to boxcar average filters.<br>
Raw samples are used to adjust the actuators.<br>

### ADC Conversion Ready
**Responsible timer**: ADC conversion ready interrupt<br>
This module samples voltage when ADC conversion is ready and appends it to simple average filter.<br>
Its main responsibility is to average and filter raw ADC samples to further get accurate moisture reading in Ready to Measure module.

### Handle User Input
**Responsible timer**: None<br>
This module constantly polls UART for presence of input.<br> 
It then echoes back characters to UART and verifies if entered command are valid.

## EEPROM layout

Basic device information (timestamps) as well as measurements are stored in EEPROM with periodicity described above.<br>
This allows for quick and persistent access to main information available on the platform.

| Address | Name                   | Description                                             |  
|---------|------------------------|---------------------------------------------------------|
| 0x0000  | EEPROM_WRITE_ADDR_MSB  | Next write address where the measurement will be stored |
| 0x0001  | EEPROM_WRITE_ADDR_LSB  |                                                         |
| 0x0002  | EEPROM_INFO_ADDR_MSB   | Stores UNIX timestamp that was saved to the device      |
| 0x0003  | EEPROM_INFO_ADDR       |                                                         |
| 0x0004  | EEPROM_INFO_ADDR       |                                                         |
| 0x0005  | EEPROM_INFO_ADDR_LSB   |                                                         |
| 0x0006  | EEPROM_DATA_START_ADDR | Measurements are stored starting from this address      |

Rest of the data is reserved for measurements.<br>
Size of one measurement pack stored in EEPROM is the size of `packed_samples` structure.<br>
More information about EEPROM handling is provided in **custom interfaces** section.

## Custom interfaces

### EEPROM management

### Time management

### User menu helpers

# User guide

# Future design consideration

### Credit

Prepared for Metropolia University of Applied Science's "Programmable System on Chip Design" course conducted by Antti Piironen, Principal Lecturer in Smart Systems Engineering.
