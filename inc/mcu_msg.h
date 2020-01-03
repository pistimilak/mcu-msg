#ifndef __MCU_MSG_PARSER__
#define __MCU_MSG_PARSER__

#include <inttypes.h>
#include "mcu_msg_cfg.h"


#ifndef NULL
#define NULL    ((void *)0)
#endif



/////////////////////////////////////////////////////////////////////////////////////////////
//                                      Parser typess                                      //
/////////////////////////////////////////////////////////////////////////////////////////////

/*
Primitive string type for other complex objects and string
Length is necessary to use same buffer where there isn't 0 terminator
*/
typedef struct msg_str {
    char*      s;         // pointer to string content in the buffer
    msg_size_t len;       // string length
} msg_str_t;


typedef struct msg_cmd {
    msg_str_t cmd;       // cmd string
} msg_cmd_t;





typedef struct msg {
    msg_str_t id;        // id string
    msg_str_t content;   // content string
} msg_t;


typedef struct msg_obj {
    msg_str_t id;        // id string
    msg_str_t content;   // content string
} msg_obj_t;


/////////////////////////////////////////////////////////////////////////////////////////////
//                                      Wrapper typess                                     //
/////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER

/*String type for wrapper*/
typedef struct msg_wrap_str {
    msg_str_t            id;        // id string
    msg_str_t            content;   // content string
    struct msg_wrap_str* next;      // next wrap string
} msg_wrap_str_t;


/*Int type for wrapper*/
typedef struct msg_wrap_int {
    msg_str_t       id;     // id string
    int             val;    // int value
    struct msg_int* next;   // next wrap integer
} msg_wrap_int_t;


/*Float type for wrapper*/
typedef struct msg_wrap_float {
    msg_str_t         id;       // id string
    float             val;      // float value
    uint8_t           prec;     // precision for printing
    struct msg_float* next;     // next wrap float
} msg_wrap_float_t;


/*cmd type for wrapper*/
typedef struct msg_wrap_cmd {
    msg_str_t            cmd;   // cmd string
    struct msg_wrap_cmd* next;  // next wrap cmd
} msg_wrap_cmd_t;


typedef struct msg_wrap_obj {
    msg_str_t            id;            // id string
    msg_wrap_int_t*      int_queue;     // wrap integer queue
    msg_wrap_float_t*    float_queue;   // wrap float queue
    msg_wrap_str_t*      string_queue;  // wrap string queues
    struct msg_wrap_obj* next;          // next object wrapper
} msg_wrap_obj_t;


typedef struct msg_wrap {
    msg_str_t       id;         // id string
    msg_wrap_obj_t* obj_queue;  // wrap object queue
    msg_wrap_cmd_t* cmd_queue;  // wrap cmd queue
} msg_wrap_t;

#endif



typedef struct msg_hnd{
    int  (*putc)              (char);                 // putchar implementation
    void (*print_msg)         (msg_t);
    void (*print_str)         (msg_str_t);
    void (*print_int)         (int);
    void (*print_float)       (float, uint8_t);
    void (*enable_buff)       (void);
    void (*disable_buff)      (void);
    void (*init_str_buff)     (char *, msg_size_t);
    void (*reset_str_buff)    (void);
 #if MCU_MSG_USE_WRAPPER
    void (*print_wrapper_msg) (msg_wrap_t);
 #endif
} msg_hnd_t;


/////////////////////////////////////////////////////////////////////////////////////////////
//                                      Parser functions                                   //
/////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Destroy message
 * 
 * @param msg message pointer
 */
void                msg_destroy (msg_t *msg);

/**
 * @brief Destroy object
 * 
 * @param obj object pointer
 */
void                msg_destroy_obj (msg_obj_t *obj);

/**
 * @brief Destroy command
 * 
 * @param cmd command pointer
 */
void msg_destroy_cmd(msg_cmd_t *cmd);

/**
 * @brief Destroy string
 * 
 * @param str string pointer
 */
void                msg_destroy_string (msg_str_t *str);

