#ifndef POYA_H
#define POYA_H

#include <vector>
#include "wiringSerial.h"  
#include "cJSON.h"

#define UART_HEAD 0xa5
#define UART_TAIL 0xa9
/*
 * serial open and close functions
 *
 ***************************************************************
 */

void open_serial_device();
void close_serial_device();

/*
 * infinova serial send and recv functions
 *
 * *************************************************************
 */

void send_faces_to_poya(std::vector<float> rect);

void recv_face_test();

void send_buffer(char *buff, size_t size);
void recv_buffer(unsigned char *buff, size_t size);

void uart_rt_poya();
void sendJsonObject(cJSON* object);
void createJsonObject(cJSON* object, int pcode, int idx, int rs, char* msg);
void send_echo_to_poya(cJSON* echo_s, cJSON* echo_n);
void send_info_to_poya();
void send_log_level_to_poya();
void board_restart();
void send_restart_succ_to_poya();
void set_time(cJSON* year, cJSON* month,cJSON*  day, cJSON* hour, cJSON* min, cJSON* sec, cJSON* mini_sec);
void send_set_time_succ_to_poya();
void send_face_para_to_poya();
void set_face_para(cJSON* minFW, cJSON* minFH, cJSON* ms, cJSON* dp);
void send_set_para_succ_to_poya();
void open_face_detect();
void send_oc_dec_succ_to_poya(int f_open);
void close_face_detect();




#endif 
