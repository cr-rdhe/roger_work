#ifndef POYA_H
#define POYA_H

#include <vector>
#include "wiringSerial.h"  



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

void send_faces_to_poya(std::vector<float> rect, void *ts);

void recv_face_test();

void send_buffer(char *buff, size_t size);
void recv_buffer(unsigned char *buff, size_t size);

static int ReadTimeStamp(void *addr, uint64_t *timestamp);
static int FillTimeStamp(void *addr, uint64_t *timestamp);

#endif 
