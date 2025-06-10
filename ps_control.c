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

#define FFT_LENGTH 8192 // FFT长度
#define HEIGHT 255 // 波形高度
#define LOW_PIX 0 // 波形低点
#define LEFT_PIX 0 // 波形左边界


char wave_type, channel;
float frequency, duty, amplitude, offset, phase;

//FFT计算结果结构体
struct Wave_data {
    unsigned char value; // 幅度值
    short index; // 频率值
} wave_data[FFT_LENGTH / 2]; // 波形数据


//ADC采集判断相关变量，将在ADC采集判断函数中被赋值
// 0:正弦波, 1:方波, 2:锯齿波, 3:三角波
int adc_signal_type = 0; // ADC采集信号类型
int adc_frequency = 0; // ADC采集频率
int adc_multiplier = 1; // ADC采集倍频系数
int adc_channel = 0; // ADC采集输出通道



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

void fft_preproc();
void fft_cal_amp(u32 *data, int length) ;
void fft_transmit();

float little_endian2real_number();// 将小端字节序转换为实数
void signal_type_judge(void);// 判断信号类型
void output_control(void);// 输出控制函数
void update_adc_output(void);// 更新ADC采集后输出的参数
void adc_output_control(void);// ADC采集输出控制函数
void adc_page_control(void);// ADC采集页面控制函数

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

void fft_preproc()
{
    int i;
    // 假设DMA得到的数组为 origin_fft_data
    // 得到幅度谱
    fft_cal_amp(origin_fft_data, FFT_LENGTH);

    // 找到最大最小值方便归一化
    double max, min;
    max = 0;
    min = 1e5; // 初始化为极大值
    for(i=0; i<FFT_LENGTH/2; i++) {
        if(fft_data_amplitude[i].value > max)
            max = fft_data_amplitude[i].value;
        if(fft_data_amplitude[i].value < min)
            min = fft_data_amplitude[i].value;
    }

    double range = max - min;
    if(range < 1e-6) {
        range = 1.0;
    }

    // 归一化后计算坐标值
    for(i=0; i<FFT_LENGTH/2; i++) {
        wave_data[i].value = (unsigned char)((fft_data_amplitude[i] - min) / range * HEIGHT + LOW_PIX);
        wave_data[i].index = (short)(i * 2 / FFT_LENGTH * WIDTH + LEFT_PIX);
    }1
}

void fft_cal_amp(u32 *data, int length) 
{
  double real, imag;
  // FFT处理函数
  // 将数据转换为double类型
  for (int i = 0; i < 2 * length; i += 2) {
    real = fixed_to_double(data[i], 30, 15);
    imag = fixed_to_double(data[i + 1], 30, 15);
    // 计算幅值
    fft_data_amplitude[i / 2].index = i / 2;
    fft_data_amplitude[i / 2].value = sqrt(real * real + imag * imag);
    // fft_data_amplitude[i / 2].value = 20 * log10(fft_data_amplitude[i / 2].value + 1e-6); // 对数幅度
  }
}

void fft_transmit()
{
    uint8_t end = 0xff;
    int i;
    for(i=0; i<FFT_LENGTH/2; i++) {
        XUartPs_SendByte(XPAR_UART1_BASEADDR, wave_data[i].value);  // 传输幅度值
        // XUartPs_SendByte(XPAR_UART1_BASEADDR, wave_data[i].index);  // 传输频率值
    }

    XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
    XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
    XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
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

    //如果是ADC采集后的倍频输出
    if(usize >= 8 && u(0) == 0x55 && (u(1) == 0xDB || u(1) == 0xDC) && u(6) == 0x0D && u(7) == 0x0A)
    {
        if(u(1) == 0xDB) // 采集通道1
        {
            channel = 0; // 设置当前通道为1

        }
        else if(u(1) == 0xDC) // 采集通道2
        {
            channel = 1; // 设置当前通道为2
        }

        udelete(8); // 删除前8字节
    }
}


void update_adc_output(void)
{
    //一些判断逻辑，更新adc_signal_type, adc_frequency, adc_multiplier, adc_channel等变量

    //更新信号类型

    //更新信号频率

    //更新倍频数
    if(usize >= 8 && u(0) == 0x55 && u(1) == 0xDB && u(6) == 0x0D && u(7) == 0x0A)
    {
        adc_multiplier = u(2) - '0'; // 倍频系数

        //重新输出
        adc_output_control();

        udelete(8); // 删除前8字节
    }
    //更新输出通道
    else if(usize >= 8 && u(0) == 0x55 && u(1) == 0xDC && u(6) == 0x0D && u(7) == 0x0A)
    {
        adc_channel = u(5) - '0'; // 输出通道

        //重新输出
        adc_output_control();

        udelete(8); // 删除前8字节
    }
    
        
    
}

void adc_output_control(void)
{
    if(adc_signal_type == 0) // 正弦波
        {
            adc_frequency *= adc_multiplier; // 计算倍频后实际频率
            set_sin_freq(); // 设置频率
            
            set_sin_amp(); // 设置振幅
            set_sin_pha(); // 设置相位
        }
        else if(adc_signal_type == 1) // 方波
        {
            adc_frequency *= adc_multiplier; // 计算倍频后实际频率
            set_squ_freq(); // 设置频率
            set_squ_amp(); // 设置振幅
            set_squ_pha(); // 设置相位
        }
        else if(adc_signal_type == 2) // 锯齿波
        {
            adc_frequency *= adc_multiplier; // 计算倍频后实际频率
            set_saw_freq_and_duty(); // 设置频率和占空比
            set_saw_amp(); // 设置振幅
            set_saw_pha(); // 设置相位
        }
        else if(adc_signal_type == 3) // 三角波
        {
            adc_frequency *= adc_multiplier; // 计算倍频后实际频率
            set_saw_freq_and_duty(); // 设置频率和占空比
            set_saw_amp(); // 设置振幅
            set_saw_pha(); // 设置相位
        }

        //选择通道
        if(adc_channel == 0) // 通道1
        {
            adc_channel = 0;
            Enable_output(); // 开启输出
        }
        else if(adc_channel == 1) // 通道2
        {
            adc_channel = 1;
            Enable_output(); // 开启输出
        }
}

void adc_page_control(void)
{
    if(usize >= 8 && u(0) == 0x55 && u(1) == 0xDA && u(6) == 0x0D && u(7) == 0x0A)
    {
        //进行FFT计算和绘图
        fft_preproc(); // 预处理FFT数据
        fft_cal_amp(origin_fft_data, FFT_LENGTH); // 计算
        fft_transmit(); // 传输FFT结果

        //总共4096个点，均匀抽取其中512个点进行绘图，所以每隔8个画一个
        for(int i = 0; i < FFT_LENGTH / 2; i += 8) 
        {
            //向曲线s0的通道0传输1个数据,add指令不支持跨页面
            printf("add s0.id,0,%d\xff\xff\xff",(int)(wave_data[i].value));
        }

        /**************************************************************************/
        //此处需要添加判断波形的函数update_adc_output()来更新adc_signal_type, adc_frequency, adc_multiplier, adc_channel等变量
        update_adc_output(); 
        //然后打印波形类型和频率等信息到串口屏上
        printf("t3.txt=\"%d\"\xff\xff\xff",adc_signal_type);
        printf("t4.txt=\"%d\"\xff\xff\xff",adc_frequency);
        /**************************************************************************/

        //按照adc采集相关变量进行波形输出，其值已经在上述判断波形函数中被正确赋值
        adc_output_control();


        udelete(8); // 删除前8字节
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
    // ADC采集页面控制
    adc_page_control(); 


}