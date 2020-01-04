/**
 * @file mcu_msg.h
 * @author Istvan Milak (istvan.milak@gmail.com)
 * @brief mcu-msg: simple parser and wrapper 
 * for low level string based communication between microcontrollers
 * It's designed for UART communication.
 * The library dosen't use string.h functions and declared buffers, all of featerus are working with pointers
 * to optimized for memory usage. Handler interface provides printing functions to standaer output
 * with redirection to char array. Wrapper uses linked list which are modifyalbe
 * @version 0.1
 * @date 2020-01-04
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>
#include "mcu_msg.h"

/*Control chars*/
#define __CTRL_MSG_FLAG           '#'
#define __CTRL_START_MSG          '{'
#define __CTRL_STOP_MSG           '}'
#define __CTRL_START_OBJ          '('
#define __CTRL_STOP_OBJ           ')'
#define __CTRL_OBJ_FLAG           '@'
#define __CTRL_KEY_FLAG           '$'
#define __CTRL_KEY_SEP            ';'
#define __CTRL_KEY_EQU            '='
#define __CTRL_CMD_START_FLAG     '<'
#define __CTRL_CMD_STOP_FLAG      '>'


/*typedef for internal string buffer*/
typedef struct msg_str_buff {
    msg_str_t  buff;
    char*      p;                         // pointer to the next element
} msg_str_buff_t;

static msg_str_buff_t __str_buff;        // internal string buffer, must be intialized always
static uint8_t __redir_outp_to_buff = 0; // redirect output to buffer indicator

/*putchar implementation: must be implemented for printing to UART or other output*/
static int (*__putc)(char) = NULL; 

/*Static function declarations*/
static void             __msg_enable_buff(void);
static void             __msg_disable_buff(void);
static void             __msg_init_str_buff(char *buff, msg_size_t buff_size);
static void             __msg_reset_str_buff(char *buff, msg_size_t buff_size);
static msg_size_t       __msg_putc_to_buff(char c);
static void             __msg_putc(char c); //use std out or redirected string buff;

static inline uint8_t   __is_ctrl_char(char c);
static inline uint8_t   __is_whitespace(char c);
static msg_size_t       __str_len(char *str);
static inline uint8_t   __is_p_in_str(msg_str_t str, char *p);
static char*            __skip_internal_str(char *start);
static msg_str_t        __find_keyword(msg_str_t str, char *keyword, char flagc, char stopc);
static msg_str_t        __find_val(msg_obj_t obj, char *key);
static void             __msg_print(msg_t msg);
static void             __msg_print_int(int i);
static void             __msg_print_float(float f, uint8_t prec);
static void             __msg_print_str(msg_str_t str);
static inline char      __define_qmark(msg_str_t str);

#if MCU_MSG_USE_WRAPPER
static void             __msg_wrapper_print_obj(msg_wrap_obj_t obj);
static inline void      __msg_wrapper_print_cmd(msg_wrap_cmd_t cmd);
static void             __msg_wrapper_print_msg(msg_wrap_t msg);
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      Parser functions                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

/*destroy string*/
void msg_destroy_string(msg_str_t *str)
{
    str->s = NULL;
    str->len = 0;
}

/*destroy message*/
void msg_destroy(msg_t *msg)
{
    msg_destroy_string(&msg->id);
    msg_destroy_string(&msg->content);
}


/*destroy object*/
void msg_destroy_obj(msg_obj_t *obj)
{
    msg_destroy_string(&obj->id);
    msg_destroy_string(&obj->content);
}

/*destroy cmd*/
void msg_destroy_cmd(msg_cmd_t *cmd)
{
    msg_destroy_string(&cmd->cmd);
}

msg_str_t msg_init_string(char *str)
{
    msg_str_t res;
    res.s = str;
    res.len = __str_len(res.s);
    return res;
}

/**
 * @brief Argument char is control char or not
 * 
 * @param c char
 * @return uint8_t comparison result
 */
