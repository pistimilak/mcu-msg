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

#ifndef __MCU_MSG_PARSER__
#define __MCU_MSG_PARSER__

#include <inttypes.h>
#include "mcu_msg_cfg.h"


#ifndef NULL
#define NULL    ((void *)0)
#endif



/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
//                                      Parser types                                       //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

/*
Primitive string type for other complex objects and string
Length is necessary to use same buffer where there isn't 0 terminator
*/
typedef struct msg_str {
    char*      s;         /* pointer to string content in the buffer */
    msg_size_t len;       /* string length */
} msg_str_t;

/*Getting string ponter*/
#define msg_str_p(str)        (str.s)


/*Command type for parser*/
typedef struct msg_cmd {
    msg_str_t cmd;       /* cmd string */
} msg_cmd_t;

/*Getting command*/
#define msg_get_cmd_content(cmd)        msg_str_p(cmd.cmd)


/*Get content for checking NULL pointers or content*/
#define msg_get_content(mobj)           msg_str_p(mobj.content)

/*message type for parser*/
typedef struct msg {
    msg_str_t id;        /* id string */
    msg_str_t content;   /* content string */
} msg_t;


/*Object type for parser*/
typedef struct msg_obj {
    msg_str_t id;        /* id string */
    msg_str_t content;   /* content string */
} msg_obj_t;





/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
//                                      Wrapper types                                      //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER

/*String type for wrapper*/
typedef struct msg_wrap_str {
    msg_str_t            id;        /* id string */
    msg_str_t            content;   /* content string */
    struct msg_wrap_str* next;      /* next wrap string */
} msg_wrap_str_t;


/*Int type for wrapper*/
typedef struct msg_wrap_int {
    int                  val;    /* int value */
    msg_str_t            id;     /* id string */
    struct msg_wrap_int* next;   /* next wrap integer */
} msg_wrap_int_t;


/*Float type for wrapper*/
typedef struct msg_wrap_float {
    float                  val;      /* float value */
    msg_str_t              id;       /* id string */
    uint8_t                prec;     /* precision for printing */
    struct msg_wrap_float* next;     /* next wrap float */
} msg_wrap_float_t;


/*cmd type for wrapper*/
typedef struct msg_wrap_cmd {
    msg_str_t            cmd;   /* cmd string */
    struct msg_wrap_cmd* next;  /* next wrap cmd */
} msg_wrap_cmd_t;


/*object type for wrapper*/
typedef struct msg_wrap_obj {
    msg_str_t            id;            /* id string */
    msg_wrap_int_t*      int_queue;     /* wrap integer queue */
    msg_wrap_float_t*    float_queue;   /* wrap float queue */
    msg_wrap_str_t*      string_queue;  /* wrap string queues */
    struct msg_wrap_obj* next;          /* next object wrapper */
} msg_wrap_obj_t;


typedef struct msg_wrap {
    msg_str_t       id;         /* id string */
    msg_wrap_obj_t* obj_queue;  /* wrap object queue */
    msg_wrap_cmd_t* cmd_queue;  /* wrap cmd queue */
} msg_wrap_t;

#endif


/*
Handler type
Handler containes the basic functions to print std out or internal string buffer
*/
typedef struct msg_hnd{
    int  (*putc)              (char c);                             /* putchar interface       */
    void (*print_msg)         (msg_t msg);                          /* print message interface */
    void (*print_str)         (msg_str_t str);                      /* print str interface     */
    void (*print_int)         (int i);                              /* print int interface     */
    void (*print_float)       (float f, uint8_t prec);              /* print float interface   */
    void (*enable_buff)       (void);                               /* enable buffering        */
    void (*disable_buff)      (void);                               /* disable buffering       */
    void (*init_str_buff)     (char *buff, msg_size_t buff_size);   /* init string buffer      */
    void (*reset_str_buff)    (void);                               /* reset string buffer     */
 #if MCU_MSG_USE_WRAPPER
    void (*print_wrapper_msg) (msg_wrap_t);
 #endif
} msg_hnd_t;


/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
//                                      Parser functions                                   //
//                                                                                         //
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
void                msg_destroy_str (msg_str_t *str);

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
 * @return uint8_t 0 if not found, digit count if found 
 */
uint8_t             msg_parser_get_int (int *res, msg_obj_t obj, char *key);

/**
 * @brief Get float from object
 * 
 * @param res_val result float pointer
 * @param obj object
 * @param key key
 * @return uint8_t 0 if not found, digit count if found 
 */
uint8_t             msg_parser_get_float (float *res_val, msg_obj_t obj, char *key);

/**
 * @brief Get string from object
 * 
 * @param obj obj
 * @param key key
 * @return msg_str_t string location if found, NULL if not found
 */
msg_str_t           msg_parser_get_str (msg_obj_t obj, char *key);

/**
 * @brief Create string handler for printing and copying
 * 
 * @param putc putchar function depends on architecture
 * @return msg_string_hnd_t result handler
 */
msg_hnd_t    msg_hnd_create (int (*putc)(char));


/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
//                                     Wrapper functions                                   //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER


/**
 * @brief Ddestroy message wrapper
 * 
 * @param msg message wrapper pointer
 */
void                msg_wrap_destroy (msg_wrap_t *msg);

