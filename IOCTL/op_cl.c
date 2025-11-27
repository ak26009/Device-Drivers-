#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include"myhd.h"

int main(){
  int fd = open("/dev/Atharva",O_RDWR);
  int st;
  int value = 100;
  
  ioctl(fd,ON);
  ioctl(fd,OFF);
  ioctl(fd,STATUS,&st);
  printf("Status = %d\n",st);
  ioctl(fd,SET_VALUE,&value);
  
  close(fd);
  return 0;
}
