#include <iostream>
#include <math.h>
#include <lapacke.h>
#include <memory>

#include "inc/Utils.hpp"
#include "inc/UserFunction.hpp"
#include "inc/DVSTrackerControl.hpp"
#include "inc/DVSTrackerControlWindow.hpp"

using namespace nst;

// Helpers
#define PI 3.1415926535897932384626433832795
#define GI 0.3989422804 // integral of Gaussian, sqrt(1/2/PI)

// Matrices for transformations between skeleton and pixel coordinates
double pxl_to_body_x[128][128];
double pxl_to_body_y[128][128];
double body_to_pxl_x[128][128];
double body_to_pxl_y[128][128];

// Matrix for filtering the noisy green events
bool filter_matrix[128][128];

// Current angles
double blue, red, yellow;

// Change in angles from ridge regression
double delta_blue, delta_red, delta_yellow;

// Derived physical angular velocities (not working yet)
double blue_vel, red_vel, yellow_vel;

// Matrix containing squared error terms for ridge regression. Contains A=X^TX and B X^Ty
double squared_error[4][4];

// Gives the events the color of the segments to which they most likely belong
double event_color = 0.0;

// Tracks the time differences between events
int delta_t = 10000;
double delta_t_vel = 0;
double last_timestamp = 0;
double last_x = 0;
double last_y = 0;
double  last_crx = 0;
double  last_cry = 0;
double last_cyx = 0;
double last_cyy = 0;
double last_targetx = 0;
double last_targety = 0;

double center_red_x = 0;
double center_red_y = 0;

double center_yellow_x = 0;
double center_yellow_y = 0;

double pred_blue_angle = 0;
double pred_red_angle = 0;
double pred_yellow_angle = 0;

double w_prob_b = 0.5;
double w_prob_r = 0.65;
double w_prob_y = 0.9;

double target_x = 0;
double target_y = 0;

// define events batch processing of events or EMA
// #define EVENT_BATCH

// tracker configuration vars
double tracker_target_x = 63.5;
double tracker_target_y = 63.5;

double pxl_to_tracker_x[128][128];
double pxl_to_tracker_y[128][128];

double cal_angle = 0; // while in calibration mode, the tracker describes a circle - this is the current angle of that tracker within the circle
//In the calibration phase, we find the parameters that allow us to trace the perimeter of the skeleton space with the tracker.
//Simplistically, this is the centre of the circle in tracker coordinates:

//The radius, in tracker units:
double tracker_radius = 1570;
//Then there is pitch (how the circle is angled away from perdendicular vertically) and yaw (horizontally).
//It may be that yaw can always be ignored, with good alignment of the devices.
//Pitch is harder to ignore, because one may need a big stack of neural computation back issues to get the tracker to the right height.
//double pitch = 0; // in radians; 0 means that the skeleton is perpendicular to the tracker; positive means that the top of the skeleton is further away from the tracker than the bottom, and v.v.
//Turns out, the tracker circle is slightly wider than it is tall. Compensate for that with this adjustable factor. Assume that tracker_y is fundamental and that x should be adjusted.
double tracker_girth = 1.02; // x = y * tracker_girth
//Finally, we need to be able to convert between angles and the steps of the tracker. This comes from a one-off calibration:
//double tracker_steps_per_radian = 2000;
double blue_event_speed = 0, red_event_speed = 0, yellow_event_speed = 0; // change per event!!! of previous


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  C A L I B R A T I O N
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// segments length in pixels on screen
double segments_len_pxls = 20;

// Center of blue segments = Center of skeleton coordinate system
double cen_x = 64, cen_y = 64;




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                             H Y P E R    P A R A M E T E R S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Exponential moving average for velocities and squared error
double ema_vel_rate   = 0.999;
double ema_error_rate = 0.99;

// Inhibiting params for ridge regression
double lambda_blue   = 64;
double lambda_red    = 32;
double lambda_yellow = 16;

