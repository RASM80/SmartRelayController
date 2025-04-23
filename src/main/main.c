
#include <iobits.h>
#include <sleep.h>
#include <mega64a.h>
#include <delay.h>
#include <i2c.h>
#include <ds1307.h>
#include <1wire.h>
#include <ds18b20.h>
#include <alcd.h>
#include <twi.h>
#include <stdio.h>
#include <defines.c>

float temp = 0;

typedef struct 
    {
      unsigned char sec;
      unsigned char min;
      unsigned char hour;
      unsigned char weekDay;
      unsigned char date;
      unsigned char month;
      unsigned char year;  
    }rtc_t;    
rtc_t time;

unsigned char get_rx_string = 0; //for indicating gets function different states
//following char is used to prevent MCU to wait for UART data forever
//conditions are in getchar func
//unit is one second
//default timeout value is GET_RX_STR_TIMEOUT_DEFAULT 3 
//(It's changed in some sections of program so GET_RX_STR_TIMEOUT char is defined to alter it)
unsigned char get_rx_string_timeout = 0;  //return gets function if timeout occurred

bool reportFlag = true;
bit debugFlag = false;
bit adaptiveWatering = false; //Coming soon
bit powerOut_report = false;  //Coming soon
//Time check will get turned off in case of saving setting to prevent any issues
bit time_check = true; 

bool lcd_timer_on = false;
bit uart_interrupt_flag = false;

unsigned char lcd_light_timeout = 0;
bool lcd_light_state = false;
bool menu_active = false;

//8 stacked variables. for 8 OUTs
bit output0 = false;
bit output1 = false;
bit output2 = false;
bit output3 = false;
bit output4 = false;
bit output5 = false;
bit output6 = false;
bit output7 = false;

//used for debug purposes
unsigned int timer_seconds = 0;

struct wateringTime{
    unsigned char H, M;
};
struct wateringTime startOut[OUT_COUNT][TIME_INTERVAL_COUNT];
struct wateringTime stopOut[OUT_COUNT][TIME_INTERVAL_COUNT];

unsigned int durationMin[OUT_COUNT];
unsigned char cycleMin[OUT_COUNT];
bool timing_mode[OUT_COUNT]; //0 = manual, 1 = sequential
bool output_state[OUT_COUNT]; //0 = SWITCH_MANUAL, 1 = TIMING
enum timing_mode{MANUAL, SEQUENCE};
enum output_state{SWITCH_MANUAL, TIMING};
//timing_mode is used to know that what timing is sued for output
// (manual timings or sequential ones)
//output state is for operating mode
// (Completely manual (switch commands) or use either 48timing modes)
//eeprom unsigned char switch_key_out_index; 
unsigned char switch_key_out_index = 8;

//1.cycle 2.error 3.aw 4.dr 5.wr 6.mr 7.pbo 8.manual 9.switching 10.timing, more will be added
//all gsm indicators are reset before getting used
enum CMD{NULL_CMD, cycle_cmd, error, aw, dr, wr, mr, pbo, manual, timing=10, switching = 9};
unsigned char contact_number_index = 0;   //Multiple numbers are considered valid
unsigned char cmd_count = 0;
unsigned char cmdBuf[20][CMD_COUNT];  //Holds commands strings (for extracting info)
unsigned char cmdBuf_ident[CMD_COUNT];//Command execution index code
//valid CMDs will get executed
bool cmdValid[CMD_COUNT];
//this array will get reported to the user
bool cmdDone[CMD_COUNT]; //for report purpose
//to know which cmd is for which output (relay)
char cmd_output_index[CMD_COUNT];
//set to 1 if a cmd changed the output timings (Used for backing up timnings) (lookup in backup section)
bool output_cmd_change[CMD_COUNT]; 

//only for debug purposes (For monitoring EEPROM errors)
enum readChar_diag{noError_r, first_start_r, rom_address_write_r,
    msb_address_r, lsb_address_r, second_start, rom_address_read};  
enum writeChar_diag{noError_w, first_start_w, rom_address_write_w,
    msb_address_w,  lsb_address_w, write_char, write_int_m, write_int_l};
enum romOP{write_rom, read_rom};


