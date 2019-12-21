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
#include "mcu_msg.h"

/*Control chars*/
#define CTRL_MSG_FLAG           '#'
#define CTRL_START_MSG          '{'
#define CTRL_STOP_MSG           '}'
#define CTRL_START_OBJ          '('
#define CTRL_STOP_OBJ           ')'
#define CTRL_OBJ_FLAG           '@'
#define CTRL_KEY_FLAG           '$'
#define CTRL_KEY_SEP            ';'
#define CTRL_KEY_EQU            '='
#define CTRL_CMD_START_FLAG     '<'
#define CTRL_CMD_STOP_FLAG      '>'

/*putchar implementation: must be implemented for printing to UART or other output*/
static int (*mcu_msg_putc)(char) = NULL; 


/*destroy string*/
void mcu_msg_destroy_string(mcu_msg_string_t *str)
{
    str->s = NULL;
    str->len = 0;
}

/*destroy message*/
void mcu_msg_destroy(mcu_msg_t *msg)
{
    mcu_msg_destroy_string(&msg->id);
    mcu_msg_destroy_string(&msg->content);
}


/*destroy object*/
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
static inline uint8_t is_ctrl_char(char c)
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
        case CTRL_CMD_START_FLAG:
        case CTRL_CMD_STOP_FLAG:
            return 1;
        default:
            return 0;
    }
}

/**
 * @brief Char is a whitespace
 * 
 * @param c char
 * @return uint8_t comparison result
 */
static inline uint8_t is_whitespace(char c)
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
// static inline uint8_t is_valid_keyword_char(char c)
// {
//     return (c == '_') || (c >= 'a' && c <= 'z') || 
//                     (c >= 'A' && c <= 'Z') || 
//                     (c >= '0' && c <= '9') ? 1 : 0;
// }
#define is_valid_keyword_char(c)        ((c == '_') || \
                                        (c >= 'a' && c <= 'z') || \ 
                                        (c >= 'A' && c <= 'Z') || \
                                        (c >= '0' && c <= '9'))

/**
 * @brief strlen implementation for internal usage
 * 
 * @param str string 
 * @return mcu_msg_size_t length of string 
 */
static mcu_msg_size_t str_len(char *str)
{
    char *p = str;
    while(*p)
        p++;
    return (p - str);
}

/**
 * @brief Deciding char pointer is in the string buffer or not
 * 
 * @param str string buffer with start pointer and length
 * @param p current pointer
 * @return uint8_t return boole result
 */
static inline uint8_t is_in_str_buff(mcu_msg_string_t str, char *p)
{
    return ((p - str.s) < str.len);
}

/**
 * @brief Skiping internal string from start qoution mark to end qmark
 * 
 * @param start start pointer
 * @return char* return end pointer
 */
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
 * @param str source string
 * @param keyword keword has to be found
 * @param flagc flag, eg. '@', '$'
 * @param stopc stop character eg. '(', '='
 * @return mcu_msg_string_t location and size of the keyword (whitout flag) or NULL if keyword not found
 */
static mcu_msg_string_t find_keyword(mcu_msg_string_t str, char *keyword, char flagc, char stopc)
{

    mcu_msg_string_t res;
    char *p = str.s;
    uint8_t equal;
    mcu_msg_size_t i;
    res.len = str_len(keyword);
    while(is_in_str_buff(str, p) && *p) {
        if(*p == '\'' || *p == '"') { //skip internal strings
            p = skip_internal_str(p);
        }
        if(is_in_str_buff(str, p)  && *p == flagc) { // if flag char detected start the analization
            res.s = p + 1;
            equal = 1;
            for(i = 0; is_in_str_buff(str, res.s + i) && i < res.len; i++) { // if not equal during the iterateion, break the loop
                if((*(res.s + i) != *(keyword + i)) || is_ctrl_char(*(res.s + i)) || 
                                    !is_valid_keyword_char(*(res.s + i))) {
                    equal = 0;
                    break;
                }
            }
            while(is_in_str_buff(str, res.s + i) && is_whitespace(*(res.s + i))) i++; //skip spaces
            if(equal && *(res.s + i) == stopc) { //if the stop char is the next, whitout spaces, return with the match string
                return res;
            } else {
                p = res.s + i; // if not matched, return continue the iteration from last checked char
            }
        }
        p++;
    }
    // if not found (loop finished whitout match) return with a destroyed string
    mcu_msg_destroy_string(&res);
    return res;
}


/**
 * @brief Getting the value start position for particular key in message object
 * 
 * @param obj message object
 * @param key key for search
 * @return char* location of the value start point or NULL if the key was not found
 */