// stddev for the soft EM implementation
const double w = 0.15;

// Likelihood of noise when using soft EM
double p_n = 0.05;

// Noise thresshold distance for hard EM (derived from the soft params this basically sets the distance so that an
// event has to be close enough so that p > 0.05 for the segments)
double n_t = 0.4;




// Calculates the transformation arrays for coordinate transforms between pixel and global coordinates
void init_transformation_arrays(void)
{
    int x,y;
    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 128; x++)
        {
            // x and y  mean pixel coordinates
            pxl_to_body_x[y][x] = (x - cen_x) / segments_len_pxls;
            pxl_to_body_y[y][x] = (y - cen_y) / segments_len_pxls;

            // x and y  mean skeleton coordinates
            body_to_pxl_x[y][x] = x * segments_len_pxls + cen_x;
            body_to_pxl_y[y][x] = y * segments_len_pxls + cen_y;

            // x and y suddenly mean pixels again
            pxl_to_tracker_x[y][x] = -50 + 4096 - y * 32; // how far down, use 4096- if needed
            pxl_to_tracker_y[y][x] = 200 + x * 32; // how far to the right
        }
    }
}


// Coordinate transform from pixel to global skeleton coordinate
void pixel_to_skeleton(double pix_x, double pix_y, double *pen_x, double *pen_y)
{
    int x = (int)pix_x;
    if (x < 0)   x = 0;
    if (x > 126) x = 126;
    int y = (int)pix_y;
    if (y < 0)   y = 0;
    if (y > 126) y = 126;

    // Interpolate the calibration arrays
    *pen_x =   (y + 1 - pix_y) * ( (x + 1 - pix_x) * pxl_to_body_x[y  ][x] + (pix_x - x) * pxl_to_body_x[y  ][x+1] )
             + (pix_y - y)     * ( (x + 1 - pix_x) * pxl_to_body_x[y+1][x] + (pix_x - x) * pxl_to_body_x[y+1][x+1] );
    *pen_y =   (y + 1 - pix_y) * ( (x + 1 - pix_x) * pxl_to_body_y[y  ][x] + (pix_x - x) * pxl_to_body_y[y  ][x+1] )
             + (pix_y - y)     * ( (x + 1 - pix_x) * pxl_to_body_y[y+1][x] + (pix_x - x) * pxl_to_body_y[y+1][x+1] );
}


// Coordinate transform from global skeleton to pixel coordinate
void skeleton_to_pixel(double *pix_x, double *pix_y, double pen_x, double pen_y)
{
    int x = (int)pen_x;
    if (x < 0)   x = 0;
    if (x > 126) x = 126;
    int y = (int)pen_y;
    if (y < 0)   y = 0;
    if (y > 126) y = 126;

    // Interpolate the calibration arrays
    *pix_x =   (y + 1 - pen_y) * ( (x + 1 - pen_x) * body_to_pxl_x[y  ][x] + (pen_x - x) * body_to_pxl_x[y  ][x+1] )
             + (pen_y - y)     * ( (x + 1 - pen_x) * body_to_pxl_x[y+1][x] + (pen_x - x) * body_to_pxl_x[y+1][x+1] );
    *pix_y =   (y + 1 - pen_y) * ( (x + 1 - pen_x) * body_to_pxl_y[y  ][x] + (pen_x - x) * body_to_pxl_y[y  ][x+1] )
             + (pen_y - y)     * ( (x + 1 - pen_x) * body_to_pxl_y[y+1][x] + (pen_x - x) * body_to_pxl_y[y+1][x+1] );
}

double tracker_offset_x = 1878;
double tracker_offset_y = 300;

void skeleton_to_tracker(double *tracker_x, double *tracker_y, double pen_x, double pen_y)
{
    *tracker_x = tracker_offset_x + pen_x / 3 * tracker_radius * tracker_girth;
    *tracker_y = 4095 - tracker_offset_y - tracker_radius - pen_y / 3 * tracker_radius;
} // skeleton_to_tracker