void deput_time();
void turnOn_output(unsigned char output_index){//changes bits only
    switch(output_index){
        case 0:
            if(output0) return;
            //SETBIT(OUT0_PORT, OUT0_PIN);
            OUT0_PORT |= (1 << OUT0_PIN);
            output0 = true;
            break;    
        case 1:
            if(output1) return;
            //SETBIT(OUT1_PORT, OUT1_PIN);
            OUT1_PORT |= (1 << OUT1_PIN);
            output1 = true;
            break;        
        case 2:
            if(output2) return;
            //SETBIT(OUT2_PORT, OUT2_PIN);
            OUT2_PORT |= (1 << OUT2_PIN);
            output2 = true;
            break;
        case 3:
            if(output3) return;
            //SETBIT(OUT3_PORT, OUT3_PIN);
            OUT3_PORT |= (1 << OUT3_PIN);
            output3 = true;
            break;
        case 4:
            if(output4) return;
            //SETBIT(OUT4_PORT, OUT4_PIN);
            OUT4_PORT |= (1 << OUT4_PIN);
            output4 = true;
            break;
        case 5:
            if(output5) return;
            //SETBIT(OUT5_PORT, OUT5_PIN);
            OUT5_PORT |= (1 << OUT5_PIN);
            output5 = true;
            break;
        case 6:
            if(output6) return;
            //SETBIT(OUT6_PORT, OUT6_PIN);
            OUT6_PORT |= (1 << OUT6_PIN);
            output6 = true;
            break;
        case 7:
            if(output7) return;
            //SETBIT(OUT7_PORT, OUT7_PIN);
            OUT7_PORT |= (1 << OUT7_PIN);
            output7 = true;
            break;
        default:
            debputsf1("out of index turn on output\r\n");
    }
    return;
}
void turnOff_output(unsigned char output_index){
    switch(output_index){
        case 0:
            if(!output0) return;
            //CLRBIT(OUT0_PORT, OUT0_PIN);
            OUT0_PORT &= ~(1 << OUT0_PIN);
            output0 = false;
            break;    
        case 1:
            if(!output1) return;
            //CLRBIT(OUT1_PORT, OUT1_PIN);
            OUT1_PORT &= ~(1 << OUT1_PIN);
            output1 = false;
            break;        
        case 2:
            if(!output2) return;
            //CLRBIT(OUT2_PORT, OUT2_PIN);
            OUT2_PORT &= ~(1 << OUT2_PIN);
            output2 = false;
            break;
        case 3:
            if(!output3) return;
            //CLRBIT(OUT3_PORT, OUT3_PIN);
            OUT3_PORT &= ~(1 << OUT3_PIN);
            output3 = false;
            break;
        case 4:
            if(!output4) return;
            //CLRBIT(OUT4_PORT, OUT4_PIN);
            OUT4_PORT &= ~(1 << OUT4_PIN);
            output4 = false;
            break;
        case 5:
            if(!output5) return;
            //CLRBIT(OUT5_PORT, OUT5_PIN);
            OUT5_PORT &= ~(1 << OUT5_PIN);
            output5 = false;
            break;
        case 6:
            if(!output6) return;
            //CLRBIT(OUT6_PORT, OUT6_PIN);
            OUT6_PORT &= ~(1 << OUT6_PIN);
            output6 = false;
            break;
        case 7:
            if(!output7) return;
            //CLRBIT(OUT7_PORT, OUT7_PIN);
            OUT7_PORT &= ~(1 << OUT7_PIN);
            output7 = false;
            break;
        default:
            debputsf1("out of index turn off output\r\n");
    }
    return;
}
void getTime();
void timeCheck();
void debug_report();
void key_function(unsigned char key);
void debputsdec1(unsigned char *str, unsigned char len);
void reset_menu();
void fillZero(char *str, unsigned char size);
void debputs_lcd_buf();
void start_stop_key(unsigned char key);
unsigned char find_key(){
    unsigned char input = 0;
    /*
    DDRA = 0x00;
    PORTA = 0xFF;
    SETBIT(DDRE, 7);
    CLRBIT(PORTE, 7);
    */
    input = 0b00111111 & ~PINA;  
    switch(input){
        case 1:     return 1;
        case 2:     return 2;
        case 4:     return 3;
        case 8:     return 4;
        case 16:    return 5;
        case 32:    return 6;
        case 64:    return 7;
        case 128:   return 8;
        default: return 0;        
    }
    return 0;               
}
unsigned char writeChar(unsigned int address, unsigned char write);
unsigned char diagnoseRom(unsigned char errCode, unsigned char opMode);
unsigned char readChar(unsigned char* data, unsigned int address);
unsigned char backup_flags();
void lcdputsf(char flash *str);

