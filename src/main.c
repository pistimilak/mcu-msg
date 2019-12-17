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


const char *test_str1 = "{@obj1($key11 =   -1123334567  ; $key12 = 123) @obj2  ($key21 = 2.1; $key22   = 'val22'; $key23 = 1000; $key24 = 12.34)}";

int main()
{
    int i;
    // char *c = find_keyword(test_str1,strlen(test_str1), "key22", '$', '=');
    // if (c != NULL)
    //     printf("found: %s\n", c);
    
    mcu_msg_obj_t obj1, obj2;
    obj1 = mcu_msg_parser_get_obj((char *)test_str1, "obj1", strlen(test_str1));

    printf("obj1.name_len: %d obj1.content_len: %d\n", obj1.name_len, obj1.content_len);
    for(i = 0; i < obj1.name_len; printf("%c",*(obj1.name + i)), i++);
    printf(":");
    for(i = 0; i < obj1.content_len; printf("%c",*(obj1.content + i)), i++);
    printf("\n");



    obj2 = mcu_msg_parser_get_obj((char *)test_str1, "obj2", strlen(test_str1));

    printf("obj2.name_len: %d obj2.content_len: %d\n", obj2.name_len, obj2.content_len);
    for(i = 0; i < obj2.name_len; printf("%c",*(obj2.name + i)), i++);
    printf(":");
    for(i = 0; i < obj2.content_len; printf("%c",*(obj2.content + i)), i++);
    printf("\n");

    int val = 0, res;
    res = mcu_msg_parser_get_int(&val, obj1, "key11");
    // if((res = mcu_msg_parser_get_int(&val, obj1, "key11")) != -1) {
    printf("r = %d val = %d\n", res, val);
    // }
    return 0;
}


