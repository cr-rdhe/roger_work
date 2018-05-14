/*
 * wiringSerial.h:
 *	Handle a serial port
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

extern int   serialOpen      (const char *device, const int baud) ;
extern void  serialClose     (const int fd) ;
extern void  serialFlush     (const int fd) ;
extern void  serialPutchar   (const int fd, const unsigned char c) ;
extern void  serialPuts      (const int fd, const unsigned char *s, size_t size) ;
extern void  serialPrintf    (const int fd, const char *message, ...) ;
extern int   serialDataAvail (const int fd) ;
extern int   serialGetchar   (const int fd) ;
extern int   serialGets      (const int fd, const unsigned char *s, size_t size);

/*
 * types2byte or byte2types functions:
 *
 * *************************************************************************
 */

extern void uint642byte(uint64_t f_num, unsigned char *byteArray);
extern uint64_t byte2uint64(unsigned char* byteArray);
extern void   float2byte  (float f_num, unsigned char *byteArray);
extern float  byte2float  (unsigned char* byteArray);
extern void   int2byte    (int f_num, unsigned char *byteArray);
extern int    byte2int    (unsigned char* byteArray);
extern void   short2byte  (short s_num, unsigned char *byteArray);
extern short  byte2short  (unsigned char* byteArray);
extern void   double2byte (double d_num, unsigned char *byteArray);
extern double byte2double (unsigned char* byteArray);
#ifdef __cplusplus
}
#endif
