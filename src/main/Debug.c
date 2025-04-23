//There is no need to read this section
//This section is only used to send some info to serial port for debug purposes
//or change settings to make sure everything works fine
void printCycle(unsigned char ouput){
    char p =0;
    char str[20];
    while(p <= TIME_INTERVAL_COUNT){
        if(startOut[ouput][p].H == 61) break;
        sprintf(str, "%d: %d,%d,%d,%d\r\n", p, startOut[ouput][p].H, startOut[ouput][p].M,
                                       stopOut[ouput][p].H, stopOut[ouput][p].M);
        puts2(str);
        fillZero(str, 20);                 
        p++;
        }
}


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
void deput_time(){
    char str[20];
    sprintf(str, "time: %d:%d:%d\r\n", time.hour, time.min, time.sec);
    debputs1(str);
    
    fillZero(str, 20);
    sprintf(str, "date: %d:%d:%d %d\r\n", time.year, time.month, time.date, time.weekDay);
    debputs1(str);
    fillZero(str, 20);
    return; 
}

//debug purposes
void setTime(){

    rtc_set_time(21,30,0);
    rtc_set_date(4, 28,6, 02);    
}

void diagRom_sub_read(unsigned char errCode){
    switch(errCode){
        case noError_r:
            //debputsf1("\r\n !!! R:No error\r\n");
            break;
        case first_start_r:
            debputsf1("\r\n !!! R:bus busy\r\n");
            break;
        case rom_address_write_r:
            debputsf1("\r\n !!! R:adr write failwrite mode\r\n");
            break;
        case msb_address_r:
            debputsf1("\r\n !!! R:M adr fail\r\n");
            break;
        case lsb_address_w:
            debputsf1("\r\n !!! R:L adr fail\r\n");
            break;
        case second_start:
            debputsf1("\r\n !!! R:Second start  failed\r\n");
            break;
        case rom_address_read:
            debputsf1("\r\n !!! R:adr write failread mode\r\n");
            break;
        default:
            debputsf1("\r\n !!! Wrong Error code\r\n");
            break;     
    }
    return;
}
void diagRom_sub_write(unsigned char errCode){
    switch(errCode){
        case noError_w:
            //debputsf1("!!! W:No error\r\n");
            break;
        case first_start_w:
            debputsf1("\r\n!!! W:bus busy\r\n");
            break;
        case rom_address_write_w:
            debputsf1("\r\n!!! W:adr write failwrite mode\r\n");
            break;
        case msb_address_w:
            debputsf1("\r\n !!! W:M adr fail\r\n");
            break;
        case lsb_address_w:
            debputsf1("\r\n !!! W:L adr fail\r\n");
            break;
        case write_char:
            debputsf1("\r\n !!! W:charwrite fail\r\n");
            break;
        case write_int_m:
            debputsf1("\r\n !!! W:intmsb fail\r\n");
            break;
        case write_int_l:
            debputsf1("\r\n !!! W:intlsb fail\r\n");
            break;
        default:
            debputsf1("\r\n !!! W:Wrong Err code\r\n");
            break;     
    }
    return;    
}    
unsigned char diagnoseRom(unsigned char errCode, unsigned char opMode){//opMode: R=1, W=0
    if(opMode == write_rom)
        diagRom_sub_write(errCode);
    else
        diagRom_sub_read(errCode);
    return errCode;         
}