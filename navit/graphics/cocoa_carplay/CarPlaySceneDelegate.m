//
//  CarPlaySceneDelegate.m
//  TestCarPlayOBJC
//
//  Created by Olaf on 21.08.24.
//

#import "CarPlaySceneDelegate.h"

@interface CarPlaySceneDelegate ()

@end

@implementation CarPlaySceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    // Use this method to optionally configure and attach the UIWindow `window` to the provided UIWindowScene `scene`.
    // If using a storyboard, the `window` property will automatically be initialized and attached to the scene.
    // This delegate does not imply the connecting scene or session are new (see `application:configurationForConnectingSceneSession` instead).
}


- (void)sceneDidDisconnect:(UIScene *)scene {
    // Called as the scene is being released by the system.
    // This occurs shortly after the scene enters the background, or when its session is discarded.
    // Release any resources associated with this scene that can be re-created the next time the scene connects.
    // The scene may re-connect later, as its session was not necessarily discarded (see `application:didDiscardSceneSessions` instead).
}


- (void)sceneDidBecomeActive:(UIScene *)scene {
    // Called when the scene has moved from an inactive state to an active state.
    // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
}


- (void)sceneWillResignActive:(UIScene *)scene {
    // Called when the scene will move from an active state to an inactive state.
    // This may occur due to temporary interruptions (ex. an incoming phone call).
}


- (void)sceneWillEnterForeground:(UIScene *)scene {
    // Called as the scene transitions from the background to the foreground.
    // Use this method to undo the changes made on entering the background.
}


- (void)sceneDidEnterBackground:(UIScene *)scene {
    // Called as the scene transitions from the foreground to the background.
    // Use this method to save data, release shared resources, and store enough scene-specific state information
    // to restore the scene back to its current state.
}

- (void)templateApplicationScene:(CPTemplateApplicationScene *)templateApplicationScene didConnectInterfaceController:(CPInterfaceController *)interfaceController {
    
}

- (void)templateApplicationScene:(CPTemplateApplicationScene *)templateApplicationScene didDisconnectInterfaceController:(CPInterfaceController *)interfaceController {
    
}

- (void)buttonClicked:(CPBarButton *)sender {
   // Do your logic here
}

- (void) templateApplicationScene:(CPTemplateApplicationScene *) templateApplicationScene
    didConnectInterfaceController:(CPInterfaceController *) interfaceController
                         toWindow:(CPWindow *) window {
    self.interfaceController = interfaceController;
    self.window = window;
    self.carPlayMapTemplate = [[CPMapTemplate alloc]init];
    
    CPBarButton * buttonone = [[CPBarButton alloc] initWithTitle:@"Pan" handler:^(CPBarButton * _Nonnull barButton) {
        
        NSLog(@"buttonone clicked");

    }];
    CPBarButton * buttontwo = [[CPBarButton alloc] initWithTitle:@"Set Destination" handler:^(CPBarButton * _Nonnull barButton) {
        
        NSLog(@"buttontwo clicked");

    }];
    CPBarButton * buttonthree = [[CPBarButton alloc] initWithTitle:@"Find Gas Station" handler:^(CPBarButton * _Nonnull barButton) {
        
        NSLog(@"buttonthree clicked");

    }];
    CPBarButton * buttonfour = [[CPBarButton alloc] initWithTitle:@"Stop Navigation" handler:^(CPBarButton * _Nonnull barButton) {
        
        NSLog(@"buttonfour clicked");

    }];
    
    self.carPlayMapTemplate.leadingNavigationBarButtons = [NSArray arrayWithObjects: buttonone, buttontwo, nil];
    self.carPlayMapTemplate.trailingNavigationBarButtons = [NSArray arrayWithObjects: buttonthree, buttonfour, nil];
    self.carPlayMapTemplate.mapDelegate = self;
    
    window.rootViewController = [[CarPlayViewController alloc] init ];
    
    [interfaceController setRootTemplate:self.carPlayMapTemplate
                                    animated:YES
                                  completion:NULL];

    }

- (void) templateApplicationScene:(CPTemplateApplicationScene *) templateApplicationScene
 didDisconnectInterfaceController:(CPInterfaceController *) interfaceController
                       fromWindow:(CPWindow *) window {
    self.interfaceController = interfaceController;
    self.window = window;
    self.carPlayMapTemplate = [CPMapTemplate init];
    self.carPlayMapTemplate.leadingNavigationBarButtons = [NSArray arrayWithObjects: [[CPBarButton alloc] initWithType:CPBarButtonTypeText handler:nil], nil];
}

- (void)okButtonTapped:(CPBarButton *)sender {
NSLog(@"Ok button was tapped: dismiss the view controller.");
}

@end
