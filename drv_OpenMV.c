#include "Basic.h"
#include "drv_OpenMV.h"

#include "hw_memmap.h"
#include "hw_ints.h"
#include "gpio.h"
#include "pin_map.h"
#include "sysctl.h"
#include "uart.h"
#include "interrupt.h"

uartPtcType_t uartPtc;/*�������սṹ��*/
line_data l_data = {0,0};
point_data p_data = {0,0};

void init_drv_OpenMV(void)
{
	//ʹ��UART����
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
	//ʹ��GPIO����
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	//GPIOģʽ���� PE0--RX PE1--TX 
	GPIOPinConfigure(GPIO_PE0_U7RX);
	GPIOPinConfigure(GPIO_PE1_U7TX);
	//GPIO��UARTģʽ����
	GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	//UARTЭ������ ������115200 8λ 1ֹͣλ  ��У��λ
	UARTConfigSetExpClk(UART7_BASE, SysCtlClockGet(), 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
					UART_CONFIG_PAR_NONE));
	
	//UART����FIFO Ĭ��FIFO LevelΪ4/8 �Ĵ�����8�ֽں�����ж�
	//���ú����1λ�Ͳ����ж�
	UARTFIFODisable(UART7_BASE);
	//ʹ��UART7�ж�
	IntEnable(INT_UART7);	
	//ʹ��UART7�����ж�
	UARTIntEnable(UART7_BASE,UART_INT_RX);
	//UART�жϵ�ַע��
	UARTIntRegister(UART7_BASE,UART7IntHandler);
	//ȫ���ж�ʹ��
	IntMasterEnable();
	InitPtcStruct(&uartPtc.uart7Ptc);/*UART��ʼ���ṹ��ֵ,*/
}

void InitPtcStruct(protocolComType_t *pUartPtc)
{
  pUartPtc->step         = 0;
  pUartPtc->aRxBufIndex  = 0;
}

/*�ο�����
https://blog.csdn.net/a1598025967/article/details/81975266
*/
//UART7�жϺ���
void UART7IntHandler(void)
{
	//��ȡ�жϱ�־ ԭʼ�ж�״̬ �������жϱ�־
		//uint32_t flag = UARTIntStatus(UART7_BASE,0);
	//��ȡ�жϱ�־ ԭʼ�ж�״̬ �����жϱ�־
	uint32_t flag = UARTIntStatus(UART7_BASE,1);
	//����жϱ�־
	UARTIntClear(UART7_BASE,flag);
	if(flag&UART_INT_RX)
		//UARTCharsAvail()//�ж�FIFO�Ƿ�������
		while(UARTCharsAvail(UART7_BASE))
		{
			unsigned char data = UARTCharGet(UART7_BASE); //���汾�ν�������
			//UARTCharPut(UART7_BASE,data);
			protocolComType_t *pUartPtc = &uartPtc.uart7Ptc;
			switch (pUartPtc->step){
				case 0:
					if(data == FRAME_HEAD_SAME_AA)/*֡ͷ��ȷ*/
            {
                pUartPtc->step = 1;/*��ת��һ����*/
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;
            }
				break;
			  case 1:
            if(data == FRAME_HEAD_DIFF_55||data == FRAME_HEAD_DIFF_66)/*֡ͷ��ȷ*/
            {
							  if(data == FRAME_HEAD_DIFF_55)
                    pUartPtc->step = 2;/*��ת������2*/
                if(data == FRAME_HEAD_DIFF_66)
									  pUartPtc->step = 3;/*��ת������3*/
								pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;
            }
            else if(data == FRAME_HEAD_SAME_AA)
                pUartPtc->step = 1;/*��һ֡ͷ�ظ����ص��ڶ�֡ͷ�жϴ�,AA AA ���*/
            else
                InitPtcStruct(pUartPtc);/*��ʼ���ṹ��ֵ,׼����һ�ν���*/
        break;
				
        case 2:   //FRAME_HEAD_DIFF_55
            if(data == FRAME_TAIL_SAME_BB)/*��⵽֡β�����ճɹ�*/
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;

							  if(RX_BUF_1_LENGTH > pUartPtc->aRxBufIndex )
                {/*�����ж�ʧ����ʧ��*/
                    InitPtcStruct(pUartPtc);/*��ʼ���ṹ��ֵ,׼����һ�ν���*/
                }
								else
								{
									p_data.x_point_err = bytex2_to_int(pUartPtc->aRxBuf_1[2],pUartPtc->aRxBuf_1[3]);
									p_data.y_point_err = bytex2_to_int(pUartPtc->aRxBuf_1[4],pUartPtc->aRxBuf_1[5]);
									
									//���汾�ν��ճɹ�����
								}
							
								InitPtcStruct(pUartPtc);/*��ʼ���ṹ��ֵ,׼����һ�ν���*/
            }
            else
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;/*ѹ�뻺����*/
            }
            break;	
						
				case 3:   //FRAME_HEAD_DIFF_66
            if(data == FRAME_TAIL_SAME_BB)/*��⵽֡β�����ճɹ�*/
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;

							  if(RX_BUF_1_LENGTH > pUartPtc->aRxBufIndex )
                {/*�����ж�ʧ����ʧ��*/
                    InitPtcStruct(pUartPtc);/*��ʼ���ṹ��ֵ,׼����һ�ν���*/
                }
								else
								{
										l_data.x_err = bytex2_to_int(pUartPtc->aRxBuf_1[2],pUartPtc->aRxBuf_1[3]);
										l_data.yaw_err = bytex2_to_int(pUartPtc->aRxBuf_1[4],pUartPtc->aRxBuf_1[5]);
									
									
							
									//���汾�ν��ճɹ�����
								}
							
								InitPtcStruct(pUartPtc);/*��ʼ���ṹ��ֵ,׼����һ�ν���*/
            }
            else
            {
                pUartPtc->aRxBuf_1[pUartPtc->aRxBufIndex++] = data;/*ѹ�뻺����*/
            }
        break;			
        default:
            InitPtcStruct(pUartPtc);/*��ʼ���ṹ��ֵ,׼����һ�ν���*/
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
