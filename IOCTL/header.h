#define MY_HEADER_H

#include <linux/ioctl.h>

#define MY_MAGIC 'k'           // Unique magic number

#define LED_ON       _IO(MY_MAGIC, 0)
#define LED_OFF      _IO(MY_MAGIC, 1)
#define GET_STATUS   _IOR(MY_MAGIC, 2, int)
#define SET_BLINK    _IOW(MY_MAGIC, 3, int)

#endif
