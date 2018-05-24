#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "poya.h"
#include "wiringSerial.h"
int main(int argc, char *argv[])
{
  open_serial_device();
  std::vector<float> rect;
  rect.push_back(1);
  rect.push_back(0.2);
  rect.push_back(0.2);
  rect.push_back(0.6);
  rect.push_back(0.6);
  
  while(1)
  {
   //send_faces_to_poya(rect);

    uart_rt_poya();
 
    //break;
  }

  //close_serial_device();
}
