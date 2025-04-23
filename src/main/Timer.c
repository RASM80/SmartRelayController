//This section is the first section was created
//means that it's the core of the project
//Keep track of timings and control outputs


//hour:min0 + min1 = newHour:newMin
void clockSum(char *hour, char *min0, int min1){
    *hour += (*min0 + min1) / 60;
    *min0 = (*min0 + min1) % 60;
    return;
}

//sub function in setCycleTime to set the stop time of given cycle struct 
void setStopTime(char H, char M, char DC, char i, unsigned char output_index){
    clockSum(&H, &M, DC);
    stopOut[output_index][i].H = H;
    stopOut[output_index][i].M = M; 
    return;       
}

//Compares 2 periods
char compTime(unsigned char hour0, unsigned char min0, 
              unsigned char hour1, unsigned char min1){ //return one if 1 > 0, 2 if equal, else zero
    if(hour0 == hour1 && min0 == min1)
        return 2;
    if(hour1 > hour0)
        return 1;
    else if(hour1 == hour0)
        if(min1 > min0)
            return 1;
    return 0;
}

//Calculates the time periods using The entire period and duty cycle numbers
void setCycleTime_init(int wholeCycle,char dutyCycle, unsigned char output_index){
    unsigned char buf[5]; 
     
    //debug purposes
    fillZero(buf, 5);
    debputsf1("setCycleTime(");
    sprintf(buf, "%d", wholeCycle);
    debputs1(buf);
    fillZero(buf, 5);
    debputsf1(", ");
    
    sprintf(buf, "%d", dutyCycle);
    debputs1(buf);
    fillZero(buf, 5);
    debputsf1(", ");
    
    sprintf(buf, "%d", output_index);
    debputs1(buf);
    fillZero(buf, 5);
    debputsf1(")\r\n");
    
    
    return;
}
void setCycleTime(int wholeCycle,char dutyCycle, unsigned char output_index){
    
    unsigned char hour = 0, min = 0, i=0;
    unsigned char testH = 0, testM = 0;
    unsigned char zeroFlag = 1; 
    
    setCycleTime_init(wholeCycle, dutyCycle, output_index);    
    
    while(i < TIME_INTERVAL_COUNT){
        if (zeroFlag != 0){        
            startOut[output_index][i].H = hour;
            startOut[output_index][i].M = min;
            
            setStopTime(hour, min, dutyCycle, i, output_index);            
            
            testH = hour;
            testM = min;
            clockSum(&testH, &testM, 2 * wholeCycle);
            if(compTime(24, 0, testH, testM) == 1){ 
                zeroFlag = 0;
                }
                
            clockSum(&hour, &min, wholeCycle);                  
        }
        else{ 
            startOut[output_index][i].H = 61;
            startOut[output_index][i].M = 61;
            stopOut[output_index][i].H = 61;
            stopOut[output_index][i].M = 61;
        }
            i++;
    }
                
}

//update time using RTC
void getTime(){
    rtc_get_time(&time.hour, &time.min, &time.sec);
    rtc_get_date(&time.weekDay, &time.date, &time.month, &time.year);
}

/*
Compares time with each output's time periods and determines
which output to be on or off
*/
void timeCheck(){
    unsigned int i = 0;
    unsigned char outputs = 0;
    bool flag = true;
    
    for(outputs=0; outputs<OUT_COUNT; outputs++){
        flag = true;
        i=0;
        if(output_state[outputs] == TIMING){
            while(i < TIME_INTERVAL_COUNT){
                if(startOut[outputs][i].H == 255) continue; //in case timing restoration goes wrong
                if(startOut[outputs][i].H == 61){
                    flag = false;
                    break;
                    }
                if(compTime(startOut[outputs][i].H, startOut[outputs][i].M, time.hour, time.min) >= 1){
                    if(compTime(time.hour, time.min, stopOut[outputs][i].H, stopOut[outputs][i].M) == 1){
                        
                        turnOn_output(outputs);
                        break;
                        }
                    }    
                i++;
            }
            if(!flag || i == TIME_INTERVAL_COUNT){
                turnOff_output(outputs);
                }
        }        
    }
}
