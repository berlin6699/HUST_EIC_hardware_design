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

float little_endian2real_number();// 将小端字节序转换为实数
void signal_type_judge(void);// 判断信号类型
void output_control(void);// 输出控制函数

//构造通道数据结构体
typedef struct 
{
    int signal_type; // 0:正弦波, 1:方波, 2:锯齿波, 3:三角波
    float frequency; // 频率
    float amplitude; // 振幅
    float offset; // 偏置
    float duty; // 占空比
}channel_data



int main()
{

    //生成两个通道数据结构体
    channel_data ch1_data, ch2_data;


    wave_type = 0;
    frequency = 1000;
    phase = 0;
    duty = 50;
    amplitude = 1;
    offset = 0;
    channel = 0;

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


void signal_type_judge
{
    //如果是选择信号类型的指令
    if(usize >= 8 && u(0) == 0x55 && u(1) == 0xAA && u(6) == 0x0D && u(7) == 0x0A)
    {
        //先判断u(3)的值，以确定波形类型
        if(u(3) == 0xAD) // 正弦波
        {
            wave_type = 0;
        }
        else if(u(3) == 0x96) // 锯齿波
        {
            wave_type = 1;
        }
        else if(u(3) == 0x94) // 方波
        {
            wave_type = 2;
        }
        else if(u(3) == 0xB8) // 三角波
        {
            wave_type = 3;
        }

        channel1_data.signal_type = wave_type;
        udelete(8);

    }
    else if(usize >= 8 && u(0) == 0x55 && u(1) == 0xAB && u(6) == 0x0D && u(7) == 0x0A)
    {
        //先判断u(3)的值，以确定波形类型
        if(u(3) == 0xAD) // 正弦波
        {
            wave_type = 0;
        }
        else if(u(3) == 0x96) // 锯齿波
        {
            wave_type = 1;
        }
        else if(u(3) == 0x94) // 方波
        {
            wave_type = 2;
        }
        else if(u(3) == 0xB8) // 三角波
        {
            wave_type = 3;
        }

        channel2_data.signal_type = wave_type;
        udelete(8);
    }

    set_wave_type(); // 设置波形类型

}


float little_endian2real_number()
{
    uint32_t value = 0;

    value |= (uint32_t)u(2 + 0) <<  0; // byte0 - LSB
    value |= (uint32_t)u(2 + 1) <<  8; // byte1
    value |= (uint32_t)u(2 + 2) << 16; // byte2
    value |= (uint32_t)u(2 + 3) << 24; // byte3 - MSB

    float result = (float)value / 100.0f;

    udelete(8); // 删除前8字节

    return result;
}

void output_control()
{
    //如果是channel1的输出控制指令
    if(usize >= 8 && u(0) == 0x55 && u(1) == 0xBA && u(6) == 0x0D && u(7) == 0x0A)
    {
        channel1_data.frequency = little_endian2real_number(); // 读取频率
        channel1_data.amplitude = little_endian2real_number(); // 读取振幅
        channel1_data.offset = little_endian2real_number(); // 读取偏置
        channel1_data.duty = little_endian2real_number(); // 读取占空比

        frequency = channel1_data.frequency;
        amplitude = channel1_data.amplitude;
        offset = channel1_data.offset;
        duty = channel1_data.duty;

        channel = 0; // 设置当前通道为1
        set_wave_type(); // 设置波形类型
        set_offset(); // 设置偏置
        if(wave_type == 0) // 正弦波
        {
            set_sin_freq(); // 设置频率
            set_sin_amp(); // 设置振幅
            set_sin_pha(); // 设置相位
        }
        else if(wave_type == 1) // 锯齿波
        {
            set_saw_freq_and_duty(); // 设置频率和占空比
            set_saw_amp(); // 设置振幅
            set_saw_pha(); // 设置相位
        }
        else if(wave_type == 2) // 方波
        {
            set_squ_freq(); // 设置频率
            set_squ_amp(); // 设置振幅
            set_squ_duty(); // 设置占空比
            set_squ_pha(); // 设置相位
        }
        else if(wave_type == 3) // 三角波
        {
            set_saw_freq_and_duty(); // 设置频率和占空比
            set_saw_amp(); // 设置振幅
            set_saw_pha(); // 设置相位
        }

        Enable_output(); // 开启输出
    }
    //如果是channel2的输出控制指令
    else if(usize >= 8 && u(0) == 0x55 && u(1) == 0xBB && u(6) == 0x0D && u(7) == 0x0A)
    {
        channel2_data.frequency = little_endian2real_number(); // 读取频率
        channel2_data.amplitude = little_endian2real_number(); // 读取振幅
        channel2_data.offset = little_endian2real_number(); // 读取偏置
        channel2_data.duty = little_endian2real_number(); // 读取占空比

        frequency = channel2_data.frequency;
        amplitude = channel2_data.amplitude;  
        offset = channel2_data.offset;
        duty = channel2_data.duty;

        channel = 1; // 设置当前通道为2
        set_wave_type(); // 设置波形类型
        set_offset(); // 设置偏置
        if(wave_type == 0) // 正弦波
        {
            set_sin_freq(); // 设置频率
            set_sin_amp(); // 设置振幅
            set_sin_pha(); // 设置相位
        }
        else if(wave_type == 1) // 锯齿波
        {
            set_saw_freq_and_duty(); // 设置频率和占空比
            set_saw_amp(); // 设置振幅
            set_saw_pha(); // 设置相位
        }
        else if(wave_type == 2) // 方波
        {
            set_squ_freq(); // 设置频率
            set_squ_amp(); // 设置振幅
            set_squ_duty(); // 设置占空比
            set_squ_pha(); // 设置相位
        }
        else if(wave_type == 3) // 三角波
        {
            set_saw_freq_and_duty(); // 设置频率和占空比
            set_saw_amp(); // 设置振幅
            set_saw_pha(); // 设置相位
        }

        Enable_output(); // 开启输出
    }

}

//串口屏控制逻辑
void TJC_USART(void)
{
    while (usize >= 8)
    {
        // 校验帧头帧尾是否匹配
        if (u(0) != 0x55 || u(6) != 0x0D || u(7) != 0x0A)
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

    // 判断信号类型
    signal_type_judge();
    // 输出控制
    output_control();
  
  

 

	if (usize >= 6 && u(0) == 0x55 && u(3) == 0xff && u(4) == 0xff && u(5) == 0xff) {
		if ((u(1) == 0xBA))//ch1输出
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
		else if (u(1) == 0xBB)//ch2输出
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