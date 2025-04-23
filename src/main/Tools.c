
//fill a str with null
void fillZero(char *str, unsigned char size){
    unsigned char i = 0;
    while(size > i){
        *str = ' ';
        i++;
        str++;    
    }
}
//check if given char is a number (0-9)
bool checkifnum(char input){
    switch(input){
        case '0':
            return true;
        case '1':
            return true;
        case '2':
            return true;
        case '3':
            return true;
        case '4':
            return true;
        case '5':
            return true;
        case '6':
            return true;
        case '7':
            return true;
        case '8':
            return true;
        case '9':
            return true;
        default: return false;
    }
    return false;
}
//return str length
unsigned char len(unsigned char* buf){
    unsigned char i;
    for(i=0; i<=255; i++){
        if(*(buf+i) == 0)
            return i;
    }
    return 255;
}
//return str (that's saved on flash) length
unsigned char lenf(flash unsigned char* buf){
    unsigned char i;
    for(i=0; i<=255; i++){
        if(*(buf+i) == 0)
            return i;
    }
    return 255;
}
//return the actual integer value in the given pointing char
unsigned char extract_number(unsigned char *str){
    unsigned char i;
    unsigned char number=0;
    for(i=0; i<3; i++){
        if(!checkifnum(*str))
            break;            
        number *= 10;
        number += *str - 48;
        str++;        
    }
    return number;
}

//peripheral functions
void puts1(char *str){
    unsigned char i;
    for(i=0; i<128; i++){
        putchar(*str);
        str++;
        if(*str == 0)
            break;
    }
    if(i == 64)
        putsf("ATENTION PUTS OVERFLOW!\r\n");
    return;
}

void puts2(char *str){
    int i;
    for(i=0; i<512; i++){
        putchar1(*str);
        str++;
        if(*str == 0)
            break;
    }
    return;
}
void putsf2(char flash *str){
    int i;
    for(i=0; i<512; i++){
        putchar1(*str);
        str++;
        if(*str == 0)
            break;
    }
    return;
}

void flush_uart0_rx(){
    #asm("cli")
    
    rx_wr_index0=0;
    rx_rd_index0=0;   
    rx_counter0=0;
    
    #asm("sei")
    return;
}

//Initializing GSM functions
bool check_call_ready(unsigned char *str){
    if(*(str) == 'C' & *(str+1) == 'a' & *(str+2) == 'l' & *(str+3) == 'l'){
        return true;   
        }
    return false;
}
void init_csq(unsigned char *str){
    unsigned char rssi = 0, ber = 0;
    unsigned char buf[4];
    //+CSQN: <rssi>,<ber>
    //012345678    9,10,11 
    
    rssi = extract_number(str+7);
    
    if(*(str+8) == ',')
        ber = extract_number(str+9);
    else
        ber = extract_number(str+10);    
     
    fillZero(buf, 4);   
    if(rssi < 10){
        debputsf1("Signal weak : ");
        sprintf(buf, "%d", rssi);  
        debputs1(buf);
        debputsf1("\r\n");
        }
         
    fillZero(buf, 4);
    if(ber > 0){
        debputsf1("High bit error rate: ");
        sprintf(buf, "%d", ber);  
        debputs1(buf);
        debputsf1("\r\n");
        }
  
     return;       
}
bool check_csq(unsigned char *str){
    if(*(str) == '+' & *(str+1) == 'C' & *(str+2) == 'S' & *(str+3) == 'Q'){
        init_csq(str);
        return true;  
        }
    return false;    
}
//kept here in case of "gets1" fails
void gets1_original(char *str, unsigned char len){
    char i = 0;
    char buf;
    for(i=0; i<len; i++){
        buf = getchar();
        if(i <= 1){
            if(buf == '\r' || buf == '\n' || buf == 0){
                i--;
                continue;
            }
        }
        if(buf == '\r' || buf == '\n')
            break;
        *str = buf;
        str++;       
    }
    *str = 0;
    return;    
}
void gets1(char *str, unsigned char len){
    char i = 0;
    unsigned char buf;
    unsigned char *keep_str = str; 
             
    get_rx_string = 3; //timeout management
    while(true){//if any status msg appears, continue
        str = keep_str;
        for(i=0; i<len; i++){
            buf = getchar();
            if(get_rx_string == 1){ //check timeout possiblity
                if(buf == 0xFF){ //timeout happened, abort (in case of noise on UART rx pin)
                    get_rx_string = 0;
                    return;
                    }
            }
            if(i <= 1){ //ignore new lines or single null chars
                if(buf == '\r' || buf == '\n' || buf == 0){
                    i--;
                    continue;
                }
            }
            if(buf == '\r' || buf == '\n') //if new line return the str
                break;
            *str = buf;
            str++;       
        }
        *str = 0;
        
        //in case of any status messages were sent by GSM in middle of SMS recieving 
        if(check_call_ready(keep_str)){
            debputsf1("gets1: CALL READY message appreared\r\n");
            continue;
            }

        else if(check_csq(keep_str)){
            debputsf1("gets1: +CSQN message appreared\r\n");
            continue;
            }
        else
            break;
            
                  
        }        
    return;    
}

