/**
 * @file msg_cfg.h
 * @author Istvan Milak (istvan.milak@gmail.com)
 * @brief Configuration for mcu-msg library
 * @version 0.1
 * @date 2019-12-19
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef __MCU_MSG_CFG_H__
#define __MCU_MSG_CFG_H__

#include <inttypes.h>

/*
define the size of message type
*/
typedef uint16_t msg_size_t;


/*
Sometime for a simple command parser applicaton, you don't need to send back formatted messages to master
This keyword can enable the wrapper features
*/
#define MCU_MSG_USE_WRAPPER         1

/*
Using buffer to store the strings
*/
#define MCU_MSG_USE_BUFFERING       0
#endif