/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2022 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#define GOOGLEGLASS_OBJC
#import "googleglass.h"
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

GOOGLEGLASS *bthbtcontrollergg1 = NULL;
CBPeripheral *discoveredPeripherals[10];
int countDiscovered=0;
bool qready = false;
dispatch_queue_t dpq;

/** C init procedure */
void bthbtcontrollergg1_init(void* pbth_arg, DISCONNECT_CB disconnect_cb, CONNECT_CB connect_cb, char *deviceserial) {
    if(bthbtcontrollergg1 == NULL) {
        bthbtcontrollergg1 = [[GOOGLEGLASS alloc] init];
        
        /** Save callbacks */
        bthbtcontrollergg1->pbth_arg = pbth_arg;
        bthbtcontrollergg1->disconnect_cb = disconnect_cb;
        bthbtcontrollergg1->connect_cb = connect_cb;
        bthbtcontrollergg1->deviceserial = deviceserial;
    }
}

/** C send procedure */
void bthbtcontrollergg1_send(unsigned char* data, int length) {
//        [bthbtcontrollergg1.discoveredPeripheral writeValue:d forCharacteristic:bthbtcontrollergg1.characteristic type:CBCharacteristicWriteWithoutResponse];
    int i=0;
    int curlen = length;
    while(i<length) {
        NSData * d = [NSData dataWithBytes:&data[i] length:curlen<20?curlen:20] ;
        dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_async(queue, ^{
            qready = [bthbtcontrollergg1.peripheralManager updateValue:d forCharacteristic:bthbtcontrollergg1.rxcharacteristic onSubscribedCentrals:nil];
        });
        
        if(qready) {
            //NSLog(@"Sent: %@ %i", d, qready);
            i+=20;
            curlen-=20;
            usleep(50000);
        } else {
            usleep(50000);
        }
    }
}

@implementation GOOGLEGLASS

@synthesize peripheralManager;
@synthesize data;
@synthesize rxcharacteristic;
@synthesize txcharacteristic;
@synthesize serialcharacteristic;
@synthesize nusservice;
@synthesize centrals;
@synthesize pbth_arg;
@synthesize disconnect_cb;
@synthesize connect_cb;
@synthesize deviceserial;

- (id)init{
    if(bthbtcontrollergg1==NULL) {
        dpq = dispatch_queue_create(nil, nil);
        peripheralManager = [[CBPeripheralManager alloc] initWithDelegate:self queue:dpq ]; //options:@{ CBCentralManagerOptionRestoreIdentifierKey:
                                                                                              //          @"myCentralManagerIdentifier" }];
        data = [[NSMutableData alloc] init];
        
    }
    return self;
}



- (void)peripheralManager:(CBPeripheralManager *)peripheral
                  central:(CBCentral *)central
