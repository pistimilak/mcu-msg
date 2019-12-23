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


typedef struct msg_string_hnd{
    int     (*putc)             (char);                 // putchar implementation
    void    (*copy_to_chr_arr)  (char *, msg_str_t);    // copy strint type to char array function pointer
    void    (*copy)             (msg_str_t, msg_str_t); // copy string type to an other one function pointer
    void    (*print)            (msg_str_t);            // print string type function pointer
} msg_string_hnd_t;


typedef struct msg {
    msg_str_t id;        // id string
    msg_str_t content;   // content string
} msg_t;


typedef struct msg_obj {
    msg_str_t id;        // id string
    msg_str_t content;   // content string
} msg_obj_t;

#if MCU_MSG_USE_BUFFERING
typedef struct msg_str_buff {
    msg_str_t  buff;
    char*      p;               // pointer to the next element
} msg_str_buff_t;
#endif

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




typedef struct msg_wrap_hnd{
    void  (*print)         (msg_wrap_t);                            // print message to output
    void  (*print_obj)     (msg_wrap_obj_t);                        // print object to output
    void  (*print_cmd)     (msg_wrap_cmd_t);                        // print command to output
// if use buffering feauters
#if MCU_MSG_USE_BUFFERING 
    msg_size_t (*print_to_buff)     (msg_str_buff_t*, msg_wrap_t);       // print message to buffer
#endif

} msg_wrap_hnd_t;
#endif


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
 * @brief 
 * 
 * @param msg 
 * @param id 
 * @return msg_obj_t 
 */
msg_obj_t           msg_parser_get_obj (msg_t msg, char *id);
msg_cmd_t           msg_parser_get_cmd (msg_t msg, char *cmd_id);
int8_t              msg_parser_get_int (int *res, msg_obj_t obj, char *key);
int8_t              msg_parser_get_float (float *res_val, msg_obj_t obj, char *key);
msg_str_t           msg_parser_get_string (msg_obj_t obj, char *key);
msg_string_hnd_t    msg_string_hnd_create (int (*putc)(char));

#if MCU_MSG_USE_BUFFERING
void msg_destroy_str_buff(msg_str_buff_t *buff);
msg_str_buff_t msg_init_str_buff(char *buff, msg_size_t buff_size);
#endif


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
msg_wrap_hnd_t      msg_wrapper_hnd_create (int (*putc)(char));
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