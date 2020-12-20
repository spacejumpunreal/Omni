#import "IOSMain.h"

@implementation IOSViewController
@end

@implementation IOSAppDelegate
@end

@implementation IOSSceneDelegate
@end

int IOSMain(int argc, const char** argv)
{
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([IOSAppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
