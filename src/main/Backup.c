//This file which is based on EEPROM.c, contains all backup and restore precedures
//Timings (Sequential & Manual), important bools 
//Locations of variables in EEPROM are all calculated before writing in the main code:

//note: Internal EEPROM is not used. all backups are done on the external one (24C32)

/*
settings:
    - option bools          (4 bytes)             (0x000 - 0x003)
    - timing_modes          (1 byte)              (0x004)  
    - whole_duration[8]     (2 * 8 bytes)         (0x005 - 0x014)
    - cycle_duration[8]     (1 * 8 bytes)         (0x015 - 0x01C)
    - output_state          (1 byte)              (0x01D)
    - Switch_key_out_index	(1 byte)			  (0x630)
	*** few addresses are reserved (0x01E - 0x01F) for possible use
	- Timings[8][48]	(4 * 8 * 48 bytes)		  (0x020 - 0x620) 
	  (For manual mode)  
	- CMDs (in case power goes out in middle of executing them) (Add if caused problems irl)
*/

//Saves or restores the duty cycle and the entire period for the sequential timing on External EEPROM
bool backup_watering_cycles(unsigned char out_index){
    unsigned char stat = 0;   
    unsigned char buf[4];
    fillZero(buf, 2);
    
    //debug reports ----------------------------
    debputsf1("Backup watering cycle output ");
    sprintf(buf, "%d", out_index);
    debputs1(buf);
    fillZero(buf, 4);//just make sure
    debputsf1(":\r\n");
    
    debputsf1("Duration: ");
    sprintf(buf, "%d", durationMin[out_index]);
    debputs1(buf);
    fillZero(buf, 4);//just make sure
    
    debputsf1(", Duty cycle: ");
    sprintf(buf, "%d", cycleMin[out_index]);
    debputs1(buf);
    fillZero(buf, 4);//just make sure
    debputsf1(":\r\n"); 
    //------------------------------------------
     
    //save the current sequential timings for the given output index
    stat = writeInt( (5 + 2*out_index), durationMin[out_index]);
    if( stat != 0 ) //to stop continuing the function
        return stat;
    stat = writeChar(21 + out_index, cycleMin[out_index]);    
    
    return stat;
}
bool restore_watering_cycles(unsigned char out_index){
    unsigned char stat = 0;
    unsigned char buf[2];
    fillZero(buf, 2);
    
    // Debug report ------------------------------
    debputsf1("Restore watering cycle output ");
    sprintf(buf, "%d", out_index);
    debputs1(buf);
    fillZero(buf, 2);//just make sure
    debputsf1(":\r\n");    
    
    //--------------------------------------------
    
    //restore the current sequential timings for the given output index
    stat = readChar(&cycleMin[out_index], 21 + out_index);
    if( stat != 0 )
        return false;
    stat = readInt(&durationMin[out_index], (5 + 2*out_index));
    if( stat != 0 )
        return false;    
    
    return true;
}

//Timed or manual (switching) control bools
bool backup_output_state(){
    unsigned char i=0, temp = 0;
    
    debputsf1("Backup output state\r\n");
    
    for(i=0; i< OUT_COUNT; i++){
        temp |= (output_state[i] << i);    
    } 
    delay_ms(10); //EEPROM write cycle time   
    return writeChar(0x001D , temp);    
}
bool restore_output_state(){
    unsigned char temp, out = 0, i;
    
    debputsf1("Restore output state\r\n");
    
    delay_ms(10); //EEPROM write cycle time
    out = readChar(&temp, 0x001D);
    for(i=0; i<OUT_COUNT; i++){
        output_state[i] = ((temp & (1 << i)) >= 1);    
    }
    return out;
}

