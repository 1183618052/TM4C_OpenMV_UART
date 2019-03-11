#pragma once

#include "Basic.h"
/*接收缓冲区长度*/
#define RX_BUF_1_LENGTH         7U
/*帧头长度*/
#define FRAME_HEADER_LENGTH     2U
/*帧尾长度(即校验和)*/
#define FRAME_TAIL_LENGTH       1U
/*帧头相同字节(第一字节)*/
#define FRAME_HEAD_SAME_AA      0xAA
/*帧尾相同字节（最后一个字节）*/
#define FRAME_TAIL_SAME_BB     0xBB
/*帧头区别字节(第二字节)*/
#define FRAME_HEAD_DIFF_55      0x55
#define FRAME_HEAD_DIFF_66      0x66
//接收协议公共变量 
typedef struct{
  volatile uint8_t step;           /*switch语句跳转条件*/
  volatile uint8_t aRxBufIndex;    /*接收数据缓冲区索引*/
  uint8_t aRxBuf_1[RX_BUF_1_LENGTH];
}protocolComType_t;

/*串口接收协议结构体*/
typedef struct{
  protocolComType_t  uart7Ptc;
}uartPtcType_t;

typedef struct{
	short x_err;
	short yaw_err;
}line_data;

typedef struct{
	short x_point_err;
	short y_point_err;
}point_data;

void UART7IntHandler(void);

void InitPtcStruct(protocolComType_t *pUartPtc);
void init_drv_OpenMV();
short bytex2_to_int(unsigned char dataHigh,unsigned char dataLow);

//UART0的中断进程


