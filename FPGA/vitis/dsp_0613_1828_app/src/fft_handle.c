#include "fft_handle.h"
#include "adc_dma_ctrl.h"
#include <math.h>

#include "adc_dma_ctrl.h"

IndexValuePair fft_data_amplitude[FFT_LENGTH];
double fixed_to_double(u32 data, u32 m, u32 n) {
  // 将 signed fixm_n数据转换为double类型
  int sign = (data >> (m - 1)) & 0x1;  // 获取符号位

  // 创建掩码提取有效位（m位）
  u32 mask = (1U << m) - 1;
  int value = data & mask;  // 提取所有m位

  // 如果是负数，进行符号扩展
  if (sign) {
    // 设置高位为1(从m位到31位)
    u32 sign_extension = ~((1U << m) - 1);
    value |= sign_extension;
  }

  // 将定点数转换为浮点数：除以2^n
  double result = (double)value / (1 << n);
  return result;
}

void fft_cal_amp(u32 *data, int length) {
  double real, imag;
  // FFT处理函数
  // 将数据转换为double类型
  for (int i = 0; i < 2 * length; i += 2) {
    real = fixed_to_double(data[i], 30, 15);
    imag = fixed_to_double(data[i + 1], 30, 15);
    // 计算幅值
    fft_data_amplitude[i / 2].index = i / 2;
    fft_data_amplitude[i / 2].value = sqrt(real * real + imag * imag);
  }
  
}

// 排序用的比较函数（降序）
int compare_pairs_desc(const void *a, const void *b) {
  IndexValuePair *pair_a = (IndexValuePair *)a;
  IndexValuePair *pair_b = (IndexValuePair *)b;

  if (pair_b->value > pair_a->value) return 1;
  if (pair_b->value < pair_a->value) return -1;
  return 0;
}

// 对fft_data_amplitude排序并返回降序索引
IndexValuePair *fft_sort_by_amplitude(int length) {
  qsort(fft_data_amplitude, length, sizeof(IndexValuePair), compare_pairs_desc);
  return fft_data_amplitude;
}