//if switch mode is on Save/Restore On/Off state
bool backup_output_switch(){//used for output on/off state
//if output is in SWITCH mode output on/off state must be preserved
    unsigned char buf[4];                              
    unsigned char i, temp = 0;
    fillZero(buf, 4);
    debputsf1("Backup output switch\r\n");
    for(i=0; i<OUT_COUNT; i++){
        if(output_state[i] == SWITCH_MANUAL)
            switch(i){
                case 0: 
                    temp |= (output0 << i);
                    break;
                case 1: 
                    temp |= (output1 << i);
                    break;
                case 2:
                    temp |= (output2 << i);
                    break;
                case 3:
                    temp |= (output3 << i);
                    break;
                case 4: 
                    temp |= (output4 << i);
                    break;
                case 5: 
                    temp |= (output5 << i);
                    break;
                case 6:
                    temp |= (output6 << i);
                    break;
                case 7:
                    temp |= (output7 << i);
                    break; 
                default: 
                    debputsf1("\r\n\r\n OUT OF INDEX OUTPUT (backup_output_switch)\r\n\r\n");
                    break;
                
            }
    }
    debputsf1("\r\n BACKUP OUT SWITCH: 0x");
    sprintf(buf, "%X", temp);
    debputs1(buf); 
    debputsf1("\r\n");
    fillZero(buf, 4); 
    return writeChar(0x001E , temp);  
}
bool restore_output_switch(){
    unsigned char i, temp = 0, out=0;
    unsigned char buf[4];                              
    fillZero(buf, 4);
    
    debputsf1("Restore output switch\r\n");
    out = readChar(&temp, 0x001E);
    
    debputsf1("\r\n RESTORE OUT SWITCH: 0x");
    sprintf(buf, "%X", temp);
    debputs1(buf);  
    debputsf1("\r\n");
    fillZero(buf, 4); 
    
    for(i=0; i<OUT_COUNT; i++){
        if(output_state[i] == SWITCH_MANUAL){
            if((temp & (1 << i)) >= 1)
                turnOn_output(i);
        } 
    }
    return out;
        
}

//If timed mode is selected Save/Restore Timing mode (Sequential/ Manual)
bool backup_timing_mode(){
    unsigned char i=0, temp = 0;
    
    debputsf1("Backup timing mode\r\n");
    
    for(i=0; i< OUT_COUNT; i++){
        temp |= (timing_mode[i] << i);    
    } 
    delay_ms(10);   
    return writeChar(0x0004 , temp);
}
bool restore_timing_mode(){
    unsigned char temp, out = 0, i;
    
    debputsf1("Restore timing mode\r\n");
    
    delay_ms(10);
    out = readChar(&temp, 0x0004);
    for(i=0; i<OUT_COUNT; i++){
        timing_mode[i] = ((temp & (1 << i)) >= 1);    
    }
    return out;
}

//For manual timing mode (save/restore all the periods)
//- cycles[8][48]	(2 * 8 * 48 bytes)	(0x028 - 0x328)
//struct wateringTime startOut[OUT_COUNT][TIME_INTERVAL_COUNT];
//struct wateringTime stopOut [OUT_COUNT][TIME_INTERVAL_COUNT];
bool sub_backup_timings(unsigned int starting_address, unsigned char out_idx){
    unsigned char index = 0, output = 1;
    unsigned char x = 0;
    unsigned char flag = 1;
    
    
    while(index < 12){
        output &= i2c_start();
        delay_ms(10);
        output &= i2c_write(0b10100000);
        delay_ms(10);
        output &= i2c_write(starting_address >> 8); 
        delay_ms(10);
        output &= i2c_write( (unsigned char)starting_address );
        starting_address += 16;
        
        for(x=0; x<4; x++){
            if(startOut[out_idx][4 * index + x].H == 61){
                if(flag != 1) {
                    break;
                    }
                flag = 0;
                }
            output &= diagnoseRom(i2c_write(startOut[out_idx][4 * index + x].H), write_rom);
            delay_ms(10);
            output &= diagnoseRom(i2c_write(startOut[out_idx][4 * index + x].M), write_rom);
            delay_ms(10);
            output &= diagnoseRom(i2c_write(stopOut[out_idx][4 * index + x].H), write_rom);
            delay_ms(10);
            output &= diagnoseRom(i2c_write(stopOut[out_idx][4 * index + x].M), write_rom);
            delay_ms(10);
        }
        if(flag == 2)break;
        else if(flag == 0 ) flag = 2;
        i2c_stop();
        index++;
    }
    
    return output;        
}
bool backup_timings(unsigned char o_indx){
    
    
    unsigned char stat = true;
    unsigned int page = 192*o_indx + 32;
    stat &= sub_backup_timings(page, o_indx);
    return stat;
}

bool sub_restore_timings(unsigned int starting_address, unsigned char out_idx, unsigned char *timing_index){
    unsigned char index = 0, output = 1;
    bool flag = true , ack = true;
    output &= i2c_start();
    delay_ms(10);
    output &= i2c_write(0b10100000);
    delay_ms(10);
    output &= i2c_write(starting_address >> 8);
    delay_ms(10);
    output &= i2c_write( (unsigned char)starting_address );
    delay_ms(10);
    output &= i2c_start();
    delay_ms(10);
    output &= i2c_write(0b10100001);
    
    delay_ms(10);
    while(index < TIME_INTERVAL_COUNT){

        startOut[out_idx][index].H = i2c_read(ack);
        delay_ms(10);          
        startOut[out_idx][index].M = i2c_read(ack);
        delay_ms(10);
        stopOut[out_idx][index].H = i2c_read(ack);
        delay_ms(10);
        if(startOut[out_idx][index].H == 61){
            ack = false;
            flag = false;
            }
        stopOut[out_idx][index].M = i2c_read(ack);
        delay_ms(10);
        if(!flag)break;
        index++;        
    }
    i2c_stop();
    *timing_index = index;
    return output;        
}
bool restore_timings(unsigned char o_indx){
    unsigned char i, stat = true;
    unsigned int page = 192*o_indx + 32;
    
    stat &= sub_restore_timings(page, o_indx, &i);
    
    empty_timing_rest(i, o_indx);
    return stat;
}