didSubscribeToCharacteristic:(CBCharacteristic *)characteristic {
    
    [peripheral setDesiredConnectionLatency:CBPeripheralManagerConnectionLatencyLow forCentral:central];
    
    // inform controller that we are connected
    if(bthbtcontrollergg1->connect_cb) {
        bthbtcontrollergg1->connect_cb(bthbtcontrollergg1->pbth_arg);
        NSLog(@"GOOGLEGLASS - Connected - %li", (unsigned long)[central maximumUpdateValueLength]);
    }
    
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral
                  central:(CBCentral *)central
didUnsubscribeFromCharacteristic:(nonnull CBCharacteristic *)characteristic {
    // inform controller that we are disconnected
    if(bthbtcontrollergg1->disconnect_cb) {
        bthbtcontrollergg1->disconnect_cb(bthbtcontrollergg1->pbth_arg);
        
    }
    [peripheralManager startAdvertising:@{ CBAdvertisementDataServiceUUIDsKey: @[nusservice.UUID]}];
    NSLog(@"GOOGLEGLASS - Disconnected");
}

- (void)peripheralManagerIsReadyToUpdateSubscribers:(CBPeripheralManager *)peripheral {
    NSLog(@"QUEUE ready");
    qready=true;
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral
            didAddService:(CBService *)service
                    error:(NSError *)error {
    if(error == nil) {
        NSLog(@"Service %@ added.", service.description);
        [peripheralManager startAdvertising:@{ CBAdvertisementDataServiceUUIDsKey: @[service.UUID]}];
    } else
        NSLog(@"Failed to add Service %@!", service.description);
}

//- (void)peripheralManager:(CBPeripheralManager *)peripheral willRestoreState:(NSDictionary<NSString *,id> *)dict {
//    if(!peripheralManager.isAdvertising)
//        [peripheralManager startAdvertising:@{CBAdvertisementDataLocalNameKey: @"GGHeadup",
//                                         CBAdvertisementDataServiceUUIDsKey: @[GG1_SERVICE_UUID]}];
//}

- (void)peripheralManagerDidUpdateState:(CBPeripheralManager *)peripheral {
    switch (peripheral.state)
    {
        case CBManagerStatePoweredOff:
            [peripheralManager stopAdvertising];
            NSLog(@"CBManagerStatePoweredOff");
            if(bthbtcontrollergg1->disconnect_cb) {
                bthbtcontrollergg1->disconnect_cb(bthbtcontrollergg1->pbth_arg);
            }
            
            break;
        case CBManagerStatePoweredOn:
            rxcharacteristic = [[CBMutableCharacteristic alloc] initWithType:[CBUUID UUIDWithString:GG1_CHARACTERISTIC_RX]
                                                                properties:CBCharacteristicPropertyNotify|CBCharacteristicPropertyRead value:nil
                                                               permissions:CBAttributePermissionsReadEncryptionRequired];
            txcharacteristic = [[CBMutableCharacteristic alloc] initWithType:[CBUUID UUIDWithString:GG1_CHARACTERISTIC_TX]
                                                                properties:CBCharacteristicPropertyWrite|CBCharacteristicPropertyWrite value:nil
                                                               permissions:CBAttributePermissionsWriteEncryptionRequired
            ];
            serialcharacteristic = [[CBMutableCharacteristic alloc] initWithType:[CBUUID UUIDWithString:GG1_CHARACTERISTIC_SERIAL]
                                                                properties:CBCharacteristicPropertyRead value:nil
                                                               permissions:CBAttributePermissionsReadable];
    
            nusservice = [[CBMutableService alloc] initWithType:[CBUUID UUIDWithString:GG1_SERVICE_UUID]
                                                                               primary:YES];
            
            [nusservice setCharacteristics:@[rxcharacteristic, txcharacteristic, serialcharacteristic]];
            
            [peripheralManager addService:nusservice];
            
            NSLog(@"CBManagerStatePoweredOn");
            break;
        case CBManagerStateResetting:
            NSLog(@"CBManagerStateResetting");
            break;
        case CBManagerStateUnauthorized:
            NSLog(@"CBManagerStateUnauthorized");
            break;
        case CBManagerStateUnknown:
            NSLog(@"CBManagerStateUnknown");
            break;
        case  CBManagerStateUnsupported:
            NSLog(@"CBManagerStateUnsupported");
            break;
        default:
            break;
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral
  didReceiveWriteRequests:(NSArray<CBATTRequest *> *)requests {
    //Central wants to write
    //Should not occur
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral
  didReceiveReadRequest:(nonnull CBATTRequest *)request {
    
    
    NSLog(@"UUID: %@", request.characteristic.UUID);
    
    if ([request.characteristic.UUID isEqual: [CBUUID UUIDWithString:GG1_CHARACTERISTIC_SERIAL]]) {
        
        
        NSString* str = [NSString stringWithUTF8String: bthbtcontrollergg1->deviceserial];
        NSData *d = [str dataUsingEncoding:NSUTF8StringEncoding];
        
        serialcharacteristic.value = d;
        
        if (request.offset > serialcharacteristic.value.length) {
            [peripheral respondToRequest:request withResult:CBATTErrorInvalidOffset];
            return;
        }
        
        request.value = [serialcharacteristic.value subdataWithRange:NSMakeRange(request.offset, serialcharacteristic.value.length - request.offset)];
    }
    
    [peripheral respondToRequest:request withResult: CBATTErrorSuccess];
}

/**
 * @brief       Tells the delegate the peripheral manager started advertising the local peripheral device’s data.
 * @param[in]   peripheral - The peripheral manager that is starting advertising.
 *              error - The reason the call failed, or nil if no error occurred.
 * @return      nothing
 *
 * Tells the delegate the peripheral manager started advertising the local peripheral device’s data.
 */
- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager *)peripheral
                                       error:(NSError *)error {
    if(error == nil)
        NSLog(@"GG1 - peripheralManager started advertising");
    else
        NSLog(@"GG1 - peripheralManager failed to start advertising: %@", error);
    
}

- (void) sendData:(unsigned char *)data withSize:(int)size {
    if(rxcharacteristic != nil && centrals.count>0) {
        for(int i = 0; i < size; i++) {
            NSData * d = [NSData dataWithBytes:&data[i++] length:1] ;
            [peripheralManager updateValue:d forCharacteristic:rxcharacteristic onSubscribedCentrals:centrals];
            //NSLog(@"Sending: %@", d);
        }
    } else {
        NSLog(@"GG1 - rxcharacteristic is null, or no central has subscribed yet: %p, %lu", rxcharacteristic, (unsigned long)centrals.count);
    }
}

@end
