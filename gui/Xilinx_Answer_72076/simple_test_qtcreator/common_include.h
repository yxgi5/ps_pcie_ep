
#ifndef _XLNX_PCIE_DMA_COMMON_H_
#define _XLNX_PCIE_DMA_COMMON_H_


#define MAX_NUMBER_OF_CHANNELS   4

#define MAX_ALLOWED_CHANNELS_IN_HW   4

#if MAX_NUMBER_OF_CHANNELS > MAX_ALLOWED_CHANNELS_IN_HW
#error "Please reduce number of DMA engines defined in MAX_NUMBER_OF_CHANNELS macro"
#endif

/* Each char device is associated to 1 channel.
 * write call of char device services Host to Card transfers for the channel
 * read call of char device services Card to Host transfer for the channel
 * */
#define MAX_NUMBER_OF_CHAR_DEVICES     MAX_NUMBER_OF_CHANNELS

/* This macro defines the number of Pcie devices that can be
 * supported by this driver when they are inserted parallelly
 * in different slots of the host machine
 */
#define MAX_EXP_DMA_DEVICES            5

#define CHAR_DRIVER_NAME               "ps_pcie_dmachan"

#define PIO_CHAR_DRIVER_NAME           "ps_pcie_pio"

#define EP_TRANSLATION_CHECK            0xCCCCCCCC

//#define XPIO_CLIENT_MAGIC 'P'
#define XPIO_CLIENT_MAGIC 'z'
#define IOCTL_EP_CHECK_TRANSLATION     _IO(XPIO_CLIENT_MAGIC, 0x01)


#define NUMBER_OF_BUFFER_DESCRIPTORS   1999

#define CHANNEL_COAELSE_COUNT          0

#define CHANNEL_POLL_TIMER_FREQUENCY   (HZ) //Lower value indicates frequent checks and greater processing.

#endif
