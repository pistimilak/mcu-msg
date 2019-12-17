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

/*Control chars*/
#define CTRL_START_MSG      '{'
#define CTRL_STOP_MSG       '}'
#define CTRL_START_OBJ      '('
#define CTRL_STOP_OBJ       ')'
#define CTRL_OBJ_FLAG       '@'
#define CTRL_KEY_FLAG       '$'
#define CTRL_KEY_SEP        ';'
#define CTRL_KEY_EQU        '='


/**
 * @brief Argument char is control char or not
 * 
 * @param c char
 * @return uint8_t comparison result
 */
static uint8_t is_ctrl_char(char c)
{
    switch(c) {
        case CTRL_START_MSG:
        case CTRL_STOP_MSG:
        case CTRL_START_OBJ:
        case CTRL_STOP_OBJ:
        case CTRL_OBJ_FLAG:
        case CTRL_KEY_FLAG:
        case CTRL_KEY_SEP:
        case CTRL_KEY_EQU:
            return 1;
        default:
            return 0;
    }
}


/**
 * @brief Argument char is valid keyword char or not
 * 
 * @param c char
 * @return uint8_t comparison result
 */
static uint8_t is_valid_keyword_char(char c)
{
    return (c == '_') || (c >= 'a' && c <= 'z') || 
                    (c >= 'A' && c <= 'Z') || 
                    (c >= '0' && c <= '9') ? 1 : 0;
}


/**
 * @brief strlen implementation for internal usage
 * 
 * @param str string 
 * @return mcu_msg_size_t length of string 
 */
static mcu_msg_size_t str_len(char *str)
{
    mcu_msg_size_t res = 0;
    char *p = str;
    while(*p) {
        res++;
        p++;
    }
    return res;
}

/**
 * @brief find the position of the keyword int message string (first occurance)
 * if the keyword found in the message string, the next (none space) char must be stopc
 * @param msg message
 * @param len message length
 * @param keyword keword has to be found
 * @param flagc flag, eg. '@', '$'
 * @param stopc stop character eg. '(', '='
 * @return char* location of the keyword (whitout flag) or NULL if keyword not found
 */
static char *find_keyword(char *msg, mcu_msg_size_t len, char *keyword, char flagc, char stopc)
{
    char *p = msg;
    char *loc;
    uint8_t equal;
    mcu_msg_size_t i;
    mcu_msg_size_t key_len = str_len(keyword);
    while((p - msg) < len && *p) {
        if(((p - msg) < len - 1) && *p == flagc) {
            loc = p + 1;
            // exp_key = keyword;
            equal = 1;
            for(i = 0; (loc + i - msg) < len && i < key_len; i++) {
                if((*(loc + i) != *(keyword + i)) || is_ctrl_char(*(loc + i)) || 
                                    !is_valid_keyword_char(*(loc + i))) {
                    equal = 0;
                    break;
                }
            }
            while((loc + i - msg) < len && *(loc + i) == ' ') i++; //skip spaces
            if(equal && *(loc + i) == stopc) {
                return loc;
            } else {
                p = loc + i;
            }
        }
        p++;
    }
    return NULL;
}


/**
 * @brief Getting the value start position for particular key in message object
 * 
 * @param obj message object
 * @param key key for search
 * @return char* location of the value start point or NULL if the key was not found
 */
static char *find_val(mcu_msg_obj_t obj, char *key)
{
    char *loc = find_keyword(obj.content, obj.content_len, key, CTRL_KEY_FLAG, CTRL_KEY_EQU); //object start with @ and terminated with space or '('
    char *p;
    if(loc == NULL) { //if keyword not found, return with NULLs and 0 lengths
        return NULL;
    }
    p = loc + str_len(key);
    if(*p != CTRL_KEY_EQU) {
        while(((p - obj.content) < (obj.content_len - 1)) && *p != CTRL_KEY_EQU) p++;
    }
    p++;
    while((p - obj.content) < obj.content_len && *p == ' ') p++; //skip spaces after equal
    return p;
}



mcu_msg_obj_t mcu_msg_parser_get_obj(char *msg, char *obj, mcu_msg_size_t len)
{
    mcu_msg_obj_t res;
    char *loc = find_keyword(msg, len, obj, CTRL_OBJ_FLAG, CTRL_START_OBJ); //object start with @ and terminated with space or '('
    char *p;
    if(loc == NULL) { //if keyword not found, return with NULLs and 0 lengths
        res.name = NULL;
        res.content = NULL;
        res.name_len = 0;
        res.content_len = 0;
        return res;
    }

    res.name = loc;
    res.name_len = str_len(obj);
    p = loc + res.name_len;
    if (*p != CTRL_START_OBJ) {
        while(((p - msg) < (len - 1)) && *p != CTRL_START_OBJ) p++;
    }
    res.content = ++p;
    res.content_len = 0;
    while((p - msg) < len && *p != CTRL_STOP_OBJ) {
        p++;
        res.content_len++;
    }
    return res;
}

int8_t mcu_msg_parser_get_int(int *res_val, mcu_msg_obj_t obj, char *key)
{
    char *p = find_val(obj, key);
    mcu_msg_size_t i;
    unsigned m = 1;
    int sign = 1;
    int8_t res = 0; // result of function

    if(p == NULL) { //key nout found
        // printf(">> key not found\n");
        return -1;
    }
    // printf(">> %s\n", p);

    switch(*p) { //if the sign is defined, set the sign variable and increment the pointer
        case '+':
            sign = 1;
            p++;
        break;
        
        case '-':
            sign = -1;
            p++;
        break;
        
        default:
        break;
    }

    for(i = 0; (p - obj.content) < obj.content_len && *p != ' ' && *p != CTRL_KEY_SEP; i++, p++) { //move to the end of the value string with i
        
        if(*p < '0' || *p > '9') {    // if non valid number, return with error
            return -1;
        }
    }

    *res_val = 0;
    --p;
    while(i--) {
        printf(">> %c\n", *p);
        *res_val += (*p-- - '0') * m;
        m *= 10;
        res++;
    }
    
    *res_val *= sign; //corrigate with the sign

    return res; // return with the digit count, if correct
}


float mcu_msg_parser_get_float(mcu_msg_obj_t obj, char *key)
{
    float res;

    return 0.0;    
}

void mcu_msg_parser_get_string(char *dest, mcu_msg_obj_t obj, char *key)
{
    
}
