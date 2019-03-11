#include "Basic.h"
#include "drv_UART_OpenMV.h"

#include "hw_memmap.h"
#include "hw_ints.h"
#include "gpio.h"
#include "pin_map.h"
#include "sysctl.h"
#include "uart.h"
#include "interrupt.h"

uartPtcType_t uartPtc;/*声明接收结构体*/
line_data l_data = {0,0};
point_data p_data = {0,0};

void init_drv_OpenMV(void)
{
	//使能UART外设
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
	//使能GPIO外设
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	//GPIO模式配置 PE0--RX PE1--TX 
	GPIOPinConfigure(GPIO_PE0_U7RX);
	GPIOPinConfigure(GPIO_PE1_U7TX);
	//GPIO的UART模式配置
	GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	//UART协议配置 波特率115200 8位 1停止位  无校验位
	UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(), 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
					UART_CONFIG_PAR_NONE));
	
	//UART禁用FIFO 默认FIFO Level为4/8 寄存器满8字节后产生中断
	//禁用后接收1位就产生中断
	UARTFIFODisable(UART7_BASE);
	//使能UART7中断
	IntEnable(INT_UART7);	
	//使能UART7接收中断
	UARTIntEnable(UART7_BASE,UART_INT_RX);
	//UART中断地址注册
	UARTIntRegister(UART7_BASE,UART7IntHandler);
	//全局中断使能
	IntMasterEnable();
	InitPtcStruct(&uartPtc.uart7Ptc);/*UART初始化结构体值,*/
}

void InitPtcStruct(protocolComType_t *pUartPtc)
{
  pUartPtc->step         = 0;
  pUartPtc->aRxBufIndex  = 0;
}

/*参考文章
https://blog.csdn.net/a1598025967/article/details/81975266
*/
//UART7中断函数
void UART7IntHandler(void)
{
	//获取中断标志 原始中断状态 不屏蔽中断标志
		//uint32_t flag = UARTIntStatus(UART7_BASE,0);
	//获取中断标志 原始中断状态 屏蔽中断标志
	uint32_t flag = UARTIntStatus(UART7_BASE,1);
	//清除中断标志
	UARTIntClear(UART7_BASE,flag);
	if(flag&UART_INT_RX)
		//UARTCharsAvail()//判断FIFO是否还有数据
		while(UARTCharsAvail(UART7_BASE))
		{
			unsigned char data = UARTCharGet(UART7_BASE); //保存本次接收数据
			//UARTCharPut(UART7_BASE,data);
			protocolComType_t *pUartPtc = &uartPtc.uart7Ptc;
			switch (pUartPtc->step){
				case 0:
					if(data == FRAME_HEAD_SAME_AA)/*帧头正确*/
            {
                pUartPtc->step = 1;/*跳转下一步骤*/
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;
            }
				break;
			  case 1:
            if(data == FRAME_HEAD_DIFF_55||data == FRAME_HEAD_DIFF_66)/*帧头正确*/
            {
							  if(data == FRAME_HEAD_DIFF_55)
                    pUartPtc->step = 2;/*跳转到步骤2*/
                if(data == FRAME_HEAD_DIFF_66)
									  pUartPtc->step = 3;/*跳转到步骤3*/
								pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;
            }
            else if(data == FRAME_HEAD_SAME_AA)
                pUartPtc->step = 1;/*第一帧头重复，回到第二帧头判断处,AA AA 情况*/
            else
                InitPtcStruct(pUartPtc);/*初始化结构体值,准备下一次接收*/
        break;
				
        case 2:   //FRAME_HEAD_DIFF_55
            if(data == FRAME_TAIL_SAME_BB)/*检测到帧尾，接收成功*/
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;

							  if(RX_BUF_1_LENGTH > pUartPtc->aRxBufIndex )
                {/*数据有丢失接收失败*/
                    InitPtcStruct(pUartPtc);/*初始化结构体值,准备下一次接收*/
                }
								else
								{
									p_data.x_point_err = bytex2_to_int(pUartPtc->aRxBuf_1[2],pUartPtc->aRxBuf_1[3]);
									p_data.y_point_err = bytex2_to_int(pUartPtc->aRxBuf_1[4],pUartPtc->aRxBuf_1[5]);
									
									//储存本次接收成功数据
								}
							
								InitPtcStruct(pUartPtc);/*初始化结构体值,准备下一次接收*/
            }
            else
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;/*压入缓冲区*/
            }
            break;	
						
				case 3:   //FRAME_HEAD_DIFF_66
            if(data == FRAME_TAIL_SAME_BB)/*检测到帧尾，接收成功*/
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;

							  if(RX_BUF_1_LENGTH > pUartPtc->aRxBufIndex )
                {/*数据有丢失接收失败*/
                    InitPtcStruct(pUartPtc);/*初始化结构体值,准备下一次接收*/
                }
								else
								{
										l_data.x_err = bytex2_to_int(pUartPtc->aRxBuf_1[2],pUartPtc->aRxBuf_1[3]);
										l_data.yaw_err = bytex2_to_int(pUartPtc->aRxBuf_1[4],pUartPtc->aRxBuf_1[5]);
									
									
							
									//储存本次接收成功数据
								}
							
								InitPtcStruct(pUartPtc);/*初始化结构体值,准备下一次接收*/
            }
            else
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;/*压入缓冲区*/
            }
        break;			
        default:
            InitPtcStruct(pUartPtc);/*初始化结构体值,准备下一次接收*/
        break;				
				}
		}
}

short bytex2_to_int(unsigned char dataHigh,unsigned char dataLow)
{
    short res = 0x0000;
    res = res | dataHigh;
    res = res << 8;
    res = res | dataLow;
    return res;
}
