//list of numbers to check at each SMS recieving
flash unsigned char const_num[2][11] = {"XXXXXXXXXX" , "XXXXXXXXXX"}; // or add more numbers


//find the OK after a command is sent to GSM
char findOK(char *str, char len){
    char i=0;
    len--;
    while(len > i){
        if(*str == 'O' && *(str+1) == 'K')
            return true;
        str++;
        i++;
    }
    return false;    
}
//Complete procedure to send SMS
char sendSMS(char *str){
    char temp = 0;    
    char buf[22];
    
    debputsf1("SendSMS initiated\r\nmsg: ");
    debputs1(str);
    debputsf1("\r\n");
    delay_ms(10);
    //putsf("AT+CMGF=1\r\n");

    if(contact_number_index == 1)
        putsf("AT+CMGS=\"9912321879\"\r\n");    
    else if(contact_number_index == 2)
        putsf("AT+CMGS=\"9122659930\"\r\n");
 
      
    while(true){ //AT+CMGS Mirror is skipped in this loop
        temp = getchar();
        if(temp == '>'){
            temp = getchar();
            if(temp == ' ')
                break;
        }    
    }
    puts1(str);
    putchar(26);
    
    temp = 0;
    do{
        gets1(buf, 10);
        if(temp >= 10){
            debputsf1("\r\n !!! Send sms couldn't find OK\r\n");
            break;
            } 
        temp++;
    }while(!findOK(buf ,10));  
    if(temp < 10)
            debputsf1("Send sms OK is found\r\n");
    
    delay_ms(100);
    
    return true;
}

//Sub functions to read SMS memory index, number and etc
//all sections will have their own error code for better troubleshooting
char sub_getNotification(char cmdNum, char* index){
    char i =0;
    char debug[16];
    char buffer[75];
    switch(cmdNum){
        case 0: //this case is skipped
        //it was used with prev GSM module (Got replaced because of low reliablity)
            debputsf1("Getting +CIEV\r\n");
            lcdputsf("Getting CIEV\r\n");
            
            gets1(buffer,75);
            if(buffer[0] != '+' || buffer[1] != 'C' || buffer[2] != 'I' || buffer[3] != 'E' || buffer[4] != 'V'){
                debputsf1("Wrong +CIEV: ");
                debputs1(buffer);
                debputsf1("\r\n");
                return 10;
            }
            else{
                lcdputsf("+CIEV success"); 
                debputsf1("+CIEV success\r\n"); 
            }
            return 1;
        case 1:
            lcdputsf("Getting CMTI");
            debputsf1("Getting +CMTI\r\n");
            
            GET_RX_STR_TIMEOUT = 15;
            
            fillZero(buffer, 40);
            gets1(buffer,75);
            GET_RX_STR_TIMEOUT = GET_RX_STR_TIMEOUT_DEFAULT;
            
            if(buffer[0] != '+' || buffer[1] != 'C' || buffer[2] != 'M' || buffer[3] != 'T' || buffer[4] != 'I'){
                debputsf1("Wrong +CMTI: ");
                debputs1(buffer);
                debputsf1("\r\n");
                return 11;
                }
            else{
                lcdputsf("+CMTI success");
                debputsf1("+CMTI success\r\n");
            }           
            
            lcdputsf("Finding index\r\n");
            debputsf1("Finding index\r\n");
            for(i=0; i<75; i++){
                if(buffer[i] == ','){
                    if(checkifnum(buffer[i+2]))
                        *index = (buffer[i+1] - 48)*10 + (buffer[i+2] - 48);
                    else
                        *index =  buffer[i+1] - 48;
                    sprintf(debug, "index:%d\r\n", *index);
                    debputs1(debug);
                    lcdputs(debug);
                    break;  
                }
            }
            if(i == 75){
                debputsf1("Didn't find index\r\n");
                return 12;
                }
            
            lcdputsf("Found index\r\n");
            debputsf1("Found index\r\n");
            break;
        default:
            debputsf1("Wrong \"sub_getNotification\" switch CMD INDEX for execution\r\n"); 
            return 0;            
        }
    return 1;    
}
char getNotification(char* index_p){
    char result = 0, i = 0;
    for(i=1; i<2; i++){ //case 0 is skipped. it was used in previous GSM.
        result = sub_getNotification(i, index_p);
        if(result != 1)
            return result;        
    }
    putsf("\r\n");
    printf("AT+CMGR=%d\r\n", *index_p);
    return 1;    
}


