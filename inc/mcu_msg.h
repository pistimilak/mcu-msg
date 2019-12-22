#ifndef __MCU_MSG_PARSER__
#define __MCU_MSG_PARSER__

#include <inttypes.h>
#include "mcu_msg_cfg.h"


#ifndef NULL
#define NULL    ((void *)0)
#endif




/*
Primitive string type for other complex objects and string
Length is necessary to use same buffer where there isn't 0 terminator
*/
typedef struct {
    char *s;                  // pointer to string content in the buffer
    mmsg_size_t len;       // string length
} mmsg_string_t;

typedef struct {
    mmsg_string_t cmd;
} mmsg_cmd_t;

typedef struct {
    int (*putc) (char);
    void (*copy_to_chr_arr) (char *, mmsg_string_t);  // copy strint type to char array function pointer
    void (*copy) (mmsg_string_t, mmsg_string_t);   // copy string type to an other one function pointer
    void (*print) (mmsg_string_t);                    // print string type function pointer
} mmsg_string_hnd_t;


typedef struct {
    mmsg_string_t id;        // id string
    mmsg_string_t content;   // content string
} mmsg_t;

typedef struct{
    mmsg_string_t id;        // id string
    mmsg_string_t content;   // content string
} mmsg_obj_t;



/////////////////////////////////// Types for wrapper //////////////////////////////
#if MCU_MSG_USE_WRAPPER

/*String type for wrapper*/
typedef struct {
    mmsg_string_t id;
    mmsg_string_t content;
    struct mmsg_string_wrap_t *next;
} mmsg_string_wrap_t;


/*Int type for wrapper*/
typedef struct {
    mmsg_string_t id;
    int val;
    struct mmsg_int_t *next;
} mmsg_int_wrap_t;


/*Float type for wrapper*/
typedef struct {
    mmsg_string_t id;
    float val;
    uint8_t prec;                   // precision for printing
    struct mmsg_float_t *next;
} mmsg_float_wrap_t;

/*cmd type for wrapper*/
typedef struct {
    mmsg_string_t cmd;
    struct mmsg_cmd_t *next;
} mmsg_cmd_wrap_t;

typedef struct {
    mmsg_string_t id;        // id string
    // mmsg_string_t content; 
    mmsg_int_wrap_t *int_queue;
    mmsg_float_wrap_t *float_queue;
    mmsg_string_wrap_t *string_queue;
    struct mmsg_obj_t *next;
} mmsg_obj_wrap_t;

typedef struct {
    mmsg_string_t id;
    mmsg_obj_wrap_t *obj_queue;
    mmsg_cmd_wrap_t *cmd_queue;
} mmsg_wrap_t;

typedef struct {
    // int (*putc) (char);
    void (*print) (mmsg_wrap_t);
    void (*print_obj) (mmsg_obj_wrap_t);
    void (*print_cmd) (mmsg_cmd_wrap_t);
    char* (*print_to_buff) (mmsg_wrap_t, char *, mmsg_size_t);
} mmsg_wrap_hnd_t;
#endif


/**
 * @brief 
 * 
 * @param msg 
 */
void mmsg_destroy(mmsg_t *msg);


/**
 * @brief 
 * 
 * @param obj 
 */
void mmsg_destroy_obj(mmsg_obj_t *obj);

/**
 * @brief 
 * 
 * @param str 
 */
void mmsg_destroy_string(mmsg_string_t *str);

/**
 * @brief 
 * 
 * @param raw_str 
 * @param id 
 * @param len 
 * @return mmsg_t 
 */
mmsg_t mmsg_get(char *raw_str, char *id, mmsg_size_t len);

/**
 * @brief 
 * 
 * @param msg 
 * @param id 
 * @return mmsg_obj_t 
 */
mmsg_obj_t mmsg_parser_get_obj(mmsg_t msg, char *id);


/**
 * @brief 
 * 
 * @param msg 
 * @param cmd_id 
 * @return mmsg_cmd_t 
 */
mmsg_cmd_t mmsg_parser_get_cmd(mmsg_t msg, char *cmd_id);


/**
 * @brief 
 * 
 * @param res 
 * @param obj 
 * @param key 
 * @return int8_t 
 */
int8_t mmsg_parser_get_int(int *res, mmsg_obj_t obj, char *key);


/**
 * @brief 
 * 
 * @param res_val 
 * @param obj 
 * @param key 
 * @return int8_t 
 */
int8_t mmsg_parser_get_float(float *res_val, mmsg_obj_t obj, char *key);


/**
 * @brief 
 * 
 * @param obj 
 * @param key 
 * @return mmsg_string_t 
 */
mmsg_string_t mmsg_parser_get_string(mmsg_obj_t obj, char *key);


/**
 * @brief 
 * 
 * @param print 
 * @return mmsg_string_hnd_t 
 */
mmsg_string_hnd_t mmsg_string_hnd_create(int (*putc)(char));

#if MCU_MSG_USE_WRAPPER

void                mmsg_wrap_destroy(mmsg_wrap_t *msg);
void                mmsg_wrap_destroy_obj(mmsg_obj_wrap_t *obj);
void                mmsg_wrap_destroy_str(mmsg_string_wrap_t *str);
void                mmsg_wrap_destroy_int(mmsg_int_wrap_t *i);
void                mmsg_wrap_destroy_float(mmsg_float_wrap_t *f);
mmsg_wrap_hnd_t     mmsg_wrapper_hnd_create(int (*putc)(char));
mmsg_wrap_t         mmsg_wrapper_init_msg(char *msg_id);
mmsg_cmd_wrap_t     mmsg_wrapper_init_cmd(char *cmd);
mmsg_obj_wrap_t     mmsg_wrapper_init_obj(char *obj_id);
mmsg_string_wrap_t  mmsg_wrapper_init_string(char *id, char *content);
mmsg_int_wrap_t     mmsg_wrapper_init_int(char *id, int val);
mmsg_float_wrap_t   mmsg_wrapper_init_float(char *id, float val, uint8_t prec);
void                mmsg_wrapper_add_string_to_obj(mmsg_obj_wrap_t *obj, mmsg_string_wrap_t *str);
void                mmsg_wrapper_add_int_to_obj(mmsg_obj_wrap_t *obj, mmsg_int_wrap_t *int_val);
void                mmsg_wrapper_add_float_to_obj(mmsg_obj_wrap_t *obj, mmsg_float_wrap_t *float_val);
void                mmsg_wrapper_add_object_to_msg(mmsg_wrap_t *msg, mmsg_obj_wrap_t *obj);
void                mmsg_wrapper_add_cmd_to_msg(mmsg_wrap_t *msg, mmsg_cmd_wrap_t *cmd);


#endif



#endif