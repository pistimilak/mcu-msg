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
    mcu_msg_size_t len;       // string length
} mcu_msg_string_t;

typedef struct {
    mcu_msg_string_t cmd;
} mcu_msg_cmd_t;

typedef struct {
    int (*putc)(char);
    void (*copy_to_chr_arr)(char *, mcu_msg_string_t);  // copy strint type to char array function pointer
    void (*copy)(mcu_msg_string_t, mcu_msg_string_t);   // copy string type to an other one function pointer
    void (*print)(mcu_msg_string_t);                    // print string type function pointer
} mcu_msg_string_hnd_t;


typedef struct {
    mcu_msg_string_t id;        // id string
    mcu_msg_string_t content;   // content string
} mcu_msg_t;

typedef struct{
    mcu_msg_string_t id;        // id string
    mcu_msg_string_t content;   // content string
} mcu_msg_obj_t;



/////////////////////////////////// Types for wrapper //////////////////////////////
#if MCU_MSG_USE_WRAPPER

/*String type for wrapper*/
typedef struct {
    mcu_msg_string_t id;
    mcu_msg_string_t content;
    mcu_msg_string_wrap_t *next;
} mcu_msg_string_wrap_t;


/*Int type for wrapper*/
typedef struct {
    mcu_msg_string_t id;
    int val;
    struct mcu_msg_int_t *next;
} mcu_msg_int_wrap_t;


/*Float type for wrapper*/
typedef struct {
    mcu_msg_string_t id;
    float val;
    struct mcu_msg_float_t *next;
} mcu_msg_float_wrap_t;

/*cmd type for wrapper*/
typedef struct mcu_msg_cmd{
    mcu_msg_string_t cmd;
    struct mcu_msg_cmd_t *next;
} mcu_msg_cmd_wrap_t;

typedef struct {
    mcu_msg_string_t id;        // id string
    mcu_msg_string_t content; 
    mcu_msg_int_wrap_t *int_queue;
    mcu_msg_float_wrap_t *float_queue;
    mcu_msg_string_wrap_t *string_queue;
    struct mcu_msg_obj_t *next;
} mcu_msg_obj_wrap_t;

typedef struct {
    mcu_msg_string_t id;
    mcu_msg_obj_t *obj_queue;
    mcu_msg_cmd_t *cmd_queue;
} mcu_msg_wrap_t;

typedef struct {
    // int (*putc) (char);
    void (*print) (mcu_msg_wrap_t *);
    void (*print_buff) (mcu_msg_wrap_t *, char *, mcu_msg_size_t);
} mcu_msg_wrap_hnd_t;
#endif


/**
 * @brief 
 * 
 * @param msg 
 */
void mcu_msg_destroy(mcu_msg_t *msg);


/**
 * @brief 
 * 
 * @param obj 
 */
void mcu_msg_destroy_obj(mcu_msg_obj_t *obj);

/**
 * @brief 
 * 
 * @param str 
 */
void mcu_msg_destroy_string(mcu_msg_string_t *str);

/**
 * @brief 
 * 
 * @param raw_str 
 * @param id 
 * @param len 
 * @return mcu_msg_t 
 */
mcu_msg_t mcu_msg_get(char *raw_str, char *id, mcu_msg_size_t len);

/**
 * @brief 
 * 
 * @param msg 
 * @param id 
 * @return mcu_msg_obj_t 
 */
mcu_msg_obj_t mcu_msg_parser_get_obj(mcu_msg_t msg, char *id);


/**
 * @brief 
 * 
 * @param msg 
 * @param cmd_id 
 * @return mcu_msg_cmd_t 
 */
mcu_msg_cmd_t mcu_msg_parser_get_cmd(mcu_msg_t msg, char *cmd_id);


/**
 * @brief 
 * 
 * @param res 
 * @param obj 
 * @param key 
 * @return int8_t 
 */
int8_t mcu_msg_parser_get_int(int *res, mcu_msg_obj_t obj, char *key);


/**
 * @brief 
 * 
 * @param res_val 
 * @param obj 
 * @param key 
 * @return int8_t 
 */
int8_t mcu_msg_parser_get_float(float *res_val, mcu_msg_obj_t obj, char *key);


/**
 * @brief 
 * 
 * @param obj 
 * @param key 
 * @return mcu_msg_string_t 
 */
mcu_msg_string_t mcu_msg_parser_get_string(mcu_msg_obj_t obj, char *key);


/**
 * @brief 
 * 
 * @param print 
 * @return mcu_msg_string_hnd_t 
 */
mcu_msg_string_hnd_t mcu_msg_string_hnd_create(void (*print)(mcu_msg_string_t));

#if MCU_MSG_USE_WRAPPER
void mcu_msg_wrapper_init_obj_queues(mcu_msg_obj_wrap_t *obj);
void mcu_msg_wrapper_add_string_to_obj(mcu_msg_obj_wrap_t *obj, mcu_msg_string_wrap_t *str);
// void mcu_msg_wrapper_add_int_to_obj(mcu_msg_obj_t *obj, mcu_msg_int_t *int_valp);
// void mcu_msg_wrapper_add_float_to_obj(mcu_msg_obj_t *obj, mcu_msg_float_t *float_valp);
#endif



#endif