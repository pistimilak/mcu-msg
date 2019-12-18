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
#define CTRL_MSG_FLAG       '#'
#define CTRL_START_MSG      '{'
#define CTRL_STOP_MSG       '}'
#define CTRL_START_OBJ      '('
#define CTRL_STOP_OBJ       ')'
#define CTRL_OBJ_FLAG       '@'
#define CTRL_KEY_FLAG       '$'
#define CTRL_KEY_SEP        ';'
#define CTRL_KEY_EQU        '='
#define CTRL_CMD_FLAG       '!'


void mcu_msg_destroy_string(mcu_msg_string_t *str)
{
    str->s = NULL;
    str->len = 0;
}

void mcu_msg_destroy(mcu_msg_t *msg)
{
    mcu_msg_destroy_string(&msg->id);
    mcu_msg_destroy_string(&msg->content);
}

void mcu_msg_destroy_obj(mcu_msg_obj_t *obj)
{
    mcu_msg_destroy_string(&obj->id);
    mcu_msg_destroy_string(&obj->content);
}



/**
 * @brief Argument char is control char or not
 * 
 * @param c char
 * @return uint8_t comparison result
 */
static uint8_t is_ctrl_char(char c)
{
    switch(c) {
        case CTRL_MSG_FLAG:
        case CTRL_START_MSG:
        case CTRL_STOP_MSG:
        case CTRL_START_OBJ:
        case CTRL_STOP_OBJ:
        case CTRL_OBJ_FLAG:
        case CTRL_KEY_FLAG:
        case CTRL_KEY_SEP:
        case CTRL_KEY_EQU:
        case CTRL_CMD_FLAG:
            return 1;
        default:
            return 0;
    }
}



