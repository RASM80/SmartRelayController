//LCD INTERFACE
//ALCD and buttons related stuff are in this section

//There are 2 types of sections in menus
//Static and Dynamic sections
//Static sections are easily managed by structs created to contain menu's infos
//(Menu titles are saved in Flash to save RAM space)
//Dynamic ones are a bit more difficult. In these sections, items on the screen get updated
//Dynamic sections are done using timers (System status, output stats are the dynamic sections)

/*
    Menus 
        - System Status (time, date, temp)       
        - Commands
        - Monitor outputs 
        - Set start/stop       
*/

/*
    Sub menus
        Commands
         - Time                 
            (8 _ from output 1 to 8)
         - Cycle                
            (8 _ from output 1 to 8)
         - Manual                         -
            (8 _ from output 1 to 8)
         - Switch               
            (8 _ from output 1 to 8)
         - Power alarm          
            (On - Off)
         - Adaptive watering    
            (On - Off)
            - All have confirming
        
        Monitor outputs 
         - 8 _ from output 1 to 8
        
               
*/

/*
extern void (*command[11])(char) = {
    ERROR_CMD,
    cycle_command, error_command, aw_command, dr_command,
    wr_command, mr_command, pbo_command, manual_command,
    switch_command, time_command
    };
*/

extern unsigned char index = 0; //used in Timer2
bool backup_output_switch();

typedef void (*menu_func)();
typedef struct
{
    flash char * label;
    menu_func action;
} menu_item;
typedef struct
{
    flash char * title;
    char item_count;
    menu_item * items[MAX_ITEM_COUNT];
    //Menu *previous_menu;
} menu;


extern unsigned char output_index = 0; //used in output monitor and commands.
bool submit_selection = false; //used in (two state flags) On/Off settings
//extern for Timer2 access (Monitor output)
//unsigned char item_number = 0;
extern char lcd_buf[2][17] = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}; 
 
    
menu *current_menu[4] = {0,0,0,0};
unsigned char selected_item_number[4] = {0,0,0,0};
menu_item *selected_item;   
enum lcd_timer {off, on};



void lcd_timer(unsigned char state){
    #asm("cli");
    //set timer
    if(state == on){
        debputsf1("Timer 2 On\r\n");
        lcd_timer_on = true;
        //TIMSK |=(0<<OCIE2) | (1<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<OCIE0) | (0<<TOIE0);
    }
    else if(state == off){
        debputsf1("Timer 2 Off\r\n");
        lcd_timer_on = false;
        //TIMSK &= (1<<OCIE2) | (0<<TOIE2) | (1<<TICIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<TOIE1) | (1<<OCIE0) | (1<<TOIE0);
    } 
    else debputsf1("!!! LCD Timer out of index input\r\n");
    
    #asm("sei");
}
void beep(){
    SETBIT(PORTG, 2);
    delay_ms(BEEP_LENGTH_ms);  
    CLRBIT(PORTG, 2);
}
void long_beep(){
    SETBIT(PORTG, 2);
    delay_ms(LONG_BEEP_LENGTH_ms);  
    CLRBIT(PORTG, 2);
}

//dynamic LCD functions
void no_task(){return;}
void system_status(){
    /*
    typedef struct {
      unsigned char sec;
      unsigned char min;
      unsigned char hour;
      unsigned char weekDay;
      unsigned char date;
      unsigned char month;
      unsigned char year;  
      }rtc_t;    
    rtc_t time;
    */
    
/*    sprintf(&lcd_buf[0][0],"%.16p", (weekday_str_list+2*time.weekDay));
    debputsf1("WEEKDAY STR:\r\n");
    debputs1(&lcd_buf[0][0]);  */
    
    sprintf(&lcd_buf[0][0],"%02d:%02d:%02d %.6f", time.hour, time.min, time.sec, temp);
    sprintf(&lcd_buf[1][0],"%p %02d/%02d/%02d", (weekday_str_list[time.weekDay]), 
    time.year, time.month, time.date);   
    return;
}
bool return_output_state(char output_index){
    switch(output_index){
        case 0:
            return output0;    
        case 1:
            return output1;
        case 2:
            return output2;
        case 3:
            return output3;
        case 4:
            return output4;
        case 5:
            return output5;
        case 6:
            return output6;
        case 7:
            return output7;
        default:
            debputsf1("! out_stat error\r\n");
            return false;
    }
    return false;
}
void monitor_output(){
    sprintf(&lcd_buf[0][0],"Output%d, %-6p", output_index+1, output_state_str_list[return_output_state(output_index)]);
    sprintf(&lcd_buf[1][0],"%p, %-p", 
    output_timing_mode_str_list[timing_mode[output_index]], output_ctrl_mode_str_list[output_state[output_index]]);
    return;
}
extern void  (*lcd_timer_function)(void) = &no_task;   
//used for special sections of menu which need constant updating
// ex: system_stats, monitor_outs, ...


