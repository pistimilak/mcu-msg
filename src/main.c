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


const char *test_str1 = "{@obj1($key11 = 11; $key12 = 'val12') @obj2  ($key21 = 2.1; $key22 = 'val22'; $key23 = 1000; $key24 = 12.34)}";

int main()
{
    int i; 
    mcu_msg_obj_t obj1;
    // printf("len: %d\n", strlen(test_str1));
    obj1 = mcu_msg_parser_get_obj((char *)test_str1, "obj2", strlen(test_str1));

    printf("obj2.name_len: %d obj2.content_len: %d\n", obj1.name_len, obj1.content_len);
    for(i = 0; i < obj1.name_len; printf("%c",*(obj1.name + i)), i++);
    printf(":");
    for(i = 0; i < obj1.content_len; printf("%c",*(obj1.content + i)), i++);
    printf("\n");

    return 0;
}


