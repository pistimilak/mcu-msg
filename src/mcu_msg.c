/**
 * @file mmsg_parser.c
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
static int (*mmsg_putc)(char) = NULL; 

/*Static function declarations*/
static inline uint8_t is_ctrl_char(char c);
static inline uint8_t is_whitespace(char c);
static mmsg_size_t str_len(char *str);
static inline uint8_t is_in_str_buff(mmsg_string_t str, char *p);
static char *skip_internal_str(char *start);
static mmsg_string_t find_keyword(mmsg_string_t str, char *keyword, char flagc, char stopc);
static mmsg_string_t find_val(mmsg_obj_t obj, char *key);
static void mmsg_str_copy_to_chr_arr(char *dest, mmsg_string_t source);
static void mmsg_str_copy(mmsg_string_t dest, mmsg_string_t source);
static void mmsg_print_int(int i);
static void mmsg_print_float(float f, uint8_t prec);
static void mmsg_print_str(mmsg_string_t str);
static inline char define_qmark(mmsg_string_t str);

#if MCU_MSG_USE_WRAPPER
static inline void mmsg_wrapper_print_obj(mmsg_obj_wrap_t obj);
static inline void mmsg_wrapper_print_cmd(mmsg_cmd_wrap_t cmd);
static void mmsg_wrapper_print_msg(mmsg_wrap_t msg);
static char* mmsg_wrapper_print_msg_to_buff(mmsg_wrap_t msg);

#endif


/////////////////////////////////////////////////////////////////////////////////////////////
//                                      Parser functions                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

/*destroy string*/
void mmsg_destroy_string(mmsg_string_t *str)
{
    str->s = NULL;
    str->len = 0;
}

/*destroy message*/
void mmsg_destroy(mmsg_t *msg)
{
    mmsg_destroy_string(&msg->id);
    mmsg_destroy_string(&msg->content);
}


/*destroy object*/
void mmsg_destroy_obj(mmsg_obj_t *obj)
{
    mmsg_destroy_string(&obj->id);
    mmsg_destroy_string(&obj->content);
}

