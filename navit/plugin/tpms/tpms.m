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

#define TPMS_OBJC
#import "tpms.h"
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

TPMS *tpmsbtcontroller = NULL;
CBPeripheral *discoveredPeripherals[10];
int countDiscovered=0;

/** C init procedure */
void tpmsbtcontroller_init(void* pbth_arg, RECEIVE_CB recv_cb, char *frontleftaddr, char *frontrightaddr, char *rearleftaddr, char *rearrightaddr, char* name) {
    if(tpmsbtcontroller == NULL) {
        tpmsbtcontroller = [[TPMS alloc] init];
        
        /** Save callbacks */
        tpmsbtcontroller->pbth_arg = pbth_arg;
        tpmsbtcontroller->recv_cb = recv_cb;
        tpmsbtcontroller->name = name;
        tpmsbtcontroller->frontleftaddr =frontleftaddr;
        tpmsbtcontroller->frontrightaddr =frontrightaddr;
        tpmsbtcontroller->rearleftaddr =rearleftaddr;
        tpmsbtcontroller->rearrightaddr =rearrightaddr;
        tpmsbtcontroller->recv_timeout = NULL;
    }
}

@implementation TPMS

@synthesize centralManager;
@synthesize data;
@synthesize pbth_arg;
@synthesize recv_cb;

- (id)init{
    if(tpmsbtcontroller==NULL) {
        centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        data = [[NSMutableData alloc] init];
    }
    return self;
}

-(void)recv_timed_out{
    NSLog(@"TPMS - Receive timeout");
    tpmsbtcontroller->recv_cb(tpmsbtcontroller->pbth_arg, NULL, NULL, NULL);
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    
    // Stop any active scan when power is off
    if (central.state != CBManagerStatePoweredOn) {
        if([centralManager isScanning])
            [centralManager stopScan];
        return;
    }
    
    if (central.state == CBManagerStatePoweredOn) {
        // Scan for BT Headup device
        [centralManager scanForPeripheralsWithServices:nil options:@{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES }];
        NSLog(@"TPMS - Scanning for TPMS started");
    }
}

- (NSString *)getBitStringForInt:(int)value {

    NSString *bits = @"";

    for(int i = 0; i < 8; i ++) {
        bits = [NSString stringWithFormat:@"%i%@", value & (1 << i) ? 1 : 0, bits];
    }

    return bits;
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI {
    
    NSString *peripheralName = [advertisementData objectForKey:@"kCBAdvDataLocalName"];
    NSData *rawData = [advertisementData objectForKey:@"kCBAdvDataManufacturerData"];
    const unsigned char *bytes = (unsigned char *)rawData.bytes;
    
    if (peripheralName) {
        
        
        
        if([peripheralName hasPrefix:[NSString stringWithCString:name encoding:[NSString defaultCStringEncoding]]]) {
            
            if(!strcmp(name, "BR")) {
                NSLog(@"%@", [self getBitStringForInt:bytes[0]]);
                tpmsbtcontroller->recv_cb(tpmsbtcontroller->pbth_arg, [[peripheral.identifier UUIDString] UTF8String], bytes, name);
            } else {
                tpmsbtcontroller->recv_cb(tpmsbtcontroller->pbth_arg, peripheralName.UTF8String, bytes, name);
            }
#if 1    
            
            NSNumber *number = [advertisementData objectForKey:@"kCBAdvDataTimestamp"];
            
            
            
            NSTimeInterval absoluteTime = number.doubleValue;
            
            NSDate *date = [NSDate dateWithTimeIntervalSinceReferenceDate:absoluteTime];
            
            NSLog(@"%@", date);
            
            for (NSString * key in advertisementData){
                
                NSLog(@"%@ %@", key, advertisementData[key]);
            }
            
            NSLog(@"%@ (%@)", peripheralName, peripheral.identifier.UUIDString);
            for(unsigned long i=0;i<data.length;i++) {
                NSLog(@"0x%02x", bytes[i]);
                
            }
#endif
            
        }
    }
    
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    // We don't connect to a peripheral
}

- (void)cleanup {
    
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    // We don't connect to a peripheral
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    // We don't use services and characteristics
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    // We don't use services and characteristics
}

- (void)peripheral:(CBPeripheral *)peripheral
didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
             error:(NSError *)error {
    // We don't use services and characteristics
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    // We dont connect to a peripheral
}

- (void)ondestroy {
    [centralManager stopScan];
}

@end