static uint8_t is_whitespace(char c)
{
    switch(c) {
        case  ' ':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
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


static char *skip_internal_str(char *start)
{
    char *p = start;
    char qmark = *start;
    if((qmark != '\'' && qmark != '"')) return NULL;
    ++p;
    while(*p && (*p != qmark)) {
        p++;
    }
    
    return *p ? ++p : NULL;
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
        if(*p == '\'' || *p == '"') { //skip internal strings
            p = skip_internal_str(p);
        }
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
            while((loc + i - msg) < len && is_whitespace(*(loc + i))) i++; //skip spaces
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
    char *loc = find_keyword(obj.content.s, obj.content.len, key, CTRL_KEY_FLAG, CTRL_KEY_EQU); //object start with @ and terminated with space or '('
    char *p;
    if(loc == NULL) { //if keyword not found, return with NULLs and 0 lengths
        return NULL;
    }
    p = loc + str_len(key);
    if(*p != CTRL_KEY_EQU) {
        while(((p - obj.content.s) < (obj.content.len - 1)) && *p != CTRL_KEY_EQU) p++;
    }
    p++;
    while((p - obj.content.s) < obj.content.len && is_whitespace(*p)) p++; //skip spaces after equal
    return p;
}


mcu_msg_t mcu_msg_get(char *raw_str, char *id, mcu_msg_size_t len)
{
    mcu_msg_t res;
    char *loc = find_keyword(raw_str, len, id, CTRL_MSG_FLAG, CTRL_START_MSG); //object start with @ and terminated with space or '('
    char *p;
    if(loc == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mcu_msg_destroy(&res);
        return res;
    }
    res.id.s = loc;
    res.id.len = str_len(id);
    p = loc + res.id.len;
    if (*p != CTRL_START_MSG) {
        while(((p - raw_str) < (len - 1)) && *p != CTRL_START_MSG) p++;
    }
    res.content.s = ++p;
    res.content.len = 0;
    while((p - raw_str) < len && *p != CTRL_STOP_MSG) {
        p++;
        res.content.len++;
    }
    return res;
}



mcu_msg_obj_t mcu_msg_parser_get_obj(mcu_msg_t msg, char *id)
{
    mcu_msg_obj_t res;
    char *loc = find_keyword(msg.content.s, msg.content.len, id, CTRL_OBJ_FLAG, CTRL_START_OBJ); //object start with @ and terminated with space or '('
    char *p;
    if(loc == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mcu_msg_destroy_obj(&res);
        return res;
    }

    res.id.s = loc;
    res.id.len = str_len(id);
    p = loc + res.id.len;
    if (*p != CTRL_START_OBJ) {
        while(((p - msg.content.s) < (msg.content.len - 1)) && *p != CTRL_START_OBJ) p++;
    }
    res.content.s = ++p;
    res.content.len = 0;
    while((p - msg.content.s) < msg.content.len && *p != CTRL_STOP_OBJ) {
        p++;
        res.content.len++;
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

    if(p == NULL)  //key nout found
        return -1;


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

    for(i = 0; (p - obj.content.s) < obj.content.len && !is_whitespace(*p) && *p != CTRL_KEY_SEP; i++, p++) { //move to the end of the value string with i
        if(*p < '0' || *p > '9') {    // if non valid number, return with error
            return -1;
        }
    }

    *res_val = 0;
    --p;
    while(i--) {
        *res_val += (*p-- - '0') * m;
        m *= 10;
        res++;
    }
    
    *res_val *= sign; //corrigate with the sign

    return res; // return with the digit count, if correct
}


int8_t mcu_msg_parser_get_float(float *res_val, mcu_msg_obj_t obj, char *key)
{
    char *p = find_val(obj, key);
    char *pf;
    mcu_msg_size_t i;
    unsigned m = 1;
    float mf = 0.1;
    int sign = 1;
    int8_t res = 0; // result of function

    if(p == NULL)  //key nout found
        return -1;


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

    //move p to dec separator or end of the value
    for(i = 0; (p - obj.content.s) < obj.content.len && !is_whitespace(*p) && *p != CTRL_KEY_SEP && *p != '.'; i++, p++) { 
        if((*p < '0' || *p > '9')) {    // if non valid number, return with error
            return -1;
        }
    }

    *res_val = 0.0;
    
    if(*p == '.') {
        pf = p + 1;
        res++;
    } else {
        pf = NULL;
    }

    --p;
    while(i--) {
        *res_val += (*p-- - '0') * m;
        m *= 10;
        res++;
    }
    
    // calculate floating point section after '.' (if there is)
    for(; pf != NULL && (pf - obj.content.s) < obj.content.len && !is_whitespace(*pf) && *pf != CTRL_KEY_SEP; pf++) {
        if(*pf < '0' || *pf > '9') {    // if non valid number, return with error
            return -1;
        }
        *res_val += (*pf - '0') * mf;
        mf /= 10;
        res++; 
    }

    *res_val *= sign; //corrigate with the sign

    return res; // return with the digit count + '.' separator, if correct
}



mcu_msg_string_t mcu_msg_parser_get_string(mcu_msg_obj_t obj, char *key)
{
    char *p = find_val(obj, key);
    char qmark;
    mcu_msg_string_t res;

    if(p == NULL) {
        mcu_msg_destroy_string(&res);
        return res;
    }
        
    qmark = *p;

    if(qmark != '\'' && qmark != '"') { // qmark not found, this is not a string
        mcu_msg_destroy_string(&res);
        return res;
    }

    res.s = ++p;
    res.len = 0;
    while((p - obj.content.s) < obj.content.len && *p != qmark) {
        res.len++;
        p++;
    }
    return res;
}

/**
 * @brief Default string copy to char array
 * 
 * @param dest destination char array
 * @param source source string type
 */
static void mcu_msg_str_copy_to_chr_arr(char *dest, mcu_msg_string_t source)
{
    mcu_msg_size_t i;
    for(i = 0; i < source.len; *(dest + i) = *(source.s + i), i++);
}

/**
 * @brief Default string copy to string type
 * 
 * @param dest destination string type
 * @param source source string type
 */
static void mcu_msg_str_copy(mcu_msg_string_t dest, mcu_msg_string_t source)
{
    mcu_msg_size_t i;
    for(i = 0, dest.len = source.len; i < source.len; *(dest.s + i) = *(source.s + i), i++);
}


mcu_msg_string_hnd_t mcu_msg_string_hnd_create(void (*print)(mcu_msg_string_t))
{
    mcu_msg_string_hnd_t hnd;
    hnd.copy_to_chr_arr = mcu_msg_str_copy_to_chr_arr;
    hnd.copy = mcu_msg_str_copy;
    hnd.print = print;
    return hnd;
}