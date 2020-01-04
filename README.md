# mcu-msg

## Description
Simple parser and wrapper for string based communication between microcontrollers.
This library is designed for UART and other serial communications.
The library can handle undefined count of message with undefined count of objects
 
The library doesn't use string.h and internal static buffers, library operates with pointers and pointer arithmetic. 
The user can choose a suitable buffer size to store messages

### Message structure
Sample message:
```
#test_msg{
<CMD1> @obj1($key11=-1123334567; $key12='string "value"') 
@obj2($key21 = -1.123456789; $key22 = 'val22'; $key23 = 1000; $key24 = 12.34)<CMD_last>}
```

*
All of keywords can contain [a-zA-Z0-9_] characters look like variable name, but it can start with number.
All of whitespace can be used between keywords and values.
*

***Message*** is the main container. It must be start with ```#```, next is the ```message_id```. 
Message content is enclosed in ```{ }``` brackets.
A message can contain *objects* and *commands*.

***Command*** is a command string between ```< >```. It's a general type of message content. 
It can be used as a boolean value whitout any other meaning.





### Parser and Wrapper introduction
There are two separated parts: ***Parser*** and ***Wrapper***. 
Parser containes the basic features. Sometimes we don't need any special solution. 
For example we want to send some command and values to a slave device, and we don't want to read back data from slave in this format,
the parser is good enough. On the other hand, if the slave mcu want to response in this protocol, we should use the wrapper feauters as well.


## Usage of Parser

## Usage of Wrapper

