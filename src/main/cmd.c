//All instructions requested by the user are done here 
// (Some instructiosn are not fully implemented because of no use)
//enum CMD{cycle, error, aw, dr, wr, mr, pbo}

//All functions below are simple 
// (Except manual functions which are a little more complicated and their code is less readable tbh)

bool manual_fail = false;


void ERROR_CMD(char index){
    unsigned char buf[3];
    debputsf1("\r\nERROR_CMD IS EXECUTED\r\n"); 
    debputsf1("cmd index: ");
    sprintf(buf, "%d", index);
    debputs1(buf);
    fillZero(buf, 3);
    debputsf1("\r\n");
    return;
}

void cycle_command(char index){
    
    //Cycle command will find the new timings and use the same 48 timing slots
    /*
    The reason that I didn't implement real time checking of time to save more RAM
    is that I tried my best to keep timers short and simple.
    This task of calculating time each half seconds for each of 8 outputs
    would cause problems for MCU interrupts.
    (By calculating time I mean the MCU has 2 numbers, the entire period and the duty cycle
     If entire period is 45m and the time is 13:00. for each second MCU has to add up many 45m to reach
     13:00 and then go for the duty cycle) (I did my best to keep timers instructions as little as possible)
    */
    int first = 0;
    unsigned char second = 0, secondLoc = 8;
    char i = 7; //cycle:<num>
    
    debputsf1("Cycle init\r\n");
    debputsf1("Cycle output: ");
    debputchar1(cmd_output_index[index] + 48);
    debputsf1("\r\n");
    
    output_cmd_change[cmd_output_index[index]] = true;
    timing_mode[cmd_output_index[index]] = SEQUENCE;
    while(i<20){
        if(cmdBuf[i][index] == '-')
            break;
        first *= 10;
        first += cmdBuf[i][index] - 48;
        i++;
        secondLoc++;
    }
    i++;
    
    while(i<20){
        if(cmdBuf[i][index] == 0)
            break;
        second *= 10;
        second += cmdBuf[i][index] - 48;
        i++;
        secondLoc++;
    }
        if(first > 30){//only 48 timing slots are considered so 
            //less than 30 minutes will cause a problem (30 min * 48 slots = 24 hour) 
            
            debputsf1("input Cycles are valid (more than 30 min)\r\n");
            durationMin[cmd_output_index[index]] = first;
            cycleMin[cmd_output_index[index]] = second;
            cmdDone[index] = true;
        }
        else
            debputsf1("input Cycles are invalid (less than 30 min)\r\n");    

    return;    
}


void error_command(char index){
    debputsf1("Error init\r\n");
    reportFlag = cmdBuf[6][index] - 48;
    //puts1("done error cmd\r\n");
    cmdDone[index] = true;
    return;    
}


void aw_command(char index){
    debputsf1("AW init\r\n");
    adaptiveWatering = cmdBuf[3][index] - 48;
    cmdDone[index] = true;
    //puts1("done aw cmd\r\n");
    return;    
}


void dr_command(char index){
    debputsf1("DR init\r\n");
    cmdDone[index] = true;
    //puts1("done dr cmd\r\n");    
}


void wr_command(char index){
    debputsf1("WR init\r\n");
    cmdDone[index] = true;
    //puts1("done wr cmd\r\n");    
}


void mr_command(char index){
    debputsf1("MR init\r\n");
    cmdDone[index] = true;
    //puts1("done mr cmd\r\n");    
}


void pbo_command(char index){
    debputsf1("PBO init\r\n");
    powerOut_report = cmdBuf[4][index] - 48;
    cmdDone[index] = true;
    //puts1("done pbo cmd\r\n");
    return;    
}

/*
bool timing_mode[OUT_COUNT]; //0 = manual, 1 = sequential
bool output_state[OUT_COUNT]; //0 = SWITCH_MANUAL, 1 = TIMING
enum timing_mode{MANUAL, SEQUENCE};
enum output_state{SWITCH_MANUAL, TIMING};
*/
void time_command(char index){//switchs from manual mode to Timed mode
    debputsf1("Time cmd ");
    debputchar1(cmd_output_index[index] + 48);
    debputsf1(" init(turn on timing state)\r\n");
    output_state[cmd_output_index[index]] = TIMING;
    cmdDone[index] = true;
    return;            
}


