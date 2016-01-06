# CAN Servo-Node

Initial development performed by partnership of [Sentera, LLC](https://sentera.com/) and [University of Minnesota](http://www.uav.aem.umn.edu/).

## System Overview
![System Diagram](/doc/System%20Diagram.png?raw=true "System Diagram")

The CAN Servo-Node is designed as a component of the Flight Management System.  The CAN Servo-Node is a Microchip dsPIC33 based module with the primary function of receiving servo commands over a CAN bus, and controlling power to a servo based on those commands.  CAN communication is documented in file [UMN FMU Communication Protocol](https://github.com/SenteraLLC/pic32-fmu.X/blob/master/doc/UMN%20FMU%20Communication%20Protocol.docx).  When commanding the position of a servo, the software performs servo calibration correction so the required PWM value is applied to the servo.

Besides the described primary function, the CAN Servo-Node also:

* Performs monitoring of servo current and voltage, and annunciation of servo status on the CAN bus.

* Performs monitoring of VSENSE1 and VSENSE2 signals (described more below), calibration correction of values, and annunciation on the CAN bus.

* Provides reading and writing of calibration correction values (for the servo, VSENSE1, and VSENSE2) on the CAN bus.

The software design for the FMU Communications Board can be found at GitHub repository: [https://github.com/SenteraLLC/pic32-fmu.X](https://github.com/SenteraLLC/pic32-fmu.X).

## Project Folder Structure
The project includes the following folders:

>[doc](/doc) - documentation for the project.

>[doxygen](/doxygen) - project file for generating Doxygen documentation.

>[hw](/hw) - hardware design schematics.

>[inc](/inc) - include (i.e. header) code files.

>[ipe](/ipe) - instructions for a device programming method which maintains unique serial numbers.

>[nbproject](/nbproject) - MPLAB X IDE project files.

>[src](/src) – source code files.

## Hardware Overview
The hardware contains the following connectors:

>J1 - Servo voltage input (note: PWM input not currently used).

>J2 - Servo voltage and PWM output.

>J3 - Power (VCC/GND) and CAN connector.

>J4 - Duplicate Power (VCC/GND) and CAN connector.

>J5 - Power (VCC/GND) output and V_SENSE1 input.

>J6 - Power (VCC/GND) output and V_SENSE2 input.

The J3 or J4 connector is used to supply power to the CAN Servo-Node and to provide connection to the CAN bus.  The duplicate connector is used to continue wiring to additional devices (e.g. an additional CAN Servo-Node) or to provide termination of the CAN bus (i.e. terminating resistor).

The J5 and J6 connectors provide power to two additional devices.  These connectors also include an analog signal (V_SENSE1/2) which the CAN Servo-Node reads.  Likely devices connected to this interface include pressure sensors, temperature sensors, potentiometers, etc.

*Note: The CAN Servo-Node requires 5V supplied power (VCC) through the J3 or J4 connector.*

## Software Overview
All source code is commented using Doxygen style formatting.  Therefore, Doxygen can be used to generate an easily navigable document which provides greater detail into the software's operation than the overview which is provided here.

### Software Executive
The software implements a preemptive, cyclic executive using four threads.  The software threads are:

1. **Reset**: Thread is executed following reset.  Within the MPLAB environment this is implemented as "main" which provides C-environment control-flow entry.

2. **10ms**: Thread is executed every 10ms and provides the primary periodic software processing.

3. **0.1ms**: Thread is executed every 0.1ms and provides a granular time reference for determining relative time.

4. **Default**: Thread is executed if any unexpected interrupts occur.

The 'Reset' thread is executed out of reset and has the lowest priority.  The '10ms' thread has a priority of 1 and therefore can preempt the 'Reset' thread.  The '0.1ms' thread has a priority of 2 and therefore can preempt both the 'Reset' and '1ms' threads.

### Software Modules
The software is a modular design with no global data access.  The software modules are explained below, and map directly to [source code](/src) file names:

>**adc**: Analog to Digital Converter (ADC) driver.

>**can**: Controller Area Network (CAN) driver.

>**cfg**: Management of configuration data used by the software.  Note: configuration data is readable and writeable through the CAN interface.

>**dio**: Discrete I/O driver.

>**i2c**: Inter-Integrated Circuit (I2C) driver.

>**ina219**: External current/power monitor (INA219) driver.

>**main**: Software executive and C-environment control-flow entry.

>**nvm**: Non-Volatile Memory (NVM) driver.

>**osc**: Oscillator (OSC) driver.

>**pwm**: Pulse-Width Modulation (PWM) driver.

>**rst**: Reset condition detection.  The reset condition is annunciated over the CAN bus so unexpected resets can be identified.

>**servo**: Received CAN messages are processed to determine the servo control type - position or PWM control.  For position control, servo calibration correction is performed.  The determined PWM value is output to the servo and servo status CAN messages are periodically transmitted.

>**tmr**: Timer (TMR) driver.

>**util**: Utility functions.

>**ver**: Version and identification management. Version CAN messages are periodically transmitted to provide node identification.

>**vsense**: VSENSE1/2 signal management. The signals are calibration corrected and their value periodically annunciated in a CAN message.

>**wdt**: Watchdog Timer (WDT) driver.

## Building the Software
The recommended method for building the software is using tool [MPLAB X IDE](http://www.microchip.com/mplabx/) with compiler [MPLAB XC16](http://www.microchip.com/xc16/).  The IDE and compiler are both free software provided by Microchip.

*Note: v3.0 of MPLAB X IDE was used during development.*

*Note: v1.24 of MPLAB XC16 was used during development.*