enum shift{left, right};
void shift_current_menu(char shift_side){
    
    if(shift_side == right){
        current_menu[3] = current_menu[2];
        current_menu[2] = current_menu[1];
        current_menu[1] = current_menu[0];
        
        selected_item_number[3] = selected_item_number[2];
        selected_item_number[2] = selected_item_number[1];
        selected_item_number[1] = selected_item_number[0];
        //selected_item_number[0] = 0;
    }
    else if(shift_side == left){
        current_menu[0] = current_menu[1];
        current_menu[1] = current_menu[2];
        current_menu[2] = current_menu[3];
        current_menu[3] = 0;
        
        selected_item_number[0] = selected_item_number[1];
        selected_item_number[1] = selected_item_number[2];
        selected_item_number[2] = selected_item_number[3]; 
        selected_item_number[3] = 0;
        
        selected_item = (current_menu[0]->items[selected_item_number[0]]); 
        //roll back to previous menu and hover prev selected item
            
    }
    else debputsf1("\r\n !!! Wrong shift_current_menu input\r\n");
    return;
}


void refresh_lcd(){
    lcd_gotoxy(0, 0);
    for(index=0; index<16; index++)
        lcd_putchar(lcd_buf[0][index]);
    
    for(index=0; index<16; index++)
        lcd_putchar(lcd_buf[1][index]);
    return;
}
void update_lcd_buffer(){
    sprintf(&lcd_buf[0][0], "%-16p", current_menu[0]->title);
    sprintf(&lcd_buf[1][0], "%-16p", selected_item->label); 
}


bool cycle_cmd_flag = false;
unsigned char horiz_cyc_index = 0;
unsigned char variable_number = 0; //for assigning numbers to Duration and Duty cycle

unsigned int lcd_duration = 0;
unsigned char lcd_duty_cyc = 0; 


void update_cycle_lcd(){
    sprintf(&lcd_buf[1][0], "  %03d  ||  %03d  ", lcd_duration, lcd_duty_cyc);
    lcd_buf[1][horiz_cyc_index] = 126;
    refresh_lcd();
    return;
}
void cyc_up_key(){
    switch(horiz_cyc_index){
        
        case 1:
            if(lcd_duration < 900)
                lcd_duration += 100;
            break;    
        case 2:
            if(lcd_duration < 990)
                lcd_duration += 10;
            break;
        case 3:
            if(lcd_duration < 999)
                lcd_duration += 1;
            break;
        
        case 10:
            if(lcd_duty_cyc < 155)
                lcd_duty_cyc += 100;
            break;    
        case 11:
            if(lcd_duty_cyc < 245)
                lcd_duty_cyc += 10;
            break;
        case 12:
            if(lcd_duty_cyc < 254)
                lcd_duty_cyc += 1; 
            break;
    }
    
    update_cycle_lcd();
    
    return;
}
void cyc_down_key(){
    switch(horiz_cyc_index){
        
        case 1:
            if(lcd_duration > 100)
                lcd_duration -= 100;
            break;    
        case 2:
            if(lcd_duration > 10)
                lcd_duration -= 10;
            break;
        case 3:
            if(lcd_duration > 1)
                lcd_duration -= 1;
            break;
        
        case 10:
            if(lcd_duty_cyc > 100)
                lcd_duty_cyc -= 100;
            break;    
        case 11:
            if(lcd_duty_cyc > 10)
                lcd_duty_cyc -= 10;
            break;
        case 12:
            if(lcd_duty_cyc > 1)
                lcd_duty_cyc -= 1; 
            break;
    }
    
    update_cycle_lcd();
    
    return;
}
void cyc_right_key(){
    // 1, 2, 3, 10, 11, 12
    if(horiz_cyc_index == 3)
        horiz_cyc_index = 10;
    else if(horiz_cyc_index == 14);
    else horiz_cyc_index++;
    
    update_cycle_lcd();
    
    return;
}
void cyc_left_key(){
    // 1, 2, 3, 10, 11, 12
    if(horiz_cyc_index == 10)
        horiz_cyc_index = 3;
    else if(horiz_cyc_index == 1);
    else horiz_cyc_index--;
    
    update_cycle_lcd();
    
    return;
}

