
import sensor,time,pyb,math,
from pyb import Pin, Timer, LED, UART
#数据
err_x = 0
err_y = 0

#发送数据
uart_buf = bytearray([0xAA,0x55,0x00,0x00,0x00,0x00,0xBB])
#串口三配置
uart = UART(3, 115200)
uart.init(115200, bits=8, parity=None, stop=1)

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)#设置灰度信息
sensor.set_framesize(sensor.QQVGA)#设置图像大小
sensor.skip_frames(20)#相机自检几张图片
sensor.set_auto_whitebal(False)#关闭白平衡
clock = time.clock()#打开时钟

while(True):
    clock.tick()
    img = sensor.snapshot()

    uart_buf = bytearray([0xAA,0x55,x>>8,x,y>>8,y,0xBB])
    uart.write(uart_buf)
    print(clock.fps(),x,y,uart_buf)

