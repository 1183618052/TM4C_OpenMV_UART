#pragma once

#include "Basic.h"
/*���ջ���������*/
#define RX_BUF_1_LENGTH         7U
/*֡ͷ����*/
#define FRAME_HEADER_LENGTH     2U
/*֡β����(��У���)*/
#define FRAME_TAIL_LENGTH       1U
/*֡ͷ��ͬ�ֽ�(��һ�ֽ�)*/
#define FRAME_HEAD_SAME_AA      0xAA
/*֡β��ͬ�ֽڣ����һ���ֽڣ�*/
#define FRAME_TAIL_SAME_BB     0xBB
/*֡ͷ�����ֽ�(�ڶ��ֽ�)*/
#define FRAME_HEAD_DIFF_55      0x55
#define FRAME_HEAD_DIFF_66      0x66
//����Э�鹫������ 
typedef struct{
  volatile uint8_t step;           /*switch�����ת����*/
  volatile uint8_t aRxBufIndex;    /*�������ݻ���������*/
  uint8_t aRxBuf_1[RX_BUF_1_LENGTH];
}protocolComType_t;

/*���ڽ���Э��ṹ��*/
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

//UART0���жϽ���


