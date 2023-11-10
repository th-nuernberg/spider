#ifndef __USERFUNCTION_HPP__9C1037A_2D7F_456C_B919_BF491C99A569
#define __USERFUNCTION_HPP__9C1037A_2D7F_456C_B919_BF491C99A569

#include <memory>
#include "Datatypes.hpp"

using namespace std;
using namespace nst;

void init_calibration_arrays(void);

// maps from different spaces of representation
void pixel_to_skeleton(double pix_x, double pix_y, double *pen_x, double *pen_y);
void skeleton_to_pixel(double *pix_x, double *pix_y, double pen_x, double pen_y);
void skeleton_to_tracker(double *lsr_x, double *lsr_y, double pen_x, double pen_y);

// init state vector
void init_pen_state();

// uses squared_error to compute color_angle and color_speed vars
void ridge_regression(void) ;

// (called many times per evt, once per dvs_e)
void track_event(shared_ptr<DVSEvent> dvs_e);

/*
 *  Each user function will be called at least every TS ms
 */
void pedestrian_tracker(DVSTrackerControl * const control, shared_ptr<DVSEvent> dvs_ev);

static const UserFunction user_functions[] = {
    {"Pedestrian detection and tracking algorithm",  pedestrian_tracker}
};


#endif /* __USERFUNCTION_HPP__9C1037A_2D7F_456C_B919_BF491C99A569 */
