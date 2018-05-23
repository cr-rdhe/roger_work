#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "poya.h"
#include "wiringSerial.h"
int main(int argc, char *argv[])
{
  //open_serial_device();
#if 1
  while(1)
#endif
  {
    std::vector<float> rect;
    rect.push_back(1);
    rect.push_back(0.2);
    rect.push_back(0.2);
    rect.push_back(0.6);
    rect.push_back(0.6);
    std::vector<unsigned int> x;
    x.push_back(1);
    x.push_back(2);
    x.push_back(3);
    x.push_back(4);
    if(1)
      send_faces_to_poya(rect);
    else
      recv_face_test();

    uart_rt_poya();
 
    break;
  }

  //close_serial_device();
}