void setCycleTime(int wholeCycle,char dutyCycle, unsigned char output_index);
bool backup_watering_cycles(unsigned char out_index);
bool backup_timing_mode();
void cyc_submit_key(){

    
    timing_mode[output_index] = SEQUENCE;
    durationMin[output_index] = lcd_duration;
    cycleMin[output_index] = lcd_duty_cyc;
    setCycleTime(lcd_duration, lcd_duty_cyc, output_index);
    
    backup_timing_mode();
    backup_watering_cycles(output_index);

    cycle_cmd_flag = false;
    long_beep();
    shift_current_menu(left);
    shift_current_menu(left);
    update_lcd_buffer();
    refresh_lcd();
    
    return;
}
void cyc_back_key(){
    cycle_cmd_flag = false; //disable cycle cmd mode
    
    shift_current_menu(left);
    update_lcd_buffer();
    refresh_lcd();
    
    return;
}



void init_menu();
void NO_CMD(){};
void sys_stat();
void lcd_cmds();
void pastOS_monitor_outs(){
    lcd_timer_function = &monitor_output;
    lcd_timer(on);
    return;
}
void monitor_outs();

void time_command(char index);
bool backup_output_state();
void pastOS_lcd_time(){
    output_state[output_index] = TIMING;
    backup_output_state();
    
    shift_current_menu(left); 
    shift_current_menu(left);

    update_lcd_buffer();
    refresh_lcd();
    return;
}
void lcd_time();

flash unsigned char cycle_out_str[] = " Cycle output\0";
void init_pastOS_lcd_cycle(){
    //right arrow 126
    cycle_cmd_flag = true; 
    horiz_cyc_index = 1;
    variable_number = 0; //for assigning numbers to Duration and Duty cycle

    lcd_duration = 0;
    lcd_duty_cyc = 0;
    
    sprintf(&lcd_buf[0][0], "%p %d", cycle_out_str, output_index+1);
    fillZero(&lcd_buf[1][0], 16);
    
    update_cycle_lcd(); 
    return;
}
void lcd_cycle();
void lcd_manual();
void lcd_pwr();
void lcd_wtr();

void lcd_switch();
void pastOS_assign_strt_stp_key(){

    time_check = false;
    switch_key_out_index = output_index;
    writeChar( 0x0630, switch_key_out_index); //0x630 
    init_menu(); //best way to return easily
    time_check = true;
    return;
}
void assign_strt_stp_key();



void (*past_out_select_cmd)() = &NO_CMD;
void (*past_submit_select_cmd)() = &NO_CMD;


/*
void (*cmd_holder[PAST_OUT_SELECT_CMD_COUNT])() = {
    NO_CMD,
    pastOS_monitor_outs, pastOS_assign_strt_stp_key,
    past_OS_lcd_time, pastOS_lcd_cycle, pastOS_lcd_switch
}; 
*/
#include <out_select.c>

//main menu items
menu_item item1 = { "System Status", &sys_stat };
menu_item item2 = { "Commands", &lcd_cmds };
menu_item item3 = { "Monitor outputs", &monitor_outs };
menu_item item4 = { "Set start/stop", &assign_strt_stp_key };
    
//main menu
menu main_menu = { " Main Menu",.item_count=4 , .items = {&item1, &item2, &item3, &item4}};  
 

