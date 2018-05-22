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
    cJSON_AddStringToObject(face_obj, "pname", "FDR");
    cJSON_AddNumberToObject(face_obj, "idx", 2);
    //uint64_t cur_time;
    //ReadTimeStamp(ts, &cur_time);
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    char* time_bytes = (char*)malloc(30);
    //uint642byte(cur_time, time_bytes_ptr);
    sprintf(time_bytes, "%d%d", ts.tv_sec, ts.tv_nsec);
    printf("time is ---%d---%d---",ts.tv_sec, ts.tv_nsec);
    //time_bytes[sizeof(uint64_t)] = '\0';
    printf("string:---%s---\n", time_bytes);
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

    fprintf(stderr, "len is %d--- \n", uart_len);
    //printf("%s\n",uart_out);
    cJSON_Delete(face_obj);	
    //free(fis);	
    free(uart_out);
}

void recv_face_test()
{
    char *uart_buffer = (char*)malloc(1000);
    int len = 0;
    while(1){
        char get_byte = (char)serialGetchar(serial_fd);
        while(get_byte != 0xa5){
            get_byte = (char)serialGetchar(serial_fd);
        }
        get_byte = (char)serialGetchar(serial_fd);
        while(get_byte != 0xa9){
            *(uart_buffer + len) = get_byte;
            get_byte = (char)serialGetchar(serial_fd);
            len += 1;
        }
	*(uart_buffer + len) = '\0';
        cJSON* jason_obj = cJSON_Parse(uart_buffer);
	cJSON *fis_obj = cJSON_GetObjectItem(jason_obj, "fis");
        	
	char* uart_jason = cJSON_Print(fis_obj);
	printf("%s\n",uart_jason);
			
	len = 0;
    }

}




void send_buffer(char *buff, size_t size)
{
   serialPuts(serial_fd, buff, size);
}

void recv_buffer(unsigned char *buff, size_t size)
{
   serialGets(serial_fd, buff, size);
}

static int ReadTimeStamp(void *addr, uint64_t *timestamp)
{
  char *addrp = (char *)addr;
  char *datap = (char *)timestamp;
  for(int i = 15; i >= 0; i--)
  {
    if(i%2)
    {
      datap[(15-i)/2] |= (addrp[i] & 0x0f);
    }
    else
    {
      datap[(15-i)/2] |= ((addrp[i] & 0x0f )<< 4);
    }
  }
  return 0;
}

static int FillTimeStamp(void *addr, uint64_t *timestamp)
{
  char *addrp = (char *)addr;
  for(int i = 15; i >= 0; i--)
  {
    addrp[15-i] = 0x50 | (uint8_t)(((*timestamp) & (uint64_t)0xf << (i*4)) >> (i*4));
  }
  return 0;
}