double blue_angle_tracker = -PI/2, red_angle_tracker = -PI/2, yellow_angle_tracker = -PI/2; // -pi/2 = heavy-end-down

// Initializes the variables
void initialize(void)
{
    // No errors to start of with
    int i,j;
    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
        {
            squared_error[i][j] = 0;
        }
    }

    // Initially the skeleton is hanging down vertically and not moving
    blue      = PI/2;
    red       = PI/2;
    yellow    = PI/2;

    delta_blue   = 0.0;
    delta_red    = 0.0;
    delta_yellow = 0.0;

    blue_vel   = 0.0;
    red_vel    = 0.0;
    yellow_vel = 0.0;

    // Intialize the filter maxtrix
    for (i=0; i<128; i++)
    {
        for (j=0; j<128; j++)
        {
            filter_matrix[i][j] = 0;
        }
    }
}


// This performs ridge regression on the squared error matrix to get the delta of the angles
void reg_least_squares(void)
{
    int i,j;

    // Fill A, which is the upper left 3 x 3 par of the squared error matrix
    double matrixA[9];
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            matrixA[3 * i + j] = squared_error[i][j];

    // Add regularization terms to turn least squares into ridge regression (more stable) the slower the segments moves in
    // reality the larger the inhibiting vtrkev->centers[2] = dvs_ev->p+1;alues
    matrixA[0] += lambda_blue;
    matrixA[4] += lambda_red;
    matrixA[8] += lambda_yellow;

    // Fill B, which is the right column of the squared error matrix
    double matrixB[3];
    for (i = 0; i < 3; i++)
        matrixB[i] = squared_error[3][i];

    // Solver which solves A*X = B,
    char uplo       = 'U';  // 'U' if upper triangle of A is stored, 'L' if lower
    integer dim     = 3;    // 3 x 3 matrix
    integer sols    = 1;    // 1 rhs to solve for (1 column in B)
    integer rowsepA = 3;    // leading dimension of A (at least n, at least 1)
    integer rowsepB = 3;    // leading dimension of B (at least n, at least 1)
    integer info    = 0;    // exit code: 0:good, <0: arg -i bad, >0: i x i minor bad
    LAPACK_dposv(&uplo, &dim, &sols, matrixA, &rowsepA, matrixB, &rowsepB, &info);

    // Error output
    if (info)
    {
        static int counter = 0;
        static int message_time = 100;
        counter++;
        if (counter == 1)
        {
            for (i = 0; i < 3; i++)
                for (j = 0; j < 3; j++)
                    printf("%f,%c", squared_error[i][j], j == 5 ? '\n' : ' ');
        }
        if (counter == message_time) { printf("%d errors\n", counter); message_time *= 10; }
        initialize(); // try to fix things for next time
        if (counter > 10) return;
        if (info > 0) printf("200Leading minor of order %i is not positive definite\n", info);
        if (info < 0) printf("Argument %i was invalid\n", info);
        if (counter == 10) printf("Too many errors. Going silent.\n");
    }

    // Get the new delta angles
    delta_blue   = matrixB[0];
    delta_red    = matrixB[1];
    delta_yellow = matrixB[2];

    // Smoothen the angular update or not
    // TODO: Smoothing doesn't really make sense without timestamps
    if (smoothing_is_closed_loop)
    {
        // Update the velocities through smoothing (exponential moving average)
        blue_vel   = ema_vel_rate * blue_vel   + (1 - ema_vel_rate) * delta_blue;
        red_vel    = ema_vel_rate * red_vel    + (1 - ema_vel_rate) * delta_red;
        yellow_vel = ema_vel_rate * yellow_vel + (1 - ema_vel_rate) * delta_yellow;

        // Update the angles with the smoothed velocities
        blue   += blue_vel;
        red    += red_vel;
        yellow += yellow_vel;
    }
    else
    {
        // No smoothing so we can update simply by adding the deltas onto the angles
        blue   += delta_blue;
        red    += delta_red;
        yellow += delta_yellow;

        // tracker update
        double rate_speed = 1;
        double rate_speed_retained = 1 - rate_speed;
        blue_event_speed   = blue_event_speed   * rate_speed_retained + (delta_blue)   * rate_speed;
        red_event_speed    = red_event_speed    * rate_speed_retained + (delta_red)    * rate_speed;
        yellow_event_speed = yellow_event_speed * rate_speed_retained + (delta_yellow) * rate_speed;
        //Now use the velocities to give the new angles for the tracker events
        blue_angle_tracker   += blue_event_speed;
        red_angle_tracker    += red_event_speed;
        yellow_angle_tracker += yellow_event_speed;
    }
}