static inline uint8_t __is_ctrl_char(char c)
{
    switch(c) {
        case __CTRL_MSG_FLAG:
        case __CTRL_START_MSG:
        case __CTRL_STOP_MSG:
        case __CTRL_START_OBJ:
        case __CTRL_STOP_OBJ:
        case __CTRL_OBJ_FLAG:
        case __CTRL_KEY_FLAG:
        case __CTRL_KEY_SEP:
        case __CTRL_KEY_EQU:
        case __CTRL_CMD_START_FLAG:
        case __CTRL_CMD_STOP_FLAG:
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
static inline uint8_t __is_whitespace(char c)
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

#define is_valid_keyword_char(c)        ((c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \
                                        (c >= '0' && c <= '9'))

/**
 * @brief strlen implementation for internal usage
 * 
 * @param str string 
 * @return msg_size_t length of string 
 */
static msg_size_t __str_len(char *str)
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
static inline uint8_t __is_p_in_str(msg_str_t str, char *p)
{
    return ((p - str.s) < str.len);
}

/**
 * @brief Skiping internal string from start qoution mark to end qmark
 * 
 * @param start start pointer
 * @return char* return end pointer
 */
static char *__skip_internal_str(char *start)
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
 * @return msg_str_t location and size of the keyword (whitout flag) or NULL if keyword not found
 */
static msg_str_t __find_keyword(msg_str_t str, char *keyword, char flagc, char stopc)
{

    msg_str_t res;
    char *p = str.s;
    uint8_t equal;
    msg_size_t i;
    res.len = __str_len(keyword);
    while(__is_p_in_str(str, p) && *p) {
        if(*p == '\'' || *p == '"') { //skip internal strings
            p = __skip_internal_str(p);
        }
        if(__is_p_in_str(str, p)  && *p == flagc) { // if flag char detected start the analization
            res.s = p + 1;
            equal = 1;
            for(i = 0; __is_p_in_str(str, res.s + i) && i < res.len; i++) { // if not equal during the iterateion, break the loop
                if((*(res.s + i) != *(keyword + i)) || __is_ctrl_char(*(res.s + i)) || 
                                    !is_valid_keyword_char(*(res.s + i))) {
                    equal = 0;
                    break;
                }
            }
            while(__is_p_in_str(str, res.s + i) && __is_whitespace(*(res.s + i))) i++; //skip spaces
            if(equal && *(res.s + i) == stopc) { //if the stop char is the next, whitout spaces, return with the match string
                return res;
            } else {
                p = res.s + i; // if not matched, return continue the iteration from last checked char
            }
        }
        p++;
    }
    // if not found (loop finished whitout match) return with a destroyed string
    msg_destroy_string(&res);
    return res;
}


/**
 * @brief Getting the value start position for particular key in message object
 * 
 * @param obj message object
 * @param key key for search
 * @return char* location of the value start point or NULL if the key was not found
 */

static msg_str_t __find_val(msg_obj_t obj, char *key)
{
    msg_str_t res  = __find_keyword(obj.content, key, __CTRL_KEY_FLAG, __CTRL_KEY_EQU); //object start with @ and terminated with space or '('
    char *p;
    if(res.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        msg_destroy_string(&res);
        return res;
    }
    if(*res.s != __CTRL_KEY_EQU) { // move pointer to 'equal'
        while(__is_p_in_str(obj.content, res.s + 1) && *res.s != __CTRL_KEY_EQU) res.s++;
    }
    res.s++;
    while(__is_p_in_str(obj.content, res.s) && __is_whitespace(*res.s)) res.s++; //skip spaces after equal

    p = res.s;
    while(__is_p_in_str(obj.content, p) && !__is_whitespace(*p) && !__is_ctrl_char(*p)) p++; //calc length
    res.len = p - res.s;

    return res;
}


/*get message*/
msg_t msg_get(char *raw_str, char *id, msg_size_t len)
{
    msg_t res;
    res.content.s = raw_str;
    res.content.len = len;
    res.id = __find_keyword(res.content, id, __CTRL_MSG_FLAG, __CTRL_START_MSG); //object start with @ and terminated with space or '('
    char *p;
    if(res.id.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        msg_destroy(&res);
        return res;
    }
    p = res.id.s + res.id.len; //init pointer to end of the id
    if (*p != __CTRL_START_MSG) { // if the next char is not START_MSG, move to the start flag
        while(__is_p_in_str(res.content, p + 1) && *p != __CTRL_START_MSG) p++;
    }
    res.content.s = ++p; // set content string pointer to the current pos
    while(__is_p_in_str(res.content, p) && *p != __CTRL_STOP_MSG) { //calc length
        p++;
    }
    res.content.len = p - res.content.s;
    return res;
}



msg_obj_t msg_parser_get_obj(msg_t msg, char *id)
{
    msg_obj_t res;
    res.id = __find_keyword(msg.content, id, __CTRL_OBJ_FLAG, __CTRL_START_OBJ); //object start with @ and terminated with space or '('
    char *p;
    if(res.id.s == NULL) { //if keyword not found, return with NULLs and 0 lengths
        msg_destroy_obj(&res);
        return res;
    }

    p = res.id.s + res.id.len;
    if (*p != __CTRL_START_OBJ) {
        while(__is_p_in_str(msg.content, p + 1) && *p != __CTRL_START_OBJ) p++;
    }
    res.content.s = ++p;
    while(__is_p_in_str(msg.content, p) && *p != __CTRL_STOP_OBJ) {
        p++;
    }
    res.content.len = p - res.content.s;
    return res;
}

msg_cmd_t msg_parser_get_cmd(msg_t msg, char *cmd_id)
{
    msg_cmd_t res;
    // return with the find result
    res.cmd = __find_keyword(msg.content, cmd_id, __CTRL_CMD_START_FLAG, __CTRL_CMD_STOP_FLAG);
    return res;
}

int8_t msg_parser_get_int(int *res_val, msg_obj_t obj, char *key)
{
    msg_str_t sval = __find_val(obj, key);
    msg_size_t i;
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

    for(i = 0; __is_p_in_str(obj.content, sval.s) && !__is_whitespace(*sval.s) && *sval.s != __CTRL_KEY_SEP; i++, sval.s++) { //move to the end of the value string with i
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


int8_t msg_parser_get_float(float *res_val, msg_obj_t obj, char *key)
{
    msg_str_t sval = __find_val(obj, key);
    char *pf;
    msg_size_t i;
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
    for(i = 0; __is_p_in_str(obj.content, sval.s) && !__is_whitespace(*sval.s) && *sval.s != __CTRL_KEY_SEP && *sval.s != '.'; i++, sval.s++) { 
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
    for(; pf != NULL && __is_p_in_str(obj.content, pf) && !__is_whitespace(*pf) && *pf != __CTRL_KEY_SEP; pf++) {
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



msg_str_t msg_parser_get_string(msg_obj_t obj, char *key)
{
    msg_str_t res = __find_val(obj, key);
    char qmark;
    char *p;

    if(res.s == NULL) {
        msg_destroy_string(&res);
        return res;
    }
        
    qmark = *res.s;

    if(qmark != '\'' && qmark != '"') { // qmark not found, this is not a string
        msg_destroy_string(&res);
        return res;
    }

    p = ++res.s;
    while(__is_p_in_str(obj.content, p) && *p != qmark) { // calc len
        p++;
    }
    res.len = p - res.s;
    return res;
}


static void __msg_enable_buff(void)
{
    __redir_outp_to_buff = 1;
}

static void __msg_disable_buff(void)
{
    __redir_outp_to_buff = 0;
}

static void __msg_init_str_buff(char *buff, msg_size_t buff_size)
{
    __str_buff.buff.len = buff_size;
    __str_buff.buff.s = buff;
    __str_buff.p = __str_buff.buff.s;
}

static void __msg_reset_str_buff(char *buff, msg_size_t buff_size)
{
    __str_buff.p = __str_buff.buff.s; //reset pointer (set to the start position)
}

static msg_size_t __msg_putc_to_buff(char c)
{

    if(!__str_buff.buff.s || !__str_buff.buff.len) return 0;

    if((__str_buff.p - __str_buff.buff.s) >= __str_buff.buff.len) // return null if position is out of buffer
        return 0;
    *__str_buff.p = c;
    __str_buff.p++;
    return __str_buff.buff.len - (__str_buff.p - __str_buff.buff.s); // return with the empty spaces
}

static void __msg_putc(char c)
{
    if (__redir_outp_to_buff) { // if output is redirected, use the internal string buffer
        __msg_putc_to_buff(c);
    } else {
        __putc(c);
    }
}

static void __msg_print_int(int i)
{
    // int8_t sign = i < 0 ? -1: 1;
    unsigned val = i < 0 ? ~i + 1 : i;
                                  // 4294967295   65535
    unsigned div = sizeof(int) > 2 ? 1000000000UL : 10000UL;
    uint8_t dig;
    uint8_t first_dig = 0;

    if(!i) {
        __msg_putc('0');
        return;
    }
    if(i < 0) __msg_putc('-');
    while(div) {
        dig = 0;
        while(val >= div) {
            val -= div;
            dig += 1;
        }
        div /= 10;
        if(!first_dig && dig) {
            first_dig = 1;
        }
        if(first_dig) __msg_putc('0' + dig);
    }
    
}

static void __msg_print_float(float f, uint8_t prec)
{
    int i_part = (int)f;
    float f_part = f < 0 ? (f - (float)i_part) * -1  : (f - (float)i_part);
    unsigned mul = 1;
    uint8_t j;
    for(j = 0; j < prec; j++ ) mul *= 10;
    __msg_print_int(i_part);
    __msg_putc('.');
    __msg_print_int((unsigned)(f_part * mul));
}


static void __msg_print_str(msg_str_t str)
{
    msg_size_t i;
    if(!__msg_putc) { //if function pointer is NULL, return
        return;
    }
    for(i = 0; i < str.len; __msg_putc(*(str.s + i)), i++);
}

static void __msg_print(msg_t msg)
{
    __msg_print_str(msg.content);
}

/**
 * @brief Create string handler and set the basic functions
 * 
 * @param putc expected print function, set to NULL if you don't need the print feature
 * @return msg_string_hnd_t handler
 */
msg_hnd_t msg_hnd_create(int (*putc)(char))
{

    msg_hnd_t hnd;

    // init string buff
    __str_buff.p = __str_buff.buff.s = NULL;
    __str_buff.buff.len = 0;

    __putc = putc;            // init putchar

    //features
    hnd.print_msg         = __msg_print;
    hnd.print_str         = __msg_print_str;
    hnd.print_int         = __msg_print_int;
    hnd.print_float       = __msg_print_float;
    hnd.enable_buff       = __msg_enable_buff;
    hnd.disable_buff      = __msg_disable_buff;
    hnd.init_str_buff     = __msg_init_str_buff;
    hnd.reset_str_buff    = __msg_reset_str_buff;
    hnd.print_wrapper_msg = __msg_wrapper_print_msg;
    
    return hnd;
}

static inline char __define_qmark(msg_str_t str)
{
    char *p = str.s;
    while(__is_p_in_str(str, p) && *p) {
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


#define __print_key_equ(key_str)        __msg_putc(__CTRL_KEY_FLAG); \
                                        __msg_print_str(key_str);    \
                                        __msg_putc(__CTRL_KEY_EQU)

#define __print_msg_start(msg)          __msg_putc(__CTRL_MSG_FLAG); \
                                        __msg_print_str(msg.id);     \
                                        __msg_putc(__CTRL_START_MSG)


#define __print_obj_start(obj)          __msg_putc(__CTRL_OBJ_FLAG);  \
                                        __msg_print_str(obj.id);      \
                                        __msg_putc(__CTRL_START_OBJ)  

static void __msg_wrapper_print_obj(msg_wrap_obj_t obj)
{
    msg_wrap_str_t *sp;
    msg_wrap_int_t *ip;
    msg_wrap_float_t *fp;
    char qmark;

    __print_obj_start(obj);
    

    // print integers
    
    for(ip = obj.int_queue; ip != NULL; ip = ip->next) {
        __print_key_equ(ip->id);
        __msg_print_int(ip->val);
        if(ip->next != NULL) __msg_putc(__CTRL_KEY_SEP);
    }

    // print floats
    if(obj.float_queue != NULL && obj.int_queue != NULL) __msg_putc(__CTRL_KEY_SEP);
    for(fp = obj.float_queue; fp != NULL; fp = fp->next) {
        __print_key_equ(fp->id);
        __msg_print_float(fp->val, fp->prec);
        if(fp->next != NULL) __msg_putc(__CTRL_KEY_SEP);
    }
    // print strings
    if(obj.string_queue != NULL && obj.float_queue != NULL) __msg_putc(__CTRL_KEY_SEP);
    for(sp = obj.string_queue; sp != NULL; sp = sp->next) {
        __print_key_equ(sp->id);
        qmark = __define_qmark(sp->content);
        __msg_putc(qmark);
        __msg_print_str(sp->content);
        __msg_putc(qmark);
        if(sp->next != NULL) __msg_putc(__CTRL_KEY_SEP);
    }

    __msg_putc(__CTRL_STOP_OBJ);
}



static inline void __msg_wrapper_print_cmd(msg_wrap_cmd_t cmd)
{
    if(cmd.cmd.s != NULL) {
        __msg_putc(__CTRL_CMD_START_FLAG);
        __msg_print_str(cmd.cmd);
        __msg_putc(__CTRL_CMD_STOP_FLAG);
    }
}



static void __msg_wrapper_print_msg(msg_wrap_t msg)
{
    msg_wrap_obj_t *pobj;
    msg_wrap_cmd_t *pcmd;

    if(!__msg_putc || msg.id.s == NULL) // return if putchar not implemented
        return;
    __print_msg_start(msg);
    
    pcmd = msg.cmd_queue;
    while(pcmd != NULL) {
        __msg_wrapper_print_cmd(*pcmd);
        pcmd = pcmd->next;   
    }
    pobj = msg.obj_queue;
    while(pobj != NULL) {
        __msg_wrapper_print_obj(*pobj);
        pobj = pobj->next;  
    }
    __msg_putc(__CTRL_STOP_MSG);
}




void msg_wrap_destroy(msg_wrap_t *msg)
{
    msg_destroy_string(&msg->id);
    msg->cmd_queue = NULL;
    msg->obj_queue = NULL;
}

void msg_wrap_destroy_obj(msg_wrap_obj_t *obj)
{
    msg_destroy_string(&obj->id);
    obj->int_queue = NULL;
    obj->float_queue = NULL;
    obj->string_queue = NULL;
    obj->next = NULL;
}

void msg_wrap_destroy_cmd(msg_wrap_cmd_t *cmd)
{
    msg_destroy_string(&cmd->cmd);
    cmd->next = NULL;
}

void msg_wrap_destroy_str(msg_wrap_str_t *str)
{
    msg_destroy_string(&str->id);
    msg_destroy_string(&str->content);
    str->next = NULL;
}

void msg_wrap_destroy_int(msg_wrap_int_t *i)
{
    msg_destroy_string(&i->id);
    i->val = 0;
    i->next = NULL;
}

void msg_wrap_destroy_float(msg_wrap_float_t *f)
{
    msg_destroy_string(&f->id);
    f->val = 0.0;
    f->next = NULL;
    f->prec = 0;
}



msg_wrap_t msg_wrapper_init_msg(char *msg_id)
{
    msg_wrap_t res;
    res.id = msg_init_string(msg_id);
    res.cmd_queue = NULL;
    res.obj_queue = NULL;
    return res;
}


msg_wrap_cmd_t msg_wrapper_init_cmd(char *cmd)
{
    msg_wrap_cmd_t res;
    res.cmd = msg_init_string(cmd);
    res.next = NULL;
    return res;
}

msg_wrap_obj_t msg_wrapper_init_obj(char *obj_id)
{
    msg_wrap_obj_t res;
    res.id = msg_init_string(obj_id);
    res.int_queue = NULL;
    res.float_queue = NULL;
    res.string_queue = NULL;
    res.next = NULL;
    return res;
}

msg_wrap_str_t msg_wrapper_init_string(char *id, char *content)
{
    msg_wrap_str_t res;
    res.id = msg_init_string(id);
    res.content = msg_init_string(content);
    res.next = NULL;
    return res;
}

msg_wrap_int_t msg_wrapper_init_int(char *id, int val)
{
    msg_wrap_int_t res;
    res.id = msg_init_string(id);
    res.val = val;
    res.next = NULL;
    return res;
}

msg_wrap_float_t msg_wrapper_init_float(char *id, float val, uint8_t prec)
{
    msg_wrap_float_t res;
    res.id = msg_init_string(id);
    res.val = val;
    res.prec = prec;
    res.next = NULL;
    return res;
}


void msg_wrapper_add_string_to_obj(msg_wrap_obj_t *obj, msg_wrap_str_t *str)
{
    msg_wrap_str_t *strp;
    if(obj->string_queue == NULL) { //empty
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


void msg_wrapper_rm_string_from_obj(msg_wrap_obj_t *obj, msg_wrap_str_t *str)
{
    msg_wrap_str_t *sp, *prev;
    for(sp = obj->string_queue, prev = NULL; sp != NULL; sp = sp->next){
        if(sp == str) { 
            if(prev == NULL) { // if p is the head of the queue, reinit the head
                obj->string_queue = str->next;
            } else {
                prev->next = str->next; //skip the expected
            }
            str->next = NULL; // reset next
            return;
        }
        prev = sp;
    }
}

void msg_wrapper_add_int_to_obj(msg_wrap_obj_t *obj, msg_wrap_int_t *int_val)
{
    msg_wrap_int_t *ip;
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


void msg_wrapper_rm_int_from_obj(msg_wrap_obj_t *obj, msg_wrap_int_t *i)
{
    msg_wrap_int_t *ip, *prev;
    for(ip = obj->int_queue, prev = NULL; ip != NULL; ip = ip->next){
        if(ip == i) {
            if(prev == NULL) { // if p is the head of the queue, reinit the head
                obj->int_queue = i->next;
            } else {
                prev->next = i->next; //skip the expected
            }
            i->next = NULL; // reset next
            return;
        }
        prev = ip;
    }
}

void msg_wrapper_add_float_to_obj(msg_wrap_obj_t *obj, msg_wrap_float_t *float_val)
{
    msg_wrap_float_t *fp;
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


void msg_wrapper_rm_float_from_obj(msg_wrap_obj_t *obj, msg_wrap_float_t *f)
{
    msg_wrap_float_t *fp, *prev;
    for(fp = obj->float_queue, prev = NULL; fp != NULL; fp = fp->next){
        if(fp == f) {
            if(prev == NULL) { // if p is the head of the queue, reinit the head
                obj->float_queue = f->next;
            } else {
                prev->next = f->next; //skip the expected
            }
            f->next = NULL; // reset next
            return;
        }
        prev = fp;
    }
}

void msg_wrapper_add_object_to_msg(msg_wrap_t *msg, msg_wrap_obj_t *obj)
{
    msg_wrap_obj_t *op;
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

void msg_wrapper_rm_obj_from_msg(msg_wrap_t *msg, msg_wrap_obj_t *obj)
{
    msg_wrap_obj_t *op, *prev;
    for(op = msg->obj_queue, prev = NULL; op != NULL; op = op->next){
        if(op == obj) {
            if(prev == NULL) { // if p is the head of the queue, reinit the head
                msg->obj_queue = obj->next;
            } else {
                prev->next = obj->next; //skip the expected
            }
            obj->next = NULL; // reset next
            return;
        }
        prev = op;
    }    
}

void msg_wrapper_add_cmd_to_msg(msg_wrap_t *msg, msg_wrap_cmd_t *cmd)
{
    msg_wrap_cmd_t *cp;
    if(msg->cmd_queue == NULL) { //if empty 
        msg->cmd_queue = cmd;
        msg->cmd_queue->next = NULL;
    } else {
        cp = msg->cmd_queue;
        while(cp->next != NULL)
            cp = cp->next;
        cp->next = cmd;
        cmd->next = NULL;
    }
}

void msg_wrapper_rm_cmd_from_msg(msg_wrap_t *msg, msg_wrap_cmd_t *cmd)
{
    msg_wrap_cmd_t *cp, *prev;
    for(cp = msg->cmd_queue, prev = NULL; cp != NULL; cp = cp->next){
        if(cp == cmd) {
            if(prev == NULL) { // if p is the head of the queue, reinit the head
                msg->cmd_queue = cmd->next;
            } else {
                prev->next = cmd->next; //skip the expected
            }
            cmd->next = NULL; // reset next
            return;
        }
        prev = cp;
    }    
}
#endif