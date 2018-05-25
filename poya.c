//g++ -std=c++0x -o main main.c poya.c cJSON.c wiringSerial.c -lm 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <time.h>

#include "poya.h"
#include "cJSON.h"
#include "wiringSerial.h"

//serial device file descripter 
static int serial_fd = -1;


const unsigned char start_signal = 0xa9;

//open the serial device using the static file descriptor
void open_serial_device()
{
	serial_fd = serialOpen("/dev/ttyPS1", 115200);
	if(serial_fd < 0){
		fprintf(stderr, "[RAINMAN] Error: Cannot open serial device: /dev/ttyAMA0 \n");
		exit(1);
	}
}
//close the serial device
void close_serial_device()
{
	if(serial_fd <= 0){
		fprintf(stderr, "[RAINMAN] Error, the serial device has not yet been opened!\n");
		exit(1);
	}
	serialClose(serial_fd);
}


void send_faces_to_poya(std::vector<float> rect)
{
    cJSON* face_obj;
    face_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(face_obj, "pcode", 8004);
    cJSON_AddNumberToObject(face_obj, "idx", 2);
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    char* time_bytes = (char*)malloc(30);
    sprintf(time_bytes, "%d%d", ts.tv_sec, ts.tv_nsec);
    //printf("time is ---%d---%d---",ts.tv_sec, ts.tv_nsec);
    //time_bytes[sizeof(uint64_t)] = '\0';
    //printf("string:---%s---\n", time_bytes);
    cJSON_AddStringToObject(face_obj, "pid", time_bytes);
    cJSON_AddNumberToObject(face_obj, "dt", 2);
    cJSON_AddNumberToObject(face_obj, "w", 1920);
    cJSON_AddNumberToObject(face_obj, "h", 1080);
    cJSON_AddNumberToObject(face_obj, "ct", 0);
    cJSON* fis = (cJSON* )cJSON_CreateArray();
    cJSON_AddItemToObject(face_obj, "fis", fis);
    int i = 0;
    float *float_ptr = (float* )malloc(sizeof(float) * 5);
    while(i < rect.size() / 5 ){
        float float_ptr[5];
        float_ptr[0]  = 0.9;
        float_ptr[1]  = rect[i * 5 + 1];
        float_ptr[2]  = rect[i * 5 + 2];
        float_ptr[3]  = rect[i * 5 + 3];
        float_ptr[4]  = rect[i * 5 + 4];
        cJSON* float_array = (cJSON* )cJSON_CreateFloatArray(float_ptr, 5);
	cJSON_AddItemToArray(fis, float_array);
        i++;
    }
    char* uart_out;
    uart_out = cJSON_Print(face_obj);
    int uart_len = strlen(uart_out);
    serialPutchar(serial_fd, 0xa5);
    send_buffer(uart_out, uart_len);
    serialPutchar(serial_fd, 0xa9);
    //fprintf(stderr, "len is %d--- \n", uart_len);
    //printf("%s\n",uart_out);
    cJSON_Delete(face_obj);	
    free(uart_out);
}

void send_buffer(char *buff, size_t size)
{
   serialPuts(serial_fd, buff, size);
}

void uart_rt_poya(){
    while(1){
        char *uart_buffer = (char*)malloc(500);
        int len = 0;
        
	char get_byte = (char)serialGetchar(serial_fd);
        fprintf(stderr, "get first byte %c \n", get_byte);
	while(get_byte != 0xa5){
            fprintf(stderr, "not get ox5a \n");
            while(serialDataAvail(serial_fd) <= 0){}
            get_byte = (char)serialGetchar(serial_fd);
	}
        //fprintf(stderr, "find the header \n");
        while(serialDataAvail(serial_fd) <= 0){}
        get_byte = (char)serialGetchar(serial_fd);
        while(get_byte != 0xa9){
            *(uart_buffer + len) = get_byte;
            while(serialDataAvail(serial_fd) <= 0){}
            get_byte = (char)serialGetchar(serial_fd);
            len += 1;
        }
	*(uart_buffer + len) = '\0';
	int test_len = strlen(uart_buffer);
        //fprintf(stderr, "testlen = %d---, len is %d--- \n",test_len,len);
        cJSON* jason_obj = cJSON_Parse(uart_buffer);
	cJSON *pc = cJSON_GetObjectItem(jason_obj, "pcode");	
        int pcode = pc->valueint;
	switch(pcode){
	    case 9000:{
	        //return echo
        	int pcode = pc->valueint;
		cJSON *es = cJSON_GetObjectItem(jason_obj, "echo_s");
		cJSON *ei = cJSON_GetObjectItem(jason_obj, "echo_n");
		send_echo_to_poya(es, ei);
		break;
	    }
	    case 9001:{
		//retrun board info
		send_info_to_poya(); 
		break;
	    }
	    case 9002:{
		//return log level
		send_log_level_to_poya();
		break;
	    }
	    case 9003:{
		//restart board
		send_restart_succ_to_poya();
 		//board_restart();
		break;
	    }
	    case 9005:{
		//set sys time
		cJSON *year = cJSON_GetObjectItem(jason_obj, "year");
		cJSON *month = cJSON_GetObjectItem(jason_obj, "month");
		cJSON *day = cJSON_GetObjectItem(jason_obj, "day");
		cJSON *hour = cJSON_GetObjectItem(jason_obj, "hour");
		cJSON *min = cJSON_GetObjectItem(jason_obj, "minute");
		cJSON *sec = cJSON_GetObjectItem(jason_obj, "second");
		cJSON *mini_sec = cJSON_GetObjectItem(jason_obj, "millisecond");
		set_time(year, month, day, hour, min, sec, mini_sec);
		send_set_time_succ_to_poya();
		break;
	    }
	    case 8000:{
		// get face para
		send_face_para_to_poya();
		break;
	   }
	    case 8001:{
		// set face fara
		cJSON *minFW = cJSON_GetObjectItem(jason_obj, "minfacewidth");
		cJSON *minFH = cJSON_GetObjectItem(jason_obj, "minfaceheight");
		cJSON *ms = cJSON_GetObjectItem(jason_obj, "minscore");
		cJSON *dp = cJSON_GetObjectItem(jason_obj, "detectpolicy");
		set_face_para(minFW, minFH, ms, dp);
		send_set_para_succ_to_poya();
		break;
	    }
	    case 8002:{
		//open face detect
		open_face_detect();
		send_oc_dec_succ_to_poya(1);
		break;
	    }
	    case 8003:{
		//close face detect
		close_face_detect();
		send_oc_dec_succ_to_poya(0);
		break;
	    }
	    case 8004:
		//face received echo
		break;
	    default:
		break;
	}	

	len = 0;
	free(uart_buffer);
        if(serialDataAvail(serial_fd) <= 0){
	    break;
	}
    }

}

