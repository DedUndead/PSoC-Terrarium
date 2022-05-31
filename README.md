# PSoC Terrarium

Despite terrarium, greenhouse and plant growing automation projects are overhyped, this little development is meant for the exploration of Programmable System on Chip capabilities. As long as my portfolio consists of Embedded projects that inherit classical principles of emb-dev, this work is focused on showing the flexibility of PSoC microcontrollers, which will be described well enough in the documentation below.

## General description

PSoC Terrarium provides a multisensing device solution to track the state of the environment, where the Terrarium system is placed.<br>
The general system description with the interfaces is presented in Figure 1.<br>

<p align="center"><img src="https://i.imgur.com/mARAkWj.png" alt="General system description"></p>
<p align="center">Figure 1. General system description</p>

The system is equipped with four three sensor for different purposes, soil moisture sensor, UART interface for user interactions as well as LED indicator and 9g servo m otor, that represents hatch.
The details about the interfaces, components, hardware and software architectures are discussed in later sections.

The prototype of Terrarium does not have a case, all the components are laid out on the breadboard. Thus, case description is not present in this documentation.

## List of components

<p>Table 1. List of components for Terrarium.</p>

| Component         | Name            | Datasheet                                                                                              |  
|-------------------|-----------------|--------------------------------------------------------------------------------------------------------|
| MCU               | PSoC 5LP        | [Link](https://www.infineon.com/dgdl/Infineon-Component_PSoC_3_PSoC_5LP_System_Reference_Guide_V5.90-Software+Module+Datasheets-v06_01-EN.pdf?fileId=8ac78c8c7d0d8da4017d0ea0ae2524ad&utm_source=cypress&utm_medium=referral&utm_campaign=202110_globe_en_all_integration-files)                                                                                                                                         |
| Soil temp sensor  | DS18B20         | [Link](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)                                       |
| Air temp sensor   | TC74            | [Link](https://ww1.microchip.com/downloads/en/DeviceDoc/21462D.pdf)                                    |
| Soil moist sensor | DFRobot         | [Link](https://media.digikey.com/pdf/Data%20Sheets/DFRobot%20PDFs/SEN0193_Web.pdf)                     |
| LED               | Any             | N/A                                                                                                    |
| 9g Servo          | Any             | N/A                                                                                                    |

### Justification for components choice

#### MCU - PSoC 5LP

Programmable System of Chip microcontrollers provide extensive functionality for quick prototyping of embedded devices. The ability to quickly map the internal hardware components in PSoC Creator allows to generate and utilize SDK through API prepared by the IDE. Widely used ARM Cortex M3 architecture makes this choice even more solid.

#### Soil temperature sensors - DS18B20

DS18B20 sensors are interfaced using OneWire bus. OneWire devices have unique IDs assigned and stored in ROM during manufacturing stage, thus solving the problem of having similar devices on one bus without a need for additional request to a manufacturer. The simplicity of hardware connections make OneWire based sensors an obvious choice when the application reqiures _up to N_ devices to be placed. One of such applications is used in the project - measuring soil temperature in different areas of terrarium. It is worth noting, that DS18B20 offers a high and configurable precision.

#### Air temperature sensors - TC74

TC74 sensor is a simple I2C device. It cannot provide as accurate measurement as DS18B20, however, its triviality is a big bonus when measuring temperature of the air. It is not a requirement to be aware of exact air temperature and, therefore, resolution up to one centigrade is reasonable.

#### Soil moisture sensor - DFRobot

DFRobot's soil temperature sensor is an analog plug-and-play solution for measuring soil temperature. Originally designed DFRobot's extension shields, it is still a decent device to use in a prototype project including PSoC Terrarium.

#### Actuators

In the current prototype, simple red LED and 9g servo motor are used for demonstration purposes.<br>
Some details on 9g related software are provided in respective section.

## Hardware architecture

### Internal hardware architecture

### External hardware architecture

## Software architecture

### Custom interfaces

### Credit

Prepared for Metropolia University of Applied Science's "Programmable System on Chip Design" course conducted by Antti Piironen, Principal Lecturer in Smart Systems Engineering.
