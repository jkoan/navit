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

#define OBD2_OBJC
#import "obd2.h"
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

OBD2 *obd2btcontroller = NULL;
CBPeripheral *discoveredPeripherals[10];
int countDiscovered=0;

/** C init procedure */
void obd2btcontroller_init(void* pbth_arg, DISCONNECT_CB disconnect_cb, CONNECT_CB connect_cb, RECEIVE_CB recv_cb, char *devicename) {
    if(obd2btcontroller == NULL) {
        obd2btcontroller = [[OBD2 alloc] init];
        
        /** Save callbacks */
        obd2btcontroller->pbth_arg = pbth_arg;
        obd2btcontroller->disconnect_cb = disconnect_cb;
        obd2btcontroller->connect_cb = connect_cb;
        obd2btcontroller->recv_cb = recv_cb;
        obd2btcontroller->devicename = devicename;
        obd2btcontroller->recv_timeout = NULL;
    }
}

/** C send procedure */
void obd2btcontroller_send(char* data, unsigned long length) {
    
    NSData * d = [NSData dataWithBytes:data length:length];
    [obd2btcontroller.discoveredPeripheral writeValue:d forCharacteristic:obd2btcontroller.characteristic_tx type:CBCharacteristicWriteWithoutResponse];
    
//    [NSThread sleepForTimeInterval:0.005f];
    
    obd2btcontroller->recv_timeout = [NSTimer scheduledTimerWithTimeInterval:2.0 target:obd2btcontroller selector:@selector(recv_timed_out) userInfo:nil repeats:NO];
//
//    if(obd2btcontroller.characteristic != NULL)
//        [obd2btcontroller.discoveredPeripheral readValueForCharacteristic:obd2btcontroller.characteristic];
}

@implementation OBD2

@synthesize centralManager;
@synthesize discoveredPeripheral;
@synthesize data;
@synthesize characteristic;
@synthesize characteristic_tx;
@synthesize connect_cb;
@synthesize devicename;
@synthesize disconnect_cb;
@synthesize pbth_arg;
@synthesize recv_cb;

- (id)init{
    if(obd2btcontroller==NULL) {
        centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        data = [[NSMutableData alloc] init];
        characteristic = NULL;
        discoveredPeripheral = NULL;
    }
    return self;
}

-(void)recv_timed_out{
    NSLog(@"OBD2 - Receive timeout");
    obd2btcontroller->recv_cb(obd2btcontroller->pbth_arg, NULL);
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    
    // Stop any active scan when power is off
    if (central.state != CBManagerStatePoweredOn) {
        if([centralManager isScanning])
            [centralManager stopScan];
        if(obd2btcontroller->disconnect_cb)
            if(obd2btcontroller->pbth_arg)
                obd2btcontroller->disconnect_cb(obd2btcontroller->pbth_arg);
        self.discoveredPeripheral=NULL;
        self.characteristic = NULL;
        return;
    }
    
    if (central.state == CBManagerStatePoweredOn) {
        // Scan for BT Headup device
        [centralManager scanForPeripheralsWithServices:@[[CBUUID UUIDWithString:OBD2_SERVICE_UUID]] options:@{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES }];
        NSLog(@"OBD2 - Scanning for OBD2 started");
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI {
    
    NSLog(@"OBD2 - Discovered %@ at %@dBm", peripheral.name, RSSI);
    
    NSLog(@"OBD2 - RSSI: %idBm", [RSSI intValue]);
    
//    const char* utf8Str = [peripheral.name UTF8String];
    //    if(!strcmp(utf8Str , devicename)) {
    NSLog(@"OBD2 - Found devicename: %s", [peripheral.name UTF8String]);
    
    if (discoveredPeripheral != peripheral) {
        self.discoveredPeripheral = peripheral;
        [centralManager stopScan];
        
        // Connect
        NSLog(@"OBD2 - Connecting to peripheral %@", peripheral);
        [centralManager connectPeripheral:peripheral options:nil];
    }
    
    //    }
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    NSLog(@"OBD2 - Failed to connect");
    [self cleanup];
}

- (void)cleanup {
    
    // See if we are subscribed to a characteristic on the peripheral
    if (discoveredPeripheral.services != nil) {
        for (CBService *service in discoveredPeripheral.services) {
            if (service.characteristics != nil) {
                for (CBCharacteristic *characteristic in service.characteristics) {
                    if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:OBD2_CHARACTERISTIC_RX]]) {
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
    NSLog(@"OBD2 - Connected");
    
    [centralManager stopScan];
    NSLog(@"OBD2 - Scanning stopped");
    
    [data setLength:0];
    
    peripheral.delegate = self;
    
    [peripheral discoverServices:@[[CBUUID UUIDWithString:OBD2_SERVICE_UUID]]];
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    if (error) {
        [self cleanup];
        return;
    }
    
    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics:@[[CBUUID UUIDWithString:OBD2_CHARACTERISTIC_RX], [CBUUID UUIDWithString:OBD2_CHARACTERISTIC_TX]] forService:service];
    }
    // Discover other characteristics
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    
    NSLog(@"OBD2 - OBD2 FOUND");
    
    if (error) {
        [self cleanup];
        return;
    }
    
    for (CBCharacteristic *characteristic in service.characteristics) {
        
        if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:OBD2_CHARACTERISTIC_RX]]) {
            self.characteristic = characteristic;
            [discoveredPeripheral setNotifyValue:TRUE forCharacteristic:(self.characteristic)];
            
        }
        
        if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:OBD2_CHARACTERISTIC_TX]]) {
            self.characteristic_tx = characteristic;
        }
        
        if(self.characteristic != NULL && self.characteristic_tx != NULL) {
            // inform obd2 that we are connected
            if(obd2btcontroller->connect_cb) {
                obd2btcontroller->connect_cb(obd2btcontroller->pbth_arg);
                
                NSLog(@"OBD2 - Connected. MTU: %lu", (unsigned long)[peripheral maximumWriteValueLengthForType:CBCharacteristicWriteWithResponse]);
            }
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral
didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
             error:(NSError *)error {
    //NSLog(@"OBD2 - DATA: %@",  characteristic.value);
    if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:OBD2_CHARACTERISTIC_RX]]) {
        const char* data = [[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding].UTF8String;
        //NSLog(@"OBD2 - DATA: %s",  data);
        
        if(obd2btcontroller->recv_timeout != NULL) {
            [obd2btcontroller->recv_timeout invalidate];
            obd2btcontroller->recv_timeout = NULL;
        }
        obd2btcontroller->recv_cb(obd2btcontroller->pbth_arg, data);
    }
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    
    discoveredPeripheral = nil;
    
    // inform obd2 that we are disconnected
    if(obd2btcontroller->disconnect_cb) {
        obd2btcontroller->recv_timeout = NULL;
        obd2btcontroller->disconnect_cb(obd2btcontroller->pbth_arg);
    }
    
    self.characteristic = NULL;
    self.characteristic_tx = NULL;
    
    NSLog(@"OBD2 -  Disonnected");
    
    [centralManager scanForPeripheralsWithServices:@[[CBUUID UUIDWithString:OBD2_SERVICE_UUID]] options:@{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES }];
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