menu_item selec_output0 = {"Output 1", &selec_out_0};
menu_item selec_output1 = {"Output 2", &selec_out_1};
menu_item selec_output2 = {"Output 3", &selec_out_2};
menu_item selec_output3 = {"Output 4", &selec_out_3};
menu_item selec_output4 = {"Output 5", &selec_out_4};
menu_item selec_output5 = {"Output 6", &selec_out_5};
menu_item selec_output6 = {"Output 7", &selec_out_6};
menu_item selec_output7 = {"Output 8", &selec_out_7};
 
//Output select
menu output_select = { " Output Select",.item_count=8, .items = {&selec_output0, &selec_output1,
    &selec_output2, &selec_output3, &selec_output4, &selec_output5, &selec_output6, &selec_output7}};
   

//Commands items
menu_item subcmd_item1 = { "Time", &lcd_time };
menu_item subcmd_item2 = { "Cycle", &lcd_cycle };
menu_item subcmd_item3 = { "Manual", &lcd_manual };
menu_item subcmd_item4 = { "Switch", &lcd_switch };
menu_item subcmd_item5 = { "Power alarm", &lcd_pwr };
menu_item subcmd_item6 = { "Water sensor", &lcd_wtr };
    
//Commands menu
menu sub_commands = { " Commands", .item_count=6, .items = { &subcmd_item1, 
    &subcmd_item2, &subcmd_item3, &subcmd_item4, &subcmd_item5, &subcmd_item6 } };


