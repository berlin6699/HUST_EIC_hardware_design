#include "freq_meas.h"
// #include "usart_hmi.h"

double freq_meas(void) {
  int threshold = 800;
  int times = 0;
  WRITE_THRESHOLD(threshold);
  WRITE_TIMES(times);
  sleep(1);
  u32 count = READ_CNT() + 1;
  if (count < 1e6) {
    times = 1e6 / count;
  }
  WRITE_TIMES(times);
  sleep(1);
  count = READ_CNT() + 1;
  double frequency = (10000000.0 * (times + 1)) / (double)count;
  printf("Frequency count: %d\r\n", count);
  printf("Frequency: %f Hz\r\n", frequency);
  return frequency;
}
