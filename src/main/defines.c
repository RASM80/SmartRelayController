
//global
#define OUT_COUNT 8
#define GSMinitTryagain 10
#define xtal 8000000L
#define I2C_TRYAGAIN 10
#define REPORT_SMS_TRYAGAIN 5
#define CMD_COUNT 8
#define TIME_INTERVAL_COUNT 48
#define getSMS_timout_seconds 30
#define getSMS_timer1_cap (getSMS_timout_seconds/5)
#define LCD_TIMOUT_SECONDS 120
#define report_time_s 300
#define BEEP_LENGTH_ms 60
#define LONG_BEEP_LENGTH_ms 150
#define MAX_ITEM_COUNT 8
#define PAST_OUT_SELECT_CMD_COUNT 6 //5 + NULL
#define GET_RX_STR_TIMEOUT_DEFAULT 3
extern unsigned char GET_RX_STR_TIMEOUT = GET_RX_STR_TIMEOUT_DEFAULT;

#define OUT0_PORT PORTD 
#define OUT1_PORT PORTD
#define OUT2_PORT PORTD
#define OUT3_PORT PORTD
#define OUT4_PORT PORTG
#define OUT5_PORT PORTG
#define OUT6_PORT PORTC
#define OUT7_PORT PORTC

#define OUT0_PIN 4 
#define OUT1_PIN 5
#define OUT2_PIN 6
#define OUT3_PIN 7
#define OUT4_PIN 0
#define OUT5_PIN 1
#define OUT6_PIN 0
#define OUT7_PIN 1

void gets1(char *str, unsigned char len);
void puts1(char *str);
void debputs1(char *str);
void debputsf1(char flash *str);

enum start_stop{stop = 8, start = 7};

//system_status weekday strings
enum weekdays{sat, sun, mon, tue, wed, thu, fri};

#define saturday "Sat\0"
#define sunday "Sun\0"
#define monday "Mon\0"
#define tuesday "Tue\0"
#define wednesday "Wed\0"
#define thursday "Thu\0"
#define friday "Fri\0"
extern flash char * weekday_str_list[] = {
    saturday,
    sunday,
    monday,
    tuesday,
    wednesday,
    thursday,
    friday
    }; 
                
//output monitor
#define ON_STATE "On\0"
#define OFF_STATE "Off\0"

extern flash char * output_state_str_list[] = {
    OFF_STATE, ON_STATE
    }; 
          
#define CYCLE_TIMING "Cycle\0"
#define MANUAL_TIMING "Manual\0"

extern flash char * output_timing_mode_str_list[] = { //order is based on the main variables that hold states
    MANUAL_TIMING, CYCLE_TIMING
    };
    
#define TIMED_STATE "Timed\0"
#define SWITCHED_STATE "Switched\0"

extern flash char * output_ctrl_mode_str_list[] = {
    SWITCHED_STATE, TIMED_STATE
    }; 


//EEPROM Locations
//#define



//Uart defines and variables
#define DATA_REGISTER_EMPTY (1<<UDRE0)
#define RX_COMPLETE (1<<RXC0)
#define FRAMING_ERROR (1<<FE0)
#define PARITY_ERROR (1<<UPE0)
#define DATA_OVERRUN (1<<DOR0)


// USART0 Receiver buffer
#define RX_BUFFER_SIZE0 512

#if RX_BUFFER_SIZE0 <= 256
extern unsigned char rx_wr_index0=0,rx_rd_index0=0;
#else
extern unsigned int rx_wr_index0=0,rx_rd_index0=0;
#endif

#if RX_BUFFER_SIZE0 < 256
extern unsigned char rx_counter0=0;
#else
extern unsigned int rx_counter0=0;
#endif

// USART0 Transmitter buffer
#define TX_BUFFER_SIZE0 128

#if TX_BUFFER_SIZE0 <= 256
extern unsigned char tx_wr_index0=0,tx_rd_index0=0;
#else
extern unsigned int tx_wr_index0=0,tx_rd_index0=0;
#endif

#if TX_BUFFER_SIZE0 < 256
extern unsigned char tx_counter0=0;
#else
extern unsigned int tx_counter0=0;
#endif 