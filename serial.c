#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static struct termios oldterminfo;

void closeserial(int fd)
{
	tcsetattr(fd, TCSANOW, &oldterminfo);
	if (close(fd) < 0)
		perror("closeserial()");
}


int openserial(char *devicename)
{
    int fd;
    struct termios attr;

    if ((fd = open(devicename, O_RDWR )) == -1) {
        perror("openserial(): open()");
        return 0;
    }
    if (tcgetattr(fd, &oldterminfo) == -1) {
        perror("openserial(): tcgetattr()");
        return 0;
    }
    attr = oldterminfo;
	attr.c_cflag = CS8;
    attr.c_cflag |= CRTSCTS | CLOCAL;
    attr.c_oflag = 0;

	attr.c_cflag = (attr.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	attr.c_iflag &= ~IGNBRK;         // disable break processing
	attr.c_lflag = 0;                // no signaling chars, no echo,
	                            // no canonical processing
	attr.c_oflag = 1;                // no remapping, no delays
	attr.c_cc[VMIN]  = 0;            // read doesn't block
	attr.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
	attr.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	attr.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,  enable reading
	attr.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	attr.c_cflag |= 0;
	attr.c_cflag &= ~CSTOPB;
	attr.c_cflag &= ~CRTSCTS;


    if (tcflush(fd, TCIOFLUSH) == -1) {
        perror("openserial(): tcflush()");
        return 0;
    }

	cfsetospeed(&attr,B38400);
    cfsetispeed(&attr,B38400);            // baudrate is declarated above

	if (tcsetattr(fd, TCSANOW, &attr) == -1) {
        perror("initserial(): tcsetattr()");
        return 0;
    }
	
	return fd;
}

int setRTS(int fd, int level)
{
    int status;

    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("setRTS(): TIOCMGET");
        return 0;
    }
    if (level)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("setRTS(): TIOCMSET");
        return 0;
    }
    return 1;
}