/*
bool reportFlag = 0;
bool debugFlag = 0;
bool adaptiveWatering = 0;
bool powerOut_report = 0;
*/
unsigned char backup_flags(){
    char temp;
    
    temp = (0 << 7) | (0 << 6) | (0 << 5) | (0 << 4) |
    (powerOut_report << 3) | ( adaptiveWatering << 2 ) | (0 << 1) | (reportFlag << 0);
    
    
    return writeChar(3, temp);
}
bool restore_flags(){
    char temp;
    char stat = 0;
    
    stat = readChar(&temp, 3);
    if(stat == 0){
        if( (temp & 0b00000001) != 0)
            reportFlag = true;
        else  reportFlag = false;

        if( (temp & 0b00000100) != 0)
            adaptiveWatering = true;
        else adaptiveWatering = false;
        if( (temp & 0b00001000) != 0)
            powerOut_report = true;
        else powerOut_report = false;
        return true;
    }
    return false;
}

//There is an On/Off pair of buttons which control one of the outputs
//This function restores which output was controlled before turning off
void restore_switch_key_index(){

    readChar((unsigned char*)&switch_key_out_index, 0x0630);
    debputsf1("Switch key restored\r\n");
}

//
void restore_output_states(){
    unsigned char i;
    for(i=0; i<OUT_COUNT; i++){
        if(timing_mode[i] == SEQUENCE){         //SEQUENCE
            restore_watering_cycles(i);
            //unsigned int durationMin[OUT_COUNT];
            //unsigned char cycleMin[OUT_COUNT];
            setCycleTime(durationMin[i], cycleMin[i], i);
            }
        else restore_timings(i);                //MANUAL            
    } 
    return;       
}

/*
In restoring all timings and switch states are restored
because user may wants to switch between Timing or manual modes 
(especially switching from manual to timing, all timings must be ready for that)
*/

void restoreSetting(){
    debputsf1("Restore setting init\r\n");
    restore_timing_mode();   //(bool) Restores timing modes (Seq or Manual) (used in restore_output_states and more)
    restore_flags();         //(bool) restores flags that are settings of the program
    restore_output_states(); //In this func depending on the timing mode restores the corresponding numbers
    restore_output_state();  //after restoring Timings, Now Timed/Manual mode will be restored
    restore_output_switch(); //Outputs that have manual switch mode on will have their previous state restored.
    debputsf1("ALL RESTORES DONE\r\n");
    return;
}


void backupSetting(){
    unsigned char i;
    
    time_check = false; //to prevent I2c interference with RTC module
    delay_ms(50);
    
    debputsf1("Backup setting init\r\n");
    
     
    debputsf1("Backing up flags\r\n");
    diagnoseRom(backup_flags(), write_rom);
    debputsf1("Backed up flags\r\n"); 
    
     
    debputsf1("Backing up timing modes\r\n");
    diagnoseRom(backup_timing_mode(), write_rom);
    debputsf1("Backed up timing modes\r\n");
    
     
    debputsf1("Backing up output state\r\n");
    diagnoseRom(backup_output_state(), write_rom);
    debputsf1("Backed up output state\r\n");
    
     
    debputsf1("Backing up output switch\r\n");
    diagnoseRom(backup_output_switch(), write_rom);
    debputsf1("Backed up output switch\r\n");
        
    for(i=0; i<OUT_COUNT; i++){
        if(output_cmd_change[i]){
            debputsf1("output ");
            debputchar1(i + 48);
            debputsf1(" backed up\r\n");
            if(timing_mode[i] == SEQUENCE){
                debputsf1("\r\nSetting sequential timings (setCycleTime)\r\n");
                setCycleTime(durationMin[i], cycleMin[i], i);
                debputsf1("backing up cycle and duty cycle\r\n");
                diagnoseRom(backup_watering_cycles(i), write_rom);
                }
            else {
                debputsf1("backing up all of start/stop timings (Manual)\r\n");
                backup_timings(i); //diagnose is implemnted in the function
                }
        }
        
        
        output_cmd_change[i] = false;
    }
    
    debputsf1("\r\nALL SETTINGS BACKUP DONE\r\n");
    
    time_check = true; //to prevent I2c interference with RTC module
    
    return;
}