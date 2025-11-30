#ifndef MYHD_H
#define MYHD_H

#define MY_MAGIC 'x'

#define CLEAR_BUFF   _IO(MY_MAGIC, 1)
#define WRITE_BUFF   _IOW(MY_MAGIC, 2, int)
#define READ_BUFF    _IOR(MY_MAGIC, 3, int)

#endif