void switch_command(char index){//changes to manual mode and sets an On/Off mode
    unsigned char i = 0;
    debputsf1("Switch cmd init(switch state)\r\n");   
    
    //0123456    789
    //switch<num> off/on
    for(i=7; i<18; i++){
        if(cmdBuf[i][index] == 'o'){
            if(cmdBuf[i+1][index] == 'n' && cmdBuf[i+2][index] == 0){
                output_state[cmd_output_index[index]] = SWITCH_MANUAL;
                delay_ms(50); //to prevent any issues if timeCheck is going to run soon
                
                cmdDone[index] = true;
                debputsf1("Switch turn on ");
                debputchar1(cmd_output_index[index] + 48);
                debputsf1("\r\n");
                turnOn_output(cmd_output_index[index]);
                break;
                }
            else if(cmdBuf[i+1][index] == 'f' && cmdBuf[i+2][index] == 'f' && cmdBuf[i+3][index] == 0){
                output_state[cmd_output_index[index]] = SWITCH_MANUAL;
                delay_ms(50);//to prevent any issues if timeCheck is going to run soon
                
                cmdDone[index] = true;
                debputsf1("Switch turn off ");
                debputchar1(cmd_output_index[index] + 48);
                debputsf1("\r\n");
                turnOff_output(cmd_output_index[index]);
                break;
                } 
            else output_state[cmd_output_index[index]] = TIMING;   
        }
        else output_state[cmd_output_index[index]] = TIMING;    
    }    
    return;
}


//Manual (start reading with  Manual_command func) ------------------------------------------

/*
Manual command asks user for 4 series of 12 cycles of timings for the corresponding output
(ex: 12:23-13:00 this is one line. each cycle is wrote in a new line)
And removes unvalid ones (numbers range or form of syntax) and applies them

User can put "end" at the end of cycles(which was given by the user) 
to let the MCU know the end of fewer cycles (less than 48)
*/
void getCycle(char *str){
    char input = 0;
    gets1(str, 12);
    input =  *(str+11);
    if(input == '\r' || input == '\n')
        *(str+11) = 0; //to have a clean state for checkCycle function
    //11 chars( 00:00-00:00 ) for cycle and a \0 at the end for fixing the string       
    return;
}
void assignCycle(char *str, unsigned char timing_index, unsigned char out_indx){
    //struct wateringTime startOut[OUT_COUNT][TIME_INTERVAL_COUNT];
    //struct wateringTime stopOut[OUT_COUNT][TIME_INTERVAL_COUNT];
    unsigned char temp = 0;
    temp = (*str - 48) * 10;
    temp += (*(str+1) - 48);
    startOut[out_indx][timing_index].H = temp;
    
    temp = (*(str+3) - 48) * 10;
    temp += (*(str+4) - 48);
    startOut[out_indx][timing_index].M = temp;
    
    temp = (*(str+6) - 48) * 10;
    temp += (*(str+7) - 48);
    stopOut[out_indx][timing_index].H = temp;
    
    temp = (*(str+9) - 48) * 10;
    temp += (*(str+10) - 48);
    stopOut[out_indx][timing_index].M = temp;
    
        
    return;
}


void sub_checkCycle(char *ret, char *str){
    
    *ret &= checkifnum(*str);
    if(*str > '2')*ret = 0;
    str++;
    *ret &= checkifnum(*str);
    if(*(str-1) == '2' && *str > '3')*ret = 0;
    str++;
    *ret &= (*str == ':');
    str++;   
    *ret &= checkifnum(*str);
    if(*str > '5')*ret = 0;
    str++;    
    *ret &= checkifnum(*str);  
}
bool checkCycle(char *str){
    unsigned char output = true;
    
    sub_checkCycle(&output, str);
    str += 5;
    output &= (*str == 45); // 45 ascii minus
    str++;    
    sub_checkCycle(&output, str);    
    str += 5;
    output &= (*str == 0);
    
    return output;    
}


bool check_end_cmd(char *str){
    char i = 0;
    while(i < 9){
        if(*(str+i) == 'e' || *(str+i) == 'E')
        if(*(str+i+1) == 'n' || *(str+i+1) == 'N')
        if(*(str+i+2) == 'd' || *(str+i+2) == 'D')
        if(*(str+i+3) == 0)
            return true;
            
        i++;
    }
     
    return false;
}

