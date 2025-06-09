#include <stdio.h>
#include "math.h"
#include "sleep.h"

/*
GPIO1的0通道对应两个SPI的SCK，1通道对应两个SPI的MOSI，2、3通道分别对应两个SPI的SS
从仿真波形来看方波的峰值在1275ns，锯齿波的峰值在1275+3*10=1305ns，正弦波的峰值在1345ns，分别延后了3个T和4个T
*/

// 相关参数
#define CLOCK_FREQ 1.0e8
#define ADC_AMP 3.3

// 相关地址
#define GPIO_DEVICE_ID XPAR_XGPIOPS_0_DEVICE_ID
#define GPIO1_SET_ADDR XPAR_XGPIO_0_BASEADDR + XGPIO_TRI_OFFSET
#define GPIO1_DATA_ADDR XPAR_XGPIO_0_BASEADDR + XGPIO_DATA_OFFSET
#define DDS_BASEADDR XPAR_USER_DUAL_DDS_0_BASEADDR

// DDS模块各寄存器偏移
#define CH1_WAVE 0
#define CH1_AMP 4
#define CH1_PHA 8
#define CH1_SIN_FRE 12
#define CH1_TRI_INC 16
#define CH1_TRI_DEC 20
#define CH1_SQU_CNT 24
#define CH1_SQU_CMP 28

#define CH2_WAVE 32
#define CH2_AMP 36
#define CH2_PHA 40
#define CH2_SIN_FRE 44
#define CH2_TRI_INC 48
#define CH2_TRI_DEC 52
#define CH2_SQU_CNT 56
#define CH2_SQU_CMP 60
#define CH_RST_EN 64

#define SPI_SCK 0
#define SPI_MOSI 1
#define SPI_SS1 2
#define SPI_SS2 3

char wave_type, channel;
float frequency, duty, amplitude, offset, phase;

void set_wave_type(void);

void set_offset(void);

void set_sin_freq(void);
void set_sin_amp(void);
void set_sin_pha(void);

void set_saw_freq_and_duty(void);
void set_saw_amp(void);
void set_saw_pha(void);

void set_squ_freq(void);
void set_squ_amp(void);
void set_squ_duty(void);
void set_squ_pha(void);

void Enable_output(void);
void Disable_output(void);

void SPI_comm(int Data, char Ch);
void writeBit(char pin, int value);


int main()
{
    wave_type = 0;
    frequency = 1000;
    phase = 0;
    duty = 50;
    amplitude = 1;
    offset = 0;
    channel = 0;

    // //初始化GPIO
    // XGpio_Config *ConfigPtr = XGpio_LookupConfig(GPIO_DEVICE_ID);
    // int status = XGpio_CfgInitialize(&s_Gpio, ConfigPtr, ConfigPtr->BaseAddress);

    // if(status != XST_SUCCESS)
    //     return XST_FAILURE;

    // 设置GPIO方向为输出
    Xil_Out8(GPIO1_SET_ADDR, 0x00);

    // 使能微处理器中断

    while (1)
    {
        //串口屏控制部分
    	TJC_USART();
    }

    return 0;
}

// 模拟SPI通信函数，时钟周期2us，0.5Mbps的传输速率，高位优先，SCK初始高，SS初始高
void SPI_comm(int Data, char Ch)
{
    char i;
    // 片选
    if(Ch == 0)
        writeBit(SPI_SS1, 0); // SPI1
    else
        writeBit(SPI_SS2, 0); // SPI2

    usleep(1);
    
    for(i=0; i<14; i++) {
        writeBit(SPI_SCK, 1); // 拉高SCK
        writeBit(SPI_MOSI, (Data & 0x2000) ? 1 : 0); // MOSI传输(第13bit)
        usleep(1);
        writeBit(SPI_SCK, 0); // 拉低SCK
        usleep(1);

        Data <<= 1; // 左移Data
    }

    writeBit(SPI_SCK, 1);   // 重新拉高SCK

    // 取消片选
    if(Ch == 0)
        writeBit(SPI_SS1, 1); // SPI1
    else
        writeBit(SPI_SS2, 1); // SPI2
}
void writeBit(char pin, int value)
{
    uint8_t reg = Xil_in8(GPIO1_DATA_ADDR);
    if(val)
        reg |= (1 << pin);  // 对应位置一
    else
        reg &= ~(1 << pin); // 对应位置零
    Xil_out8(GPIO1_DATA_ADDR, reg);
}


// 设置波形
// 0-正弦波，1-锯齿波，2-方波
void set_wave_type()
{
    if(channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_WAVE, wave_type);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_WAVE, wave_type);
}

