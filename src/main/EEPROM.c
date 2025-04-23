//EEPROM functions (Based on I2C)
/*
void i2c_init(void);
unsigned char i2c_start(void);
void i2c_stop(void);
unsigned char i2c_read(unsigned char ack);
unsigned char i2c_write(unsigned char data);
*/
/*
enum readChar_diag{noError_r, first_start_r, rom_address_write_r,
    msb_address_r, lsb_address_r, second_start, rom_address_read};
      
enum writeChar_diag{noError_w, first_start_w, rom_address_write_w,
    msb_address_w,  lsb_address_w, write_char, write_int_m, write_int_l};
*/



bool sub_readChar(unsigned char num, unsigned int address){
    switch (num){
      case 1:
        return i2c_start();
      case 2:
        return i2c_write(0b10100000);
      case 3:
        return i2c_write(address >> 8);
      case 4:
        return i2c_write( (unsigned char)address );
      case 5:
        return i2c_start();
      case 6:
        return i2c_write(0b10100001);     
    }
return 0;
}
unsigned char readChar(unsigned char* data, unsigned int address){
    unsigned char x = 0, y = 0;
    
          
    for(y=1; y<7; y++){
        for(x=0; x<I2C_TRYAGAIN; x++){    
            if(sub_readChar(y, address)) break;
            delay_ms(10);
        }
        if(x == 10) return y;
    }
    *data = i2c_read(0);
    i2c_stop();
    return 0;    
}
unsigned char readInt(unsigned int* num, unsigned int address){
    char i[2];
    char x=0, y=0;
    
     for(y=1; y<7; y++){
        for(x=0; x<I2C_TRYAGAIN; x++){    
            if(sub_readChar(y, address)) break;
            delay_ms(10);
        }
        if(x == 10) return y;
    }   
    
    i[0] = i2c_read(1);
    i[1] = i2c_read(0);
    *num = (i[0] << 8) | i[1]; 
    i2c_stop();
    return 0;    
}

bool sub_writeChar(unsigned int num, unsigned int address, unsigned int write){
    switch (num){
      case 1:
        return i2c_start();
      case 2:
        return i2c_write(0b10100000);
      case 3:
        return i2c_write(address >> 8);
      case 4:
        return i2c_write( (unsigned char)address );
      case 5:
        return i2c_write( (unsigned char)write);
      case 6:
        return i2c_write(write >> 8); 
      case 7:
        return i2c_write( (unsigned char)write );    
    }
return 0;
}
unsigned char writeChar(unsigned int address, unsigned char write){
    unsigned char x = 0, y = 0;
      
    for(y=1; y<6; y++){
        for(x=0; x<I2C_TRYAGAIN; x++){    
            if(sub_writeChar(y, address, write)) break;
            //printf("%d", y);
            delay_ms(10);
        }
        if(x == 10) return y;
    }
    
    i2c_stop();    
    delay_ms(10);
    return 0;
}
unsigned char writeInt(unsigned int address, unsigned int write){
    unsigned char x = 0, y = 0;
      
    for(y=1; y<8; y++){
        if(y == 5) continue;
        for(x=0; x<I2C_TRYAGAIN; x++){    
            if(sub_writeChar(y, address, write)) break;
            //printf("%d", y);
            delay_ms(10);
        }
        if(x == 10) return y;
    }
    
    
    i2c_stop();    
    delay_ms(10);
    return 0;
}
