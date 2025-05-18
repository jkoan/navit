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

#define BTHEADUP_OBJC
#import "btheadup.h"
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

BTHeadup *bthbtcontroller = NULL;
CBPeripheral *discoveredPeripherals[10];
int countDiscovered=0;

/** C init procedure */
void bthbtcontroller_init(void* pbth_arg, DISCONNECT_CB disconnect_cb, CONNECT_CB connect_cb, char *devicename) {
    if(bthbtcontroller == NULL) {
        bthbtcontroller = [[BTHeadup alloc] init];
        
        /** Save callbacks */
        bthbtcontroller->pbth_arg = pbth_arg;
        bthbtcontroller->disconnect_cb = disconnect_cb;
        bthbtcontroller->connect_cb = connect_cb;
        bthbtcontroller->devicename = devicename;
    }
}

/** C send procedure */
void bthbtcontroller_send(unsigned char* data, int length) {

        NSData * d = [NSData dataWithBytes:data length:length];
        [bthbtcontroller.discoveredPeripheral writeValue:d forCharacteristic:bthbtcontroller.characteristic type:CBCharacteristicWriteWithoutResponse];

}

@implementation BTHeadup

@synthesize centralManager;
@synthesize discoveredPeripheral;
@synthesize data;
@synthesize characteristic;

- (id)init{
    if(bthbtcontroller==NULL) {
        centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        data = [[NSMutableData alloc] init];
        characteristic = NULL;
        discoveredPeripheral = NULL;
    }
    return self;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {

    // Stop any active scan when power is off
    if (central.state != CBManagerStatePoweredOn) {
        if([centralManager isScanning])
            [centralManager stopScan];
        if(bthbtcontroller->disconnect_cb)
            if(bthbtcontroller->pbth_arg)
                bthbtcontroller->disconnect_cb(bthbtcontroller->pbth_arg);
        self.discoveredPeripheral=NULL;
        self.characteristic = NULL;
        return;
    }
    
    if (central.state == CBManagerStatePoweredOn) {
        // Scan for BT Headup device
        [centralManager scanForPeripheralsWithServices:@[[CBUUID UUIDWithString:NUS_SERVICE_UUID]] options:@{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES }];
        NSLog(@"BTHEADUP - Scanning for BTHEADUP started");
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI {
    
    NSLog(@"BTHEADUP - Discovered %@ at %@dBm", peripheral.name, RSSI);
    
    NSLog(@"BTHEADUP - RSSI: %idBm", [RSSI intValue]);
    
    const char* utf8Str = [peripheral.name UTF8String];
    if(!strcmp(utf8Str , devicename)) {
        NSLog(@"BTHEADUP - Found devicename: %s", [peripheral.name UTF8String]);
        
        
        if (discoveredPeripheral != peripheral) {
            self.discoveredPeripheral = peripheral;
            [centralManager stopScan];
            
            // Connect
            NSLog(@"BTHEADUP - Connecting to peripheral %@", peripheral);
            [centralManager connectPeripheral:peripheral options:nil];
        }
        
    }
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    NSLog(@"BTHEADUP - Failed to connect");
    [self cleanup];
}

- (void)cleanup {
    
    // See if we are subscribed to a characteristic on the peripheral
    if (discoveredPeripheral.services != nil) {
        for (CBService *service in discoveredPeripheral.services) {
            if (service.characteristics != nil) {
                for (CBCharacteristic *characteristic in service.characteristics) {
                    if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:NUS_CHARACTERISTIC_RX]]) {
                        if (characteristic.isNotifying) {
                            [discoveredPeripheral setNotifyValue:NO forCharacteristic:characteristic];
                            return;
                        }
                    }
                }
            }
        }
    }
    
    [centralManager cancelPeripheralConnection:discoveredPeripheral];
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    NSLog(@"BTHEADUP - Connected");
    
    [centralManager stopScan];
    NSLog(@"BTHEADUP - Scanning stopped");
    
    [data setLength:0];
    
    peripheral.delegate = self;
    
    [peripheral discoverServices:@[[CBUUID UUIDWithString:NUS_SERVICE_UUID]]];
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    if (error) {
        [self cleanup];
        return;
    }
    
    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics:@[[CBUUID UUIDWithString:NUS_CHARACTERISTIC_RX]] forService:service];
    }
    // Discover other characteristics
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    
    NSLog(@"BTHEADUP - DIS FOUND");
    
    if (error) {
        [self cleanup];
        return;
    }
    
    for (CBCharacteristic *characteristic in service.characteristics) {
        if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:NUS_CHARACTERISTIC_RX]]) {
            self.characteristic = characteristic;
            
            // inform btheadup that we are connected
            if(bthbtcontroller->connect_cb) {
                bthbtcontroller->connect_cb(bthbtcontroller->pbth_arg);
            }
            NSLog(@"BTHEADUP -  Connected");
        }
    }
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    discoveredPeripheral = nil;
    // inform btheadup that we are disconnected
    if(bthbtcontroller->disconnect_cb) {
        bthbtcontroller->disconnect_cb(bthbtcontroller->pbth_arg);
    }
    NSLog(@"BTHEADUP -  Disonnected");
    [centralManager scanForPeripheralsWithServices:@[[CBUUID UUIDWithString:NUS_SERVICE_UUID]] options:@{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES }];
}

- (void)ondestroy {
    [centralManager stopScan];
}

- (void) sendData:(unsigned char *)data withSize:(int)size {
    for(int i = 0; i < size; i++) {
        NSData * d = [NSData dataWithBytes:&data[i++] length:1] ;
        [discoveredPeripheral writeValue:d forCharacteristic:characteristic type:CBCharacteristicWriteWithoutResponse];
    }
}

@end
