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

<p align="center"><img src="https://i.imgur.com/lSghwnF.png" alt="General system description"></p>
<p align="center">Figure 1. General system description</p>

The system is equipped with four sensor for different purposes, soil moisture sensor, UART interface for user interactions as well as LED indicator and 9g servo m otor, that represents hatch.
The details about the interfaces, components, hardware and software architectures are discussed in later sections.

The prototype of Terrarium does not have a case, all the components are laid out on the breadboard. Thus, case description is not present in this documentation.

# List of components

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

"Internal hardware" refers to the PSoC Creator hardware modules that are used to generate an SDK for the project.<br>
Overall design is presented in Figure 3.

<p align="center"><img src="https://i.imgur.com/aUdpNXd.png" alt="General system description"></p>
<p align="center">Figure 3. PSoC hardware modules design</p>

**Interfaces** section includes all main interfaces that are used for communication in the environmant: UART, I2C, ADC, OneWire.<br>
**Memory** section includes EEPROM component.<br>
**Actuators related HW** includes means of interacting with the actuators: PWM, GPIO.<br>
**Timers** section includes all the timers that are used for controlling the software flow. It is described in details in Overall Software Architecture chapter.

## External hardware architecture

<p align="center"><img src="https://i.imgur.com/N4sUtry.png" alt="General system description"></p>
<p align="center">Figure 2. External hardware architecture.</p>

Follow the link to view schematics in full resolution: https://i.imgur.com/N4sUtry.png

# Software architecture

## Overall software architecture logic

Overall software architecture is based on modularity.
All the logic is performed inside modules, that are called based on the internal hardware timers.
Figure 4 showcases module names and their periodicity.

<p align="center"><img src="https://i.imgur.com/DzGZtyc.png" alt="General system description"></p>
<p align="center">Figure 4. Software architecture modules</p>

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

### DS18B20 modules
**Responsible timer**: Timer_OneWire<br>
**DS18B20_Ready_To_Convert** starts in the beginning of the execution.<br>
It issues convert command for all OneWire enabled sensors and then starts OneShot Timer that will interrupt after 800 ms to start **DS18B20_Sample_Ready** module.<br>
The following module will then update OneWire samples that will be accessed in Ready to Measure module.

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

### Moisture sensor
**Files**: moisture_sensor<br>
This abstraction provides interface for handling moisture sensor. It uses analog input on the microcontroller to provide moisture reading.
The analog input (ADC_DelSig component) emulates Arduino's ADC converter because the sensor was originally developed for Arduino UNO and DFRobot's shield.<br>
There is no transfer function for the sensor, thus, the moisture is obtained by performing linear mapping from ADC range to percentage.<br>

| Configuration  | Description                                     |  
|----------------|-------------------------------------------------|
| MOIST_VALUE_MV | ADC reading when the sensor is exposed to water |
| DRY_VALUE_MV   | ADC reading when the sensor is exposed to air   |

Moisture sensor requires manual calibration, thus these values are unique to every set up.

| Function                                 | Parameters | Description                                          |  
|------------------------------------------|------------|------------------------------------------------------|
| **void** initialize_soil_moisture_sensor |            | Initialize hardware related to the abstraction (ADC) |
| **int** get_soil_moisture                |            | Get soil moisture in percent                         |

### I2C Driver
**Files**: i2c_driver<br>
Simple interface for interacting with I2C bus. It's not the most generic interface but it can be simply expanded using PSoC I2C API for further development.
This interface only allows to read one byte from specified slave->register.

The communication template:<br>
S | SLAVEADDR | W | REGADDR | ACK | RS | R | DATA | NAK | ST

| Configuration  | Description                                      |  
|----------------|--------------------------------------------------|
| I2C_ERROR      | I2C error code returned by API when error occurs |

| Function                | Parameters                                           | Description                                               |  
|-------------------------|------------------------------------------------------|-----------------------------------------------------------|
| **void** initialize_i2c |                                                      | Initialize I2C hardware components                        |
| **int16** read_i2c_data | **uint8** slave_address, **uint8** register_address  | Read data from slave using comm. template described above |

### Soil temperature sensors
**Files**: temperature_soil<br>
This abstraction is built on top of OneWire interface.
It is meant for DS18B20 temperature sensor devices, however, it can smoothly operate with any OneWire enabled devices added to the bus.

During initialization, all the sensor found on the bus are stored in devices_on_bus structure that is later accessed by public interface functions.
To alter number of sensor present on OneWire bus, change NUMBER_OF_SOIL_TEMP_SENSORS parameter.

| Configuration                    | Description                                      |  
|----------------------------------|--------------------------------------------------|
| NUMBER_OF_SOIL_TEMP_SENSORS      | Number of soil temperature sensors on the bus    |