void sendJsonObject(cJSON* object){
    char* uart_out;
    uart_out = cJSON_Print(object);
    int uart_len = strlen(uart_out);
    serialPutchar(serial_fd, UART_HEAD);
    send_buffer(uart_out, uart_len);
    serialPutchar(serial_fd, UART_TAIL);
    //fprintf(stderr, "len is %d--- \n", uart_len);
    //printf("%s\n",uart_out);
    //fflush(stdout);
    cJSON_Delete(object);	
    free(uart_out);
}

void createJsonObject(cJSON* object, int pcode, int idx, int rs, char* msg){
    cJSON_AddNumberToObject(object, "pcode", pcode);
    cJSON_AddNumberToObject(object, "idx", idx);
    cJSON_AddNumberToObject(object, "rs", rs);
    cJSON_AddStringToObject(object, "msg", msg);
}


void send_echo_to_poya(cJSON* echo_s, cJSON* echo_n){
    cJSON* echo_obj;
    char* msg = "success";
    echo_obj = cJSON_CreateObject();
    createJsonObject(echo_obj, 9000, 1, 0, msg);
    cJSON_AddStringToObject(echo_obj, "echo_n", echo_s->valuestring);
    cJSON_AddNumberToObject(echo_obj, "echo_s", echo_n->valueint);
    sendJsonObject(echo_obj);
}

void send_info_to_poya(){
    fprintf(stderr," in send info to poya \n");
    cJSON* info_obj;
    char* msg = "success";
    info_obj = cJSON_CreateObject();
    createJsonObject(info_obj, 9001, 2, 0, msg);
    // system, software, firmware, systime, cpuinfo, meminfo not define yet
    sendJsonObject(info_obj);
}

void send_log_level_to_poya(){
    cJSON* logl_obj;
    char* msg = "success";
    logl_obj = cJSON_CreateObject();
    createJsonObject(logl_obj, 9002, 2, 0, msg);
    sendJsonObject(logl_obj);
}

void send_restart_succ_to_poya(){
    cJSON* ress_obj;
    char* msg = "success";
    ress_obj = cJSON_CreateObject();
    createJsonObject(ress_obj, 9003, 2, 0, msg);
    sendJsonObject(ress_obj);
}

void set_time(cJSON* year, cJSON* month,cJSON*  day, cJSON* hour, cJSON* min, cJSON* sec, cJSON* mini_sec){

}

void send_set_time_succ_to_poya(){
    cJSON* sett_obj;
    char* msg = "success";
    sett_obj = cJSON_CreateObject();
    createJsonObject(sett_obj, 9005, 2, 0, msg);
    sendJsonObject(sett_obj);
}

void send_face_para_to_poya(){
    cJSON* facepara_obj;
    char* msg = "success";
    facepara_obj = cJSON_CreateObject();
    createJsonObject(facepara_obj, 8000, 2, 0, msg);
    cJSON_AddNumberToObject(facepara_obj, "minfacewidth", 45);
    cJSON_AddNumberToObject(facepara_obj, "minfaceheight", 45);
    cJSON_AddNumberToObject(facepara_obj, "minscore", 60);// float not support by cjson
    cJSON_AddNumberToObject(facepara_obj, "detectpolicy", 0);
    sendJsonObject(facepara_obj);
}

void set_face_para(cJSON* minFW, cJSON* minFH, cJSON* ms, cJSON* dp){

}

void send_set_para_succ_to_poya(){
    cJSON* setp_obj;
    char* msg = "success";
    setp_obj = cJSON_CreateObject();
    createJsonObject(setp_obj, 8001, 2, 0, msg);
    sendJsonObject(setp_obj);
}

void open_face_detect(){

}

void send_oc_dec_succ_to_poya(int f_open){
    cJSON* ocdec_obj;
    char* msg = "success";
    int pcode = f_open == 1? 8002: 8003;
    ocdec_obj = cJSON_CreateObject();
    createJsonObject(ocdec_obj, pcode, 2, 0, msg);
    sendJsonObject(ocdec_obj);

}

void close_face_detect(){

}










