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

#include "infinova.h"

/*
 * size = ts + count +( rect +quality + id + quality_detail ) * N ;
 *
 * **********************************************************************************
 */
#define BYTEARRSIZE(N)   sizeof(uint64_t) + sizeof(int) + ( sizeof(int)*4 + sizeof(float) + sizeof(int) + sizeof(float)*5 ) * N

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

void init_struct(inf_detect_result_t &result, tagCMD_HEADR &headr, int dataLength,
                 std::vector<float> rect,void *ts)
{
  headr.m_StartCode[0] = 'I';
  headr.m_StartCode[1] = 'N';
  headr.m_StartCode[2] = 'F';
  headr.m_CmdType      = CMD_ACK_RESULT;
  headr.m_DataLength   = dataLength;
  //ts
  ReadTimeStamp(ts, &result.ts);

  //count
  result.count = rect.size()/5;

  //tracker
  result.tracker = (inf_tracker_t *)malloc(sizeof(inf_tracker_t)*result.count);
  for(int i = 0; i < rect.size()/5; i++)
  {
    //rect
    result.tracker[i].rect.left   = (int)rect[i * 5 + 1] * 1920 ;
    result.tracker[i].rect.top    = (int)rect[i * 5 + 2] * 1080 ;
    result.tracker[i].rect.right  = (int)rect[i * 5 + 3] * 1920 ;
    result.tracker[i].rect.bottom = (int)rect[i * 5 + 4] * 1080 ;

    //quality
    result.tracker[i].quality = 0.9;

    //id
    result.tracker[i].id = i;

    //quality_detail
    result.tracker[i].quality_detail.confidence = 0.9;
    result.tracker[i].quality_detail.yaw        = 33 ;
    result.tracker[i].quality_detail.pitch      = 33 ;
    result.tracker[i].quality_detail.roll       = 33 ;
    result.tracker[i].quality_detail.clarity    = 99 ;
  }
}

void send_result_to_infinova(std::vector<float> rect, void *ts)
{
  int add_offset = 0;
  inf_detect_result_t result;
  tagCMD_HEADR headr;
  int ByteArrSize = BYTEARRSIZE(rect.size()/5) + sizeof(uint64_t);
  unsigned char ByteArrary[ByteArrSize];

  init_struct(result, headr, BYTEARRSIZE(rect.size()/5), rect, ts);

  //start
  ByteArrary[0] = headr.m_StartCode[0];
  ByteArrary[1] = headr.m_StartCode[1];
  ByteArrary[2] = headr.m_StartCode[2];
  ByteArrary[3] = headr.m_CmdType;
  int2byte(headr.m_DataLength, ByteArrary + 4);
  add_offset += sizeof(uint64_t);

  //ts
  uint642byte(result.ts, ByteArrary + add_offset);

  add_offset += sizeof(uint64_t);
  //count
  int2byte(result.count, ByteArrary + add_offset);

  add_offset += sizeof(int);

  //tracker
  for(int i = 0; i < rect.size()/5; i++)
  {
    //rect
    int2byte(result.tracker[i].rect.left  , ByteArrary  + add_offset                 );
    int2byte(result.tracker[i].rect.top   , ByteArrary  + add_offset + sizeof(int)*1 );
    int2byte(result.tracker[i].rect.right , ByteArrary  + add_offset + sizeof(int)*2 );
    int2byte(result.tracker[i].rect.bottom, ByteArrary  + add_offset + sizeof(int)*3 );
    add_offset += sizeof(int) * 4;

    //quality
    float2byte(result.tracker[i].quality, ByteArrary + add_offset);
    add_offset += sizeof(float);

    //id
    int2byte(result.tracker[i].id, ByteArrary + add_offset);
    add_offset += sizeof(int);

    //quality_detail
    float2byte(result.tracker[i].quality_detail.confidence, ByteArrary + add_offset );
    float2byte(result.tracker[i].quality_detail.yaw       , ByteArrary + add_offset + sizeof(float)*1);
    float2byte(result.tracker[i].quality_detail.pitch     , ByteArrary + add_offset + sizeof(float)*2);
    float2byte(result.tracker[i].quality_detail.roll      , ByteArrary + add_offset + sizeof(float)*3);
    float2byte(result.tracker[i].quality_detail.clarity   , ByteArrary + add_offset + sizeof(float)*4);
    add_offset += sizeof(float)*5;
  }

  send_buffer(ByteArrary, ByteArrSize);
  free(result.tracker);

  for(int i = 0; i < ByteArrSize; i++)
  printf("%2x ",ByteArrary[i]);
  printf("\n");
}

void recv_result_to_infinova(inf_detect_result_t **presult, tagCMD_HEADR **pheadr)
{

  unsigned char ByteArrary_H[sizeof(uint64_t)];
  tagCMD_HEADR *headr = (tagCMD_HEADR *)malloc(sizeof(tagCMD_HEADR));
  recv_buffer(ByteArrary_H, sizeof(uint64_t));
  headr->m_StartCode[0] = ByteArrary_H[0];
  headr->m_StartCode[1] = ByteArrary_H[1];
  headr->m_StartCode[2] = ByteArrary_H[2];
  headr->m_CmdType = ByteArrary_H[3];
  headr->m_DataLength = byte2int(ByteArrary_H+4);

  printf("Headr : %c %c %c %d %d \n", headr->m_StartCode[0], headr->m_StartCode[1], headr->m_StartCode[3], headr->m_CmdType, headr->m_DataLength);

  unsigned char ByteArrary_D[headr->m_DataLength];
  recv_buffer(ByteArrary_D, headr->m_DataLength);
  inf_detect_result_t *result = (inf_detect_result_t *)malloc(sizeof(inf_detect_result_t));
  int add_offset = 0;
  //ts
  result->ts = byte2uint64(ByteArrary_D);

  add_offset += sizeof(uint64_t);
  //count
  result->count = byte2int(ByteArrary_D + add_offset);

  add_offset += sizeof(int);

  //tracker
  result->tracker = (inf_tracker_t *)malloc(sizeof(inf_tracker_t)*result->count);
  for(int i = 0; i < result->count; i++)
  {
    //rect
    result->tracker[i].rect.left   = byte2int( ByteArrary_D  + add_offset                 );
    result->tracker[i].rect.top    = byte2int( ByteArrary_D  + add_offset + sizeof(int)*1 );
    result->tracker[i].rect.right  = byte2int( ByteArrary_D  + add_offset + sizeof(int)*2 );
    result->tracker[i].rect.bottom = byte2int( ByteArrary_D  + add_offset + sizeof(int)*3 );
    add_offset += sizeof(int) * 4;

    //quality
    result->tracker[i].quality = byte2float( ByteArrary_D + add_offset);
    add_offset += sizeof(float);

    //id
    result->tracker[i].id = byte2int( ByteArrary_D + add_offset);
    add_offset += sizeof(int);

    //quality_detail
    result->tracker[i].quality_detail.confidence = byte2float( ByteArrary_D + add_offset );
    result->tracker[i].quality_detail.yaw        = byte2float( ByteArrary_D + add_offset + sizeof(float)*1);
    result->tracker[i].quality_detail.pitch      = byte2float( ByteArrary_D + add_offset + sizeof(float)*2);
    result->tracker[i].quality_detail.roll       = byte2float( ByteArrary_D + add_offset + sizeof(float)*3);
    result->tracker[i].quality_detail.clarity    = byte2float( ByteArrary_D + add_offset + sizeof(float)*4);
    add_offset += sizeof(float)*5;
  }

  *presult = result;
  *pheadr = headr;
}

void send_buffer(unsigned char *buff, size_t size)
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
