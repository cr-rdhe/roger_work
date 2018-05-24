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

#include "poya.h"
#include "cJSON.h"
#include "wiringSerial.h"

//serial device file descripter 
static int serial_fd = -1;


const unsigned char start_signal = 0xa9;

//open the serial device using the static file descriptor
void open_serial_device()
{
	serial_fd = serialOpen("/dev/ttyAMA0", 115200);
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


void send_faces_to_poya(std::vector<float> rect, void *ts)
{
    cJSON* face_obj;
    face_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(face_obj, "pcode", 8004);
    cJSON_AddStringToObject(face_obj, "pname", "FDR");
    cJSON_AddNumberToObject(face_obj, "idx", 2);
    uint64_t cur_time;
    ReadTimeStamp(ts, &cur_time);
    char* time_bytes = (char*)malloc(sizeof(uint64_t) + 1);
    //uint642byte(cur_time, time_bytes_ptr);
    sprintf(time_bytes, "%x", cur_time);
    time_bytes[sizeof(uint64_t)] = '\0';
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
    //serialPutchar(serial_fd, 0xa5);
    //send_buffer(uart_out, uart_len);
    //serialPutchar(serial_fd, 0xa9);

    printf("%s\n",uart_out);
    cJSON_Delete(face_obj);	
    //free(fis);	
    free(uart_out);
}

void recv_face_test()
{
 /* int count ,j,x;
  unsigned int nextTime ;
  for (count = 0 ; count < 32 ; )
  {
    //if (millis () > nextTime)
    {
      printf ("\nOut: %3d: ", count) ;
      fflush (stdout) ;
      serialPutchar (serial_fd, count) ;
      nextTime += 300 ;
      ++count ;
    }

    //delay (3) ;
    for (j= 1;j< 65530; j++) x = j*j;
	
    while (serialDataAvail (serial_fd) <= 0){}
    printf ("the available data num is  %d---", serialDataAvail(serial_fd)) ;
    fflush (stdout) ;
    if((count % 2) == 0){
      printf (" -> %3d", serialGetchar (serial_fd)) ;
      fflush (stdout) ;
    }
  }*/

    char *uart_buffer = (char*)malloc(2000);
    int len = 0;
    fprintf(stderr, "[RAINMAN] hahaha \n");
    while(serialDataAvail(serial_fd) <= 0){}
    while(serialDataAvail(serial_fd)){
	
        char get_byte = (char)serialGetchar(serial_fd);
        //fprintf(stderr, "get first byte %c \n", get_byte);
	while(get_byte != 0xa5){
            fprintf(stderr, "not get ox5a \n");
            while(serialDataAvail(serial_fd) <= 0){}
            get_byte = (char)serialGetchar(serial_fd);
	}
        fprintf(stderr, "find the header \n");
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
        //fprintf(stderr, "parse finish--- \n");
	char* u_jason = cJSON_Print(jason_obj);
	printf("%s\n",u_jason);
	/*cJSON *fis_obj = cJSON_GetObjectItem(jason_obj, "fis");
        	
	char* uart_jason = cJSON_Print(fis_obj);
	printf("%s\n",uart_jason);
        fflush (stdout) ;
	*/		
	len = 0;
    }

}

void send_uart_test(int pc){
    cJSON* test_obj;
    test_obj = cJSON_CreateObject();
    int pcode = pc;
    fprintf(stderr, "pcode is %d--- \n", pcode);
    cJSON_AddNumberToObject(test_obj, "pcode", pcode);
    cJSON_AddNumberToObject(test_obj, "idx", 2);
    switch(pcode){
        case 9000:{
            cJSON_AddStringToObject(test_obj, "echo_s", "haaaalo");
            cJSON_AddNumberToObject(test_obj, "echo_n", 999);
	    break;
	}
	case 9002:{	  
            cJSON_AddNumberToObject(test_obj, "loglevel", 2);
            break;
	} 
	case 9005:{	  
            cJSON_AddNumberToObject(test_obj, "year", 2);
            cJSON_AddNumberToObject(test_obj, "month", 3);
            cJSON_AddNumberToObject(test_obj, "day", 4);
            cJSON_AddNumberToObject(test_obj, "hour", 5);
            cJSON_AddNumberToObject(test_obj, "minute", 6);
            cJSON_AddNumberToObject(test_obj, "second", 7);
            cJSON_AddNumberToObject(test_obj, "millisecond", 888);
            break;
	}
	case 8001:{	  
            cJSON_AddNumberToObject(test_obj, "minfacewidth", 23);
            cJSON_AddNumberToObject(test_obj, "minfaceheight", 24);
            cJSON_AddNumberToObject(test_obj, "minscore", 25);
            cJSON_AddNumberToObject(test_obj, "detectpolicy", 0);
            break;
	} 
	
	default:
	    break;
    }

    char* uart_out;
    uart_out = cJSON_Print(test_obj);
    int uart_len = strlen(uart_out);
    serialPutchar(serial_fd, 0xa5);
    send_buffer(uart_out, uart_len);
    serialPutchar(serial_fd, 0xa9);
    //fprintf(stderr, "len is %d--- \n", uart_len);
    //printf("%s\n",uart_out);
    cJSON_Delete(test_obj);	
    free(uart_out);

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
