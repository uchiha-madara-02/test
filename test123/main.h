#ifndef MAIN_H
#define MAIN_H
#define NA 1
#define EU 0
#define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)));
#define DEBUG 0
#define DEBUGP(x) if (DEBUG == 1) { x ; }
#define NOP __asm__ __volatile__ ("nop")
#define DELAY_CNT 25
#define freq_to_timerval(x) (x / 1000)
struct IrCode {
  uint8_t timer_val;
  uint8_t numpairs;
  uint8_t bitcompression;
  const uint16_t *times;
  const uint8_t *codes;
};

#endif