bool sub_checkNumber(unsigned char* num){
    
     
    unsigned char* keep_num = num;
    bool return_value = false;;
    bool search_result = true;
    unsigned char d, c;
    for(c=0; c<2; c++){
        search_result = true;
        for(d=0; d<10; d++, num++){
            if(const_num[c][d] != *num)
                search_result = false;    
        }
        num = keep_num;
        if(search_result){ 
            contact_number_index = c + 1; //for reporting back result
            return_value = true;
            }
    }
    return return_value;
}
char checkNumber(){
    char buffer[75];
    unsigned char number[11];
    char i=0, j=0;
    
    debputsf1("Getting AT+CMGR\r\n");
    
    gets1(buffer, 75);
    if(buffer[0] == 'A' && buffer[1] == 'T')
        debputsf1("Got AT+CMGR\r\n");
    debputsf1("Getting +CMGR\r\n");
    
    gets1(buffer, 75);
    if(buffer[0] != '+' || buffer[1] != 'C' || buffer[2] != 'M' || buffer[3] != 'G' || buffer[4] != 'R'){
        debputsf1("Wrong +CMGR: ");
        debputs1(buffer);
        debputsf1("\r\n");
        return 13;
        }
    else{                     
        debputsf1("+CMGR success\r\n");
        lcdputsf("+CMGR success");
    }
    
    
    for(i=0; i<73; i++){
        if(buffer[i] == '+' && buffer[i+1] == '9' && buffer[i+2] == '8'){
            debputsf1("Found +98\r\n");
            lcdputsf("Found +98\r\n");
            for(j=0; j<10; j++){
                number[j] = buffer[j+i+3];
            }
            number[10] = 0;
            break;          
        }
    }
    if(i == 73) return 14;
    debputsf1("Found number:");
    debputs1(number);
    debputsf1("\r\n");
    lcdputsf("Got NUMBER\r\n");
        
    if(!sub_checkNumber(number)){
        debputsf1("\r\n\r\nWRONG NUMBER!!!!\r\n\r\n");
        return 2;
        }
    debputsf1("Correct number\r\n");
    lcdputsf("NUMBER IS CHECKED\r\n");
    
    
    return 1;
}

