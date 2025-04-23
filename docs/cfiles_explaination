# File Explanations

- **A07.c**  
  The main program entry point. It initializes hardware, restores settings, and runs the core loop, managing interrupts, GSM communication, and timing checks. It oversees SMS processing and relay state control.

- **Backup.c**  
  Handles saving and restoring settings and timings to/from an external EEPROM (24C32). It manages data like timing modes, cycle durations, and relay states in case of power outage.

- **cmd.c**  
  Processes SMS or LCD-set commands, such as setting cycles, toggling error reporting, or scheduling manual timings. It **validates** and executes user instructions for relay control.

- **Debug.c**  
  Offers debugging tools, including serial output for monitoring variables and system states. It aids in logging, diagnosing issues, and verifying system behavior during development.

- **defines.c**  
  Centralizes global definitions, constants, and enumerations (e.g., pin mappings, buffer sizes, LCD strings), ensuring consistent configuration across the project.

- **EEPROM.c**  
  Provides low-level I2C functions for EEPROM read/write operations with error handling.

- **GSM.c**  
  Manages GSM module communication for SMS-based control, including sending/receiving messages, parsing commands, and validating sender numbers for remote operation.

- **Initialize.c**  
  Sets up hardware components (ports, interrupts, timers, LCD, RTC) before the main loop, ensuring the microcontroller environment is ready for operation.

- **LCD_Interface.c**  
  Implements the LCD display and button interface, handling menu navigation, status updates, and local user interactions for system control and monitoring.

- **out_select.c**  
  Assists the LCD menu system by managing relay selection for monitoring or configuration. separated for reducing LCD_Interface.c size

- **Timer.c**  
  Controls timing functions, calculating cycle times, comparing schedules, and updating relay states for automated time-based relay operation.

- **Tools.c**  
  Supplies utility functions (e.g., string manipulation, number extraction, UART helpers) used across the project for various support tasks.