mmsg_string_t mmsg_init_string(char *str)
{
    mmsg_string_t res;
    res.s = str;
    res.len = str_len(res.s);
    return res;
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
 * @return mmsg_size_t length of string 
 */
static mmsg_size_t str_len(char *str)
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
static inline uint8_t is_in_str_buff(mmsg_string_t str, char *p)
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
 * @return mmsg_string_t location and size of the keyword (whitout flag) or NULL if keyword not found
 */
static mmsg_string_t find_keyword(mmsg_string_t str, char *keyword, char flagc, char stopc)
{

    mmsg_string_t res;
    char *p = str.s;
    uint8_t equal;
    mmsg_size_t i;
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
    mmsg_destroy_string(&res);
    return res;
}


/**
 * @brief Getting the value start position for particular key in message object
 * 
 * @param obj message object
 * @param key key for search
 * @return char* location of the value start point or NULL if the key was not found
 */

static mmsg_string_t find_val(mmsg_obj_t obj, char *key)
{
    mmsg_string_t res  = find_keyword(obj.content, key, CTRL_KEY_FLAG, CTRL_KEY_EQU); //object start with @ and terminated with space or '('
    char *p;
    if(res.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mmsg_destroy_string(&res);
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
mmsg_t mmsg_get(char *raw_str, char *id, mmsg_size_t len)
{
    mmsg_t res;
    res.content.s = raw_str;
    res.content.len = len;
    res.id = find_keyword(res.content, id, CTRL_MSG_FLAG, CTRL_START_MSG); //object start with @ and terminated with space or '('
    char *p;
    if(res.id.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mmsg_destroy(&res);
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



mmsg_obj_t mmsg_parser_get_obj(mmsg_t msg, char *id)
{
    mmsg_obj_t res;
    res.id = find_keyword(msg.content, id, CTRL_OBJ_FLAG, CTRL_START_OBJ); //object start with @ and terminated with space or '('
    char *p;
    if(res.id.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        mmsg_destroy_obj(&res);
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

mmsg_cmd_t mmsg_parser_get_cmd(mmsg_t msg, char *cmd_id)
{
    mmsg_cmd_t res;
    // return with the find result
    res.cmd = find_keyword(msg.content, cmd_id, CTRL_CMD_START_FLAG, CTRL_CMD_STOP_FLAG);
    return res;
}

int8_t mmsg_parser_get_int(int *res_val, mmsg_obj_t obj, char *key)
{
    mmsg_string_t sval = find_val(obj, key);
    mmsg_size_t i;
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


int8_t mmsg_parser_get_float(float *res_val, mmsg_obj_t obj, char *key)
{
    mmsg_string_t sval = find_val(obj, key);
    char *pf;
    mmsg_size_t i;
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



mmsg_string_t mmsg_parser_get_string(mmsg_obj_t obj, char *key)
{
    mmsg_string_t res = find_val(obj, key);
    char qmark;
    char *p;

    if(res.s == NULL) {
        mmsg_destroy_string(&res);
        return res;
    }
        
    qmark = *res.s;

    if(qmark != '\'' && qmark != '"') { // qmark not found, this is not a string
        mmsg_destroy_string(&res);
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
static void mmsg_str_copy_to_chr_arr(char *dest, mmsg_string_t source)
{
    mmsg_size_t i;
    for(i = 0; i < source.len; *(dest + i) = *(source.s + i), i++);
}

/**
 * @brief Default string copy to string type
 * 
 * @param dest destination string type
 * @param source source string type
 */
static void mmsg_str_copy(mmsg_string_t dest, mmsg_string_t source)
{
    mmsg_size_t i;
    for(i = 0, dest.len = source.len; i < source.len; *(dest.s + i) = *(source.s + i), i++);
}

static void mmsg_print_int(int i)
{
    int8_t sign = i < 0 ? -1 : 1;
    //                     int32->2147483647  int16:32767
    long div;
    char dig;

    if(!mmsg_putc) //if function pointer is NULL, return
        return;
    if(!i) {
        mmsg_putc('0');
        return;
    }

    switch(sign) {
        case -1: div = sizeof(int) >= 4 ? -1000000000L : -10000L; mmsg_putc('-'); break;
        default: div = sizeof(int) >= 4 ?  1000000000L :  10000L; break;
    }
    while(div) {
        if(sign == -1 ? (i > div) : (i < div)) {
            div /= 10;
            continue;
        }
        dig = '0' + ((i / div) % 10);
        mmsg_putc(dig);
        div /= 10;
    }
    
}

static void mmsg_print_float(float f, uint8_t prec)
{
    int i_part = f;
    float f_part = f - i_part;
    long mul = f < 0.0 ? -10 : 10;
    uint8_t j;
    char dig;
    if(!mmsg_putc) //if function pointer is NULL, return
        return;

    if(!i_part) {
        if(mul < 0) mmsg_putc('-');
        mmsg_putc('0');
    } else {
        mmsg_print_int(i_part);  
    }
      
    mmsg_putc('.');
    for(j = 0; j < prec; mul *= 10, j++) {
        dig = '0' + ((long)(f_part * mul) % 10);
        mmsg_putc(dig);
    }
}

static void mmsg_print_str(mmsg_string_t str)
{
    mmsg_size_t i;
    if(!mmsg_putc) { //if function pointer is NULL, return
        return;
    }
    for(i = 0; i < str.len; mmsg_putc(*(str.s + i++)));
}

/**
 * @brief Create string handler and set the basic functions
 * 
 * @param putc expected print function, set to NULL if you don't need the print feature
 * @return mmsg_string_hnd_t handler
 */
mmsg_string_hnd_t mmsg_string_hnd_create(int (*putc)(char))
{
    mmsg_string_hnd_t hnd;
    mmsg_putc = putc;            // init putchar
    hnd.copy_to_chr_arr = mmsg_str_copy_to_chr_arr;
    hnd.copy = mmsg_str_copy;
    hnd.print = mmsg_print_str;
    return hnd;
}

static inline char define_qmark(mmsg_string_t str)
{
    char *p = str.s;
    while(is_in_str_buff(str, p) && *p) {
        switch(*p) {
            case '"' :  return '\'';
            case '\'' : return '"';
            default: break;
        }
        p++;
    }
    return '"'; // default
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                     Wrapper functions                                   //
/////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER

mmsg_wrap_hnd_t mmsg_wrapper_hnd_create(int (*putc)(char))
{
    mmsg_wrap_hnd_t hnd;
    mmsg_putc = putc; // init putchar
    hnd.print = mmsg_wrapper_print_msg;
    hnd.print_to_buff = mmsg_wrapper_print_msg_to_buff;
    hnd.print_obj = mmsg_wrapper_print_obj;
    hnd.print_cmd = mmsg_wrapper_print_cmd;
    return hnd;
}

#define __print_key_equ(key_str)        mmsg_putc(CTRL_KEY_FLAG); \
                                        mmsg_print_str(key_str);   \
                                        mmsg_putc(CTRL_KEY_EQU)


static inline void mmsg_wrapper_print_obj(mmsg_obj_wrap_t obj)
{
    mmsg_string_wrap_t *sp;
    mmsg_int_wrap_t *ip;
    mmsg_float_wrap_t *fp;
    
    mmsg_putc(CTRL_OBJ_FLAG);
    mmsg_print_str(obj.id);
    mmsg_putc(CTRL_START_OBJ);
    char qmark;
    // print integers
    for(ip = obj.int_queue; ip != NULL; ip = ip->next, ip != NULL ? mmsg_putc(CTRL_KEY_SEP) : NULL) {
        __print_key_equ(ip->id);
        mmsg_print_int(ip->val);
    }

    // print floats
    for(fp = obj.float_queue, obj.int_queue != NULL ? mmsg_putc(CTRL_KEY_SEP) : NULL; 
                fp != NULL; fp = fp->next, fp != NULL ? mmsg_putc(CTRL_KEY_SEP) : NULL) {
        __print_key_equ(fp->id);
        mmsg_print_float(fp->val, fp->prec);
    }

    // print strings
    for(sp = obj.string_queue, obj.float_queue != NULL ? mmsg_putc(CTRL_KEY_SEP) : NULL; 
                sp != NULL; sp = sp->next, sp != NULL ? mmsg_putc(CTRL_KEY_SEP) : NULL) {
        __print_key_equ(sp->id);
        qmark = define_qmark(sp->content);
        mmsg_putc(qmark);
        mmsg_print_str(sp->content);
        mmsg_putc(qmark);
    }

    mmsg_putc(CTRL_STOP_OBJ);
}

static inline void mmsg_wrapper_print_cmd(mmsg_cmd_wrap_t cmd)
{
    if(cmd.cmd.s != NULL) {
        mmsg_putc(CTRL_CMD_START_FLAG);
        mmsg_print_str(cmd.cmd);
        mmsg_putc(CTRL_CMD_STOP_FLAG);
    }
}


static void mmsg_wrapper_print_msg(mmsg_wrap_t msg)
{
    mmsg_obj_wrap_t *pobj;
    mmsg_cmd_wrap_t *pcmd;

    if(!mmsg_putc || msg.id.s == NULL) // return if putchar not implemented
        return;
    mmsg_putc(CTRL_MSG_FLAG);
    mmsg_print_str(msg.id);
    mmsg_putc(CTRL_START_MSG);
    
    pcmd = msg.cmd_queue;
    while(pcmd != NULL) {
        mmsg_wrapper_print_cmd(*pcmd);
        pcmd = pcmd->next;   
    }
    pobj = msg.obj_queue;
    while(pobj != NULL) {
        mmsg_wrapper_print_obj(*pobj);
        pobj = pobj->next;   
    }
    mmsg_putc(CTRL_STOP_MSG);
}


static char* mmsg_wrapper_print_msg_to_buff(mmsg_wrap_t msg)
{
    return NULL;
}

void mmsg_wrap_destroy(mmsg_wrap_t *msg)
{
    mmsg_destroy_string(&msg->id);
    msg->cmd_queue=NULL;
    msg->obj_queue=NULL;
}

void mmsg_wrap_destroy_obj(mmsg_obj_wrap_t *obj)
{
    mmsg_destroy_string(&obj->id);
    obj->int_queue = NULL;
    obj->float_queue = NULL;
    obj->string_queue = NULL;
    obj->next = NULL;
}

void mmsg_wrap_destroy_str(mmsg_string_wrap_t *str)
{
    mmsg_destroy_string(&str->id);
    mmsg_destroy_string(&str->content);
    str->next = NULL;
}

void mmsg_wrap_destroy_int(mmsg_int_wrap_t *i)
{
    mmsg_destroy_string(&i->id);
    i->val = 0;
    i->next = NULL;
}

void mmsg_wrap_destroy_float(mmsg_float_wrap_t *f)
{
    mmsg_destroy_string(&f->id);
    f->val = 0.0;
    f->next = NULL;
    f->prec = 0;
}



mmsg_wrap_t mmsg_wrapper_init_msg(char *msg_id)
{
    mmsg_wrap_t res;
    res.id = mmsg_init_string(msg_id);
    res.cmd_queue = NULL;
    res.obj_queue = NULL;
    return res;
}


mmsg_cmd_wrap_t mmsg_wrapper_init_cmd(char *cmd)
{
    mmsg_cmd_wrap_t res;
    res.cmd = mmsg_init_string(cmd);
    res.next = NULL;
    return res;
}

mmsg_obj_wrap_t mmsg_wrapper_init_obj(char *obj_id)
{
    mmsg_obj_wrap_t res;
    res.id = mmsg_init_string(obj_id);
    res.int_queue = NULL;
    res.float_queue = NULL;
    res.string_queue = NULL;
    res.next = NULL;
    return res;
}

mmsg_string_wrap_t mmsg_wrapper_init_string(char *id, char *content)
{
    mmsg_string_wrap_t res;
    res.id = mmsg_init_string(id);
    res.content = mmsg_init_string(content);
    res.next = NULL;
    return res;
}

mmsg_int_wrap_t mmsg_wrapper_init_int(char *id, int val)
{
    mmsg_int_wrap_t res;
    res.id = mmsg_init_string(id);
    res.val = val;
    res.next = NULL;
    return res;
}

mmsg_float_wrap_t mmsg_wrapper_init_float(char *id, float val, uint8_t prec)
{
    mmsg_float_wrap_t res;
    res.id = mmsg_init_string(id);
    res.val = val;
    res.prec = prec;
    res.next = NULL;
    return res;
}


void mmsg_wrapper_add_string_to_obj(mmsg_obj_wrap_t *obj, mmsg_string_wrap_t *str)
{
    mmsg_string_wrap_t *strp;
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

void mmsg_wrapper_add_int_to_obj(mmsg_obj_wrap_t *obj, mmsg_int_wrap_t *int_val)
{
    mmsg_int_wrap_t *ip;
    if(obj->int_queue == NULL) { //first element
        obj->int_queue = int_val;
        obj->int_queue->next = NULL;
    } else {
        ip = obj->int_queue;

        while(ip->next != NULL) 
            ip = ip->next;
        ip->next = int_val;
        int_val->next = NULL;
    }
}


void mmsg_wrapper_add_float_to_obj(mmsg_obj_wrap_t *obj, mmsg_float_wrap_t *float_val)
{
    mmsg_float_wrap_t *fp;
    if(obj->float_queue == NULL) { //first element
        obj->float_queue = float_val;
        obj->float_queue->next = NULL;
    } else {
        fp = obj->float_queue;

        while(fp->next != NULL) 
            fp = fp->next;
        fp->next = float_val;
        float_val->next = NULL;
    }
}

void mmsg_wrapper_add_object_to_msg(mmsg_wrap_t *msg, mmsg_obj_wrap_t *obj)
{
    mmsg_obj_wrap_t *op;
    if(msg->obj_queue == NULL) { //if empty 
        msg->obj_queue = obj;
        msg->obj_queue->next = NULL;
    } else {
        op = msg->obj_queue;
        while(op->next != NULL)
            op = op->next;
        op->next = obj;
        obj->next = NULL;
    }
}

void mmsg_wrapper_add_cmd_to_msg(mmsg_wrap_t *msg, mmsg_cmd_wrap_t *cmd)
{
    mmsg_cmd_wrap_t *cp;
    if(msg->cmd_queue == NULL) { //if empty 
        msg->cmd_queue = cmd;
        msg->cmd_queue->next = NULL;
    } else {
        cp = msg->obj_queue;
        while(cp->next != NULL)
            cp = cp->next;
        cp->next = cmd;
        cmd->next = NULL;
    }
}


#endif