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
#include "mcu_msg.h"

/*string printer on i386*/

char *test_str1 = "#test_msg{<CMD1>\t\t @obj1($key11 =\t-1123334567  ; $key12 = 'string \"value\"')\n\
\r@obj2  ($key21 =   -1.123456789; $key22   = 'val22'; $key23 = 1000; $key24 = 12.34)<CMD_last>}";


void print_int(int i);
void print_float(float f, uint8_t prec);

int main()
{
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
    msg_string_hnd_t str_hnd = msg_string_hnd_create(putchar);
    
    printf("TEST mcu-msg-parser\n");
    printf("===================\n");

    printf("test_str1 = \"%s\"\n\n", test_str1);

    //##############################################################################################
    printf(">> getting test_msg...\n");
    msg = msg_get(test_str1, "test_msg", strlen(test_str1));
    if(msg.content.s != NULL) {
        printf("msg.id_len: %d msg.content_len: %d\n", msg.id.len, msg.content.len);
        str_hnd.print(msg.id);
        printf(":");
        str_hnd.print(msg.content);
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
    str_hnd.print(obj1.id); printf(":"); str_hnd.print(obj1.content);
    printf("\n\n");

    //##############################################################################################
    printf(">> getting obj2...\n");
    obj2 = msg_parser_get_obj(msg, "obj2");
    printf("obj2.id_len: %d obj2.content_len: %d\n", obj2.id.len, obj2.content.len);
    str_hnd.print(obj2.id); printf(":"); str_hnd.print(obj2.content);
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
        str_hnd.print(str); printf(" len: %d\n\n", str.len);
    } else {
        printf("error getting string\n\n");
    }
    
    


///////////////////////////////////////////////////////////////////////////////////////////////
//                                 MCU-MSG Wrapper test                                      //
///////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER  

    printf("TEST mcu-msg-wrapper\n");
    printf("====================\n");

    msg_wrap_hnd_t wrapper_hnd = msg_wrapper_hnd_create(putchar);
    msg_wrap_t msg_wrap;
    msg_wrap_obj_t obj1_wrap, obj2_wrap;
    msg_wrap_str_t str1, str2, str3;
    msg_wrap_int_t i1, i2;
    msg_wrap_float_t f1, f2;
    msg_wrap_cmd_t cmd_wrap, cmd_rem;
   
    
    msg_wrap = msg_wrapper_init_msg("wrapped_msg");
    printf("Initialized msg: #"); str_hnd.print(msg_wrap.id); printf("\n\n");
    
    obj1_wrap = msg_wrapper_init_obj("wrapped_obj1");
    printf("Initialized obj: @"); str_hnd.print(obj1_wrap.id); printf("\n\n");


    str1 = msg_wrapper_init_string("str1", "This is \"string 1\"");
    printf("Initialized str: $"); str_hnd.print(str1.id); putchar('='); str_hnd.print(str1.content); printf("\n\n");
    
    str2 = msg_wrapper_init_string("str2", "This is 'string 2'");
    printf("Initialized str: $"); str_hnd.print(str2.id); putchar('='); str_hnd.print(str2.content); printf("\n\n");

    str3 = msg_wrapper_init_string("str3", ".... \"string 3\"");
    printf("Initialized str: $"); str_hnd.print(str3.id); putchar('='); str_hnd.print(str3.content); printf("\n\n");

    /* Preapare object */
    printf("Add '$str2', '$str3' to '@wrapped_obj1'...\n\n");
    msg_wrapper_add_string_to_obj(&obj1_wrap, &str2);
    msg_wrapper_add_string_to_obj(&obj1_wrap, &str3);

    printf("Add '@wrapped_obj1' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_object_to_msg(&msg_wrap, &obj1_wrap);

    cmd_wrap = msg_wrapper_init_cmd("CMD_WRAP");
    printf("Initialized cmd: <"); str_hnd.print(cmd_wrap.cmd); printf(">\n\n");
    
    cmd_rem = msg_wrapper_init_cmd("CMD_REMOVEABLE");
    printf("Initialized cmd: <"); str_hnd.print(cmd_rem.cmd); printf(">\n\n");

    i1 = msg_wrapper_init_int("i1", -3244);
    printf("Initialized int: $"); str_hnd.print(i1.id); printf("=%d\n\n", i1.val);

    i2 = msg_wrapper_init_int("i2", 456789);
    printf("Initialized int: $"); str_hnd.print(i2.id); printf("=%d\n\n", i2.val);
    
    f1 = msg_wrapper_init_float("f1", 1.23456, 6);
    printf("Initialized float: $"); str_hnd.print(f1.id); printf("=%.6f\n\n", f1.val);

    f2 = msg_wrapper_init_float("f2", -0.3345, 6);
    printf("Initialized float: $"); str_hnd.print(f2.id); printf("=%.6f\n\n", f2.val);

    obj2_wrap = msg_wrapper_init_obj("wrapped_obj2");
    printf("Initialized obj: @"); str_hnd.print(obj2_wrap.id); printf("\n\n");

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
    wrapper_hnd.print(msg_wrap);
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
#endif
    printf("\n\n");
#endif

    end = clock();
    exec_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Execution time: %f s\n", exec_time);
    return 0;
}