After configuration has been changed, the Terrarium will be reorganized to print, measure and operate with different number of OneWire sensors. In order for the system to work correctly, samples saved to EEPROM must be cleared (refer to User Guide).

| Function                                    | Parameters      | Description                                                             |  
|---------------------------------------------|-----------------|-------------------------------------------------------------------------|
| **void** initialize_soil_temp_sensors       |                 | Find all devices present on the bus and save their addressed            |
| **float** get_soil_temperature              | **uint8** index | Issue read command for the sensor with **index** index on the bus       |
| **float** start_conversion_soil_temp_sensor | **uint8** index | Issue conversion command for the sensor with **index** index on the bus |

Technically, number of sensor on OneWire bus is unlimited. In the software, the limit is 255 (uint8 limitation). Consider also interference when having long physical bus.<br>
This interface uses binary search algorithm to identify devices present on the bus. If requirements are time critical, consider delays during initialization when adding large amount of sensors to the bus.<br>
Index of the sensor is determined during initialization and hidden behind the static interface. Refer to main.c to see how the API can be used in iterative manner to access all sensors on the bus.<br>

### Heater
**Files**: heater<br>
This interface provides an adjustment functions for heater simulator. In this demo project, simple neon-red LED is used as a simulator.

| Configuration     | Description                                                    |  
|-------------------|----------------------------------------------------------------|
| HEATER_ON_TEMP_C  | Heater is turned on when this value is reached                 |
| HEATER_OFF_TEMP_C | Heater is turned back off once temperature is above this value |

| Function               | Parameters            | Description                                         |  
|------------------------|-----------------------|-----------------------------------------------------|
| **void** adjust_heater | **int16** temperature | Adjust heater actuator according to **temperature** |

### Hatch
**Files**: hatch<br>
This interface provides an adjustment functions for hatch simulator.
In this demo project, 9g servo is used as a simulator.

| Configuration      | Description                                      |  
|--------------------|--------------------------------------------------|
| HATCH_OPEN_TEMP_C  | When this value is reached, hatch will be opened |

Hatch opens for 20% after each centigrade above HATCH_OPEN_TEMP_C.

| Function                    | Parameters            | Description                                        |  
|-----------------------------|-----------------------|----------------------------------------------------|
| **void** inititialize_hatch |                       | Initialize related hardware components (PWM)       |
| **void** adjust_hatch       | **int16** temperature | Adjust hatch actuator according to **temperature** |

### Average filter
**Files**: average_filter<br>
This file provides basic interface for the simplest filter: average filter.
The samples are saved to the running sum and the filtered result is an average of saved samples.

To start using the filter, AverageFilter structure must be created.<br>
The filter clears running sum once **.filter_length** is reached.

| Function                      | Parameters                                           | Description                                                    |  
|-------------------------------|------------------------------------------------------|----------------------------------------------------------------|
| **void** add_sample_to_filter | **AverageFilter\*** filter, **const int** new_sample | Update running sum with a new sample                           |
| **int** get_filtered_result   | **AverageFilter\*** filter                           | Get filtered result (average) of the current samples collected |

### Moving average (boxcar) filter
**Files**: moving_average_filter<br>
Moving Average Filter is a simple interface for quick calculations and saving of samples to sliding window. 
This interface relies on a structure with a sliding window. Details can be viewed from the source code.

| Configuration | Description                  |  
|---------------|------------------------------|
| FILTER_LENGTH | Length of the sliding window |

To start using the filter, AverageFilter structure must be created.<br>
Consider the size of PSoC heap when adjusting the size of the sliding window. Sliding window of a bigger size may lead to overflow.

| Function                         | Parameters                                                 | Description                                                    |  
|----------------------------------|------------------------------------------------------------|----------------------------------------------------------------|
| **void** add_sample_to_MA_filter | **MovingAverageFilter\*** filter, **const int** new_sample | Update sliding window with a new sample                        |
| **int** get_MA_filtered_result   | **MovingAverageFilter\*** filter                           | Get filtered result (average) of the current samples collected |

### EEPROM interface
**Files**: main<br>
EEPROM interface provides API to communicate with EEPROM on the device. It is created according to EEPROM layout described in the respective section.<br>
When clearing memory, it is enough to reset next writing address. The layout therefore allows to extend EEPROM lifetime by saving amount of operations and boost performance by simplifying clearing operation to a single internal API call. Refer to the source code to see details.

When saving samples to the memory, samples should be packed into **packed_samples** structure. This way, EEPROM is utilized byte-to-byte without missing any space.

