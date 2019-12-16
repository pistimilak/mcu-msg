#ifndef __MCU_MSG_PARSER__
#define __MCU_MSG_PARSER__

#include <inttypes.h>

#define TMP_BUFF_SIZE       100

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
// mcu_msg_size_t   str_len(char *str);
#endif