/**
 * @brief Get message from buffer
 * 
 * @param raw_str string buffer (char array)
 * @param id id string
 * @param len size of buffer
 * @return msg_t message (empty if not found)
 */
msg_t               msg_get (char *raw_str, char *id, msg_size_t len);

/**
 * @brief Get object from message
 * 
 * @param msg message
 * @param id object id
 * @return msg_obj_t result object
 */
msg_obj_t           msg_parser_get_obj (msg_t msg, char *id);

/**
 * @brief Get command from message
 * 
 * @param msg message
 * @param cmd_id command string
 * @return msg_cmd_t result command
 */
msg_cmd_t           msg_parser_get_cmd (msg_t msg, char *cmd_id);

/**
 * @brief Get integer from object
 * 
 * @param res result integer pointer
 * @param obj object
 * @param key key
 * @return int8_t -1 if not found, digit count if found 
 */
int8_t              msg_parser_get_int (int *res, msg_obj_t obj, char *key);

/**
 * @brief Get float from object
 * 
 * @param res_val result float pointer
 * @param obj object
 * @param key key
 * @return int8_t -1 if not found, digit count if found 
 */
int8_t              msg_parser_get_float (float *res_val, msg_obj_t obj, char *key);

/**
 * @brief Get string from object
 * 
 * @param obj obj
 * @param key key
 * @return msg_str_t string location if found, NULL if not found
 */
msg_str_t           msg_parser_get_string (msg_obj_t obj, char *key);

/**
 * @brief Create string handler for printing and copying
 * 
 * @param putc putchar function depends on architecture
 * @return msg_string_hnd_t result handler
 */
msg_hnd_t    msg_hnd_create (int (*putc)(char));


/////////////////////////////////////////////////////////////////////////////////////////////
//                                     Wrapper functions                                   //
/////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER

void                msg_wrap_destroy (msg_wrap_t *msg);
void                msg_wrap_destroy_obj (msg_wrap_obj_t *obj);
void                msg_wrap_destroy_cmd (msg_wrap_cmd_t *cmd);
void                msg_wrap_destroy_str (msg_wrap_str_t *str);
void                msg_wrap_destroy_int (msg_wrap_int_t *i);
void                msg_wrap_destroy_float (msg_wrap_float_t *f);
msg_wrap_t          msg_wrapper_init_msg (char *msg_id);
msg_wrap_cmd_t      msg_wrapper_init_cmd (char *cmd);
msg_wrap_obj_t      msg_wrapper_init_obj (char *obj_id);
msg_wrap_str_t      msg_wrapper_init_string (char *id, char *content);
msg_wrap_int_t      msg_wrapper_init_int (char *id, int val);
msg_wrap_float_t    msg_wrapper_init_float (char *id, float val, uint8_t prec);
void                msg_wrapper_add_string_to_obj (msg_wrap_obj_t *obj, msg_wrap_str_t *str);
void                msg_wrapper_add_int_to_obj (msg_wrap_obj_t *obj, msg_wrap_int_t *int_val);
void                msg_wrapper_add_float_to_obj (msg_wrap_obj_t *obj, msg_wrap_float_t *float_val);
void                msg_wrapper_add_object_to_msg (msg_wrap_t *msg, msg_wrap_obj_t *obj);
void                msg_wrapper_add_cmd_to_msg (msg_wrap_t *msg, msg_wrap_cmd_t *cmd);
void                msg_wrapper_rm_string_from_obj (msg_wrap_obj_t *obj, msg_wrap_str_t *str);
void                msg_wrapper_rm_int_from_obj (msg_wrap_obj_t *obj, msg_wrap_int_t *i);
void                msg_wrapper_rm_float_from_obj (msg_wrap_obj_t *obj, msg_wrap_float_t *f);
void                msg_wrapper_rm_obj_from_msg (msg_wrap_t *msg, msg_wrap_obj_t *obj);
void                msg_wrapper_rm_cmd_from_msg (msg_wrap_t *msg, msg_wrap_cmd_t *cmd);
#endif


#endif /*EOF*/