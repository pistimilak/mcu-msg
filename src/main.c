/**
 * @file main.c
 * @author Istvan Milak (istvan.milak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-12-15
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "mcu_msg.h"

/*string printer on i386*/

char test_str1[] = "#test_msg{<CMD1>\t\t @obj1($key11=-1123334567  ; $key12 = 'string \"value\"')\n\
\r@obj2  ($key21 =   -1.123456789; $key22   = 'val22'; $key23 = 1000; $key24 = 12.34)<CMD_last>}";


typedef struct {
    char *buff;
    msg_size_t buff_size;
} thread_arg;



pthread_mutex_t outp_lock;
pthread_t thr_master, thr_slave;
thread_arg common_buff;

void *thread_mcu_master_fnc(void *arg);
void *thread_mcu_slave_fnc(void *arg);



int main()
{
    /*start the clock*/
    clock_t begin = clock();
    clock_t end;
    double exec_time = 0.0;
    

///////////////////////////////////////////////////////////////////////////////////////////////
//                                 MCU-MSG Parser test                                       //
///////////////////////////////////////////////////////////////////////////////////////////////

    msg_t msg;
    msg_obj_t obj1, obj2;
    msg_cmd_t cmd;
    msg_hnd_t hnd = msg_hnd_create((int (*) (char))putchar);
    
    printf("\n");
    printf("TEST mcu-msg-parser\n");
    printf("===================\n\n\n");

    printf("test_str1 = \"%s\"\n\n", test_str1);

   
    printf(">> getting test_msg...\n");
    msg = msg_get(test_str1, "test_msg", sizeof(test_str1));
    if(msg_get_content(msg) != NULL) {
        printf("msg.id_len: %d msg.content_len: %d\n", msg.id.len, msg.content.len);
        hnd.print_str(msg.id);
        printf(":");
        hnd.print_str(msg.content);
        printf("\n\n");
    } else {
        printf("message not found!\n\n");
    }

    
    printf(">> getting CMD1 cmd...\n");
    cmd = msg_parser_get_cmd(msg, "CMD1");
    printf("%s\n\n", msg_get_cmd_content(cmd) ? "True" : "False");

    printf(">> getting CMD2 cmd...\n");
    cmd = msg_parser_get_cmd(msg, "CMD2");
    printf("%s\n\n", msg_get_cmd_content(cmd) ? "True" : "False");

    printf(">> getting CMD_last cmd...\n");
    cmd = msg_parser_get_cmd(msg, "CMD_last");
    printf("%s\n\n", msg_get_cmd_content(cmd) ? "True" : "False");

    
    printf(">> getting obj1...\n");
    obj1 = msg_parser_get_obj(msg, "obj1");
    printf("obj1.id_len: %d obj1.content_len: %d\n", obj1.id.len, obj1.content.len);
    hnd.print_str(obj1.id); printf(":"); hnd.print_str(obj1.content);
    printf("\n\n");

    
    printf(">> getting obj2...\n");
    obj2 = msg_parser_get_obj(msg, "obj2");
    printf("obj2.id_len: %d obj2.content_len: %d\n", obj2.id.len, obj2.content.len);
    hnd.print_str(obj2.id); printf(":"); hnd.print_str(obj2.content);
    printf("\n\n");
    
    
    printf(">> getting obj1->key11 integer...\n");
    int ival = 0, res;
    float fval = 0.0;
    res = msg_parser_get_int(&ival, obj1, "key11");
    printf("r = %d ival = %d\n\n", res, ival);
    
    
    printf(">> getting obj2->key21 float...\n");
    res = msg_parser_get_float(&fval, obj2, "key21");
    printf("r = %d fval = %.11f\n\n", res, fval);

    
    printf(">> getting obj1->key12 string...\n");
    msg_str_t str = msg_parser_get_str(obj1, "key12");
    if(msg_str_p(str) != NULL) {
        hnd.print_str(str); printf(" len: %d\n\n", str.len);
    } else {
        printf("error getting string\n\n");
    }
    
    


///////////////////////////////////////////////////////////////////////////////////////////////
//                                 MCU-MSG Wrapper test                                      //
///////////////////////////////////////////////////////////////////////////////////////////////
#if MCU_MSG_USE_WRAPPER  
    printf("\n");
    printf("TEST mcu-msg-wrapper\n");
    printf("====================\n\n\n");

    // msg_wrap_hnd_t wrapper_hnd = msg_wrapper_hnd_create(putchar);
    msg_wrap_t msg_wrap;
    msg_wrap_obj_t obj1_wrap, obj2_wrap;
    msg_wrap_str_t str1, str2, str3;
    msg_wrap_int_t i1, i2;
    msg_wrap_float_t f1, f2;
    msg_wrap_cmd_t cmd_wrap, cmd_rem;
    msg_t msg_reparsed;
    msg_obj_t obj_reparsed;
    float f_val; 
    int i_val;
    
    msg_wrap = msg_wrapper_create_msg("wrapped_msg");
    printf("Initialized msg: #"); hnd.print_str(msg_wrap.id); printf("\n\n");
    
    obj1_wrap = msg_wrapper_create_obj("wrapped_obj1");
    printf("Initialized obj: @"); hnd.print_str(obj1_wrap.id); printf("\n\n");


    str1 = msg_wrapper_create_str("str1", "This is \"string 1\"");
    printf("Initialized str: $"); hnd.print_str(str1.id); putchar('='); hnd.print_str(str1.content); printf("\n\n");
    
    str2 = msg_wrapper_create_str("str2", "This is 'string 2'");
    printf("Initialized str: $"); hnd.print_str(str2.id); putchar('='); hnd.print_str(str2.content); printf("\n\n");

    str3 = msg_wrapper_create_str("str3", ".... \"string 3\"");
    printf("Initialized str: $"); hnd.print_str(str3.id); putchar('='); hnd.print_str(str3.content); printf("\n\n");

    /* Preapare object */
    printf("Add '$str2', '$str3' to '@wrapped_obj1'...\n\n");
    msg_wrapper_add_str_to_obj(&obj1_wrap, &str2);
    msg_wrapper_add_str_to_obj(&obj1_wrap, &str3);

    printf("Add '@wrapped_obj1' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_obj_to_msg(&msg_wrap, &obj1_wrap);

    cmd_wrap = msg_wrapper_create_cmd("CMD_WRAP");
    printf("Initialized cmd: <"); hnd.print_str(cmd_wrap.cmd); printf(">\n\n");
    
    cmd_rem = msg_wrapper_create_cmd("CMD_REMOVEABLE");
    printf("Initialized cmd: <"); hnd.print_str(cmd_rem.cmd); printf(">\n\n");

    i1 = msg_wrapper_create_int("i1", -3244);
    printf("Initialized int: $"); hnd.print_str(i1.id); printf("=%d\n\n", i1.val);

    i2 = msg_wrapper_create_int("i2", 456789);
    printf("Initialized int: $"); hnd.print_str(i2.id); printf("=%d\n\n", i2.val);
    
    f1 = msg_wrapper_create_float("f1", 1.23456, 6);
    printf("Initialized float: $"); hnd.print_str(f1.id); printf("=%.6f\n\n", f1.val);

    f2 = msg_wrapper_create_float("f2", -0.3345, 6);
    printf("Initialized float: $"); hnd.print_str(f2.id); printf("=%.6f\n\n", f2.val);

    obj2_wrap = msg_wrapper_create_obj("wrapped_obj2");
    printf("Initialized obj: @"); hnd.print_str(obj2_wrap.id); printf("\n\n");

    printf("Add '$i1', '$i2', '$f1', '$f2' and '$str1' to '@wrapped_obj2'\n\n");
    msg_wrapper_add_int_to_obj(&obj2_wrap, &i1);
    msg_wrapper_add_int_to_obj(&obj2_wrap, &i2);
    msg_wrapper_add_float_to_obj(&obj2_wrap, &f1);
    msg_wrapper_add_float_to_obj(&obj2_wrap, &f2);
    msg_wrapper_add_str_to_obj(&obj2_wrap, &str1);

    printf("Add '<CMD_WRAP>' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_cmd_to_msg(&msg_wrap, &cmd_wrap);

    printf("Add '@wrapped_obj2' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_obj_to_msg(&msg_wrap, &obj2_wrap);
    
    printf("Add '<CMD_REMOVABLE>' to '#wrapped_msg'...\n\n");
    msg_wrapper_add_cmd_to_msg(&msg_wrap, &cmd_rem);

    printf("Removing $str2 from @wrapped_obj1...\n\n");
    msg_wrapper_rm_string_from_obj(&obj1_wrap, &str2);
    
    printf("Removing $str1 from @wrapped_obj2...\n\n");
    msg_wrapper_rm_string_from_obj(&obj2_wrap, &str1);

    printf("Removing $f1 from @wrapped_obj2...\n\n");
    msg_wrapper_rm_float_from_obj(&obj2_wrap, &f1);

    printf("Removing $i2 from @wrapped_obj2...\n\n");
    msg_wrapper_rm_int_from_obj(&obj2_wrap, &i2);

    printf("Removing @wrapped_obj1 from #wrapped_msg...\n\n");
    msg_wrapper_rm_obj_from_msg(&msg_wrap, &obj1_wrap);

    printf("Removing <CMD_REMOVABLE> from #wrapped_msg...\n\n");
    msg_wrapper_rm_cmd_from_msg(&msg_wrap, &cmd_rem);

    printf("Add '@wrapped_obj1' AGAIN to '#wrapped_msg'...\n\n");
    msg_wrapper_add_obj_to_msg(&msg_wrap, &obj1_wrap);

    printf("Wrapped message:\n");
    printf("---------------\n\n");
    hnd.print_wrapper_msg(msg_wrap);


    printf("\n\n");

    char buff[1000] = {0};
    hnd.init_str_buff(buff, 1000);
    hnd.enable_buff();
    hnd.print_wrapper_msg(msg_wrap);
    printf("Wrapped message buffered\n");
    hnd.print_wrapper_msg(msg_wrap);
    printf("Wrapped message (buffered again)\n");
    hnd.disable_buff();

    printf("\n\n");
    printf("Buffer content:\n");
    printf("%s\n\n", buff);

    printf("Reparsing '#wrapped_msg' '@wrapped_obj2'...\n\n");
    msg_reparsed = msg_get(buff, "wrapped_msg", 1000);
    hnd.print_msg(msg_reparsed); printf("\n\n");
    obj_reparsed = msg_parser_get_obj(msg_reparsed, "wrapped_obj2");
    hnd.print_str(obj_reparsed.content); printf(" len: %d\n\n", obj_reparsed.content.len);


    msg_parser_get_float(&f_val, obj_reparsed, "f2");
    msg_parser_get_int(&i_val, obj_reparsed, "i1");
    printf("reparsed $i = %d $f2 = %f\n\n", i_val, f_val);
    printf("\n\n");


    /*Emulating master slave communication*/
    
    printf("Emulating a master - slave communaication:\n");
    printf("-----------------------------------------\n\n");
    memset(buff, 0, 1000);
    common_buff.buff = buff;
    common_buff.buff_size = 1000;

    if(pthread_mutex_init(&outp_lock, NULL)) return 0;
    
    pthread_create(&thr_master, NULL, thread_mcu_master_fnc, (void *) &common_buff);
    pthread_create(&thr_slave, NULL, thread_mcu_slave_fnc, (void *) &common_buff);


    pthread_join(thr_master, NULL);
    pthread_join(thr_slave, NULL);
    pthread_mutex_destroy(&outp_lock);
#endif

    printf("\n\n");

    end = clock();
    exec_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Execution time: %f s\n", exec_time);
    return 0;
}



void *thread_mcu_master_fnc(void *arg)
{
        thread_arg *buff = (thread_arg *) arg;
        msg_hnd_t hnd;
        msg_t msg_in;
        msg_obj_t temp_obj;
        msg_wrap_t msg_out;
        msg_wrap_cmd_t cmd;

        float T1, T2;

        /*Create handler*/
        hnd = msg_hnd_create((int (*) (char))putchar);

        /*init common string buffer*/
        hnd.init_str_buff(buff->buff, buff->buff_size);
        
        /*Init message wrappeper*/
        msg_out = msg_wrapper_create_msg("MASTER_MSG");
        
        /*Init command*/
        cmd = msg_wrapper_create_cmd("Get_Temp");

        /*Add command to the message*/
        msg_wrapper_add_cmd_to_msg(&msg_out, &cmd);

        /*Enable string buffer and send message*/
        pthread_mutex_lock(&outp_lock); // LOCK
        
        hnd.enable_buff();
        hnd.print_wrapper_msg(msg_out);
        hnd.disable_buff();
        

        /*Print message to stdout*/
        printf("Master >> ");
        hnd.print_wrapper_msg(msg_out);
        printf("\n");

        pthread_mutex_unlock(&outp_lock); // UNLOCK

        /*Polling the common buffer*/
        while(1) {
            
            msg_in = msg_get(buff->buff, "SLAVE_MSG", buff->buff_size);
            if(msg_get_content(msg_in) != NULL) { // msg is arrived
                
                pthread_mutex_lock(&outp_lock); // LOCK
                
                temp_obj = msg_parser_get_obj(msg_in, "Temp");

                if(msg_get_content(temp_obj) != NULL) {
                    
                    if(msg_parser_get_float(&T1, temp_obj, "T1")) {
                    printf("Master >> T1 = %f (from Slave)\n", T1);
                    }
                    if(msg_parser_get_float(&T2, temp_obj, "T2")) {
                        printf("Master >> T2 = %f (from Slave)\n", T2);
                    }
                    
                }
                
                pthread_mutex_unlock(&outp_lock); // UNLOCK

                break;
            }
            usleep(1000);
        }

        return 0;
}



void *thread_mcu_slave_fnc(void *arg)
{
        thread_arg *buff = (thread_arg *) arg;
        msg_hnd_t hnd;
        msg_t msg_in;
        msg_cmd_t cmd;
        msg_wrap_obj_t temp_obj;
        msg_wrap_t msg_out;
        msg_wrap_float_t T1;
        msg_wrap_float_t T2;

        /*Create handler*/
        hnd = msg_hnd_create((int(*)(char))putchar);

        /*init common string buffer*/
        hnd.init_str_buff(buff->buff, buff->buff_size);

        /*Init message wrappeper*/
        msg_out = msg_wrapper_create_msg("SLAVE_MSG");

        /*Init object*/
        temp_obj = msg_wrapper_create_obj("Temp");

        /*Init Temperatures*/
        T1 = msg_wrapper_create_float("T1", 32.45, 2);
        T2 = msg_wrapper_create_float("T2", 29.34, 2);

        msg_wrapper_add_float_to_obj(&temp_obj, &T1);
        msg_wrapper_add_float_to_obj(&temp_obj, &T2);
        msg_wrapper_add_obj_to_msg(&msg_out, &temp_obj);

        /*Polling the commong buffer*/
        while(1) {
            
            
            
            msg_in = msg_get(buff->buff, "MASTER_MSG", buff->buff_size);
            

            if(msg_get_content(msg_in) != NULL) { //message arrived

                

                cmd = msg_parser_get_cmd(msg_in, "Get_Temp");
                if(msg_get_cmd_content(cmd) != NULL) { //command arrived

                    pthread_mutex_lock(&outp_lock); // LOCK

                    /*Send to the master*/
                    hnd.enable_buff();
                    hnd.print_wrapper_msg(msg_out);
                    hnd.disable_buff();

                    /*Print to stdout*/
                    printf("Slave: >> ");
                    hnd.print_wrapper_msg(msg_out);
                    printf("\n");
                    
                    pthread_mutex_unlock(&outp_lock); // UNLOCK

                    break;
                }
            }
            usleep(1000);
        }

        return 0;

}