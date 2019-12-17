#ifndef __MCU_MSG_PARSER__
#define __MCU_MSG_PARSER__

#include <inttypes.h>

#ifndef NULL
#define NULL    ((void *)0)
#endif


typedef uint16_t mcu_msg_size_t;

typedef struct {
    char *name;
    char *content;
    mcu_msg_size_t name_len;
    mcu_msg_size_t content_len;
} mcu_msg_obj_t;


mcu_msg_obj_t  mcu_msg_parser_get_obj(char *msg, char *obj, mcu_msg_size_t len);
int8_t mcu_msg_parser_get_int(int *res, mcu_msg_obj_t obj, char *key);
int8_t mcu_msg_parser_get_float(float *res_val, mcu_msg_obj_t obj, char *key);
void mcu_msg_parser_get_string(char *dest, mcu_msg_obj_t obj, char *key);
// char *find_keyword(char *msg, mcu_msg_size_t len, char *keyword, char flagc, char stopc);
#endif