//used with the external Start/Stop pair of buttons
interrupt [EXT_INT0] void ext_int0_isr(void)
{
    start_stop_key(start);
    debputsf1("IT0\r\n");    
}

//used with the external Start/Stop pair of buttons
interrupt [EXT_INT1] void ext_int1_isr(void)
{
    start_stop_key(stop); 
    debputsf1("IT1\r\n");    
}


//debug purposes only
interrupt [EXT_INT4] void ext_int4_isr(void)
{
    debug_report();
    debputsf1("IT4\r\n");
}

//Connected to the switch to control the debug report
interrupt [EXT_INT5] void ext_int5_isr(void)
{   
    if( TSTBIT(PINE, 5) )
        debugFlag = false;
    else
        debugFlag = true;    
    debputsf1("IT5\r\n");
}




#include <LCD_Interface.c>
void init_button_timer(unsigned char state){

    if(state == on){

        //turn off INT7 to stop further interrupts
        EIMSK &= ~(1<<INT7);
        EIFR &= ~(1<<INT7);

        DDRA = 0b11000000;    //distinct mode
        PORTA = 0b00111111;
        SETBIT(DDRE, 7);
        CLRBIT(PORTE, 7);

        // Timer/Counter 0 initialization
        // Clock source: System Clock
        // Clock value: 7.813 kHz
        // Mode: Normal top=0xFF
        // OC0 output: Disconnected
        // Timer Period: 32.768 ms
        ASSR=0<<AS0;
        TCCR0=(0<<WGM00) | (0<<COM01) | (0<<COM00) | (0<<WGM01) | (1<<CS02) | (1<<CS01) | (1<<CS00);
        TCNT0=0x00;
        OCR0=0x00;

        TIMSK |= (1<<TOIE0);
    }
    else if (state == off){

        //restore INT7 settings
        EIMSK |= (1<<INT7);
        EIFR |= (1<<INT7);

        DDRA = 0b11111111;     //common mode
        PORTA = 0b00000000;
        SETBIT(PORTE, 7);
        CLRBIT(DDRE, 7);
        
        //turn off key scanning timer
        ASSR=0<<AS0;
        TCCR0=(0<<WGM00) | (0<<COM01) | (0<<COM00) | (0<<WGM01) | (0<<CS02) | (0<<CS01) | (0<<CS00);
        TCNT0=0x00;
        OCR0=0x00;

        TIMSK &= ~(1<<TOIE0);
    }
    else debputsf1("\r\n !!! wrong init_button_timer input\r\n");
    return;
}
interrupt [EXT_INT7] void ext_int7_isr(void)
{

    lcd_light_timeout = 0;
    SETBIT(PORTC, 5);  //lcd_light
    lcd_light_state = true; //brought to the end of block to work with the double_push input needed for init
                            //(pushing any button 2 times will initialize the menu)

    init_button_timer(on);
    
    debputsf1("IT7\r\n");
}



char rx_buffer0[RX_BUFFER_SIZE0];
char tx_buffer0[TX_BUFFER_SIZE0];
// This flag is set on USART0 Receiver buffer overflow
bool rx_buffer_overflow0;

//UART Interrupt makes the process more robust and saves time for MCU by a lot
interrupt [USART0_RXC] void usart0_rx_isr(void)
{
    char status,data;   
    
    uart_interrupt_flag = true;
    
    status=UCSR0A;
    data=UDR0;
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0){
       rx_buffer0[rx_wr_index0++]=data;
    #if RX_BUFFER_SIZE0 == 256
       // special case for receiver buffer size=256
       if (++rx_counter0 == 0) rx_buffer_overflow0=1;
    #else
       if (rx_wr_index0 == RX_BUFFER_SIZE0) rx_wr_index0=0;
       if (++rx_counter0 == RX_BUFFER_SIZE0)
          {
          rx_counter0=0;
          rx_buffer_overflow0=1;
          }
    #endif
       }
}

