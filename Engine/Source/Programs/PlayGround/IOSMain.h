#import <UIKit/UIKit.h>

@interface IOSViewController : UIViewController
@end

@interface IOSAppDelegate : UIResponder<UIApplicationDelegate>
@end

@interface IOSSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property (strong, nonatomic) UIWindow * window;
@end

int IOSMain(int argc, const char** argv);


