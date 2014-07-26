#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "serial.h"
#include "mq_debug.h"

struct xenon_info
{
   int pd_id;
   int index;
   int info_len;
   int dev_id_len;
   int dev_id;
   int send_info;
};

static void check_device(char* input_str);

static unsigned char xenon_rfdb_header[4]={0x4b,0xf3,0x52,0xec};

static void check_device(char* input_str)
{
	FILE *file = popen("ls /dev/ttyUSB*", "r");
	fgets(input_str, 13, file);
	pclose(file);
}

int main(int argc,char* argv[])
{
    int fd,ret,count=0,check_sum=0,i;
    char serialdev[16] = {0};
	unsigned char cmd=0x00,recv=0x00,recv_start=0;
	struct xenon_info rfdb_info={0}; 

	check_device(serialdev);
	
    fd = openserial(serialdev);
    if (!fd) {
        mq_debug("Error while initializing %s.\n", serialdev);
        return 1;
    }
	
    setRTS(fd, 1);
    sleep(1);       /* pause 1 second */
    setRTS(fd, 0);

	cmd=0x00;
	ret=write(fd,&cmd,1);
	if(!ret)
		return 1;
	sleep(1);
	cmd=0x28;
	ret=write(fd,&cmd,1);
	if(!ret)
		return 1;
	cmd=0x04;
	ret=write(fd,&cmd,1);
	if(!ret)
		return 1;

	for(;;)
	{
		ret=read(fd,&recv,1);
		if(recv || recv_start)
		{
			if(count<4)
			{
				if(recv!=xenon_rfdb_header[count])
				{
					mq_debug("header failed!!\n");
					return 1;
				}
				else
				{
					recv_start=1;
				}
			}
			else
			{
				if(count==4)
				{
					check_sum+=recv;
					rfdb_info.pd_id=recv;
				}
				if(count==5)
				{
					check_sum+=recv;
					rfdb_info.pd_id+=(recv<<4);
					mq_debug("dev pd_id=%d\n",rfdb_info.pd_id);
				}
				if(count==6)
				{
					check_sum+=recv;
					rfdb_info.index=recv;
					mq_debug("dev index=%d\n",rfdb_info.index);
				}
				if(count==7)
				{
					check_sum+=recv;
					rfdb_info.dev_id_len=recv>>5;
					rfdb_info.info_len=recv&0x1f;				
					mq_debug("dev info_len=%d\n",rfdb_info.info_len);
					mq_debug("dev dev_id_len=%d\n",rfdb_info.dev_id_len);
				}

				if(count>7 && count<=(7+rfdb_info.dev_id_len))
				{
					check_sum+=recv;
					rfdb_info.dev_id+=(recv<<(count-8)*8);

					mq_debug("dev dev_id=%d\n",rfdb_info.dev_id);
				}

				if(count>(7+rfdb_info.dev_id_len) && count<=(7+rfdb_info.dev_id_len+rfdb_info.info_len))
				{
					check_sum+=recv;
					rfdb_info.send_info+=(recv<<(count-(8+rfdb_info.dev_id_len))*4);
					mq_debug("dev send_info=%d\n",rfdb_info.send_info);

				}

				if(count==(7+rfdb_info.dev_id_len+rfdb_info.info_len+1))
				{
					mq_debug("dev check_sum=%x,recv=%x\n",check_sum,recv);
					if((check_sum&0xff)==recv)
					{
						count=-1;
						recv_start=0;
						check_sum=0;
						rfdb_info.dev_id=rfdb_info.dev_id_len=rfdb_info.index=rfdb_info.info_len=rfdb_info.pd_id=rfdb_info.send_info=0;
						recv=0;
					}
					else
					{
						return 1;
					}
				}
				//mq_debug("read ret=%d\n",recv);
			}
			count+=1;
		}

	}

    closeserial(fd);
    return 0;
}
