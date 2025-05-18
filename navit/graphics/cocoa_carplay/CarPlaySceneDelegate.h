//
//  CarPlaySceneDelegate.h
//  TestCarPlayOBJC
//
//  Created by Olaf on 21.08.24.
//

#import <UIKit/UIKit.h>
#import <CarPlay/CarPlay.h>
#import "CarPlayViewController.h"

@interface CarPlaySceneDelegate : UIResponder <CPTemplateApplicationSceneDelegate, CPMapTemplateDelegate>

@property (strong, nonatomic) CPWindow * window;
@property (strong, nonatomic) CPInterfaceController * interfaceController;
@property (strong, nonatomic) CPBarButton * button;
@property (strong, nonatomic) CPMapTemplate * carPlayMapTemplate;
@property (strong, nonatomic) CarPlayViewController * carPlayViewController;

@end

