#include<linux/ioctl.h>
#define Magic 'M'

#define ON        _IO(Magic,0)
#define OFF       _IO(Magic,1)
#define STATUS    _IOR(Magic,2,int)
#define SET_VALUE _IOW(Magic,3,int)