#ifndef _DEBUG_TERMINAL_IO_
#define _ALTERNATE_GETCHAR_
#pragma used+
char getchar(void)
{
char data;
while (rx_counter0==0){
    if(get_rx_string >= 3){
        get_rx_string = 1;
        get_rx_string_timeout = 0;
    }
    if(get_rx_string == 1)
        if(get_rx_string_timeout >= GET_RX_STR_TIMEOUT)
            return 0xFF;
}
data=rx_buffer0[rx_rd_index0++];
#if RX_BUFFER_SIZE0 != 256
if (rx_rd_index0 == RX_BUFFER_SIZE0) rx_rd_index0=0;
#endif
#asm("cli")
--rx_counter0;
#asm("sei")
return data;
}
#pragma used-
#endif


interrupt [USART0_TXC] void usart0_tx_isr(void)
{
if (tx_counter0)
   {
   --tx_counter0;
   UDR0=tx_buffer0[tx_rd_index0++];
#if TX_BUFFER_SIZE0 != 256
   if (tx_rd_index0 == TX_BUFFER_SIZE0) tx_rd_index0=0;
#endif
   }
}

#ifndef _DEBUG_TERMINAL_IO_
#define _ALTERNATE_PUTCHAR_
#pragma used+
void putchar(char c)
{
while (tx_counter0 == TX_BUFFER_SIZE0);
#asm("cli")
delay_ms(5);
if (tx_counter0 || ((UCSR0A & DATA_REGISTER_EMPTY)==0))
   {
   tx_buffer0[tx_wr_index0++]=c;
#if TX_BUFFER_SIZE0 != 256
   if (tx_wr_index0 == TX_BUFFER_SIZE0) tx_wr_index0=0;
#endif
   ++tx_counter0;
   }
else
   UDR0=c;
#asm("sei")
}
#pragma used-
#endif



// USART1 Transmitter buffer
#define TX_BUFFER_SIZE1 32
char tx_buffer1[TX_BUFFER_SIZE1];

#if TX_BUFFER_SIZE1 <= 256
unsigned char tx_wr_index1=0,tx_rd_index1=0;
#else
unsigned int tx_wr_index1=0,tx_rd_index1=0;
#endif

#if TX_BUFFER_SIZE1 < 256
unsigned char tx_counter1=0;
#else
unsigned int tx_counter1=0;
#endif


interrupt [USART1_TXC] void usart1_tx_isr(void)
{

if (tx_counter1)
   {
   --tx_counter1;
   UDR1=tx_buffer1[tx_rd_index1++];
#if TX_BUFFER_SIZE1 != 256
   if (tx_rd_index1 == TX_BUFFER_SIZE1) tx_rd_index1=0;
#endif
   }
}

//UART1 is used for debug purposes and in case of new issues it will help locating the cause
#pragma used+
void putchar1(char c)
{
while (tx_counter1 == TX_BUFFER_SIZE1);
#asm("cli")
if (tx_counter1 || ((UCSR1A & DATA_REGISTER_EMPTY)==0))
   {
   tx_buffer1[tx_wr_index1++]=c;
#if TX_BUFFER_SIZE1 != 256
   if (tx_wr_index1 == TX_BUFFER_SIZE1) tx_wr_index1=0;
#endif
   ++tx_counter1;
   }
else
   UDR1=c;
#asm("sei")
}
#pragma used-


interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    //This timer is used to manage buttons
    //The reason for implementing timer for reading buttons is to debounce them 
    /* First input after menu timeout (no input) will trigger the 
       EXT INT 7 (common end of buttons)which initializes the menu*/
    // after that buttons will be monitored by this timer (Timer0)  
    
    //Timer Period: 32.768 ms
    
    static unsigned char key;
    static unsigned char shift_count;
    static unsigned char prev_key;
    static unsigned char lcd_time_stack = 0;
      
    key = find_key();
    
    // if timer sees the same input for many times
    // after a certain amount it will run the button function

    if(key == 0 || (prev_key != 0 && prev_key != key)){
        shift_count = 0;
        prev_key = 0;
    }
    shift_count++;

    if(shift_count == 4){  //only runs at the forth continous timer cycles (not less not more)


        lcd_light_timeout = 0;
        key_function(key);

        }
    prev_key = key;

    if(lcd_timer_on){
    //merged this Timer and dynamic_lcd_functions timer to make the system more robust 
    //Timer3 has too many operations it's better to keep ISRs short and simple

        if(lcd_time_stack >= 31){ //32 * 32.768 ms = 1048ms
        
            //does the special function if assigned (no_task, monitor_output & system_status functions)
            (*lcd_timer_function)();
            refresh_lcd();
            lcd_time_stack = 0;
        }
        lcd_time_stack++;
    }
}


interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{      
    TCNT1H=0x79B9 >> 8;
    TCNT1L=0x79B9 & 0xff;
    //-- ~1 second ------------------
    
    //one second timer used for UART recieve timeout
    
    //gets reset in getchar func
    get_rx_string_timeout++;
    return;       
}


interrupt [TIM3_OVF] void timer3_ovf_isr(void)
{   
    TCNT3H=0xE17C >> 8;
    TCNT3L=0xE17C & 0xff;
    //-------------------- 
    
    // Timings, Relays, LCD light timeout are checked in this timer
    
    
    //reset menu and turn off lcd light if time is out
    if(lcd_light_timeout >= LCD_TIMOUT_SECONDS){
        init_button_timer(off);
        CLRBIT(PORTC, 5);   //turn off lcd light
        lcd_light_state = false;
        reset_menu();
        lcd_light_timeout = 0; 
               
    }
    
    //Get temperature for dynamic system status report on LCD 
    if(lcd_timer_on && (lcd_timer_function == &system_status))
        temp = ds18b20_temperature(0);
      
    //keep track of seconds for LCD light 
    if(lcd_light_state)
        lcd_light_timeout++;
             
    //update time variables using RTC
    if(time_check) //made conditional to prevent interfere when saving settings.
        getTime();
        
    //Print output stat (Debug purpose)
    if(timer_seconds >= report_time_s){
        deput_time();
        debputs1("Output stats:\r\n");
        timer_seconds = 0;
        }
    
    //Main purpose of the timer:
    //Check the time and compare with all timings of all timed Relays and switch them On/Off if needed
    timeCheck();
    timer_seconds++;         
    
    return;
}


#include <Tools.c>
#include <Timer.c>
#include <GSM.c>
#include <cmd.c>
#include <EEPROM.c>
#include <Backup.c>


#include <Debug.c>
#include <Initialize.c>


void restore_time(){
//update RTC time using gsm local time
    char tbuf[30];
    char i = 0; 
    char hour, min, sec;
    
    
    get_rx_string = 6;
    putsf("AT+CCLK?\r\n"); 
    gets1(tbuf, 15);
    fillZero(tbuf, 15);
    
    gets1(tbuf, 30);
    
    get_rx_string = GET_RX_STR_TIMEOUT_DEFAULT;
    
    if(tbuf[0]=='+', tbuf[1]=='C', tbuf[2]=='C'){
        for(i=0; i<30; i++){
            if(tbuf[i] == ','){
                hour = (tbuf[i+1]-48)*10 + (tbuf[i+2]-48);
                min = (tbuf[i+4]-48)*10 + (tbuf[i+5]-48);
                sec = (tbuf[i+7]-48)*10 + (tbuf[i+8]-48);
                
                break;
            }    
        }
        rtc_set_time(hour,min,sec);
        debputsf1("Time is set successfully\r\n");
    }
    else {
        debputsf1("Time is set unsuccessfully\r\nCCLK: ");
        debputs1(tbuf);
        debputsf1("\r\n");
        }
    
    gets1(tbuf, 4); 
}
//Debug purpose
void debug_report(){
    unsigned char out = 0, i =0;
    debputsf1("\r\n REPORTING VARIABLES\r\n\r\n");
    
    
    
    //unsigned int durationMin[OUT_COUNT];
    //unsigned char cycleMin[OUT_COUNT];
    puts1("\r\n Cycles:\r\n");
    for(i=0; i<CMD_COUNT; i++){
        printf("dur%d:%d, cyc:%d:%d\r\n",i, durationMin[i], i, cycleMin[i]);
    }puts1("\r\n");
    
    puts1("\r\n Timings:\r\n");
    for(i=0; i<CMD_COUNT; i++){
        printf("\r\nTIMING %d\r\n",i);
        printCycle(i);
    }puts1("\r\n\r\n");
    
    
    
    
    printf("res:%d rep:%d aw:%d pbo:%d\r\n\r\n", out, reportFlag, adaptiveWatering, powerOut_report);  
        
    puts1("commands");
    for(i=0; i<CMD_COUNT; i++){
        printf("%d:%d ",i, cmdBuf_ident[i]);
    }puts1("\r\n");
        
    puts1("valids");
    for(i=0; i<CMD_COUNT; i++){
        printf("%d:%d ",i, cmdValid[i]);
    }puts1("\r\n");
        
    puts1("done");
    for(i=0; i<CMD_COUNT; i++){
        printf("%d:%d ",i, cmdDone[i]);
    }puts1("\r\n");
        
    
    puts1("cmd_output_index: ");
    for(i=0; i<CMD_COUNT; i++){
        printf("%d:%d ",i, cmd_output_index[i]);
    }puts1("\r\n");
    
    puts1("output_cmd_change: ");
    for(i=0; i<CMD_COUNT; i++){
        printf("%d:%d ",i, output_cmd_change[i]);
    }puts1("\r\n");
    
    puts1("timing_mode: ");
    for(i=0; i<CMD_COUNT; i++){
        printf("%d:%d ",i, timing_mode[i]);
    }puts1("\r\n");
    
    puts1("output_state: ");
    for(i=0; i<CMD_COUNT; i++){
        printf("%d:%d ",i, output_state[i]);
    }puts1("\r\n");
    
    
    puts1("output On/Off: ");
    printf("0:%d ", output0);
    printf("1:%d ", output1);
    printf("2:%d ", output2);
    printf("3:%d ", output3);
    printf("4:%d ", output4);
    printf("5:%d ", output5);
    printf("6:%d ", output6);
    printf("7:%d ", output7);
    puts1("\r\n");
}                         

