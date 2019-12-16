/**
 * @file mcu_msg_parser.c
 * @author Istvan Milak (istvan.milak@gmail.com)
 * @brief Simple message parser for embedded applications, eg serial comm
 * @version 0.1
 * @date 2019-12-15
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <stdio.h>
#include "mcu_msg_parser.h"

/*
format:
{@obj1($key11 = 11; $key12 = 'val12') @obj2($key21 = 2.1; $key22 = 'val22')}
*/

/*temporary buff*/
// static char buff[TMP_BUFF_SIZE] = {'\0'};
// static char *pbuff = buff;

/*
#define check_buff_state()      (((pbuff - buff) < TMP_BUFF_SIZE) ? 1 : 0)
*/

static mcu_msg_size_t   str_len(char *str)
{
    mcu_msg_size_t res = 0;
    char *p = str;
    while(*p) {
        res++;
        p++;
    }
    return res;
}

mcu_msg_obj_t mcu_msg_parser_get_obj(char *msg, char *obj, mcu_msg_size_t len)
{
    char *p = msg;
    char *pobj_loc;
    char *pobj_expected;
    mcu_msg_obj_t res;
    mcu_msg_size_t obj_len = str_len(obj);
    mcu_msg_size_t i;
    uint8_t equal;

    while((p - msg) < len && *p) {
        if(*p == '@' && ((p - msg) < len - 1)) { // if a object start
            pobj_loc = (p + 1); // location of found object
            pobj_expected = obj; // expected object pointer
            equal = 1;
            for(i = 0; (((pobj_loc + i) - msg) < len) && (i < obj_len); i++) { //comparison
                if(*(pobj_loc + i) != *(pobj_expected + i)) {
                    equal = 0;
                    break;
                }
            }
            
            while((((pobj_loc + i) - msg) < len) && *(pobj_loc + i) == ' ') i++; // skip spaces

            if((((pobj_loc + i) - msg) < len) && equal && //if comparison is OK and not a substring
                (*(pobj_loc + i) == '(')) {
                    res.name = pobj_loc;                //storing object name location
                    res.name_len = obj_len;             //storing object name length
                    res.content = pobj_loc + ++i;       //storing object content location
                    res.content_len = 0;                //calculate and storing content length
                    while((((pobj_loc + i) - msg) < len) && *(pobj_loc + i) != ')') {
                        i++;
                        res.content_len++;
                    }
                    return res;
            } else {
                p = pobj_loc + i;
            }
            
        }
        p++;
    }

    //if not found, return NULL char pointers and 0 lens;
    res.name = NULL;
    res.name_len = 0;
    res.content = NULL;
    res.content_len = 0;
    return res;
}


int mcu_msg_parser_get_int(mcu_msg_obj_t obj, char *key)
{
    
}