static mcu_msg_string_t find_val(mcu_msg_obj_t obj, char *key)
{
    mcu_msg_string_t res  = find_keyword(obj.content, key, CTRL_KEY_FLAG, CTRL_KEY_EQU); //object start with @ and terminated with space or '('
    char *p;
    if(res.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mcu_msg_destroy_string(&res);
        return res;
    }
    if(*res.s != CTRL_KEY_EQU) { // move pointer to 'equal'
        while(is_in_str_buff(obj.content, res.s + 1) && *res.s != CTRL_KEY_EQU) res.s++;
    }
    res.s++;
    while(is_in_str_buff(obj.content, res.s) && is_whitespace(*res.s)) res.s++; //skip spaces after equal

    p = res.s;
    while(is_in_str_buff(obj.content, p) && !is_whitespace(*p) && !is_ctrl_char(*p)) p++; //calc length
    res.len = p - res.s;

    return res;
}


/*get message*/
mcu_msg_t mcu_msg_get(char *raw_str, char *id, mcu_msg_size_t len)
{
    mcu_msg_t res;
    res.content.s = raw_str;
    res.content.len = len;
    res.id = find_keyword(res.content, id, CTRL_MSG_FLAG, CTRL_START_MSG); //object start with @ and terminated with space or '('
    char *p;
    if(res.id.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mcu_msg_destroy(&res);
        return res;
    }
    p = res.id.s + res.id.len; //init pointer to end of the id
    if (*p != CTRL_START_MSG) { // if the next char is not START_MSG, move to the start flag
        while(is_in_str_buff(res.content, p + 1) && *p != CTRL_START_MSG) p++;
    }
    res.content.s = ++p; // set content string pointer to the current pos
    while(is_in_str_buff(res.content, p) && *p != CTRL_STOP_MSG) { //calc length
        p++;
    }
    res.content.len = p - res.content.s;
    return res;
}



mcu_msg_obj_t mcu_msg_parser_get_obj(mcu_msg_t msg, char *id)
{
    mcu_msg_obj_t res;
    res.id = find_keyword(msg.content, id, CTRL_OBJ_FLAG, CTRL_START_OBJ); //object start with @ and terminated with space or '('
    char *p;
    if(res.id.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mcu_msg_destroy_obj(&res);
        return res;
    }

    p = res.id.s + res.id.len;
    if (*p != CTRL_START_OBJ) {
        while(is_in_str_buff(msg.content, p + 1) && *p != CTRL_START_OBJ) p++;
    }
    res.content.s = ++p;
    while(is_in_str_buff(msg.content, p) && *p != CTRL_STOP_OBJ) {
        p++;
    }
    res.content.len = p - res.content.s;
    return res;
}

mcu_msg_cmd_t mcu_msg_parser_get_cmd(mcu_msg_t msg, char *cmd_id)
{
    mcu_msg_cmd_t res;
    // return with the find result
    res.cmd = find_keyword(msg.content, cmd_id, CTRL_CMD_START_FLAG, CTRL_CMD_STOP_FLAG);
    return res;
}

int8_t mcu_msg_parser_get_int(int *res_val, mcu_msg_obj_t obj, char *key)
{
    mcu_msg_string_t sval = find_val(obj, key);
    mcu_msg_size_t i;
    unsigned m = 1;
    int sign = 1;
    int8_t res = 0; // result of function

    if(sval.s == NULL)  //key nout found
        return -1;


    switch(*sval.s) { //if the sign is defined, set the sign variable and increment the pointer
        case '+':
            sign = 1;
            sval.s++;
        break;
        
        case '-':
            sign = -1;
            sval.s++;
        break;
        
        default:
        break;
    }

    for(i = 0; is_in_str_buff(obj.content, sval.s) && !is_whitespace(*sval.s) && *sval.s != CTRL_KEY_SEP; i++, sval.s++) { //move to the end of the value string with i
        if(*sval.s < '0' || *sval.s > '9') {    // if non valid number, return with error
            return -1;
        }
    }

    *res_val = 0;
    --sval.s;
    while(i--) { //convert value to integer
        *res_val += (*sval.s-- - '0') * m;
        m *= 10;
        res++;
    }
    
    *res_val *= sign; //corrigate with the sign

    return res; // return with the digit count, if correct
}