//Debug purpose
void check_reset_source(unsigned char reset_src){
    unsigned char buf[5];
    reset_src &= 0b00011111;
    switch(reset_src){
        case 1:
            debputsf1("\r\n\r\nPOWER-ON RESET\r\n\r\n");
            break;
        case 2:
            debputsf1("r\n\r\nEXTERNAL RESET\r\n\r\n");
            break;
        case 4:
            debputsf1("r\n\r\nBROWN-OUT RESET\r\n\r\n");
            break;
        case 8:
            debputsf1("r\n\r\nWATCHDOG RESET\r\n\r\n");
            break;
        case 16:
            debputsf1("JTAG RESET");
            break;
        default: debputsf1("\r\n\r\nNO RESET SORUCE FOUND\r\n\r\n");
    }
    debputsf1("MCUCSR(reset source): ");
    sprintf(buf, "0x%02X", reset_src); 
    debputs1(buf);
    debputsf1("\r\n");    
}

//if any input from UART is detected, it will be reviewed by this functio
//false input is ignored and valid inputs that are instructions from user are gathered and executed
void gsm_instructions(){
    //any error code returned by nested functions will get reported to the debug Serial port(UART1) by following str
    static unsigned char sms_status_buf[4];
    
    //This var will keep the validity of the recieved SMS
    static unsigned char sms_status = false; 
    
      
    
    debputsf1("Recieving SMS\r\n");
    lcdputsf("Getting sms"); 
    
    //All interactions with GSM including: Reading SMS notif and gsm memory index, checking the number,  
    //reading SMS lines, validating instructions, deleting the msg after extracting info 
    sms_status = getSMS();
    
    //After extracting all the info needed from the msg, instructions will get executed
    // New settings and timings are also saved on the external EEPROM
    if(sms_status == true){
        debputsf1("\r\nSMS Recieved and valid\r\n");    
        lcdputsf("Execute cmd");
        debputsf1("Executing SMS\r\n");
        executeCMD();
        debputsf1("Execution done\r\n");
        
        //#asm("wdr")
        lcdputsf("Backup setting");
        debputsf1("Backup setting\r\n");
        backupSetting();
        lcdputsf("Backup done");
        debputsf1("Backup done\r\n");
        
        debputsf1("Sending execution report SMS\r\n");
        report_execution_result();
        debputsf1("execution report sent\r\n\r\n----------\r\n\r\n");
         
        //debug_report();   
    }
    else{
        debputsf1("\r\nSMS recieve Error\r\n");
        debputsf1("Error code: ");
        sprintf(sms_status_buf, "%d", sms_status);
        debputs1(sms_status_buf);
        fillZero(sms_status_buf, 4); 
        
        debputsf1("\r\n");    
    }
       
    //empty the long uart rx string to start clean and prevent overflow issues
    flush_uart0_rx();
}


