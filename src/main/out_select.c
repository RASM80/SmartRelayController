//A small extra file to make LCD_interface smaller
 
void past_out_selec_cmd(){
    
    (*past_out_select_cmd)();
    //past_out_select_cmd = &NO_CMD;
    return;
}


void selec_out_0(){
    output_index = 0;
    past_out_selec_cmd();
    return;
}
void selec_out_1(){
    output_index = 1;
    past_out_selec_cmd();
    return;
}
void selec_out_2(){
    output_index = 2;
    past_out_selec_cmd();
    return;
}
void selec_out_3(){
    output_index = 3;
    past_out_selec_cmd();
    return;
}
void selec_out_4(){
    output_index = 4;
    past_out_selec_cmd();
    return;
}
void selec_out_5(){
    output_index = 5;
    past_out_selec_cmd();
    return;
}
void selec_out_6(){
    output_index = 6;
    past_out_selec_cmd();
    return;
}
void selec_out_7(){
    output_index = 7;
    past_out_selec_cmd();
    return;
}