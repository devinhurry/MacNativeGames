/* The phone gyro's hand-over API, kept by the iPhone / iPad motion aiming (phone_motion.m).
   Main thread unless noted. The Settings switch and the app wiring stay phone_motion's own (TOTK_PHONE_GYRO). */
#import <Foundation/Foundation.h>

void totk_device_motion_note_touch(void);      /* the touch layer saw a finger: the phone's gyro takes the aiming over */

#ifdef MNG_MOTION_FILTER
#import <UIKit/UIKit.h>
void totk_phone_motion_start(void);
void totk_device_motion_bind_view(UIView *view); /* iOS 27+: Core Motion follows this display's view axes */
void totk_device_motion_geometry_changed(UIInterfaceOrientation orientation);
void totk_phone_motion_source_changed(void);
#endif
