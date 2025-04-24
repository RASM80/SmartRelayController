# SmartRelayController
8-channel relay controller with SMS control, scheduling, and automation for local/remote device management.



# 8-Channel Relay Controller

![banner](https://github.com/RASM80/SmartRelayController/raw/main/images/overview.jpg?raw=true)


## Overview

The 8-Channel Relay Controller is a flexible automation tool designed to manage up to eight independent relays. Built around the **ATmega64A** microcontroller, this project offers a range of control options tailored for both manual and automated tasks. Whether you're switching devices locally via an intuitive **LCD interface** or remotely through **SMS**, this controller provides reliable operation With support for periodic on/off cycles and custom daily schedules, it’s a versatile solution for precise relay control. 

My goal was to optimize the program’s execution speed on the MCU. However, this required more RAM, and the Atmega32’s limited memory made it insufficient for the task

**C files are written in Codevision platform but migrating to AVR-GCC or other compilers should be effortless.**

![prot](https://github.com/RASM80/SmartRelayController/blob/main/images/proteus_overview.jpg?raw=true)


## Features

- **Multi-Channel Control**: Independently manage up to 8 relays for diverse applications.
- **Local Control**: Navigate and configure settings using an onboard LCD menu and buttons.
- **Remote Control via SMS**: Send commands from anywhere using a GSM module (SIM800C in this prorject).
- **Periodic Cycles**: Automate relay switching with configurable on/off intervals.
- **Custom Timing Schedules**: Set specific start/stop times for each relay throughout the day.
- **Real-Time Clock (RTC)**: Accurate timing with the DS1307 RTC module.
- **Temperature Monitoring**: Integrated DS18B20 sensor for environmental feedback.
- **Persistent Settings**: Backup and restore configurations using an external 24C32 EEPROM.
- **Robust Design**: Built for reliability with error handling and debug support. (Reports Variables and states through the second UART)

## Hardware Requirements

To build and run this project, you'll need the following components:

- **ATmega64A Microcontroller**
- **8-Channel Relay Module**
- **GSM Module (e.g., SIM800C)**: Enables SMS-based remote control. (using any other GSM modules may need tackling with the **initialization** code)
- **DS1307 RTC Module**: Provides RTC via I2C.
- **DS18B20 Temperature Sensor**
- **16x2 Alphanumeric LCD** (connected to PORTC pins).
- **Buttons**: For local navigation and control (wired to PORTA and interrupts) (4 btn for navigation & 2 for singe output On/Off purpose)
- **24C32 External EEPROM**: Stores settings persistently via I2C.

Check the `initialize()` function in `Initialize.c` for specific pin assignments.

## Software Requirements

The firmware is developed for the ATmega64A using the CodeVisionAVR IDE. To compile and upload the code, you'll need:

- **CodeVisionAVR IDE**: The primary development environment (includes its own C compiler).
- **Programmer**: A compatible AVR programmer (e.g., AVRISP mkII or USBasp).
- **Libraries**: Included with CodeVisionAVR for I2C, DS1307, DS18B20, and LCD support.

Alternatively, with some adaptation, the code could be compiled using AVR-GCC, though CodeVisionAVR is recommended for seamless compatibility.

## Installation

### Hardware Setup


3. **Wire the GSM Module**: Link to UART pins (USART0) for SMS functionality.
4. **Hook Up the RTC**: Connect the DS1307 module to the I2C bus (TWI pins).
6. **Connect the LCD**: Wire to PORTC pins as specified in `Initialize.c`. (or change LCD pins as needed)
8. **Attach the EEPROM**: Wire the 24C32 to the I2C bus.
9. **Power the System**: Ensure a stable power supply for all components. (This is especially important if the control board and relay board are connected to the same power supply.

Refer to the code comments, `defines.c` and `Initialize.c` for detailed pin mappings.

### Software Setup

1. **Clone the Repository**: Download this project to your local machine.
   ```bash
   git clone https://github.com/RASM80/SmartRelayController.git
   ```
2. **Open in CodeVisionAVR**: Launch the IDE and load the project (starting with `main.c`).
3. **Configure Settings**: Update the `const_num` array in `GSM.c` with authorized phone numbers for SMS control.
4. **Compile the Code**: Build the project in CodeVisionAVR.
5. **Upload to the Microcontroller**: Use your programmer to flash the compiled firmware onto the ATmega64A.

## Usage

### Local Control

The LCD interface provides a menu-driven system for configuring the controller:

- **System Status**: View the current time, date, and temperature.
- **Commands**: Set timing modes, cycles, or manual states for each relay.
- **Monitor Outputs**: Check the status of all 8 relays (on/off, mode).
- **Set Start/Stop**: Assign a relay to external start/stop buttons. (in my application there are On/Off buttons which can be mapped on one of outputs by this command)

Navigate using the buttons (Up, Down, Submit, Back), as defined in `LCD_Interface.c`. The menu updates dynamically with real-time data when applicable.

### Remote Control via SMS

Send commands to the GSM module’s phone number from an authorized device (listed in `const_num`). Supported commands include:

- **`cycle<output>:<period>-<duty>`**: Set a periodic cycle (values in minutes) (e.g., `cycle1:30-5` turns relay 1 on for 5 minutes every 30 minutes).
- **`switch<output> on/off`**: Manually toggle a relay (This command will forcefully disable scheduled timings) (e.g., `switch2 on`).
- **`time<output>`**: Re-enable scheduled timings for a relay (uses pre-set schedules).
- **`manual<output>`**: Switch to manual mode and prompt for custom schedules (e.g., `manual1`, followed by time slots like `12:00-13:00` + `end` for execution).

**Note**: There are some commands which serve no purpose yet. (They will be implemented in future if needed)

**Note**: `<output>` ranges from 1 to 8. Only SMS from authorized numbers are processed. After execution, the system sends a confirmation SMS listing completed commands.

### Periodic Cycles

Configure relays to switch on and off at regular intervals:
- Via SMS: Use the `cycle` command (e.g., `cycle3:60-10`).
- Via LCD: Navigate to "Commands" > "Cycle," select an output, and set the period and duty cycle.

### Custom Timing

Set specific daily schedules for relays:
- Via SMS: Use `manual<output>` followed by up to 48 time slots (e.g., `12:00-13:00`, terminated with `end`).
- Via Backup: Custom schedules are stored in EEPROM and restored on startup (see `Backup.c`).

![sms_test](https://github.com/RASM80/SmartRelayController/blob/main/images/sms_test.jpg?raw=true)

## Contributing
contributions are welcome!