| Function                             | Parameters                                  | Description                                                    |  
|--------------------------------------|---------------------------------------------|----------------------------------------------------------------|
| **void** save_samples_to_eeprom      | **packed_samples** samples                  | Save **samples** to EEPROM next writing address                |
| **uint16** print_samples_from_eeprom |                                             | Print **samples** stored in EEPROM                             |
| **void** init_eeprom_layout          |                                             | Perform validity of EEPROM layout                              |
| **void** erase_samples_from_eeprom   |                                             | Erase samples by resetting next write address                  |
| **uint8** set_date                   | **uint** day, **uint** month, **uint** year | Set date, store new timestamp to EEPROM                        |
| **uint8** set_time                   | **uint** hour, **uint** minute              | Set time, store new timestamp to EEPROM                        |
| **struct tm** get_time_from_eeprom   |                                             | Get timestamp from EEPROM in a form of time.tm structure       |
| **void** save_time_to_eeprom         | **uint32** timestamp                        | Save new **UNIX** timestamp to EEPROM                          |
| **uint32** get_time_from_eeprom_unix |                                             | Get timestamp from EEPROM in a form of UNIX timestamp          |

When EEPROM is filler, the writing address is reset and the samples are abandoned. Thus, consider saving valuable information regularly with a client-side script.

Time is tracked using the hardware timer and standard C libraries (time). It adjusts the timestamp every minute and stores in UNIX form in EEPROM.

### User menu helpers
**Files**: main<br>
These functions are used to print text to UART.

| Configuration      | Description                                                      | 
|--------------------|------------------------------------------------------------------|
| DEVICE_INFO_PROMPT | Information prompt that will be printed when ? command is issued |

If you are altering this project, DEVICE_INFO_PROMPT must contain original developer's name: **Pavel Arefyev**.

| Function                             | Parameters                  | Description                                      |  
|--------------------------------------|-----------------------------|--------------------------------------------------|
| **void** print_sample                | **packed_samples\*** sample | Print measurement stored in **sample** structure |
| **void** print_current_time          |                             | Print current time on the device                 |
| **void** print_help                  |                             | Print user help information                      |

### Private interfaces
**Files**: onewire<br>
Private interfaces are considered lower-level API in the project. No information about configuration will be present in documentation. Therefore, refer to source files mentioned above.

# User guide

As mentioned, the codebase allows flexibility in design choices.<br>
If you would like to replace any of the existing components with the alternative, some configuration tweaking is required (I2C address, for instance) but it is simple and effective.

Follow these steps in order to set up your PSoC Terrarium:
1. Clone repository and open **psoc_project.cydsn** or download source from [Drive](https://drive.google.com/file/d/17nfClusA1car1u5MrSCX8ocyo4IAia4A/view?usp=sharing). 
2. Connect all the components required according to hardware schematics. If you would like to expand OneWire bus by adding more temperature sensors (unlimited), change **NUMBER_OF_SOIL_TEMP_SENSORS** configuration in temperature_soil.h to match your set up. 
3. Tweak any other configuration if required (review Custom interfaces section).
4. Connect PSoC and program your device in the IDE.
5. Open terminal for serial connection with 57600 baudrate.
6. If you have changed NUMBER_OF_SOIL_TEMP_SENSORS configuration, **clear** device memory first by issuing **"C"** command to the terminal.

Now the device is fully operational. Figure 5 showcases menu help interface.

<p align="center"><img src="https://i.imgur.com/hJmz6ME.png" alt="General system description"></p>
<p align="center">Figure 5. User terminal</p>

After each successful or unsuccessful opeartion all the options will be displayed on the screen again.

**Tsoil\[index\]** represents temperature of the soil measured by OneWire based sensor number **index**.
It will automatically adjust the printing according to your OneWire bus setup.

<p align="center"><img src="https://i.imgur.com/Dzim1VJ.png" alt="General system description"></p>
<p align="center">Figure 6. "A" command output example.</p>

# Future design consideration

This section briefly describes issues that could be addressed in future development.

### External memory
Samples should be additionally stored on external SD card. The alternative and much better solution is to save samples to cloud, where they could be analyzed well later.

### Device time tracking
To make time tracking more accurate, device could be synced with the real-time servers or use external RTC.<br>
However, it poses redesign issues regarding date and time user configuration. Perhaps, if those options should be kept, usage of standard libraries and hardware timers is sufficient. 

### Moisture sensor calibration function
Calibration function could be implemented to speed up the process of environment set up. It may happen, that re-calibration is required after a certain time of using the device. Calibration function could be implemented as a part of user interface (CALIB <SENSOR_NAME>).

# Credit

Prepared for Metropolia University of Applied Science's "Programmable System on Chip Design" course conducted by Antti Piironen, Principal Lecturer in Smart Systems Engineering.