// Calculates the moving average of the squared distances to the segmentss of the events
void expectation_maximization(double b_x, double b_y, double r_x, double r_y, double y_x, double y_y)
{
    int i,j;

    // The likelihoods for z = b, r, y, n
    double likelihood[4];

    // Used to calculate x*x^T and x*y to increment A and B
    double dist[4];

    // We have to calculate the squared error matrix for each source and then get a weighted average with their
    // Likelihoods
    double seperate_squared_error[4][4][4];
    double combo[4][4];

    // Squared error for z = n
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            seperate_squared_error[0][i][j] = 0;

    // Likelihood for z = n
    likelihood[0] = p_n;

    // Squared error for z = b
    dist[0] = b_x;
    dist[1] = 0;
    dist[2] = 0;
    dist[3] = b_y;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            seperate_squared_error[1][i][j] = dist[i] * dist[j];

    // Likelihood for z = b
    // Likelihood is zero if its out of the reach of the segments, else its calculated with the gaussian likelihood
    if (b_x < -(1.0) || b_x > (1.0))
        likelihood[1] = 0;
    else
        likelihood[1] = GI/w * exp(-b_y*b_y / (2*w*w));

    // Squared error for z = r
    dist[0] = cos(blue - red);
    dist[1] = r_x;
    dist[2] = 0;
    dist[3] = r_y;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            seperate_squared_error[2][i][j] = dist[i] * dist[j];

    // Likelihood for z = r
    // Likelihood is zero if its out of the reach of the segments, else its calculated with the gaussian likelihood
    if (r_x < -(1.0) || r_x > (1.0))
        likelihood[2] = 0;
    else
        likelihood[2] = GI/w * exp(-r_y*r_y / (2*w*w));

    // Squared error for z = y
    dist[0] = cos(blue - yellow);
    dist[1] = cos(red - yellow);
    dist[2] = y_x;
    dist[3] = y_y;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            seperate_squared_error[3][i][j] = dist[i] * dist[j];

    // Likelihood for z = y
    // Likelihood is zero if its out of the reach of the segments, else its calculated with the gaussian likelihood
    if (y_x < -(1.0) || y_x > (1.0))
        likelihood[3] = 0;
    else
        likelihood[3] = GI/w * exp(-y_y*y_y / (2*w*w));

    // Get the max likelihood for coloring the event accordingly
    double max_likelihood = likelihood[0];
    event_color = 0.0;
    for (i = 1; i < 4; i++)
    {
        if (likelihood[i] > max_likelihood)
        {
            event_color = i;
            max_likelihood = likelihood[i];
        }
    }

    // Hard EM only tracks the most likely z. Comment for soft EM
    for (i = 0; i < 4; i++)
    {
        if (i != event_color)
        {
            likelihood[i] = 0;
        }
    }

    // Norm the likelihoods
    double norm = 0;
    int num_sources = 4;
    for (i = 0; i < num_sources; i++)
    {
        norm += likelihood[i];
    }
    for (i = 0; i < num_sources; i++)
    {
        likelihood[i] /= norm;
    }

    // Separately blend each source into estimate, then merge all these new estimates back into one
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            int s;
            combo[i][j] = 0;
            for (s = 0; s < num_sources; s++)
            {
                combo[i][j] += likelihood[s] * seperate_squared_error[s][i][j];
            }
            squared_error[i][j] = ema_error_rate * squared_error[i][j] + (1 - ema_error_rate) * combo[i][j];
        }
    }

    // Perform the regularized least squares
    reg_least_squares();
}


