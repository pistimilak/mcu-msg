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


/*Int type for wrapper*/
typedef struct {
    mcu_msg_string_t id;
    int val;
} mcu_msg_int_t;


/*Float type for wrapper*/
typedef struct {
    mcu_msg_string_t id;
    float val;
} mcu_msg_float_t;


/*cmd type for wrapper*/

typedef struct {
    mcu_msg_string_t cmd;
#if MCU_MSG_USE_WRAPPER
    struct mcu_msg_cmd_t *next;
#endif
} mcu_msg_cmd_t;


typedef struct {
    void (*copy_to_chr_arr)(char *, mcu_msg_string_t);  // copy strint type to char array function pointer
    void (*copy)(mcu_msg_string_t, mcu_msg_string_t);   // copy string type to an other one function pointer
    void (*print)(mcu_msg_string_t);                    // print string type function pointer
} mcu_msg_string_hnd_t;


typedef struct {
    mcu_msg_string_t id;        // id string
    mcu_msg_string_t content;   // content string
} mcu_msg_t;

typedef struct {
    mcu_msg_string_t id;        // id string
    mcu_msg_string_t content;   // content string
} mcu_msg_obj_t;


#if MCU_MSG_USE_WRAPPER
typedef struct {
    mcu_msg_string_t id;
    mcu_msg_obj_t *obj_queue;
    mcu_msg_cmd_t *cmd_queue;
} mcu_msg_wrapper_t;
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
 * @return uint8_t 
 */
uint8_t mcu_msg_is_cmd_att(mcu_msg_t msg, char *cmd_id);

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

#endif