//saving CMDs on strings for info extraction
void assignCMD(char *str){
    char i = 0;
    for(i=0; i<20; i++){
        cmdBuf[i][cmd_count] = *(str+i);
        if(cmdBuf[i][cmd_count] == 0)
            break;
    }
    return;
}
//clean start
void emptyCMD(){//flushes cmd buffers
    unsigned char x, y;
    for(y=0; y < CMD_COUNT ; y++){
        for(x=0; x<20; x++){
            cmdBuf[x][y] = 0;
        }
    }
    return;
}
//Checks the output index given by user (SMS)
bool check_output_number(char num){
    if(num >= '1' && num <= '8')
        return true;
    return false;
}
//check validity of Switch command value (On//off, else return false)
bool switch_check_inp(unsigned char *cmd){
    //switch<num> off /on
    //0123456    78910
    if(*(cmd + 8) == 'o'){
        if(*(cmd + 9) == 'f' && *(cmd + 10) == 'f')
            return true;
        else if(*(cmd + 9) == 'n')
            return true;   
    }
    return false;
}
//To extract info from commands sent by user
char checkCMD(char *cmd){
    char x = 0;
    bool flag= true, returnValue = true;
    cmd_output_index[cmd_count] = 0;
    if(*(cmd) == 'c' && *(cmd+1) == 'y' &&*(cmd+2) == 'c' &&*(cmd+3) == 'l' &&*(cmd+4) == 'e' &&*(cmd+6) == ':'){
        if(check_output_number(*(cmd+5))){
            cmd_output_index[cmd_count] = *(cmd+5) - 49; //48asci + 1 array index 
            x=7;
            flag= true;
            while(x<20){
                if( !checkifnum( *(cmd+x) ) ){
                    if(*(cmd+x) == '-' && flag)
                        flag = false;
                    else if(*(cmd+x) == 0) break;
                    else{
                        returnValue = false;
                        break;
                    }
                }     
                x++;    
            }
            if(returnValue) return 1;
            }  
    }
    
    if(*(cmd) == 'e' && *(cmd+1) == 'r' &&*(cmd+2) == 'r' &&*(cmd+3) == 'o' &&*(cmd+4) == 'r' &&*(cmd+5) == ':'){
        if( *(cmd + 6) == '1' || *(cmd + 6) == '0')
            if( *(cmd + 7) == 0)
                return 2;       
    }
    
    if(*(cmd) == 'a' && *(cmd+1) == 'w' &&*(cmd+2) == ':'){
        if( *(cmd + 3) == '1' || *(cmd + 3) == '0')
            if( *(cmd + 4) == 0)
                return 3;
    }
    
    if(*(cmd) == 'd' && *(cmd+1) == 'r'){
        if(check_output_number(*(cmd + 2)) && *(cmd + 3) == 0){
            cmd_output_index[cmd_count] = *(cmd+2) - 49;
            return 4;
            }
    }
    
    if(*(cmd) == 'w' && *(cmd+1) == 'r'){
        if(check_output_number(*(cmd + 2)) && *(cmd + 3) == 0){
            cmd_output_index[cmd_count] = *(cmd+2) - 49;
            return 5;
            }
    }
    
    if(*(cmd) == 'm' && *(cmd+1) == 'r'){
        if(check_output_number(*(cmd + 2)) && *(cmd + 3) == 0){
            cmd_output_index[cmd_count] = *(cmd+2) - 49;
            return 6;
            }
    }
    
    if(*(cmd) == 'p' && *(cmd+1) == 'b' &&*(cmd+2) == 'o' &&*(cmd+3) == ':'){
        if( *(cmd + 4) == '1' || *(cmd + 4) == '0')
            if( *(cmd + 5) == 0)
                return 7;
    }
    
    if(*(cmd) == 'm' && *(cmd+1) == 'a' &&*(cmd+2) == 'n' 
    &&*(cmd+3) == 'u' &&*(cmd+4) == 'a' &&*(cmd+5) == 'l'){
        if(check_output_number(*(cmd+6)) && *(cmd+7) == 0){
            cmd_output_index[cmd_count] = *(cmd+6) - 49;
            return 8; //continue    
        }    
    }
    
    if(*(cmd) == 't' && *(cmd+1) == 'i' && *(cmd+2) == 'm' && *(cmd+3) == 'e'){
        if(check_output_number(*(cmd+4))){
            cmd_output_index[cmd_count] = *(cmd+4) - 49;
            return timing;
            }
    }
    
    if(*(cmd) == 's' && *(cmd+1) == 'w' &&*(cmd+2) == 'i' && *(cmd+3) == 't'
       &&*(cmd+4) == 'c' && *(cmd+5) == 'h'){
        if(check_output_number(*(cmd+6))){
            if(switch_check_inp(cmd)){
                cmd_output_index[cmd_count] = *(cmd+6) - 49;
                return switching; 
                }
            }
    }
    //to add more commands
    
    
    return 0;            
}

//Remove validity of duplicated commands
void validateCMD(char cmdIdent, unsigned char *dupeFlag){    
    unsigned char temp=0;
    
    if(cmdIdent == 8)// cycle and manual duplications are not allowed
        cmdIdent = 1;
    cmdIdent--;
    // decrease Ident by one because checkCMD zero return value
    // is preserved for false returning
    //0.cycle 1.error 2.aw 3.dr 4.wr 5.mr 6.pbo 7.manual, more will be added    
    
    
    //indexes for commands    
    //char bools for outputs
    cmdValid[cmd_count] = true;
    if(cmdIdent == timing) return;
    temp = *(dupeFlag + cmdIdent);
    temp &= (1 << cmd_output_index[cmd_count]);
    if(temp != 0)
        cmdValid[cmd_count] = false;
        
        
    if(cmdValid[cmd_count]){
        temp = 0;
        temp = (1 << cmd_output_index[cmd_count]);
        *(dupeFlag + cmdIdent) |= temp;
    }
    
}

