#ifndef LiASCONSTANTS_HPP
#define LiASCONSTANTS_HPP

// Number of axis
#define LiAS_NUM_AXIS 6

// These are the offsets of the encoders when they are set to zero in the reference position.
// They are taken from the manual
#define LiAS_ENCODEROFFSETS { 0, 0, 0, 0, 0, 0 }

// These conversion factors are based on the reduction values taken from the manuel
#define LiAS_CONV1 200 
#define LiAS_CONV2 200
#define LiAS_CONV3 200
#define LiAS_CONV4 101
#define LiAS_CONV5 100
#define LiAS_CONV6 101

// Resolution of the encoders
#define LiAS_ENC_RES  2000

// Conversion from encoder ticks to radiants
#define LiAS_TICKS2RAD { 2*M_PI / (LiAS_CONV1 * LiAS_ENC_RES), 2*M_PI / (LiAS_CONV2 * LiAS_ENC_RES), 2*M_PI / (LiAS_CONV3 * LiAS_ENC_RES), 2*M_PI / (LiAS_CONV4 * LiAS_ENC_RES), 2*M_PI / (LiAS_CONV5 * LiAS_ENC_RES), 2*M_PI / (LiAS_CONV6 * LiAS_ENC_RES)}

// Joint position limits
#define LiAS_JOINTPOSITIONLIMITS { 4./2., 2./2., 4./2., 7.5/2., 4./2., 9./2. }

// Joint speed limits
#define LiAS_JOINTSPEEDLIMITS { 1.0, 1.0, 2.0, 2.0, 2.0, 3.0 }

// Conversion from angular speed to voltage
#define LiAS_RADproSEC2VOLT { 3.0, 4.0, 3.0, 3.0, 2.355, 2.2 } // NOT CORRECT!

// Offsets in volts to minimalize drift
#define LiAS_OFFSETSinVOLTS { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}

// Channels on IP_Digital_24 for controlling the brake and amplifier enable
#define LiAS_ENABLE_CHANNEL 0
#define LiAS_BRAKE_CHANNEL  1



#endif