// 设置偏置
void set_offset(void)
{
    short off_word;
    off_word = (short)(pow(2, 13) * offset / ADC_AMP);

    // 超限处理
    if(off_word > 8191)
        off_word = 8191;
    else if(off_word < -8192)
        off_word = -8192;

    off_word = ((uint16_t)off_word) & 0x3fff;   // 保留低14bit

    SPI_comm(off_word, channel);
}

// 设置正弦波频率
void set_sin_freq(void)
{
    unsigned freq_word;
    freq_word = (unsigned)(pow(2, 32) * frequency / CLOCK_FREQ);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_SIN_FRE, freq_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_SIN_FRE, freq_word);
}
// 设置正弦波幅度
void set_sin_amp(void)
{
    unsigned amp_word;
    amp_word = (unsigned)(amplitude / ADC_AMP * (pow(2, 13) - 1));
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_AMP, amp_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_AMP, amp_word);
}
// 设置正弦波相位
void set_sin_pha(void)
{
    unsigned pha_word;
    pha_word = (unsigned)(pow(2, 32) * phase / 360);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_PHA, pha_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_PHA, pha_word);
}

// 设置锯齿波频率和占空比
void set_saw_freq_and_duty(void)
{
    // 定义三个无符号整数变量，分别表示递增字、递减字和相位字
    unsigned inc_word, dec_word;
    inc_word = (unsigned)(pow(2, 31) * frequency * (1 + duty) / CLOCK_FREQ / duty);
    dec_word = (unsigned)(pow(2, 31) * frequency * (1 + duty) / CLOCK_FREQ);
    if (channel == 0)
    {
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_TRI_INC, inc_word);
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_TRI_DEC, dec_word);
    }
    else
    {
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_TRI_INC, inc_word);
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_TRI_DEC, dec_word);
    }
}
// 设置锯齿波幅度
void set_saw_amp(void)
{
    unsigned amp_word;
    amp_word = (unsigned)((pow(2, 14) - 2) * amplitude / ADC_AMP);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_AMP, amp_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_AMP, amp_word);
}
// 设置锯齿波相位
void set_saw_pha(void)
{
    unsigned pha_word;
    pha_word = (unsigned)(pow(2, 32) * phase / 360);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_PHA, pha_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_PHA, pha_word);
}

// 设置方波频率
void set_squ_freq(void)
{
    unsigned cnt_word;
    cnt_word = (unsigned)(pow(2, 31) * frequency / CLOCK_FREQ);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_SQU_CNT, cnt_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_SQU_CNT, cnt_word);
}
// 设置方波幅度
void set_squ_amp(void)
{
    unsigned amp_word;
    amp_word = (unsigned)(amplitude / ADC_AMP * (pow(2, 13) - 1));
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_AMP, amp_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_AMP, amp_word);
}
// 设置方波占空比
void set_squ_duty(void)
{
    unsigned cmp_word;
    cmp_word = (unsigned)(pow(2, 32) * (1 - duty / 100));
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_SQU_CMP, cmp_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_SQU_CMP, cmp_word);
}
// 设置方波相位
void set_squ_pha(void)
{
    unsigned pha_word;
    pha_word = (unsigned)(pow(2, 32) * phase / 360);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_PHA, pha_word);
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_PHA, pha_word);
}

// 开启输出
void Enable_output(void)
{
    char origin_state = USER_DUAL_DDS_mReadReg(DDS_BASEADDR, CH_RST_EN);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN, origin_state | 0x4); // reg16的bit2为1
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN, origin_state | 0x8); // reg16的bit3为1
}
// 关闭输出
void Disable_output(void)
{
    char origin_state = USER_DUAL_DDS_mReadReg(DDS_BASEADDR, CH_RST_EN);
    if (channel == 0)
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN, origin_state & 0xb); // reg16的bit2为0
    else
        USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN, origin_state & 0x7); // reg16的bit3为0
}