int8_t mcu_msg_parser_get_float(float *res_val, mcu_msg_obj_t obj, char *key)
{
    mcu_msg_string_t sval = find_val(obj, key);
    char *pf;
    mcu_msg_size_t i;
    unsigned m = 1;
    float mf = 0.1;
    int sign = 1;
    int8_t res = 0; // result of function

    if(sval.s == NULL)  //key nout found
        return -1;


    switch(*sval.s) { //if the sign is defined, set the sign variable and increment the pointer
        case '+':
            sign = 1;
            sval.s++;
        break;
        
        case '-':
            sign = -1;
            sval.s++;
        break;
        
        default:
        break;
    }

    //move p to dec separator or end of the value
    for(i = 0; is_in_str_buff(obj.content, sval.s) && !is_whitespace(*sval.s) && *sval.s != CTRL_KEY_SEP && *sval.s != '.'; i++, sval.s++) { 
        if((*sval.s < '0' || *sval.s > '9')) {    // if non valid number, return with error
            return -1;
        }
    }

    *res_val = 0.0;
    
    if(*sval.s == '.') {
        pf = sval.s + 1;
        res++;
    } else {
        pf = NULL;
    }

    --sval.s;
    while(i--) { // convert integer part
        *res_val += (*sval.s-- - '0') * m;
        m *= 10;
        res++;
    }
    
    // calculate floating point section after '.' (if there is)
    for(; pf != NULL && is_in_str_buff(obj.content, pf) && !is_whitespace(*pf) && *pf != CTRL_KEY_SEP; pf++) {
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
    mcu_msg_string_t res = find_val(obj, key);
    char qmark;
    char *p;

    if(res.s == NULL) {
        mcu_msg_destroy_string(&res);
        return res;
    }
        
    qmark = *res.s;

    if(qmark != '\'' && qmark != '"') { // qmark not found, this is not a string
        mcu_msg_destroy_string(&res);
        return res;
    }

    p = ++res.s;
    while(is_in_str_buff(obj.content, p) && *p != qmark) { // calc len
        p++;
    }
    res.len = p - res.s;
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



void mcu_msg_print_str(mcu_msg_string_t str)
{
    mcu_msg_size_t i;
    for(i = 0; i < str.len; mcu_msg_putc(*(str.s + i++)));
}

/**
 * @brief Create string handler and set the basic functions
 * 
 * @param print expected print function, set to NULL if you don't need the print feature
 * @return mcu_msg_string_hnd_t handler
 */
mcu_msg_string_hnd_t mcu_msg_string_hnd_create(int (*putc)(char))
{
    mcu_msg_string_hnd_t hnd;
    mcu_msg_putc = putc;            // init putchar
    hnd.copy_to_chr_arr = mcu_msg_str_copy_to_chr_arr;
    hnd.copy = mcu_msg_str_copy;
    hnd.print = mcu_msg_print_str;
    return hnd;
}

/*Wrapper functions*/
#if MCU_MSG_USE_WRAPPER
void mcu_msg_wrapper_init_obj_queues(mcu_msg_obj_wrap_t *obj)
{
    obj->int_queue = NULL;
    obj->float_queue = NULL;
    obj->string_queue = NULL;
}

// void mcu_msg_

void mcu_msg_wrapper_add_string_to_obj(mcu_msg_obj_wrap_t *obj, mcu_msg_string_wrap_t *str)
{
    mcu_msg_string_wrap_t *strp;
    if(obj->string_queue == NULL) { //first element
        obj->string_queue = str;
        obj->string_queue->next = NULL;
    } else {
        strp = obj->string_queue;

        while(strp->next != NULL) 
            strp = strp->next;
        strp->next = str;
        str->next = NULL;
    }
}

// void mcu_msg_wrapper_add_int_to_obj(mcu_msg_obj_t *obj, mcu_msg_int_t *int_valp)
// {
//     mcu_msg_int_t *ip;
//     if(obj->int_queue = NULL) { //first element
//         obj->int_queue = int_valp;
//         obj->string_queue->next = NULL;
//     } else {
//         ip = obj->int_queue;

//         while(ip->next != NULL) ip++;
//         ip->next = int_valp;
//         int_valp->next = NULL;
//     }
// }


// void mcu_msg_wrapper_add_float_to_obj(mcu_msg_obj_t *obj, mcu_msg_float_t *float_valp)
// {
//     mcu_msg_float_t *fp;
//     if(obj->float_queue = NULL) { //first element
//         obj->float_queue = float_valp;
//         obj->float_queue->next = NULL;
//     } else {
//         fp = obj->float_queue;

//         while(fp->next != NULL) fp++;
//         fp->next = float_valp;
//         float_valp->next = NULL;
//     }
// }

// void mcu_msg_wrapper_print_obj(mcu_msg_obj_t obj){

// }

// void mcu_msg_add_object_to_msg(mcu_msg_t *msg, mcu_msg_obj_t *obj)
// {
//     mcu_msg_obj_t *op;
//     if
// }

#endif