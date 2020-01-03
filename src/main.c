/**
 * @file main.c
 * @author Istvan Milak (istvan.milak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-12-15
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "mcu_msg.h"

/*string printer on i386*/

char test_str1[] = "#test_msg{<CMD1>\t\t @obj1($key11=-1123334567  ; $key12 = 'string \"value\"')\n\
\r@obj2  ($key21 =   -1.123456789; $key22   = 'val22'; $key23 = 1000; $key24 = 12.34)<CMD_last>}";


void print_int(int i , int (*__putchar)(char));
void print_float(float f, uint8_t prec, int (*__putchar)(char));

int main()
{
    // print_int(-1000, putchar);
    // putchar('\n');
    // print_int(1000, putchar);
    // putchar('\n');
    // print_int(-128, putchar);
    // putchar('\n');
    // print_int(127, putchar);
    // putchar('\n');
    // print_int(256, putchar);
    // putchar('\n');
    // print_int(-32768, putchar);
    // putchar('\n');
    // print_int(32767, putchar);
    // putchar('\n');
    // print_int(65535, putchar);
    // putchar('\n');
    // print_int(-2147483648, putchar);
    // putchar('\n');
    // print_int(2147483647, putchar);
    // putchar('\n');
    // print_int(4294967295, putchar);
    // putchar('\n');

    // print_float(-1000.1234, 4,putchar);
    // putchar('\n');
    // print_float(3.1234, 4,putchar);
    // putchar('\n');
    // print_float(-123.56789, 6,putchar);
    // putchar('\n');
    // print_float(-456.12, 2,putchar);
    // putchar('\n');
    // print_float(67.128789, 4,putchar);
    // putchar('\n');

    // return;
    /*start the clock*/
    clock_t begin = clock();
    clock_t end;
    double exec_time = 0.0;



///////////////////////////////////////////////////////////////////////////////////////////////
//                                 MCU-MSG Parser test                                       //
///////////////////////////////////////////////////////////////////////////////////////////////

    msg_t msg;
    msg_obj_t obj1, obj2;
    msg_cmd_t cmd;
    msg_hnd_t hnd = msg_hnd_create(putchar);
    
    printf("TEST mcu-msg-parser\n");
    printf("===================\n");

    printf("test_str1 = \"%s\"\n\n", test_str1);

    //##############################################################################################
    printf(">> getting test_msg...\n");
    msg = msg_get(test_str1, "test_msg", sizeof(test_str1));
    if(msg.content.s != NULL) {
        printf("msg.id_len: %d msg.content_len: %d\n", msg.id.len, msg.content.len);
        hnd.print_str(msg.id);
        printf(":");
        hnd.print_str(msg.content);
        printf("\n\n");
    } else {
        printf("message not found!\n\n");
    }

    //##############################################################################################
    printf(">> getting CMD1 cmd...\n");
    cmd = msg_parser_get_cmd(msg, "CMD1");
    printf("%s\n\n", cmd.cmd.s ? "True" : "False");

    printf(">> getting CMD2 cmd...\n");
    cmd = msg_parser_get_cmd(msg, "CMD2");
    printf("%s\n\n", cmd.cmd.s ? "True" : "False");

    printf(">> getting CMD_last cmd...\n");
    cmd = msg_parser_get_cmd(msg, "CMD_last");
    printf("%s\n\n", cmd.cmd.s ? "True" : "False");

    //##############################################################################################
    printf(">> getting obj1...\n");
    obj1 = msg_parser_get_obj(msg, "obj1");
    printf("obj1.id_len: %d obj1.content_len: %d\n", obj1.id.len, obj1.content.len);
    hnd.print_str(obj1.id); printf(":"); hnd.print_str(obj1.content);
    printf("\n\n");

    //##############################################################################################
    printf(">> getting obj2...\n");
    obj2 = msg_parser_get_obj(msg, "obj2");
    printf("obj2.id_len: %d obj2.content_len: %d\n", obj2.id.len, obj2.content.len);
    hnd.print_str(obj2.id); printf(":"); hnd.print_str(obj2.content);
    printf("\n\n");
    
    //##############################################################################################
    printf(">> getting obj1->key11 integer...\n");
    int ival = 0, res;
    float fval = 0.0;
    res = msg_parser_get_int(&ival, obj1, "key11");
    printf("r = %d ival = %d\n\n", res, ival);
    
    //##############################################################################################
    printf(">> getting obj2->key21 float...\n");
    res = msg_parser_get_float(&fval, obj2, "key21");
    printf("r = %d fval = %.11f\n\n", res, fval);

    //##############################################################################################
    printf(">> getting obj1->key12 string...\n");
    msg_str_t str = msg_parser_get_string(obj1, "key12");
    if(str.s != NULL) {
        hnd.print_str(str); printf(" len: %d\n\n", str.len);
    } else {
        printf("error getting string\n\n");
    }
    
    


///////////////////////////////////////////////////////////////////////////////////////////////
//                                 MCU-MSG Wrapper test                                      //
///////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER  

    printf("TEST mcu-msg-wrapper\n");
    printf("====================\n");

    // msg_wrap_hnd_t wrapper_hnd = msg_wrapper_hnd_create(putchar);
    msg_wrap_t msg_wrap;
    msg_wrap_obj_t obj1_wrap, obj2_wrap;
    msg_wrap_str_t str1, str2, str3;
    msg_wrap_int_t i1, i2;
    msg_wrap_float_t f1, f2;
    msg_wrap_cmd_t cmd_wrap, cmd_rem;
    msg_t msg_reparsed;
    msg_obj_t obj_reparsed;
    float f_val; 
    int i_val;
    
    msg_wrap = msg_wrapper_init_msg("wrapped_msg");
    printf("Initialized msg: #"); hnd.print_str(msg_wrap.id); printf("\n\n");
    
    obj1_wrap = msg_wrapper_init_obj("wrapped_obj1");
    printf("Initialized obj: @"); hnd.print_str(obj1_wrap.id); printf("\n\n");


    str1 = msg_wrapper_init_string("str1", "This is \"string 1\"");
    printf("Initialized str: $"); hnd.print_str(str1.id); putchar('='); hnd.print_str(str1.content); printf("\n\n");
    
    str2 = msg_wrapper_init_string("str2", "This is 'string 2'");
    printf("Initialized str: $"); hnd.print_str(str2.id); putchar('='); hnd.print_str(str2.content); printf("\n\n");

    str3 = msg_wrapper_init_string("str3", ".... \"string 3\"");
    printf("Initialized str: $"); hnd.print_str(str3.id); putchar('='); hnd.print_str(str3.content); printf("\n\n");

    /* Preapare object */
    printf("Add '$str2', '$str3' to '@wrapped_obj1'...\n\n");
    msg_wrapper_add_string_to_obj(&obj1_wrap, &str2);
    msg_wrapper_add_string_to_obj(&obj1_wrap, &str3);

    printf("Add '@wrapped_obj1' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_object_to_msg(&msg_wrap, &obj1_wrap);

    cmd_wrap = msg_wrapper_init_cmd("CMD_WRAP");
    printf("Initialized cmd: <"); hnd.print_str(cmd_wrap.cmd); printf(">\n\n");
    
    cmd_rem = msg_wrapper_init_cmd("CMD_REMOVEABLE");
    printf("Initialized cmd: <"); hnd.print_str(cmd_rem.cmd); printf(">\n\n");

    i1 = msg_wrapper_init_int("i1", -3244);
    printf("Initialized int: $"); hnd.print_str(i1.id); printf("=%d\n\n", i1.val);

    i2 = msg_wrapper_init_int("i2", 456789);
    printf("Initialized int: $"); hnd.print_str(i2.id); printf("=%d\n\n", i2.val);
    
    f1 = msg_wrapper_init_float("f1", 1.23456, 6);
    printf("Initialized float: $"); hnd.print_str(f1.id); printf("=%.6f\n\n", f1.val);

    f2 = msg_wrapper_init_float("f2", -0.3345, 6);
    printf("Initialized float: $"); hnd.print_str(f2.id); printf("=%.6f\n\n", f2.val);

    obj2_wrap = msg_wrapper_init_obj("wrapped_obj2");
    printf("Initialized obj: @"); hnd.print_str(obj2_wrap.id); printf("\n\n");

    printf("Add '$i1', '$i2', '$f1', '$f2' and '$str1' to '@wrapped_obj2'\n\n");
    msg_wrapper_add_int_to_obj(&obj2_wrap, &i1);
    msg_wrapper_add_int_to_obj(&obj2_wrap, &i2);
    msg_wrapper_add_float_to_obj(&obj2_wrap, &f1);
    msg_wrapper_add_float_to_obj(&obj2_wrap, &f2);
    msg_wrapper_add_string_to_obj(&obj2_wrap, &str1);

    printf("Add '<CMD_WRAP>' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_cmd_to_msg(&msg_wrap, &cmd_wrap);

    printf("Add '@wrapped_obj2' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_object_to_msg(&msg_wrap, &obj2_wrap);
    
    printf("Add '<CMD_REMOVABLE>' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_cmd_to_msg(&msg_wrap, &cmd_rem);

    printf("Removing $str2 from @wrapped_obj1...\n\n");
    msg_wrapper_rm_string_from_obj(&obj1_wrap, &str2);
    
    printf("Removing $str1 from @wrapped_obj2...\n\n");
    msg_wrapper_rm_string_from_obj(&obj2_wrap, &str1);

    printf("Removing $f1 from @wrapped_obj2...\n\n");
    msg_wrapper_rm_float_from_obj(&obj2_wrap, &f1);

    printf("Removing $i2 from @wrapped_obj2...\n\n");
    msg_wrapper_rm_int_from_obj(&obj2_wrap, &i2);

    printf("Removing @wrapped_obj1 from #wrapped_msg...\n\n");
    msg_wrapper_rm_obj_from_msg(&msg_wrap, &obj1_wrap);

    printf("Removing <CMD_REMOVABLE> from #wrapped_msg...\n\n");
    msg_wrapper_rm_cmd_from_msg(&msg_wrap, &cmd_rem);

    printf("Add '@wrapped_obj1' AGAIN to '#wrapped_msg'...\n\n");
    msg_wrapper_add_object_to_msg(&msg_wrap, &obj1_wrap);

    printf("Wrapped message:\n");
    printf("---------------\n\n");
    hnd.print_wrapper_msg(msg_wrap);


    printf("\n\n");

#if MCU_MSG_USE_BUFFERING
    char buff[1000] = {0};
    msg_size_t freespc; // free space in buffer
    msg_str_buff_t sbuff = msg_init_str_buff(buff, 1000);
    
    freespc = wrapper_hnd.print_to_buff(&sbuff, msg_wrap);
    printf("Wrapped message (buffered, free space: %d)\n", freespc);
    freespc = wrapper_hnd.print_to_buff(&sbuff, msg_wrap);
    printf("Wrapped message (buffered again, free space: %d)\n", freespc);
    
    printf("\n\n");
    printf("Buffer content:\n");
    printf("%s\n\n", buff);

    printf("Reparsing '#wrapped_msg' '@wrapped_obj2'...\n\n");
    msg_reparsed = msg_get(buff, "wrapped_msg", 1000);
    hnd.print(msg_reparsed.content); printf("\n\n");
    obj_reparsed = msg_parser_get_obj(msg_reparsed, "wrapped_obj2");
    hnd.print(obj_reparsed.content); printf(" len: %d\n\n", obj_reparsed.content.len);
    msg_parser_get_float(&f_val, obj_reparsed, "f2");
    msg_parser_get_int(&i_val, obj_reparsed, "i1");
    printf("reparsed $i = %d $f2 = %f\n\n", i_val, f_val);
#endif

    printf("\n\n");
#endif

    end = clock();
    exec_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Execution time: %f s\n", exec_time);
    return 0;
}

void print_int(int i , int (*__putchar)(char))
{
    int8_t sign = i < 0 ? -1: 1;
    unsigned val = i < 0 ? ~i + 1 : i;
                                  // 4294967295   65535
    unsigned div = sizeof(int) > 2 ? 1000000000UL : 10000UL;
    uint8_t dig;
    uint8_t first_dig = 0;
    if(sign == -1) __putchar('-');
    while(div) {
        dig = 0;
        while(val >= div) {
            val -= div;
            dig += 1;
        }
        div /= 10;
        if(!first_dig && dig) {
            first_dig = 1;
        }
        if(first_dig) __putchar('0' + dig);
    }
}

void print_float(float f, uint8_t prec, int (*__putchar)(char))
{
    int i_part = (int)f;
    float f_part = f < 0 ? (f - (float)i_part) * -1  : (f - (float)i_part);
    unsigned mul = 1;
    uint8_t j;
    for(j = 0; j < prec; j++ ) mul *= 10;
    print_int(i_part, __putchar);
    __putchar('.');
    print_int((unsigned)(f_part * mul), __putchar);
}

/*
void *thread_mcu_master_fnc(void *arg)
{
    
    msg_str_buff_t *common_buff = (msg_str_buff_t *)arg;
    msg_t answer;
    msg_string_hnd_t hnd = msg_string_hnd_create(putchar);
    msg_wrap_hnd_t wrap_hnd = msg_wrapper_hnd_create(putchar);
    msg_wrap_t msg_fan_ctrl = msg_wrapper_init_msg("FAN");
    msg_wrap_cmd_t cmd_fan1_enable = msg_wrapper_init_cmd("Fan1_Enabled");
    msg_wrap_cmd_t cmd_fan2_enable = msg_wrapper_init_cmd("Fan2_Enabled");
    msg_wrap_obj_t fan1 = msg_wrapper_init_obj("Fan1_Ctrl");
    msg_wrap_int_t fan1_speed = msg_wrapper_init_int("Fan1_Speed", 540);
    msg_wrap_float_t sens1_alarm = msg_wrapper_init_float("Sens1_AlarmLimit_C", 32.5, 2);
    msg_wrap_obj_t fan2 = msg_wrapper_init_obj("Fan2_Ctrl");
    msg_wrap_int_t fan2_speed = msg_wrapper_init_int("Fan2_Speed", 3021);
    msg_wrap_float_t sens2_alarm = msg_wrapper_init_float("Sens2_AlarmLimit_C", 34.234, 2);
    msg_wrapper_add_float_to_obj(&fan1, &sens1_alarm);
    msg_wrapper_add_int_to_obj(&fan1, &fan1_speed);
    msg_wrapper_add_float_to_obj(&fan2, &sens2_alarm);
    msg_wrapper_add_int_to_obj(&fan2, &fan2_speed);
    msg_wrapper_add_object_to_msg(&msg_fan_ctrl, &fan1);
    msg_wrapper_add_cmd_to_msg(&msg_fan_ctrl, &cmd_fan1_enable);
    msg_wrapper_add_object_to_msg(&msg_fan_ctrl, &fan2);
    msg_wrapper_add_cmd_to_msg(&msg_fan_ctrl, &cmd_fan2_enable);

    msg_wrap_t msg_sys_ctrl = msg_wrapper_init_msg("SYS");;
    msg_wrap_cmd_t cmd_lcd_clear = msg_wrapper_init_cmd("CMD_LCD_Clear");
    msg_wrap_str_t lcd_default_str = msg_wrapper_init_string("LCD_Def", "Fan Control System");


    if(!wrap_hnd.print_to_buff(common_buff, msg_sys_ctrl)) printf("Buffer is full!");
    if(!wrap_hnd.print_to_buff(common_buff, msg_fan_ctrl)) printf("Buffer is full!");

    do {
        answer = msg_get(common_buff->buff.s, "slave_answer", common_buff->buff.len);
        usleep(100);
    } while(!answer.content.len);
    printf("[Master MCU] Salve sent answer:");
    hnd.print(answer.content);
    
    fan1_speed.val = 4095;
    fan2_speed.val = 2048;
    sens1_alarm.val = 42.35;
    msg_wrapper_rm_cmd_from_msg(&msg_fan_ctrl, &cmd_fan1_enable);
    msg_wrapper_rm_cmd_from_msg(&msg_fan_ctrl, &cmd_fan2_enable);
    

    msg_reset_str_buff(common_buff); // reset writer pointer to start of buffer
    if(!wrap_hnd.print_to_buff(common_buff, msg_sys_ctrl)) printf("Buffer is full!"); // send new 
    
    do {
        answer = msg_get(common_buff->buff.s, "slave_answer", common_buff->buff.len);
        usleep(100);
    } while(!answer.content.len);
    printf("[Master MCU] Salve sent answer:");
    


    return;
}



void *thread_mcu_slave_fnc(void *arg)
{
    
    msg_str_buff_t *common_buff = (msg_str_buff_t *)arg;
    msg_t msg_from_master;
    msg_obj_t fan1_obj;
    msg_wrap_hnd_t hnd = msg_wrapper_hnd_create(putchar);
    msg_wrap_t answ_msg = msg_wrapper_init_msg("slave_answer");
    msg_wrap_cmd_t cmd_sys_done = msg_wrapper_init_cmd("SYSTEM_Tasks_Done");

    int fan1_speed;
    float fan1_sens_lim;

    do {
        msg_from_master = msg_get(common_buff->buff.s, "SYS", common_buff->buff.len);
        usleep(100);
    } while(!msg_from_master.content.len);

    do {
        msg_from_master = msg_get(common_buff->buff.s, "FAN", common_buff->buff.len);
        usleep(100);
    } while(!msg_from_master.content.len);

    msg_parser_get_obj(msg_from_master, "Fan1_Ctrl");
    msg_parser_get_int(&fan1_speed, fan1_obj, "Fan1_Speed");
    msg_parser_get_float(&fan1_sens_lim, fan1_obj, "Sens1_AlarmLimit_C");
    msg_wrapper_add_cmd_to_msg(&answ_msg, &cmd_sys_done);

    printf("[Slave MCU] fan1_speed = %d, fan1_sens_lim: %f\n", fan1_speed, fan1_sens_lim);
    do {
        msg_from_master = msg_get(common_buff->buff.s, "FAN", common_buff->buff.len);
        usleep(100);
    } while(!msg_from_master.content.len);

    msg_parser_get_obj(msg_from_master, "Fan1_Ctrl");
    msg_parser_get_int(&fan1_speed, fan1_obj, "Fan1_Speed");
    msg_parser_get_float(&fan1_sens_lim, fan1_obj, "Sens1_AlarmLimit_C");
    msg_wrapper_add_cmd_to_msg(&answ_msg, &cmd_sys_done);
    printf("[Slave MCU] fan1_speed = %d, fan1_sens_lim: %f\n", fan1_speed, fan1_sens_lim);


}
*/