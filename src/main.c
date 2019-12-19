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
#include "mcu_msg_parser.h"

/*string printer on i386*/
void printf_mcu_msg_str(mcu_msg_string_t str);

const char *test_str1 = "#test_msg{<CMD1>\t\t @obj1($key11 =\t-1123334567  ; $key12 = 'string \"value\"')\n@obj2  ($key21 =   -1.123456789; $key22   = 'val22'; $key23 = 1000; $key24 = 12.34)}";


int main()
{

    
    mcu_msg_t msg;
    mcu_msg_obj_t obj1, obj2;
    mcu_msg_string_hnd_t str_hnd = mcu_msg_string_hnd_create(printf_mcu_msg_str);

    printf("TEST mcu-msg-parser\n");
    printf("-------------------\n");

    printf("test_str1 = \"%s\"\n\n", test_str1);

    printf(">> getting test_msg...\n");
    msg = mcu_msg_get(test_str1, "test_msg", strlen(test_str1));
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
    printf(">> getting obj1...\n");
    obj1 = mcu_msg_parser_get_obj(msg, "obj1");
    printf("obj1.id_len: %d obj1.content_len: %d\n", obj1.id.len, obj1.content.len);
    str_hnd.print(obj1.id); printf(":"); str_hnd.print(obj1.content);
    printf("\n\n");

    //##############################################################################################
    printf(">> getting obj2...\n");
    obj2 = mcu_msg_parser_get_obj(msg, "obj2");
    printf("obj2.id_len: %d obj2.content_len: %d\n", obj2.id.len, obj2.content.len);
    str_hnd.print(obj2.id); printf(":"); str_hnd.print(obj2.content);
    printf("\n\n");
    
    //##############################################################################################
    printf(">> getting obj1->key11 intiger...\n");
    int ival = 0, res;
    float fval = 0.0;
    res = mcu_msg_parser_get_int(&ival, obj1, "key11");
    printf("r = %d ival = %d\n\n", res, ival);
    
    //##############################################################################################
    printf(">> getting obj2->key21 float...\n");
    res = mcu_msg_parser_get_float(&fval, obj2, "key21");
    printf("r = %d fval = %.11f\n\n", res, fval);

    //##############################################################################################
    printf(">> getting obj1->key12 string...\n");
    mcu_msg_string_t str = mcu_msg_parser_get_string(obj1, "key12");
    if(str.s != NULL) {
        str_hnd.print(str); printf("\n\n");
    } else {
        printf("error getting string\n\n");
    }
    
    return 0;
}

void printf_mcu_msg_str(mcu_msg_string_t str)
{
    mcu_msg_size_t i;
    for(i = 0; i < str.len; printf("%c", *(str.s + i++)));
}
