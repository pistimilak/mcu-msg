#ifndef __MCU_MSG_PARSER__
#define __MCU_MSG_PARSER__

#include <inttypes.h>

#ifndef NULL
#define NULL    ((void *)0)
#endif


typedef uint16_t mcu_msg_size_t;

typedef struct {
    char *s;                  // pointer to string content in the buffer
    mcu_msg_size_t len;             // 
} mcu_msg_string_t;


typedef struct {
    void (*copy)(char *dest, mcu_msg_string_t source);
    void (*print)(mcu_msg_string_t str);
} mcu_msg_string_hnd_t;


typedef struct {
    // char *id;
    // char *content;
    // mcu_msg_size_t id_len;
    // mcu_msg_size_t content_len;
    mcu_msg_string_t id;
    mcu_msg_string_t content;
} mcu_msg_t;

typedef struct {
    // char *id;                     // pointer to the object name in the buffer
    // char *content;                  // pointer to the object content in the buffer
    // mcu_msg_size_t id_len;        // object name length
    // mcu_msg_size_t content_len;     // content length
    mcu_msg_string_t id;
    mcu_msg_string_t content;
} mcu_msg_obj_t;



void mcu_msg_destroy(mcu_msg_t *msg);
void mcu_msg_destroy_obj(mcu_msg_obj_t *obj);
void mcu_msg_destroy_string(mcu_msg_string_t *str);
mcu_msg_t mcu_msg_get(char *raw_str, char *id, mcu_msg_size_t len);
mcu_msg_obj_t mcu_msg_parser_get_obj(mcu_msg_t msg, char *id);
int8_t mcu_msg_parser_get_int(int *res, mcu_msg_obj_t obj, char *key);
int8_t mcu_msg_parser_get_float(float *res_val, mcu_msg_obj_t obj, char *key);
mcu_msg_string_t mcu_msg_parser_get_string(mcu_msg_obj_t obj, char *key);
mcu_msg_string_hnd_t mcu_msg_string_hnd_init(void (*print)(mcu_msg_string_t));

#endif