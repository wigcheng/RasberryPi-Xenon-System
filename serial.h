#ifndef _SERIAL_H_
#define _SERIAL_H_

void closeserial(int fd);
int openserial(char *devicename);
int setRTS(int fd, int level);

#endif