void lcdputs(char *str){
    if(menu_active)return;  //no messages if menu is active
    delay_ms(100);
    lcd_clear();
    lcd_puts(str);
    return;    
}
void lcdputsf(char flash *str){
    if(menu_active)return;
    delay_ms(100);
    lcd_clear();
    lcd_putsf(str);
    return;
}


//all functions that are started with "deb" are used for debug purposes
void debputs1(char *str){
    unsigned char length = len(str);
    unsigned char i;
    
    if(debugFlag >= true){ 
        puts2(str);
        if(length >= 255)
            delay_ms(30);
        else
            for(i=0 ; i<length; i++)
                delay_ms(2);
        }
    return;                      
}
void debputchar1(char chr){
    if(debugFlag >= true)
        putchar1(chr);
    delay_ms(2);
    return;    
}
void debputsf1(char flash *str){
    unsigned char length = lenf(str);
    unsigned char i;
    if(debugFlag >= true){
        putsf2(str);
        if(length >= 255)
            delay_ms(30);
        else
            for(i=0 ; i<length; i++)
                delay_ms(2);
        }
    return;
}
void debputsdec1(unsigned char *str, unsigned char len){
    //unused function. prints string in number form
    //debug purpose only
    unsigned char eight_columns, char_count = 0;
    bool end_flag = true; 
    unsigned char buffer[4];
    
    while(true){
        for(eight_columns=0; eight_columns<8; eight_columns++){
            if(char_count >= len){
                end_flag = false;
                break;
                }
            sprintf(buffer, "%d ", *(str + char_count));
            debputs1(buffer);
            fillZero(buffer, 4);
            char_count++;            
        }
        debputsf1("\r\n");
        if(!end_flag) break;
    }
    return;
}
void debputs_lcd_buf(){
    unsigned char i;

    debputsf1("LCD:\r\n");
    for(i=0; i<16; i++){
        debputchar1( lcd_buf[0][i] );    
    }
    debputsf1("\r\n");
    for(i=0; i<16; i++){
        debputchar1( lcd_buf[1][i] );    
    }
    return;
}

//UART MAIN CODES
//software functions

char sendSMS(char *str);
char reportSMS(char *report){ //for future features
    char z=0;
    if(reportFlag){
        for(z=0; z < REPORT_SMS_TRYAGAIN; z++){
            if(sendSMS(report) == 1) return true;
            delay_ms(5000);   
        }
        if(z == REPORT_SMS_TRYAGAIN) return false;  
    }
    else return 2;
}
//lowercase given str
void lowerCase(char *str){
    while(*str != 0){
        switch (*str){
            case 'A':
                *str = 'a';
                break;
            case 'B':
                *str = 'b';
                break;
            case 'C':
                *str = 'c';
                break;
            case 'D':
                *str = 'd';
                break;
            case 'E':
                *str = 'e';
                break;
            case 'F':
                *str = 'f';
                break;
            case 'G':
                *str = 'g';
                break;
            case 'H':
                *str = 'h';
                break;
            case 'I':
                *str = 'i';
                break;
            case 'J':
                *str = 'j'; 
            case 'K':
                *str = 'k';
                break;
            case 'L':
                *str = 'l';
                break;
            case 'M':
                *str = 'm';
                break;
            case 'N':
                *str = 'n';
                break;
            case 'O':
                *str = 'o';
                break;
            case 'P':
                *str = 'p';
                break;
            case 'Q':
                *str = 'q';
                break;
            case 'R':
                *str = 'r';
                break;
            case 'S':
                *str = 's';
                break;
            case 'T':
                *str = 't';
                break;
            case 'U':
                *str = 'u';
            case 'V':
                *str = 'v';
                break;
            case 'W':
                *str = 'w';
                break;
            case 'X':
                *str = 'x';
                break;
            case 'Y':
                *str = 'y';
                break;
            case 'Z':
                *str = 'z';
                break;
            default: break;
            
        }
        str++;
    }
}
//After manual timing process ends, this function will empty 
//the rest of useless timing slots to prevent conflicts with time check nad backup
void empty_timing_rest(char timing_index, char outIndex){
    /*
    struct wateringTime startOut[OUT_COUNT][TIME_INTERVAL_COUNT];
    struct wateringTime stopOut[OUT_COUNT][TIME_INTERVAL_COUNT];
    */
    unsigned char buf[4];
    debputsf1("Emptying cycles");
    sprintf(buf, "%d", outIndex);
    debputs1(buf);
    fillZero(buf, 4);
    debputsf1(" from ");
    sprintf(buf, "%d", timing_index);
    debputs1(buf);
    fillZero(buf, 4);
    debputsf1(" to 48\r\n");
     
    while(timing_index < TIME_INTERVAL_COUNT){
        
        startOut[outIndex][timing_index].M = 61;
        startOut[outIndex][timing_index].H = 61;
        
        stopOut[outIndex][timing_index].H = 61;
        stopOut[outIndex][timing_index].M = 61;
        
        timing_index++;
    }
    return;
}

