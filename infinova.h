#ifndef INFINOVA_H
#define INFINOVA_H

#include <vector>
#include "wiringSerial.h"     

#define CMD_BEGIN_INIT            1
#define CMD_ACK_INIT              2
#define CMD_MODEL_START           3
#define CMD_ACK_START             4
#define CMD_MODEL_STOP            5
#define CMD_ACK_STOP              6
#define CMD_MODEL_EXIT            7
#define CMD_ACK_EXIT              8
#define CMD_QUERY_STAT            9
#define CMD_ACK_STAT              10 
#define CMD_QUERY_RESULT          11
#define CMD_ACK_RESULT            12
#define CMD_REQ_UPDATE            13
#define CMD_ACK_UPDATE            14
#define CMD_START_TRANSMIT        15
#define CMD_ACK_TRANSMIT          16
#define CMD_CHECK_UPDATE          17
#define CMD_CONFIRM_UPDATE        18
#define CMD_SET_FACE_AREA         19
#define CMD_ACK_FACE_AREA         20
#define CMD_SET_MIN_FACE_SIZE     21
#define CMD_ACK_MIN_FACE_SIZE     22
#define CMD_SET_MAX_FACE_SIZE     23
#define CMD_ACK_MAX_FACE_SIZE     24
#define CMD_SET_CONFIDENCE        25
#define CMD_ACK_CONFIDENCE        26
#define CMD_SET_MAX_FACE_NUM      27
#define CMD_ACK_MAX_FACE_NUM      28
#define CMD_SET_DROP_LEN          29
#define CMD_ACK_DROP_LEN          30

/*
 * infinova structs:
 *
 ********************************************************************
 */

typedef struct int_rect_t{
  int left;
  int top;
  int right;
  int bottom;
} inf_rect_t;

typedef struct inf_qualify{
  float confidence;
  float yaw;
  float pitch;
  float roll;
  float clarity;
} inf_quality_t;

typedef struct inf_tracker_t{
  inf_rect_t rect;
  float quality;
  int id;
  inf_quality_t quality_detail;
} inf_tracker_t;

typedef struct inf_detect_result_t{
  uint64_t ts;
  int count;
  inf_tracker_t *tracker;
} inf_detect_result_t;

typedef struct{
  unsigned char m_StartCode[3];
  unsigned char m_CmdType;
  int m_DataLength;
}tagCMD_HEADR;

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

void init_struct(inf_detect_result_t &result, tagCMD_HEADR &headr, int dataLength,
                 std::vector<float> rect,void *ts);
void send_result_to_infinova(std::vector<float> rect,
                             void *ts);
void recv_result_to_infinova(inf_detect_result_t **presult, tagCMD_HEADR **pheadr);

void send_buffer(unsigned char *buff, size_t size);
void recv_buffer(unsigned char *buff, size_t size);

static int ReadTimeStamp(void *addr, uint64_t *timestamp);
static int FillTimeStamp(void *addr, uint64_t *timestamp);

#endif

