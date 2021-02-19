#import <UIKit/UIKit.h>

@interface IOSViewController : UIViewController
@end

@interface IOSAppDelegate : UIResponder<UIApplicationDelegate>
@end

@interface IOSSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property (strong, nonatomic) UIWindow *  _Nullable window;
@end

int IOSMain(int argc, char* _Nullable * _Nonnull argv);


