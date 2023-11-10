#ifndef __UTILS_HPP__B69DECA2_4EA9_41CC_9E83_A9BBBB6E06C6
#define __UTILS_HPP__B69DECA2_4EA9_41CC_9E83_A9BBBB6E06C6

#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <memory>

/**
  * Various macros useful in algorithms
  */
#define LENGTH(X) (sizeof X / sizeof X[0])
#define DVS_DATA_RATE 12000000 // baud
#define integer int

#define EVENT_FILENAME_SIZE          60 // chars
#define CONTROL_LOOP_TS              10 // ms
#define VISUAL_UPDATE                10 // ms
#define DEF_REPLAY_DELAY              1 // ms

// delay for replay
extern unsigned long replayDelay; //ms

#endif /* __UTILS_HPP__B69DECA2_4EA9_41CC_9E83_A9BBBB6E06C6 */

