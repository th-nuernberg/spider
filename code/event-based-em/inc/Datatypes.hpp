#ifndef __DVSTRACKERTYPES_HPP__FA25AD5E_D235_4C44_88AE_DCAF7423C619
#define __DVSTRACKERTYPES_HPP__FA25AD5E_D235_4C44_88AE_DCAF7423C619

#include <ostream>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <bitset>
#include <memory>
#include <QString>
#include <array>

using namespace std;

namespace nst {

  class DVSTrackerControl;

#define SKELETON_CONFIG                       4   // skeleton segments defined by 4 points (start+end coord in 2D)

  /**
 * struct DVSEvent - A single DVS event.
 *
 */
  struct DVSEvent {
    uint8_t id;
    uint64_t t;
    uint16_t x, y;
    uint8_t  p;

    typedef enum {
      TIMEFORMAT_0BYTES,
      TIMEFORMAT_2BYTES,
      TIMEFORMAT_3BYTES,
    } timeformat_t;
  };

  /**
 * struct TrackerEvent - a single estimate of the tracker algorithm
 */
  struct TrackerEvent{
    // refers to the end of the yellow segment
    array <double, SKELETON_CONFIG> yellowsegments;
    // upper skeleton segments centers, red and blue segment
    array<double, SKELETON_CONFIG> redsegments;
    array<double, SKELETON_CONFIG> bluesegments;
    // centers of the segments
    array<double,3> centers;
  };

  /**
 * A user function
 */
  struct UserFunction {
    const char *name;
    void (*fn)(DVSTrackerControl * const control,
               std::shared_ptr<DVSEvent> dvs_ev);
  };

}

#endif /* __DVSTRACKERTYPES_HPP__FA25AD5E_D235_4C44_88AE_DCAF7423C619 */

