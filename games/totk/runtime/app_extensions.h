/* Optional application lifecycle hooks. The application supplies these when
   MNG_APP_EXTENSIONS is enabled; the native frontend needs no implementation. */
#pragma once
#import <Foundation/Foundation.h>
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
UIInterfaceOrientationMask runtime_ios_orientations(void);
void runtime_ios_present_launcher(UIViewController *parent, void (^start)(void));
void runtime_ios_environment(int debug_log);
void runtime_ios_layout_boot_overlay(UIView *overlay, UILabel *stage, UILabel *detail, UIProgressView *progress);
#else
void runtime_app_directories(NSString **data, NSString **logs);
int runtime_app_prepare_launch(const char *profile);
void runtime_front_attach(void *view, void (*settings)(void));
void runtime_front_focus_changed(int on);
int runtime_front_settings(void *window);
#endif