// Filtering
bool event_filter(shared_ptr<DVSEvent> dvs_ev)
{
    bool pass = false;
    if (dvs_ev)
    {
        if (dvs_ev->p == 0)
        {
            // It can definitely pass
            pass = true;

            // Mark it in the filter
            filter_matrix[dvs_ev->x][dvs_ev->y] = 0;
        }
        else if (filter_matrix[dvs_ev->x][dvs_ev->y] == 0)
        {
            // It can pass
            pass = true;

            // Mark this event in the filter
            filter_matrix[dvs_ev->x][dvs_ev->y] = 1;
        }
        else
        {
            // Can't pass and no need to change filter
            pass = false;
        }
    }

    return pass;
}


// Visualize everything
void pedestrian_tracker(DVSTrackerControl * const  control, shared_ptr<DVSEvent> dvs_ev)
{
    static bool init_run = true;
    static struct TrackerEvent *trkev = new TrackerEvent();

    // check initial run for calibration setup
    if(init_run)
    {
        initialize();
        init_transformation_arrays();

        tracker_target_x = tracker_offset_x + sin(cal_angle) * tracker_radius * tracker_girth;
        tracker_target_y = 4095 - tracker_offset_y - tracker_radius + cos(cal_angle) * tracker_radius;
        init_run = false;
    }

    // Align to physical position (turn 90 degrees and mirror)
    if (dvs_ev)
    {
        int temp_y = dvs_ev->y;
        dvs_ev->y = dvs_ev->x;
        dvs_ev->x = 127 - temp_y;
    }

    // If it passed the filter lets go
    if (event_filter(dvs_ev))
    {
        // Blue segments
        double c_r_x = cos(blue);  // red end
        double c_r_y = sin(blue);

        // Red segments
        double c_y_x = c_r_x + cos(red); // yellow end
        double c_y_y = c_r_y + sin(red);


        // for incoming events update tracker display
        // Empty blue segments end
        double blue_x = -cos(blue);
        double blue_y = -sin(blue);

        // Empty red segments end
        double red_x = c_r_x - cos(red);
        double red_y = c_r_y - sin(red);

        // Tracking point
        double c_t_x = c_y_x + cos(yellow);
        double c_t_y = c_y_y + sin(yellow);

        // Empty yellow segments end
        double yellow_x = c_y_x - cos(yellow);
        double yellow_y = c_y_y - sin(yellow);

        // Temporary pixel point for transformations
        double p_x, p_y;

        double ex, ey;

        // Calculate the time difference between the current and the last event
        delta_t = (int)dvs_ev->t - last_timestamp;
        last_timestamp = (int)dvs_ev->t;

        // check the timestamp
        if(delta_t > 0)
        {

            // calculate the velocities
            // yellow
            double t = atan2(c_y_y - c_t_y, c_y_x - c_t_x);
            double last_t = atan2(last_cyy - last_targety, last_cyx - last_targetx);
            yellow_vel = (t - last_t)/delta_t;

            t = atan2(c_r_y - c_y_y, c_r_x - c_y_x);
            last_t = atan2(last_cry - last_cyy, last_crx - last_cyx);
            red_vel = (t - last_t)/delta_t;

            t = atan2(-c_r_y, - c_r_x);
            last_t = atan2(-last_cry, -last_crx);
            blue_vel = (t - last_t)/delta_t;

            // compute the future centers (here replace vars) TODO
            // red
            center_red_x = c_r_x + delta_t*(-blue_vel*sin(blue));
            center_red_y = c_r_y + delta_t*(blue_vel*cos(blue));
            // yellow
            center_yellow_x = c_y_x + delta_t*(-blue_vel*sin(blue) - red_vel*sin(red));
            center_yellow_y = c_y_y + delta_t*(blue_vel*cos(blue) + red_vel*cos(red));

            // predict the angles
            pred_blue_angle = blue + blue_vel*delta_t;
            pred_red_angle = red + red_vel*delta_t;
            pred_yellow_angle = yellow + yellow_vel*delta_t;

            // transform
            pixel_to_skeleton(dvs_ev->x, dvs_ev->y, &ex, &ey);

            // find rectangular coordinates of event for each segments (dist to next hinge, dist to left)
            // prediction based
            double b_x =  cos(pred_blue_angle) * ex + sin(pred_blue_angle) * ey;
            double b_y = -sin(pred_blue_angle) * ex + cos(pred_blue_angle) * ey;

            double r_x =  cos(pred_red_angle) * (ex - center_red_x) + sin(pred_red_angle) * (ey - center_red_y);
            double r_y = -sin(pred_red_angle) * (ex - center_red_x) + cos(pred_red_angle) * (ey - center_red_y);

            double y_x =  cos(pred_yellow_angle) * (ex - center_yellow_x) + sin(pred_yellow_angle) * (ey - center_yellow_y);
            double y_y = -sin(pred_yellow_angle) * (ex - center_yellow_x) + cos(pred_yellow_angle) * (ey - center_yellow_y);


            // track event, here or for each incoming DVS event  in the slot ?
            expectation_maximization(b_x, b_y, r_x, r_y, y_x, y_y);

//            // Get the pixel position of the blue segments
//            skeleton_to_pixel(&p_x, &p_y, c_r_x, c_r_y);
//            trkev->bluesegments[0] = p_x;
//            trkev->bluesegments[1] = p_y;
//            skeleton_to_pixel(&p_x, &p_y, blue_x, blue_y);
//            trkev->bluesegments[2] = p_x;
//            trkev->bluesegments[3] = p_y;

//            // Get the pixel position of the red segments
//            skeleton_to_pixel(&p_x, &p_y, c_y_x, c_y_y);
//            trkev->redsegments[0] = p_x;
//            trkev->redsegments[1] = p_y;
//            skeleton_to_pixel(&p_x, &p_y, red_x, red_y);
//            trkev->redsegments[2] = p_x;
//            trkev->redsegments[3] = p_y;

//            // Get the pixel position of the yellow segments
//            skeleton_to_pixel(&p_x, &p_y, c_t_x, c_t_y);
//            trkev->yellowsegments[0] = p_x;
//            trkev->yellowsegments[1] = p_y;
//            skeleton_to_pixel(&p_x, &p_y, yellow_x, yellow_y);
//            trkev->yellowsegments[2] = p_x;
//            trkev->yellowsegments[3] = p_y;

            //Visualize event for debuging
            trkev->centers[0] = dvs_ev->x;
            trkev->centers[1] = dvs_ev->y;
            trkev->centers[2] = event_color;


            // update the tracker event and emit signal for gui update
            control->setTrackerEvent(trkev);

            // update the tracker parametrization
            blue_x = cos(blue_angle_tracker); // center of red segments
            blue_y = sin(blue_angle_tracker);
            red_x = blue_x + cos(red_angle_tracker); // center of yellow segments
            red_y = blue_y + sin(red_angle_tracker);
            target_x = red_x + cos(yellow_angle_tracker); // end of yellow segments
            target_y = red_y + sin(yellow_angle_tracker);
            }
        last_x = dvs_ev->x;
        last_y = dvs_ev->y;
        last_crx = c_r_x;
        last_cry = c_r_y;
        last_cyx = c_y_x;
        last_cyy = c_y_y;
        last_targetx = c_t_x;
        last_targety = c_t_y;
    }
}