void deleteCycle(unsigned char index, unsigned char cycle_count, unsigned char out_index){
    //struct wateringTime startOut[OUT_COUNT][TIME_INTERVAL_COUNT];
    //struct wateringTime stopOut[OUT_COUNT][TIME_INTERVAL_COUNT];
    unsigned char  i, j;
    j = cycle_count;
    if(j == 48) j--; //to prevent out of index access
    for(i=index; i < j; i++){
        startOut[out_index][i].H = startOut[out_index][i+1].H;
        startOut[out_index][i].M = startOut[out_index][i+1].M;
        stopOut[out_index][i].H  = stopOut[out_index][i+1].H;
        stopOut[out_index][i].M  = stopOut[out_index][i+1].M;            
    }
    if(cycle_count == 48){
        startOut[out_index][i].H = 61;
        startOut[out_index][i].M = 61;
        stopOut[out_index][i].H  = 61;
        stopOut[out_index][i].M  = 61;
    }
        
    
    return;   
}
/*bool time_between_period(unsigned char timeH, unsigned char timeM,
unsigned char interv1H, unsigned char interv1M, unsigned char interv2H, unsigned char interv2M)
{

    return false;
}*/

bool sub_check_timing_conflict(unsigned char o, 
                              unsigned char first, unsigned char second)
{
    //check all 4 timings if they are between any of timings 
    //struct wateringTime startOut[OUT_COUNT][TIME_INTERVAL_COUNT];
    //struct wateringTime stopOut[OUT_COUNT][TIME_INTERVAL_COUNT];
   
    //first
    //first start to second interval
    if(compTime(startOut[o][first].H, startOut[o][first].M,
                stopOut[o][second].H, stopOut[o][second].M) >= 1)
        if(compTime(startOut[o][second].H, startOut[o][second].M,
                    startOut[o][first].H, startOut[o][first].M) >= 1)
            return false;
            
    
    //second
    //first stop to second interval
    if(compTime(stopOut[o][first].H, stopOut[o][first].M,
                stopOut[o][second].H, stopOut[o][second].M) >= 1)
        if(compTime(startOut[o][second].H, startOut[o][second].M,
                    stopOut[o][first].H, stopOut[o][first].M) >= 1)
            return false; 
    
    //third
    //second start to first interval
    if(compTime(startOut[o][second].H, startOut[o][second].M,
                stopOut[o][first].H, stopOut[o][first].M) >= 1)
        if(compTime(startOut[o][first].H, startOut[o][first].M,
                    startOut[o][second].H, startOut[o][second].M) >= 1)
            return false;
    
    //forth
    //second stop to first interval
    if(compTime(stopOut[o][second].H, stopOut[o][second].M,
                stopOut[o][first].H, stopOut[o][first].M) >= 1)
        if(compTime(startOut[o][first].H, startOut[o][first].M,
                    stopOut[o][second].H, stopOut[o][second].M) >= 1)
            return false;
        
    return true;    
}
void check_timing_conflict(unsigned char *time_count, unsigned char cmd_index){
    unsigned char buffer[4];
    unsigned char i, j;
    unsigned char temp = *time_count;
    fillZero(buffer, 4);
    
    temp--;
    for(i=0; i<temp; i++)
        for(j=i+1; j<=temp; j++){
            if(!sub_check_timing_conflict(cmd_output_index[cmd_index], i, j)){
                debputsf1("deleting cycle ");
                sprintf(buffer, "%d", j);
                debputs1(buffer);
                debputsf1(". conflicts with ");
                sprintf(buffer, "%d", i);
                debputs1(buffer);
                debputsf1("\r\n");
                
                deleteCycle(j, *time_count, cmd_output_index[cmd_index]);
                *time_count -= 1;
                j--;
                temp--;
                }          
        }
    return;    
}