void past_lcd_wtr(){
    debputsf1("Adpative water flag ");
    if(submit_selection)
        debputsf1("set");
    else
        debputsf1("cleared");
    debputsf1("\r\n");
    adaptiveWatering = submit_selection;
    debputsf1("Backup flags\r\n");
    backup_flags();
    
    shift_current_menu(left); 
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void past_lcd_pwr(){
    debputsf1("Power out report flag ");
    
    if(submit_selection)
        debputsf1("set");
    else
        debputsf1("cleared");
    debputsf1("\r\n");
    powerOut_report = submit_selection;
    debputsf1("Backup flags\r\n");
    backup_flags();
    
    shift_current_menu(left);
    update_lcd_buffer();
    refresh_lcd();
    return;
}

void lcd_on_submit(){
    submit_selection = true;
    (*past_submit_select_cmd)();
    return;
}
void lcd_off_submit(){
    submit_selection = false; 
    (*past_submit_select_cmd)();
    return;
}
menu_item subitem_on = { "On", &lcd_on_submit};
menu_item subitem_off = { "Off", &lcd_off_submit};

menu submit = {" Select", .item_count=2, .items = { &subitem_on, &subitem_off}};

    
void sys_stat(){
    lcd_timer_function = &system_status;
    lcd_timer(on);
    return;
}
void lcd_cmds(){
    selected_item_number[0] = 0;
    current_menu[0] = &sub_commands;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void monitor_outs(){
    
    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_out_select_cmd = &pastOS_monitor_outs;
    current_menu[0] = &output_select;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
       
    return;
}
void lcd_time(){
    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_out_select_cmd = &pastOS_lcd_time;
    current_menu[0] = &output_select;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void lcd_cycle(){
    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_out_select_cmd = &init_pastOS_lcd_cycle;
    current_menu[0] = &output_select;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void lcd_manual(){
    //Do not implement yet
    //Because of difficult interface
    return;
}
void lcd_pwr(){
    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_submit_select_cmd = &past_lcd_pwr;
    current_menu[0] = &submit;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void lcd_wtr(){
    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_submit_select_cmd = &past_lcd_wtr;
    current_menu[0] = &submit;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    return;
    return;
}
void past_submit_lcd_switch(){
    output_state[output_index] = SWITCH_MANUAL;
    if(submit_selection)
        turnOn_output(output_index);
    else
        turnOff_output(output_index);
    
    delay_ms(10);
    backup_output_state();    
    backup_output_switch();
    
    shift_current_menu(left);
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void pastOS_lcd_switch(){
    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_submit_select_cmd = &past_submit_lcd_switch;
    current_menu[0] = &submit;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void lcd_switch(){
    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_out_select_cmd = &pastOS_lcd_switch;
    current_menu[0] = &output_select;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    return;
}
void assign_strt_stp_key(){

    selected_item_number[0] = 0;
    //preserve CMD to know what to do after output selection.
    past_out_select_cmd = &pastOS_assign_strt_stp_key;
    current_menu[0] = &output_select;
    selected_item = (current_menu[0]->items[0]);
    update_lcd_buffer();
    refresh_lcd();
    
    return;
}




void init_menu(){
    current_menu[0] = &main_menu;
    current_menu[1] = 0;
    current_menu[2] = 0;
    current_menu[3] = 0;
    
    selected_item = &item1;   
    past_out_select_cmd = &NO_CMD;
    selected_item_number[0] = 0;
    lcd_timer_function = &no_task;
    lcd_timer(off);
    
    
    fillZero(&lcd_buf[0][0], 16);
    fillZero(&lcd_buf[1][0], 16);
    
    
    update_lcd_buffer(); 
    debputs_lcd_buf();
    refresh_lcd();
    //lcd_timer(on); 
    
    debputsf1("Menu is initialized\r\n");
}


/*
unsigned char cmd_holder = 0;
menu *current_menu;
menu_item *selected_item;  
*/
   
void reset_menu(){
    menu_active = false; //to activate menu system in 2nd button press
    lcd_timer(off);
    lcdputsf("IDLE");
    // first time is used to light up the LCD
    
    return;
}

void no_key(){
    debputsf1("\r\n!!! No_key function was executed!\r\n");
    return;
}

void up_key(){
    if (lcd_timer_function != (&no_task) )return; //to stop Up/Down functions in dynamic sections
    if( selected_item_number[0] == 0) return; 
    selected_item_number[0]--;
    selected_item = current_menu[0]->items[selected_item_number[0]];
    
    update_lcd_buffer();
    refresh_lcd();
    return;
}


void down_key(){
    if (lcd_timer_function != (&no_task) )return;
    if( selected_item_number[0] + 1 >= current_menu[0]->item_count) return;
    selected_item_number[0]++;
    
    selected_item = current_menu[0]->items[selected_item_number[0]];
    
    update_lcd_buffer();
    refresh_lcd();
    return;
}


void right_key(){
    return;
}


void left_key(){
    return;
}


void submit_key(){
    if (lcd_timer_function != (&no_task) )return; //prevent out of index actions if Timer2 is on 
    if(current_menu[0] != &submit)
        shift_current_menu(right);
    selected_item->action();
    return;
}

    
void back_key(){
    if (lcd_timer_function != (&no_task) ){
        lcd_timer(off);
        lcd_timer_function = &no_task;
        }
    if(current_menu[1] == 0) return;
    shift_current_menu(left);
    update_lcd_buffer();
    refresh_lcd();
    return;
}

bool backup_output_switch();
bool backup_output_state();
void start_stop_key(unsigned char key){ //assign commands
    
    long_beep();
    output_state[switch_key_out_index] = SWITCH_MANUAL;
    debputsf1("Switch key ");
    
    if(key == start){
        debputsf1("Start");
        turnOn_output(switch_key_out_index);        
    }
    else if(key == stop){
        debputsf1("Stop");
        turnOff_output(switch_key_out_index);
    }
    else{
        debputsf1("\r\n start/stop function wrong key input\r\n");
        output_state[switch_key_out_index] = TIMING;
        return;
        }
     
    debputsf1("\r\n");      
    backup_output_state();
    backup_output_switch();    
    
    return;
}





extern void (*key_command[7])(void) = {
    no_key,
    up_key, down_key, submit_key , right_key,
    back_key,left_key 
    };
    
extern void (*cycle_key_command[7])(void) = {
    no_key,
    cyc_up_key, cyc_down_key, cyc_submit_key , cyc_right_key,
    cyc_back_key , cyc_left_key 
    };
    
void key_function(unsigned char key){
    unsigned char buf[4];   
    
    
    sprintf(buf, "%d", key);
    debputsf1("Key: ");
    debputs1(buf);
    debputsf1(" is pressed\r\n");
    
    
    if(!lcd_light_state) return;
    if(!menu_active && lcd_light_state){
        menu_active = true;
        init_menu();
        return;
        }
    
    beep();
    if(cycle_cmd_flag)
        (*cycle_key_command[key])();
    else
        (*key_command[key])();
    
    //refresh_lcd();
    return;
}
    
    

/*
    buttons:
     - Up
     - Down
     - Right
     - Left
     - Back
     - Enter
     - On
     - Off
*/




