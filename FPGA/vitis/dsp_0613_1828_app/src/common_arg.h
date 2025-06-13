#ifndef _COMMON_ARG_H
#define _COMMON_ARG_H

typedef struct 
{
    int channel; // 0和1
    int signal_type; // 0:正弦波, 1:方波, 2:锯齿波, 3:三角波
    float frequency; // 频率
    float amplitude; // 振幅
    float offset; // 偏置
    float duty; // 占空比
    float phase; // 相位
} channel_data;


#endif