#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int fd;
    char write_buf[100];
    char read_buf[100];
    int opt;

    fd = open("/dev/mychardev", O_RDWR);
    if(fd<0){
      perror("File not opened!!\n");
    }
    
    while(1){
      printf("Choose Operation\n");
      printf("1.Write\n2.Read\n3.Exit\n");
      scanf("%d",&opt);      
      
      switch(opt){
      
      case 1:      
      printf("Enter the string you want to write!\n");
      getchar();   // <-- simple fix
      fgets(write_buf, sizeof(write_buf), stdin);
      write(fd, write_buf, strlen(write_buf));  
      printf("Wrote to device: %s\n", write_buf);
      break;
      
      case 2:
      read(fd, read_buf, sizeof(read_buf));
      printf("Read from device: %s\n", read_buf);
      break;
      
      case 3:
      exit(0);
      break;
      }
    }
    close(fd);
    return 0;
}