/**
 * @brief Destroy object wrapper
 * 
 * @param obj object wrapper pointer
 */
void                msg_wrap_destroy_obj (msg_wrap_obj_t *obj);

/**
 * @brief Destroy command wrapper
 * 
 * @param cmd command wrapper pointer
 */
void                msg_wrap_destroy_cmd (msg_wrap_cmd_t *cmd);

/**
 * @brief Destroy string wrapper
 * 
 * @param str string wrapper pointer
 */
void                msg_wrap_destroy_str (msg_wrap_str_t *str);

/**
 * @brief Destroy integer wrapper
 * 
 * @param i integer wrapper pointer
 */
void                msg_wrap_destroy_int (msg_wrap_int_t *i);

/**
 * @brief Destroy float wrapper
 * 
 * @param f float wrapper pointer
 */
void                msg_wrap_destroy_float (msg_wrap_float_t *f);

/**
 * @brief Create message wrapper
 * 
 * @param msg_id Expected message ID
 * @return msg_wrap_t message wrapper
 */
msg_wrap_t          msg_wrapper_create_msg (char *msg_id);

/**
 * @brief Create command wrapper
 * 
 * @param cmd Expected command string
 * @return msg_wrap_cmd_t command wrapper
 */
msg_wrap_cmd_t      msg_wrapper_create_cmd (char *cmd);

/**
 * @brief Create object wrapper
 * 
 * @param obj_id Expected object ID
 * @return msg_wrap_obj_t object wrapper
 */
msg_wrap_obj_t      msg_wrapper_create_obj (char *obj_id);

/**
 * @brief Create string wrapper
 * 
 * @param id Expected string ID
 * @param content Init content
 * @return msg_wrap_str_t string wrapper
 */
msg_wrap_str_t      msg_wrapper_create_str (char *id, char *content);

/**
 * @brief Create integer wrapper
 * 
 * @param id Expected ID
 * @param val Init value
 * @return msg_wrap_int_t integer wrapper
 */
msg_wrap_int_t      msg_wrapper_create_int (char *id, int val);

/**
 * @brief Create float wrapper
 * 
 * @param id Expected ID
 * @param val Init value
 * @param prec precision of printing
 * @return msg_wrap_float_t float wrapper
 */
msg_wrap_float_t    msg_wrapper_create_float (char *id, float val, uint8_t prec);

/**
 * @brief Add string wrapper to string queue of object wrapper
 * 
 * @param obj object wrapper pointer
 * @param str string wrapper pointer
 */
void                msg_wrapper_add_str_to_obj (msg_wrap_obj_t *obj, msg_wrap_str_t *str);

/**
 * @brief Add integer wrapper to integer queue of object wrapper
 * 
 * @param obj object wrapper pointer
 * @param int_val integer wrapper pointer
 */
void                msg_wrapper_add_int_to_obj (msg_wrap_obj_t *obj, msg_wrap_int_t *int_val);

/**
 * @brief Add float wrapper to float queue of object wrapper
 * 
 * @param obj object wrapper pointer
 * @param float_val float wrapper pointer
 */
void                msg_wrapper_add_float_to_obj (msg_wrap_obj_t *obj, msg_wrap_float_t *float_val);

/**
 * @brief Add object wrapper to object queue of message wrapper
 * 
 * @param msg message wrapper pointer
 * @param obj object wrapper pointer
 */
void                msg_wrapper_add_obj_to_msg (msg_wrap_t *msg, msg_wrap_obj_t *obj);

/**
 * @brief Add command wrapper to command queue of message wrapper
 * 
 * @param msg message wrappar pointer
 * @param cmd command wrapper pointer
 */
void                msg_wrapper_add_cmd_to_msg (msg_wrap_t *msg, msg_wrap_cmd_t *cmd);

/**
 * @brief Remove string wrapper from string queue of object wrapper
 * 
 * @param obj object wrapper pointer
 * @param str string wrapper pointer
 */
void                msg_wrapper_rm_string_from_obj (msg_wrap_obj_t *obj, msg_wrap_str_t *str);

/**
 * @brief Remove integer wrapper from integer queue of object wrapper
 * 
 * @param obj object wrapper pointer
 * @param i interge wrapper pointer
 */
void                msg_wrapper_rm_int_from_obj (msg_wrap_obj_t *obj, msg_wrap_int_t *i);

/**
 * @brief Remove float wrapper from float queue of object wrapper
 * 
 * @param obj object wrapper pointer
 * @param f float wrapper pointer
 */
void                msg_wrapper_rm_float_from_obj (msg_wrap_obj_t *obj, msg_wrap_float_t *f);

/**
 * @brief Remove object wrapper from object queue of message wrapper
 * 
 * @param msg message wrapper pointer
 * @param obj object wrapper pointer
 */
void                msg_wrapper_rm_obj_from_msg (msg_wrap_t *msg, msg_wrap_obj_t *obj);

/**
 * @brief Remove command wrapper from command queue of message wrapper
 * 
 * @param msg message wrapper pointer
 * @param cmd command wrapper pointer
 */
void                msg_wrapper_rm_cmd_from_msg (msg_wrap_t *msg, msg_wrap_cmd_t *cmd);
#endif


#endif /*EOF*/