void Recv_Process(void)
{
    if (strcmp((char *)instrReg, "sin") == 0)
    {
        // xil_printf("Output SIN wave.\n");
        wave_type = 0;
        set_wave_type();
        set_sin_amp();
        set_sin_freq();
    }
    else if (strcmp((char *)instrReg, "squ") == 0)
    {
        // xil_printf("Output SQUARE wave.\n");
        wave_type = 2;
        set_wave_type();
        set_squ_amp();
        set_squ_duty();
        set_squ_freq();
    }
    else if (strcmp((char *)instrReg, "tri") == 0)
    {
        // xil_printf("Output TRIANGLE wave.\n");
        wave_type = 1;
        set_wave_type();
        set_saw_amp();
        set_saw_freq_and_duty();
    }
    else if (strcmp((char *)instrReg, "fre") == 0)
    {
        // xil_printf("Adjust FREQUENCY to %d Hz.\n",*(int*)(dataReg));
        frequency = (float)(*(int *)(dataReg));
        switch (wave_type)
        {
        case 0:
            set_sin_freq();
            break;
        case 1:
            set_saw_freq_and_duty();
            break;
        case 2:
            set_squ_freq();
            set_squ_duty();
            break;
        default:
            break;
        }
    }
    else if (strcmp((char *)instrReg, "amp") == 0)
    {
        // xil_printf("Adjust AMPLITUDE to %d mV.\n",*(int*)(dataReg));
        amplitude = *(int *)(dataReg) / 1000.0;
        switch (wave_type)
        {
        case 0:
            set_sin_amp();
            // set_offset();
            break;
        case 1:
            set_saw_amp();
            // set_offset();
            break;
        case 2:
            set_squ_amp();
            // set_offset();
            break;
        default:
            break;
        }
    }
    else if (strcmp((char *)instrReg, "set") == 0)
    {
        // xil_printf("Adjust OFFSET to %d mV.\n",*(int*)(dataReg));
        offset = *(int *)(dataReg) / 1000.0;
        set_offset();
    }
    else if (strcmp((char *)instrReg, "dut") == 0)
    {
        // xil_printf("Adjust DUTY to %d per cent.\n",*(int*)(dataReg));
        duty = (float)(*(int *)(dataReg));
        switch (wave_type)
        {
        case 1:
            set_saw_freq_and_duty();
            break;
        case 2:
            set_squ_duty();
            break;
        default:
            break;
        }
    }
    else if (strcmp((char *)instrReg, "ena") == 0)
    {
        // xil_printf("OUTPUT ENABLE.\n");
        Enable_output();
    }
    else if (strcmp((char *)instrReg, "dis") == 0)
    {
        // xil_printf("OUTPUT DISABLE.\n");
        Disable_output();
    }
    else
    {
        // xil_printf("ERROR.\n");
        Disable_output();
    }
}

//串口屏控制逻辑
void TJC_USART(void)
{
  while (usize >= 6)
  {
		// 校验帧头帧尾是否匹配
		if (u(0) != 0x55 || u(3) != 0xff || u(4) != 0xff || u(5) != 0xff)
		{
			// 不匹配删除1字节
			udelete(1);
		}
		else
		{
		// 匹配，跳出循环
		break;
		}
  }


		// 进行解析
	if (usize >= 6 && u(0) == 0x55 && u(3) == 0xff && u(4) == 0xff && u(5) == 0xff) {
		if ((u(1) == 0x00))//ch1输出
		{
            
            channel = 0;
            wave_type = u(2); // 波形类型
            frequency = t6.txt // 频率
            amplitude = t7.txt // 振幅
            offset = t8.txt // 偏置
            duty = t9.txt // 占空比

            set_wave_type();
            set_offset();
            if(wave_type == 0) // 正弦波
            {
                set_sin_freq();
                set_sin_amp();
                set_sin_pha();
            }
            else if(wave_type == 1) // 锯齿波
            {
                set_saw_freq_and_duty();
                set_saw_amp();
                set_saw_pha();
            }
            else if(wave_type == 2) // 方波
            {
                set_squ_freq();
                set_squ_amp();
                set_squ_duty();
                set_squ_pha();
            }
            
		  
		}
		else if (u(1) == 0x01)//ch2输出
		{
            channel = 1;
            wave_type = u(2); // 波形类型
            frequency = t17.txt // 频率
            amplitude = t18.txt // 振幅
            offset = t19.txt // 偏置
            duty = t22.txt // 占空比

            set_wave_type();
            set_offset();
            if(wave_type == 0) // 正弦波
            {
                set_sin_freq();
                set_sin_amp();
                set_sin_pha();
            }
            else if(wave_type == 1) // 锯齿波
            {
                set_saw_freq_and_duty();
                set_saw_amp();
                set_saw_pha();
            }
            else if(wave_type == 2) // 方波
            {
                set_squ_freq();
                set_squ_amp();
                set_squ_duty();
                set_squ_pha();
            }
		  udelete(6);
		}
		
		else if(u(1)==0xbb)
		{
			if(u(2)==0x00)
			{
				
			}
			else if(u(2)==0x01)
			{
				
			}

		}
		udelete(6);
	}

}