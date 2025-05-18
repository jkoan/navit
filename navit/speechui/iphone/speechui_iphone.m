//
//  sirispeechui.m
//  navit
//
//  Created by olf on 31.07.22.
//

#import <Foundation/Foundation.h>
#import <Intents/Intents.h>
//#import "URActionIntent.h"

//- (void)userTriggeredAnAction:(DataAction*)action
//{
//    // Constructor intent and set parameters
//    // IMPORTANT: Parameters must match one of the shortcut types that you defined in the intents definition file. Otherwise the donation with fail.
//    URActionIntent* intent = [[URActionIntent alloc] init];
//    intent.remote = self.remote.ID;
//    intent.remoteName = self.remote.name;
//    intent.action = action.name;
//    intent.actionName = [[action.name stringByReplacingOccurrencesOfString:@"_" withString:@" "] capitalizedString];
//
//    // Set a suggested phrase (displayed when creating shortcuts)
//    intent.suggestedInvocationPhrase = [NSString stringWithFormat:@"%@ %@", intent.remoteName, intent.actionName];
//
//    // Donate the interaction shortcut
//    INInteraction* interaction = [[INInteraction alloc] initWithIntent:intent response:nil];
//    [interaction donateInteractionWithCompletion:^(NSError * _Nullable error)
//    {
//        if (error)
//        {
//            NSLog(@"Failed to donate interaction: %@ ", [error localizedDescription] );
//        }
//    }];
//}

