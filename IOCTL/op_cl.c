#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include"myhd.h"

int main(){
  int fd = open("/dev/Atharva",O_RDWR);
  int st;
  int value = 100;

  // when we call this, the system switches from user mode > kernel mode 
  // then it will find for the ioctl structure
  ioctl(fd,ON);
  ioctl(fd,OFF);
  ioctl(fd,STATUS,&st);
  printf("Status = %d\n",st);
  ioctl(fd,SET_VALUE,&value);
  
  close(fd);
  return 0;
}
