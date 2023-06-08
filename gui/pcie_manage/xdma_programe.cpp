#include <unistd.h>
#include "xdma_programe.h"
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DEVICE_NAME_WRITE "/dev/ps_pcie_dmachan0_0"
#define DEVICE_NAME_READ "/dev/ps_pcie_dmachan1_0"
//#define CHAR_DRIVER_NAME               "ps_pcie_dmachan"
xdma_programe::xdma_programe()
    :QThread()
{
    memset(devReadHandle, 0, sizeof(devReadHandle));
    memset(devWriteHandle, 0, sizeof(devWriteHandle));
    pReadyData = NULL;
}

xdma_programe::~xdma_programe()
{
    for(int i=0;i<3;i++)
    {
        if(devReadHandle[i] != -1)
        {
            close(devReadHandle[i]);
        }
        if(devWriteHandle[i] != -1)
        {
            close(devWriteHandle[i]);
        }
    }
}

int xdma_programe::read_pack(int hand, char *pData, int len, unsigned int offset)
{
    int ret;

    if(hand != -1)
    {
//        lseek(hand, offset, SEEK_SET);
//        read(hand, pData, len);
//        return len;

        ret = pread (hand, pData, len, offset);
        return ret;
    }
    else
    {
        return 0;
    }
}


int xdma_programe::write_pack(int hand, char *pData, int len, unsigned int offset)
{
//    char buff[1920*200*4];
//    lseek(hand, offset, SEEK_SET);
//    for(int i=0,n;i<len;i+=n)
//    {
//        n = ((len-i)>=(int)sizeof(buff)) ? (int)sizeof(buff) : (len-i);
//        memcpy(buff, pData+i, n);
//        write(hand, buff, n);
//    }
//    return len;

    int ret;
    ret = pwrite (hand, pData, len, offset);
    return ret;
}

void xdma_programe::run()
{
    while(1)
    {
        if(pReadyData)
        {
            if(mode < 3)
            {
                read_pack(devReadHandle[mode], pReadyData, pReadyLen, pReadyOffset);
                emit opt_end(0);
            }
            else
            {
                write_pack(devWriteHandle[mode-3], pReadyData, pReadyLen, pReadyOffset);
                emit opt_end(1);
            }
            pReadyData = NULL;
        }
        usleep(10000);
    }
}

void xdma_programe::opt_pack(int mode, char *pData, int len, unsigned int offset)
{
    this->mode = mode;
    this->pReadyLen = len;
    this->pReadyOffset = offset;
    this->pReadyData = pData;
}


bool xdma_programe::getDevice()
{
    bool b = false;

    for(int i=0;i<3;i++)
    {
        if(devReadHandle[i] != -1)
        {
            close(devReadHandle[i]);
        }
        if(devWriteHandle[i] != -1)
        {
            close(devWriteHandle[i]);
        }
    }
    memset(devReadHandle, 0, sizeof(devReadHandle));
    memset(devWriteHandle, 0, sizeof(devWriteHandle));
    //获取设备路径
	
    if(devReadHandle[0] == 0)
    {
//        int fd;
//        char dma_path[50];
//        memset (dma_path, '\0', 50);
//        sprintf (dma_path, "/dev/%s%d_%d",CHAR_DRIVER_NAME,1,0);

//        fd = open(dma_path, O_RDWR);
//        fd = open(DEVICE_NAME, O_RDWR);
//        printf("%s, fd = %d\n",DEVICE_NAME, fd);

        devReadHandle[0] = open(DEVICE_NAME_READ, O_RDONLY);
        printf("%s, devReadHandle[0] = %d\n",DEVICE_NAME_READ, devReadHandle[0]);
		if(devReadHandle[0] == -1)
		{
			b = false;
		}
        else
        {
            b = true;
        }

        devWriteHandle[0] = open(DEVICE_NAME_WRITE, O_WRONLY);;
        printf("%s, devWriteHandle[0] = %d\n",DEVICE_NAME_WRITE, devWriteHandle[0]);
        if(devWriteHandle[0] == -1)
        {
            b = false;
        }
        else
        {
            b = true;
        }

	}


    return b;
}


