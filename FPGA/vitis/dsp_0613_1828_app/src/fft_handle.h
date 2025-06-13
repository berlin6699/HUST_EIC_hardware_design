#ifndef _FFT_HANDLE_H_
#define _FFT_HANDLE_H_
#include "xil_types.h"
typedef struct {
  int index;
  double value;
} IndexValuePair;

extern IndexValuePair fft_data_amplitude[];
double fixed_to_double(u32 data, u32 m, u32 n);
void fft_cal_amp(u32 *data, int length);
IndexValuePair *fft_sort_by_amplitude(int length);
#endif