//After checking SMS info, Now the commands are extracted from text (Invalid ones are ignored)
//Debug messages explain each setion themselves
char getCMD(){
    //return: 0=problem, 1=success, 2=w number, 3=sms exceeds 10 lines, 4=CMD syntax
    //5=len exceed 
    unsigned char debug[10];
    unsigned char buffer[75];
    unsigned char smsLines = 0;
    char cmdFlag = 0;
    unsigned char dupFlags[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //these flags are used to prevent duplicated CMDs
    //0.cycle 1.error 2.aw 3.dr 4.wr 5.mr 6.pbo 7.manual, 8.switch more will be added
    //time cmd doesn't need prevention for duplicate (no difference anyways)
    
    cmd_count = 0;
    emptyCMD();
    fillZero(buffer, 21);
    lcdputsf("getting CMDs\r\n");
    
    gets1(buffer,75);    
    while(!findOK(buffer, 20)){ 
        cmdBuf_ident[cmd_count] = 0;
        if(smsLines == CMD_COUNT){
            debputsf1("CMD count overflow\r\n");
            return 3;
            }        
        lowerCase(buffer);
        if(len(buffer) > 20){
            debputsf1("CMD length overflow (20 chars)\r\n");        
            return 5;
        }
        
        cmdFlag = checkCMD(buffer);
        
        //This section is for debug purposes
        sprintf(debug, "CMD%d: ", cmd_count);
        debputs1(debug);
        fillZero(debug, 10);
        debputs1(buffer);
        debputsf1("\r\n");
            
        if(cmdFlag != 0){           
            debputsf1("is assigned\r\n");
           
            assignCMD(buffer);
            cmdBuf_ident[cmd_count] = cmdFlag;
            validateCMD(cmdFlag, dupFlags);
            cmd_count += 1;     
        }
        else {
            debputsf1("is invalid (checkCMD)\r\ncode 0 \r\n");
            
            return 4; 
        }
        //-----------------------------------------------
        fillZero(buffer, 21);
        GET_RX_STR_TIMEOUT = 3;
        gets1(buffer,75);  //get next line of commands     
        smsLines++;    
    }
    //executeCMD(*cmdCount); moved out of block
    return true;
}


void deleteMessage(char index){
    char buffer[4];
    printf("AT+CMGDA=\"DEL ALL\"\r\n", index);
    
    GET_RX_STR_TIMEOUT = 6;
    gets1(buffer, 4);
    debputsf1("Delete AT+CMGDA OK: ");
    debputs1(buffer);
    debputsf1("\r\n");

    
    if(findOK(buffer, 10)){
        debputsf1("Message deleted\r\n");
        lcdputsf("Msg deleted");
        }
    else{
        debputsf1("Message deletion failed\r\n");
        lcdputsf("Deletion failed");
        }
    return;
}

//2 main functions of this file, process of recieving SMS begins here
unsigned char getSMS_sub_functions(char function){
    static char msg_index = 0;
    unsigned char sub_result = 0;
    switch(function){
        case 0:
            debputsf1("Getting SMS notification\r\n");
            sub_result = getNotification(&msg_index);
            if(sub_result == 1) 
                debputsf1("SMS notification successful\r\n");
            else
                debputsf1("SMS notification failed\r\n"); 
            break;
        case 1:
            debputsf1("Checking number\r\n");
            sub_result = checkNumber();
            if(sub_result == 1) 
                debputsf1("Number check successful\r\n");
            else
                debputsf1("Number check failed\r\n");
            break;
        case 2:
            debputsf1("Getting CMDs\r\n");
            sub_result = getCMD();
            if(sub_result == 1) 
                debputsf1("getCMD successful\r\n");
            else
                debputsf1("getCMD failed\r\n");
            deleteMessage(msg_index);
            break;
        default:
            debputsf1("Wrong \"getSMS_sub_functions\" switch function INDEX for execution\r\n"); 
            return 0;
    }
    return sub_result;
}
unsigned char getSMS(){
    char sub_result = 0;
    unsigned char i = 0; 
    
    
    for(i=0; i<3; i++){
        sub_result = getSMS_sub_functions(i);
        if(sub_result != true){
            debputsf1("SMS failed\r\n");
            return sub_result;
            }
    }    
    debputsf1("SMS success\r\n"); 
    lcdputsf("Success\r\n"); 
    return true;
}
void initGSM(){
//initializes GSM
//GSM needs 1 seconds of zero pulse to get started (Or halted)
    DDRC |= 0b01000000;
    
    
    PORTC &= 0b10111111;   
    delay_ms(1100);
    PORTC |= 0b01000000;
        
    
    return;
}

//monitor GSM at the startup
//GSM Initialize
bool startupMsg(){   
    char buf[40];
    bool flag = true;
    lcdputs("Getting started");
    
    GET_RX_STR_TIMEOUT = 20;
    gets1(buf, 20);
    
     
    if(buf[0] == 'N' || buf[1] == 'N' || buf[2] == 'N'){ //to skip noise chars
        debputsf1("Sim800 is ON already\r\n");
        delay_ms(2000);
        initGSM(); 
        gets1(buf, 20);      
    }
    else{
        debputsf1("Couldn't spot NORMAL POWER DOWN\r\n");
        debputsf1("msg: ");
        debputs1(buf);    
    }        
    
    debputsf1("RDY ");
    lcdputs("RDY");
    if(buf[0] != 'R' || buf[1] != 'D' || buf[2] != 'Y'){
        flag = false;
        debputsf1("failed\r\n");
        debputsf1("RDY: ");
        debputs1(buf);
        debputsf1("\r\n");
        }  
    else{
        debputsf1("\r\nRDY PASS: ");
        debputs1(buf);
        debputsf1("\r\n");
    }
    
    GET_RX_STR_TIMEOUT = 10;   
    gets1(buf, 12);
    
    debputsf1("+CFUN ");
    lcdputs("+CFUN");
              
    if(buf[0] != '+' || buf[1] != 'C' || buf[2] != 'F'){
        flag = false;
        debputsf1("failed\r\n");
        debputsf1("+CFUN: ");
        debputs1(buf);
        debputsf1("\r\n");
        }
    else{
        debputsf1("\r\n+CFUN PASS: ");
        debputs1(buf);
        debputsf1("\r\n");
    }
        
    gets1(buf, 15);
    
    debputsf1("+CPIN ");
    lcdputs("+CPIN");          
    if(buf[0] != '+' || buf[1] != 'C' || buf[2] != 'P'){
        flag = false;
        debputsf1("failed\r\n");
        debputsf1("+CPIN: ");
        debputs1(buf);
        debputsf1("\r\n");
        } 
    else{
        debputsf1("\r\n+CPIN PASS: ");
        debputs1(buf);
        debputsf1("\r\n");
    }
          
    fillZero(buf, 40);
    gets1(buf, 15);
    
    debputsf1("SMS Ready\r\n");
    lcdputs("SMS Ready");
          
    if(buf[0] != 'S' || buf[1] != 'M' || buf[2] != 'S'){
        flag = false;
        debputsf1("failed\r\n");
        debputsf1("SMS: ");
        debputs1(buf);
        debputsf1("\r\n");
        }    
    else{
        debputsf1("\r\nSMS PASS: ");
        debputs1(buf);
        debputsf1("\r\n");
    }
    fillZero(buf, 40);
    
    gets1(buf, 10); //Enter char
    gets1(buf, 40);
    debputsf1("+CZTV: ");
    debputs1(buf);
    debputsf1("\r\n");
    
    gets1(buf, 40);
    debputsf1("+PSUTTZ: ");
    debputs1(buf);
    debputsf1("\r\n");
    
    gets1(buf, 40);
    debputsf1("DST: ");
    debputs1(buf);
    debputsf1("\r\n");     
    
    gets1(buf, 40);
    debputsf1("+CIEV: ");
    debputs1(buf);
    debputsf1("\r\n");
    
    GET_RX_STR_TIMEOUT = GET_RX_STR_TIMEOUT_DEFAULT;
    debputsf1("Startup message done.\r\n");
    return flag;        
}
bool initialize_sim800(){
    bool output = false;
    
    lcdputsf("initialize GSM");
    initGSM(); 
    if(startupMsg()){ 
        output = true;
        debputsf1("GSM init successful\r\n");
        lcdputsf("GSM init successful");
        }
    else{
        output = false;
        debputsf1("GSM init failed\r\n");
        lcdputsf("GSM init failed");    
    }
    /*putsf("AT\r");
    gets1(okBuf, 20); 
    gets1(okBuf, 20);*/
    
    
    /*while(i < GSMinitTryagain){
        debputsf1("GSM startup msg try ");
        debputchar1(i + 48);
        debputsf1("\r\n");
        if(startupMsg()){
            i = GSMinitTryagain + 1;
            break;
        }
        //resetGSM();
        i++;
    }
    if(i != GSMinitTryagain + 1){
        debputsf1("GSM startup message completely failed\r\n");
        
        lcdputsf("GSM Initializing failed");
    }
    else{
        debputsf1("GSM startup message completely successful\r\n");
        lcdputsf("GSM Initializing successful");
    }
    delay_ms(500);*/
    
    /*
    do{ 
        debputsf1("CCWA ");
        sprintf(number_buf, "%d", i);
        debputs1(number_buf);
        fillZero(number_buf, 4);
        debputsf1("\r\n");
        i++;
        
        lcdputsf("CCWA");
        putsf("AT+CCWA=0\r");
        gets1(okBuf, 10);
        
        if(okBuf[3] != 'C' || okBuf[4] != 'C' || okBuf[5] != 'W' || okBuf[6] != 'A')
            continue;            
        gets1(okBuf, 4);
        delay_ms(500);
        }while(!findOK(okBuf, 4));
    debputsf1("CCWA SET\r\n");
    i = 0;
    do{ 
        debputsf1("CLIP ");
        sprintf(number_buf, "%d", i);
        debputs1(number_buf);
        fillZero(number_buf, 4);
        debputsf1("\r\n");
        i++;
        
        lcdputsf("CLIP");
        putsf("AT+CLIP=0\r");
        gets1(okBuf, 10);
        
        if(okBuf[3] != 'C' || okBuf[4] != 'L' || okBuf[5] != 'I' || okBuf[6] != 'P')
            continue;
            
        gets1(okBuf, 4);
        
        delay_ms(500);
        }while(!findOK(okBuf, 4));
    debputsf1("CLIP SET\r\n");
    i=0;
    
    do{ 
        debputsf1("CRC ");
        sprintf(number_buf, "%d", i);
        debputs1(number_buf);
        fillZero(number_buf, 4);
        debputsf1("\r\n");
        i++;
        
        lcdputsf("CRC");
        putsf("AT+CRC=0\r");
        gets1(okBuf, 10);
        
        if(okBuf[3] != 'C' || okBuf[4] != 'R' || okBuf[5] != 'C')
            continue;
            
        gets1(okBuf, 4);
        
        delay_ms(500);
        }while(!findOK(okBuf, 4));
    debputsf1("CRC SET\r\n");
    i=0;
     
    do{ 
        debputsf1("COLP");
        sprintf(number_buf, "%d", i);
        debputs1(number_buf);
        fillZero(number_buf, 4);
        debputsf1("\r\n");
        i++;
        
        lcdputsf("COLP");
        putsf("AT+COLP=0\r");
        gets1(okBuf, 10);
        
        if(okBuf[3] != 'C' || okBuf[4] != 'O' || okBuf[5] != 'L' || okBuf[6] != 'P')
            continue;
            
        gets1(okBuf, 4);
        
        delay_ms(500);
        }while(!findOK(okBuf, 4));
    debputsf1("COLP SET\r\n");
    i=0;
    
    do{ 
        debputsf1("CSSN");
        sprintf(number_buf, "%d", i);
        debputs1(number_buf);
        fillZero(number_buf, 4);
        debputsf1("\r\n");
        i++;
        
        lcdputsf("CSSN");
        putsf("AT+CSSN=0\r");
        gets1(okBuf, 10);
        
        if(okBuf[3] != 'C' || okBuf[4] != 'S' || okBuf[5] != 'S' || okBuf[6] != 'N')
            continue;
            
        gets1(okBuf, 4);
        
        delay_ms(500);
        }while(!findOK(okBuf, 4));
    debputsf1("CSSN SET\r\n");
    */
    /*do{ 
        debputsf1("CPMS ");
        sprintf(number_buf, "%d", i);
        debputs1(number_buf);
        fillZero(number_buf, 4);
        debputsf1("\r\n"); 
        i++;
        
        lcdputsf("CPMS");
        putsf("AT+CPMS=\"ME\",\"SM\",\"ME\"\r\n");
        delay_ms(500);
        }while(!find_CPMS_OK());
    debputsf1("CPMS SET\r\n");
    i=0;
    
    do{ 
        debputsf1("CNMI ");
        sprintf(number_buf, "%d", i);
        debputs1(number_buf);
        fillZero(number_buf, 4);
        debputsf1("\r\n");
        i++;    
    
        lcdputsf("CNMI");
        putsf("AT+CNMI=1,1,0,0,0\r");
        gets1(okBuf, 20);
        
        if(okBuf[3] != 'C' || okBuf[4] != 'N' || okBuf[5] != 'M' || okBuf[6] != 'I')
            continue;
            
        gets1(okBuf, 4);
        
        delay_ms(500);
        }while(!findOK(okBuf, 4));
    debputsf1("CNMI SET\r\n");*/
    lcdputsf("initialize done");
    debputsf1("GSM initialize done\r\n"); 
    
    return output;
}