bool get_cycle_notif(unsigned char *cmd_index){
    GET_RX_STR_TIMEOUT = 30;
    if( getNotification(cmd_index) != 1) return false;
    if(!checkNumber()) return false;   
    return true;
}
bool get_series_of_cycles(unsigned char *cycle, unsigned char *cycle_count,
                          unsigned char index, unsigned char msg_index){
    unsigned char j=0;
    unsigned char valid_cycle_count = 0;
    unsigned char buf[3];
    fillZero(buf, 3);
    
    debputsf1("Getting series of cycles\r\n");
    GET_RX_STR_TIMEOUT = 2;
    while(j < 12){
        fillZero(cycle+12*valid_cycle_count, 12);
        getCycle(cycle+12*valid_cycle_count);

        if(check_end_cmd(cycle+12*j)){
            debputsf1("Got end word\r\n");
            gets(buf, 3);
            fillZero(buf, 3);
            break;
        }
               
        if(checkCycle(cycle+12*j)){
           assignCycle(cycle+12*valid_cycle_count, *cycle_count, cmd_output_index[index]);
           *cycle_count += 1;
           valid_cycle_count++;
           }
        else{
            sprintf(buf, "%d", j);
            debputsf1("Cycle ");
            debputs1(buf);
            fillZero(buf, 3);
            debputsf1(" is invalid\r\n");   
        }
                   
       j++;     
    }
    
    gets(buf, 3);
    debputsf1("end of Getting series of cycles\r\n");
    deleteMessage(msg_index);
    GET_RX_STR_TIMEOUT = 15;
    if(j >= 12){
        debputsf1("Getting cycles not ended yet\r\nproceeding to next series\r\n");
        return false;
        }
    return true;
}
void manual_command_setup(unsigned char *report_buf, unsigned char cmd_index){
    char i = 0;
    sprintf(report_buf, "Getting output %d cycles:", cmd_output_index[cmd_index] + 1);
    debputs1(report_buf);
    debputsf1("\r\n");
    while(i < 3){
        if(sendSMS(report_buf))
            break;
        delay_ms(500);
        i++;
    }
    if(i == 3){
        manual_fail = true;
        lcdputsf("Manual sms failed"); 
        debputsf1("Failed to send manual init SMS\r\n");
        
        return;    
        }   
    fillZero(report_buf, 20);
    delay_ms(100);    
}
void manual_command_cycle_setup(unsigned char *report_buf, unsigned char i){
    char x = 0;
    sprintf(report_buf, "Send cycle series %d:", i+1);
    debputs1(report_buf);
    debputsf1("\r\n");
    while(x < 3){
        if(sendSMS(report_buf))
            break;
        delay_ms(2000);
        x++;
    }
    if(x == 3){
        manual_fail = true;
        debputsf1("Failed to send manual getCycle SMS\r\n");
        lcdputsf("Cycle sms failed");
        return;
    }
    else{
        debputsf1("manual getCycle SMS successful\r\n");
        lcdputsf("Cycle sms successful");
    }       
    fillZero(report_buf, 20);        
}
void manual_command(char index){
    /*
    Procedural:
        1_ send start SMS
        2_ get cycles 12 by 12 (4 loops)
        3_ check each cycle to be valid (char type and length)
            *Only skip the corrupt cycles not all of them
        4_ assign each cycle 1 by 1
        5_ end the cycles with "end" line
        9_ delete rest of 48 cycles
        10_delete msg 
    */ 
    //timing_count for last cycle index
    //index for output index (to know which output cycles to work with)
    unsigned char report_buf[20];
    unsigned char buffer[12][12];
    unsigned char i=0, validSMS = false;
    unsigned char msg_index = 0;
    unsigned char timing_count = 0;
    
    manual_fail = false;
    
    debputsf1("Manual init\r\n");
    
    output_cmd_change[cmd_output_index[index]] = true;
    timing_mode[cmd_output_index[index]] = MANUAL;
    manual_command_setup(report_buf, index);
    if(manual_fail){
        delay_ms(50); //for Sim800 stability
        sendSMS("MANUAL EXECUTION FAILED\rignoring rest of cycles"); 
        // only '\r' (carriage return) is used for new line because in linux '\r' is enough
        debputsf1("\r\n\r\n !!!MANUAL EXECUTION FAILED\r\n\r\n");
        return;
    }       
    
    while(i < 4){           
        validSMS = false;       
        manual_command_cycle_setup(report_buf, i);
        if(manual_fail){
            delay_ms(50); //for Sim800 stability
            sendSMS("MANUAL EXECUTION FAILED");
            debputsf1("\r\n\r\n !!!MANUAL EXECUTION FAILED\r\n\r\n");
            return;
        }
                
        if(!get_cycle_notif(&msg_index)) {
            debputsf1("Manual SMS get notif failed\r\n");
            debputsf1("\r\n\r\n !!!MANUAL EXECUTION FAILED\r\n\r\n");
            return;
        }
        else debputsf1("Manual SMS get notif successful\r\n");
        GET_RX_STR_TIMEOUT = GET_RX_STR_TIMEOUT_DEFAULT;
                
        if(get_series_of_cycles(&buffer[0][0] , &timing_count, index, msg_index)){
            validSMS = true;
            break;
            }        
        validSMS = true;
        i++;                        
        }
    if(validSMS){
        debputsf1("Manual cmd is done properly\r\n");
        cmdDone[index] = true;
        }
    check_timing_conflict(&timing_count, index);
    empty_timing_rest(timing_count, cmd_output_index[index]);  
    return;
}
/*
    1. Run getCycle func 4 time to get 12 cycles each time
    2. end with "end" cmd
*/
//-----------------------------------------------------------------------------------