void main_initialize(){
    
    //If gsm init fails, MCU will not update the RTC time using gsm local time
    bool gsm_working = false;        
    
    //used for debugging purposes
    //if MCU fails to initialize reset source can be the cause
    unsigned char reset_source = MCUCSR;
    MCUCSR = 0;     
    
    //The Oscillator Calibration Register  used for tuning Internal oscilator
    //Calibration value is saved on the flash
    //It's loaded on the OSCCAL register at the beginning of program
    /*the Calibration value is measured ny the manufacturer and written on the signature byte 
      which can be read with SPI or JTAG programmer*/ 
    //it's used to achieve +-1% accuracy with internal RC oscilator
    OSCCAL = 0x9B;
    
    //A few NOPs are considered to prevent any conflict since OSCCAL is changed
    #asm
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
    #endasm
    delay_us(500); //for more stability
         
    //in case of reset clear any items left
    lcd_clear();        
    lcdputsf("POWER ON"); 
    
    #asm("SEI") 
    //initializing Atmega peripherals, Interrupts and etc  
    initialize(); //timer init separated to prevent error while initializing other sections
    initialize_gets_timeout_timer();
    
    //restore debug flag state using PINE5
    initialize_debug_flag(); 
     
    //debug purposes
    check_reset_source(reset_source); 
    
    // switch key index is for pair of On/Off push buttons to know which output to control
    /*It's setting is restored before anything else 
    to get the On/Off switches working ASAP (Interrupt is already inited)*/
    // the index can changed by the LCD interface
    restore_switch_key_index(); 
     
    //GSM Initialization
    debputsf1("Initializing GSM\r\n");
    gsm_working = initialize_sim800();
    debputsf1("GSM main init function finished\r\n");
    lcdputsf("GSM Inited");
       
    //disabling main Interrupt flag to prevent any conflicts with restoring process
    #asm("CLI")
                      
    
    //restoring last saved necessary flags and timings to get back to working
    lcdputsf("Restore setting");
    debputsf1("Restore setting\r\n");
    restoreSetting(); 
    lcdputsf("Setting restored");
    debputsf1("Setting restored\r\n");   
                
    //Update RTC time using gsm local time (AT+CCLK)
    debputsf1("Restore time\r\n");
    if(gsm_working){
        restore_time();//all timers init
        debputsf1("Time is set\r\n");
        }
    else{
        debputsf1("GSM not working. system time may be inaccurate\r\n");
    }
    
    //reseting lcd interface
    
    debputsf1("init timer\r\n");
    initialize_lcd_interface();
    initialize_timers();//all timers init
    debputsf1("Timer and Timer Int inited\r\n"); 
    
    lcdputsf("All Inited");
    debputsf1("\r\n\r\n Main initialization finished \r\n\r\n");
        
    //emptying uart0 char list (mainly used after each SMS instruction set to start clean)
    flush_uart0_rx();
              
    #asm("sei")  
    return;    
}


void main(void)
{                                                        
    main_initialize();
    
    //Turning on the blue LED to notify the user 
    //that MCU has passed the initialization section
    PORTC |= 0b00010000; //ON led   
      
    
    while(true){
        lcdputsf("IDLE");
                  
        //MCU will stay in the following loop until there is a new SMS (or even noise on the UART0 RX)
        #asm("sei") //just to make sure
        debputsf1("\r\n\r\n SLEEP\r\n\r\n");
        do{
            uart_interrupt_flag = false; //false if interrupts any other than uart recieve happen       
            
            //No Sleep is needed since the application is not low power, but it's done anwyay 
            MCUCR = 0b00100000;
            #asm("SLEEP")
            MCUCR = 0b00000000;
            
            }while(!uart_interrupt_flag);
        debputsf1("\r\n WAKE\r\n\r\n");
         
        //The long process of recieving, extracting info, executing and backing up new stuff
        gsm_instructions();
                        
        }
    
}