#ifndef XYPLATFORMCONSTANTS_HPP
#define XYPLATFORMCONSTANTS_HPP

/* All units are in mm if nothing else is specified */

// Indeed...
#define XY_NUM_AXIS 2

// Since there are no endlimits switches, we make some in software in m
#define X_AXIS_MIN_POS 0.0
#define X_AXIS_MAX_POS 0.2
#define Y_AXIS_MIN_POS 0.0
#define Y_AXIS_MAX_POS 0.2

/* Drive Offset:  In the general case, you're motors will still drift
   although you send 0V to the drives.
   As you can see, we didn't tune them yet
*/
#define XY_OFFSETSinVOLTS { 0.0 , 0.0 }
#define XY_SCALES { 37 , 37 }

// Encoder scale (Updated after calibrating with K600)
//#define XY_PULSES_PER_MM {400,400}
#define XY_PULSES_PER_MM {400,400}

// Encoder offset
#define XY_ENCODEROFFSETS {0,0}

// Joint speedlimits in m/s
#define XY_JOINTSPEEDLIMITS {2.0,2.0}

#endif //XYPLATFORMCONSTANTS_HPP