//enum CMD{NULL_CMD, cycle_cmd, error, aw, dr, wr, mr, pbo, manual, timing=10, switching = 9};
extern void (*command[11])(char) = {
    ERROR_CMD,
    cycle_command, error_command, aw_command, dr_command,
    wr_command, mr_command, pbo_command, manual_command,
    switch_command, time_command
    };


void report_execution_result(){
    unsigned char report_buf[25];
    unsigned char i , j=0;
    unsigned char done_cmd_count = 0; //report to user if even no cmd was done
    fillZero(report_buf, 25);
    for(i=0; i<CMD_COUNT; i++){
        if(cmdDone[i]){
            report_buf[j*2] = i + 49;
            report_buf[j*2+1] = ',';        
            j++;
            done_cmd_count++;
        }
    }
    
    j *= 2;
    report_buf[j] = ' ';
    j++;
    report_buf[j] = 'd';
    j++;
    report_buf[j] = 'o';
    j++;
    report_buf[j] = 'n';
    j++;
    report_buf[j] = 'e';
    j++;
    report_buf[j] = 0;
    
    if(done_cmd_count == 0){
        fillZero(report_buf, 25);
        sprintf(report_buf, "No command is done");    
    }
    sendSMS(report_buf);
    
    return;    
}
void sub_executeCMD(unsigned char index){
    //A switch was used before but a function pointer made it simpler
    //enum CMD{cycle, error, aw, dr, wr, mr, pbo, manual}
    
    
    (*command[cmdBuf_ident[index]])(index);
    //clear done bools for new message command instructions
    /*switch(cmdBuf_ident[index]){
        case cycle_cmd:
            debputsf1("Executing Cycle\r\n");
            cycle_command(index); 
            break;
        case error: 
            debputsf1("Executing Error\r\n");
            error_command(index);
            break;
        case aw:  
            debputsf1("Executing AW\r\n");
            aw_command(index);
            break;
        case dr:
            debputsf1("Executing DR\r\n"); 
            dr_command(index);
            break;
        case wr:
            debputsf1("Executing WR\r\n"); 
            wr_command(index);
            break;
        case mr: 
            debputsf1("Executing MR\r\n");
            mr_command(index);
            break;
        case pbo: 
            debputsf1("Executing PBO\r\n");
            pbo_command(index);
            break;
        case manual:
            debputsf1("Executing Manual\r\n"); 
            manual_command(index);
            break;
        case switching:
            debputsf1("Executing Switching\r\n");
            switch_command(index);
            break;
            
        case timing:
            debputsf1("Executing Timing\r\n");
            time_command(index);
            break;
        default:
            debputsf1("Wrong \"sub_executeCMD\" cmd index\r\n"); 
            break;
    }*/
    return;
}
void empty_cmdDone(){
    //fill the cmdDone array with zero (start clean)
    unsigned char i = 0;
    while(i < CMD_COUNT){
        cmdDone[i] = false;
        i++;
    }
    return;
}
void executeCMD(){
    //After all cmds are extracted from the SMS this function will execute all of the valid ones
    unsigned char c;
    unsigned char buffer[4];
    empty_cmdDone();
    for(c=0; c < cmd_count; c++){
        if(cmdValid[c]){
            debputsf1("\r\n Executing cmd ");
            sprintf(buffer, "%d", c);
            debputs1(buffer);
            fillZero(buffer, 4);
            debputsf1("\r\n"); 
            
            sub_executeCMD(c);
            }
    }
    
    
    return;
}
