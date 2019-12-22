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

const char *test_str1 = "#test_msg{<CMD1>\t\t @obj1($key11 =\t-1123334567  ; $key12 = 'string \"value\"')\n\
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

    // mmsg_t msg;
    // mmsg_obj_t obj1, obj2;
    // mmsg_cmd_t cmd;
    // mmsg_string_hnd_t str_hnd = mmsg_string_hnd_create(putchar);
    
    // printf("TEST mcu-msg-parser\n");
    // printf("-------------------\n");

    // printf("test_str1 = \"%s\"\n\n", test_str1);

    // //##############################################################################################
    // printf(">> getting test_msg...\n");
    // msg = mmsg_get(test_str1, "test_msg", strlen(test_str1));
    // if(msg.content.s != NULL) {
    //     printf("msg.id_len: %d msg.content_len: %d\n", msg.id.len, msg.content.len);
    //     str_hnd.print(msg.id);
    //     printf(":");
    //     str_hnd.print(msg.content);
    //     printf("\n\n");
    // } else {
    //     printf("message not found!\n\n");
    // }

    // //##############################################################################################
    // printf(">> getting CMD1 cmd...\n");
    // cmd = mmsg_parser_get_cmd(msg, "CMD1");
    // printf("%s\n\n", cmd.cmd.s ? "True" : "False");

    // printf(">> getting CMD2 cmd...\n");
    // cmd = mmsg_parser_get_cmd(msg, "CMD2");
    // printf("%s\n\n", cmd.cmd.s ? "True" : "False");

    // printf(">> getting CMD_last cmd...\n");
    // cmd = mmsg_parser_get_cmd(msg, "CMD_last");
    // printf("%s\n\n", cmd.cmd.s ? "True" : "False");

    // //##############################################################################################
    // printf(">> getting obj1...\n");
    // obj1 = mmsg_parser_get_obj(msg, "obj1");
    // printf("obj1.id_len: %d obj1.content_len: %d\n", obj1.id.len, obj1.content.len);
    // str_hnd.print(obj1.id); printf(":"); str_hnd.print(obj1.content);
    // printf("\n\n");

    // //##############################################################################################
    // printf(">> getting obj2...\n");
    // obj2 = mmsg_parser_get_obj(msg, "obj2");
    // printf("obj2.id_len: %d obj2.content_len: %d\n", obj2.id.len, obj2.content.len);
    // str_hnd.print(obj2.id); printf(":"); str_hnd.print(obj2.content);
    // printf("\n\n");
    
    // //##############################################################################################
    // printf(">> getting obj1->key11 integer...\n");
    // int ival = 0, res;
    // float fval = 0.0;
    // res = mmsg_parser_get_int(&ival, obj1, "key11");
    // printf("r = %d ival = %d\n\n", res, ival);
    
    // //##############################################################################################
    // printf(">> getting obj2->key21 float...\n");
    // res = mmsg_parser_get_float(&fval, obj2, "key21");
    // printf("r = %d fval = %.11f\n\n", res, fval);

    // //##############################################################################################
    // printf(">> getting obj1->key12 string...\n");
    // mmsg_string_t str = mmsg_parser_get_string(obj1, "key12");
    // if(str.s != NULL) {
    //     str_hnd.print(str); printf(" len: %d\n\n", str.len);
    // } else {
    //     printf("error getting string\n\n");
    // }
    
    


///////////////////////////////////////////////////////////////////////////////////////////////
//                                 MCU-MSG Wrapper test                                      //
///////////////////////////////////////////////////////////////////////////////////////////////
    
    // print_int(-2147483648L);
    // putchar('\n');
    // print_int(2147483647L);
    // putchar('\n');
    // print_int(-456L);
    // putchar('\n');
    // print_float(1.234, 3);
    // putchar('\n');
    // printf("printf(1.234) = %.3f\n", 1.234);
    // print_float(-456.823456, 8);
    // putchar('\n');
    // printf("printf(-456.823456) = %.6f\n", -456.823456);
    
    printf("TEST mcu-msg-wrapper\n");
    printf("-------------------\n");

    mmsg_wrap_hnd_t wrapper_hnd = mmsg_wrapper_hnd_create(putchar);
    mmsg_wrap_t msg_wrap;
    mmsg_obj_wrap_t obj1_wrap, obj2_wrap;
    mmsg_string_wrap_t str1, str2, str3;
    mmsg_int_wrap_t i1, i2;
    mmsg_float_wrap_t f1, f2;
    mmsg_cmd_wrap_t cmd_wrap;
    // const char * cmd_str = "CMD_BLABLA";

    msg_wrap = mmsg_wrapper_init_msg("wrapped_msg");
    obj1_wrap = mmsg_wrapper_init_obj("wrapped_obj1");
    str1 = mmsg_wrapper_init_string("str1", "This is \"string 1\"");
    str2 = mmsg_wrapper_init_string("str2", "This is \"string 2\"");
    str3 = mmsg_wrapper_init_string("str3", ".... \"string 3\"");
    
    /* Preapare object */
    // mmsg_wrapper_add_string_to_obj(&obj1_wrap, &str1);
    mmsg_wrapper_add_string_to_obj(&obj1_wrap, &str2);
    mmsg_wrapper_add_string_to_obj(&obj1_wrap, &str3);

    mmsg_wrapper_add_object_to_msg(&msg_wrap, &obj1_wrap);

    cmd_wrap = mmsg_wrapper_init_cmd("CMD_WRAP");
    i1 = mmsg_wrapper_init_int("i1", 0);
    i2 = mmsg_wrapper_init_int("i2", 456789);
    f1 = mmsg_wrapper_init_float("f1", 1.23456, 6);
    f2 = mmsg_wrapper_init_float("f2", -0.3345, 6);

    obj2_wrap = mmsg_wrapper_init_obj("wrapped_obj2");
    mmsg_wrapper_add_int_to_obj(&obj2_wrap, &i1);
    mmsg_wrapper_add_int_to_obj(&obj2_wrap, &i2);
    mmsg_wrapper_add_float_to_obj(&obj2_wrap, &f1);
    mmsg_wrapper_add_float_to_obj(&obj2_wrap, &f2);
    mmsg_wrapper_add_string_to_obj(&obj2_wrap, &str1);

    mmsg_wrapper_add_cmd_to_msg(&msg_wrap, &cmd_wrap);
    mmsg_wrapper_add_object_to_msg(&msg_wrap, &obj2_wrap);
    
    // wrapper_hnd.print_cmd(cmd_wrap);
    // putchar('\n');
    // wrapper_hnd.print_obj(obj1_wrap);
    // putchar('\n');
    // wrapper_hnd.print_obj(obj2_wrap);
    // putchar('\n');
    wrapper_hnd.print(msg_wrap);
    putchar('\n');

    end = clock();
    exec_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Execution time: %f s\n", exec_time);
    return 0;
}


