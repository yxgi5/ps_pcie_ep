#include "xdma_programe.h"
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

//#define DEVICE_NAME_WRITE "/dev/ps_pcie_dmachan0_0"
//#define DEVICE_NAME_READ "/dev/ps_pcie_dmachan1_0"
//#define DEVICE_NAME_DEFAULT "/dev/xdma/card0/c2h0"
//#define DEVICE_NAME_DEFAULT "/dev/xdma0_c2h_0"
#define DEVICE_NAME_DEFAULT "/dev/ps_pcie_dmachan1_0"
//#define IMG_RAM_POS     (512*1024*1024)
#define IMG_RAM_POS (0x10000000)
//#define IMG_RAM_POS (0x1000000)

xdma_programe::xdma_programe()
{
    dev_fd = -1;
}

xdma_programe::~xdma_programe()
{
    if(dev_fd != -1)
    {
        close(dev_fd);
    }
}

int xdma_programe::read_pack(char *pData, int len)
{
    int ret;

    if(dev_fd != -1)
    {
//        lseek(dev_fd, IMG_RAM_POS, SEEK_SET);
//        read(dev_fd, pData, len);
//        return len;
        if(len<=4194304)
        {
            ret = pread (dev_fd, pData, len, IMG_RAM_POS);
        }
        else
        {
            int offset=0;

            
            while((offset+4194304)<len)
            {
                ret = pread (dev_fd, pData+offset, 4194304, IMG_RAM_POS+offset);
                offset += 4194304;
            }

            ret = pread (dev_fd, pData+offset, len-offset, IMG_RAM_POS+offset);
        }
        
        return ret;
    }
    else
    {
        return 0;
    }
}

bool xdma_programe::getDevice()
{

    if(dev_fd == -1)
    {
        dev_fd = open(DEVICE_NAME_DEFAULT, O_RDWR | O_NONBLOCK);
    }
    if(dev_fd == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}


