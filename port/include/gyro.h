#ifndef _IN_GYRO_H
#define _IN_GYRO_H

#include <PR/ultratypes.h>

// call after SDL sensors are enabled for this controller slot
void gyroInitController(s32 cidx);

// call before the SDL controller handle is closed
void gyroCloseController(s32 cidx);

// feed raw sensor events (rad/s)
void gyroFeedAngVel(s32 cidx, float gx, float gy, float gz);
void gyroFeedAccel(s32 cidx, float ax, float ay, float az);

// call once per frame
void gyroUpdateAll(void);

// call after changing the auto-calibration mode
void gyroReconfigureCalibrationMode(s32 cidx);

// returns frame-scaled gyro orientation in radians
void gyroGetOrientation(s32 cidx, f32 *yaw, f32 *pitch, f32 *roll);

#endif /* _IN_GYRO_H */
