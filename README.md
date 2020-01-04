# mcu-msg

## Description
Simple parser and wrapper for string based communication between microcontrollers.
This library is designed for UART and other serial communications.
The library can handle undefined count of message with undefined count of subobjects
 
The library doesn't use string.h and internal declared buffers, library operates with pointers and pointer arithmetic. 
The user can choose a suitable buffer size to store messages

### Message structure
Sample message:
```
#test_msg {
<CMD1> @obj1($key11=-1123334567; $key12='string "value"') 
@obj2($key21 = -1.123456789; $key22 = 'val22'; $key23 = 1000; $key24 = 12.34)<CMD_last> }
```

*All of keywords can contain [a-zA-Z0-9_] characters look like variable name, but it can start with number.
All of whitespace can be used between keywords and values.*

***Message*** is the main container. It must be start with ```#```, next is the ```message_id```. 
Message content is enclosed in ```{ }``` brackets.
A message can contain *objects* and *commands*.

***Command*** is a command string between ```< >```. It's a general type of message content. 
It can be used as a boolean value whitout any other meaning.

***Object*** is a complex container. *Key-Val pairs* are included in an object. Object start with ```@``` and an ```object_id```. Object content is enclosed in ```( )``` brackets. Separator is not necessary between objects and commands.

***Key*** can describe three type of values: *string, integer and float*. Key must start with ```$``` flag and it's followed by the ```key_id```. The value for a key must be start with ```=``` char and the value must be terminated by ```;``` separator. The last element in the object can be terminated by ```)``` character so at here *;* separator can be skiped.

***String value*** must be in qouation marks. The ```"``` or ```'``` can be used, but if there is one of these in the string, please use the other one. Wrapper will use *"* as default, except there is in the string, in this case *'* is selected.

***Float value*** must use ```.``` as floating point separator.



### Parser and Wrapper introduction
There are two separated parts: ***Parser*** and ***Wrapper***. 
Parser containes the basic features. Sometimes we don't need any special solution. 
For example we want to send some command and values to a slave device, and we don't want to read back data from slave in this format,
the parser is absolutely enough. On the other hand, if the slave mcu want to response in this protocol, we should use the wrapper feauters as well.
The interface for enabling wrapper usage and configuring the size type is the "mcu_msg_cfg.h" file. 


## Usage of Parser

First of all, you should get a message from the buffer. If the message is found, the function returns with message type, which containes the string type id and content. If it's not found, the result is a message object with destroyd strings, that means lenght is 0 and *s* pointer is NULL.
If you have a message, you can get objects from it with the same usage. After you can get the key values from the object. get-str function will return a string type, (destroyed if not found), int and float with number of digits (0 if not found)
Message handler can use the printer features. In the constructor you can set a basic putchar function to print to the UART or stdout. With the handler, you can redirect this std print output to a string buffer (it's useful in wrapper). For this you should enable buffering and disable if you want to use again the std out putchar.

Example (message string is same as above):

```c
/*Declarations*/
msg_t msg;
msg_obj_t obj1;
msg_cmd_t cmd;

/*Creating handler*/
msg_hnd_t hnd = msg_hnd_create((int (*) (char))putchar); // it an be your putchar implementation

/*Get message*/
msg = msg_get(test_str1, "test_msg", sizeof(test_str1));

/*Get command*/
cmd = msg_parser_get_cmd(msg, "CMD1");

/*Get 'object1'*/
obj1 = msg_parser_get_obj(msg, "obj1");

/*Get key11 integer*/
res = msg_parser_get_int(&ival, obj1, "key11");

/*Print msg to std out (eg. UARTx)*/
hnd.print_msg(obj1.id);
```


## Usage of Wrapper
Wrapper is made for sending values in this format to other MCUs.
For the wrapper there are some new defined type. The reason is the gluing objects and cmds to message and key-value pairs to object. Linked list solution is used to solve this problem. In the message type, there are an object queue and a command queue. In the object wrapper there are integer, float and string queues. In a print procedure, the handler will print first the commands, and after the objects, this order is fixed.
If you want to create a message wrapper, you should attache key-val pairs to the objects, add objects and commands to the message.
The configuration of this object model takes a little bit more time, but after you can change the values in the existing model, and send immediately to the output whitout any modification. There are create, destroy, add and remove interface for defined types.

Example (Using string buffer):
```c
/*Declarations*/
char buff[1000];
msg_hnd_t hnd;
msg_wrap_t msg_out;
msg_wrap_cmd_t cmd;

/*Create handler*/
hnd = msg_hnd_create((int (*) (char))putchar); // it an be your putchar implementation

/*Init string buffer*/
hnd.init_str_buff(buff, 1000);

/*Init message wrappeper*/
msg_out = msg_wrapper_create_msg("MASTER_MSG");

/*Init command*/
cmd = msg_wrapper_create_cmd("Get_Temp");

/*Add command to the message*/
msg_wrapper_add_cmd_to_msg(&msg_out, &cmd);

/*Enable string buffer and send message*/
hnd.enable_buff();
hnd.print_wrapper_msg(msg_out);
hnd.disable_buff();

/*Sendinf buffer via UART*/
UART_Send_Buff(buff, 1000); // only example
```
Example (Immediately send):
```c
msg_hnd_t hnd;
msg_wrap_obj_t temp_obj;
msg_wrap_t msg_out;
msg_wrap_float_t T1;
msg_wrap_float_t T2;

/*Create handler*/
hnd = msg_hnd_create((int (*) (char))putchar); // it an be your putchar implementation

/*Init message wrappeper*/
msg_out = msg_wrapper_create_msg("SLAVE_MSG");

/*Init object*/
temp_obj = msg_wrapper_create_obj("Temp");

/*Init Temperatures*/
T1 = msg_wrapper_create_float("T1", 32.45, 2);
T2 = msg_wrapper_create_float("T2", 29.34, 2);

/*Add T1 and T2 to object*/
msg_wrapper_add_float_to_obj(&temp_obj, &T1);
msg_wrapper_add_float_to_obj(&temp_obj, &T2);

/*Add object to message*/
msg_wrapper_add_obj_to_msg(&msg_out, &temp_obj);

/*Print message to the output*/
hnd.print_wrapper_msg(msg_out);
 
```
