#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "poya.h"
#include "wiringSerial.h"
int main(int argc, char *argv[])
{
  open_serial_device();
  while(1)
  {
    std::vector<float> rect;
    rect.push_back(1);
    rect.push_back(0.2);
    rect.push_back(0.2);
    rect.push_back(0.6);
    rect.push_back(0.6);
    unsigned char x[16] = {
              0x50,0x50,
              0x50,0x50,
              0x50,0x50,
              0x50,0x50,
              0x53,0x58,
              0x55,0x52,
              0x5b,0x5a,
              0x50,0x55, };
  
    if(0)
      send_faces_to_poya(rect, x);
    else{
      recv_face_test();
    }
   // break;
  }

  close_serial_device();
}
