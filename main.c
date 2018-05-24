#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "poya.h"
#include "wiringSerial.h"
int main(int argc, char *argv[])
{
  open_serial_device();
  printf ("input is--- %s---", argv[2]) ;
  fflush (stdout) ;
  int input = atoi(argv[2]); 
  while(1)
  { 
     //recv_face_test();
     send_uart_test(input);
     recv_face_test();
     break;
  }

  close_serial_device();
}
