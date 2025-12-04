#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
int main(){
    int choice,fd;
    char buf[1];
    printf("1. Read(waits/blocked until write func)\n");
    printf("2. write (to wake up read)\n");
    printf("Enter your choice: ");
    scanf("%d",&choice);
    switch(choice){
        case 1: 
                printf("Opening /dev/myworkque in read mode\n");
                fd = open("/dev/myworkque",O_RDONLY);
                if(fd < 0){
                    perror("open");
                    return 1;
                }
                printf("Reading...(until write)\n");
                if(read(fd, buf, 1) < 0){
                    perror("read");
                }else{
                    printf("Read unblocked\n");
                }
                close(fd);
        case 2:
                printf("Opening /dev/myworkque in write mode\n");
                fd = open("/dev/myworkque",O_WRONLY);
                if(fd < 0){
                    perror("open");
                    return 1;
                }
                printf("Writing\n");
                if(write(fd,"X", 1) < 0){
                    perror("write");
                }else{
                    printf("Write done\n");
                }
                close(fd);
        default:
                printf("Invalid choice\n");
    }
    return 0;
}
