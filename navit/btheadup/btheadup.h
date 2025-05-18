/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2008 Navit Team
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

#ifndef BTHEADUP_H
#define BTHEADUP_H

typedef void(*DISCONNECT_CB)(void*);
typedef void(*CONNECT_CB)(void*);
void bthbtcontroller_init(void* pbth_arg, DISCONNECT_CB disconnect_cb, CONNECT_CB connect_cb, char *devicename);
void bthbtcontroller_send(unsigned char* data, int length);

#ifdef BTHEADUP_OBJC
#import <CoreBluetooth/CoreBluetooth.h>

// Nordic NUS defines
#define NUS_SERVICE_UUID      @"6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_CHARACTERISTIC_RX @"6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_CHARACTERISTIC_TX @"6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define DIS_SVC_UUID          @"180a"
#define DIS_CHARACTERISTIC_SERIAL @"2A25"

@interface BTHeadup : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate> {

@public
    DISCONNECT_CB disconnect_cb;
    CONNECT_CB connect_cb;
    void * pbth_arg;
    char * devicename;
}

@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) CBPeripheral *discoveredPeripheral;
@property (strong, nonatomic) NSMutableData *data;
@property (strong, nonatomic) CBCharacteristic *characteristic;
@property (nonatomic) void * pbth_arg;
@property (nonatomic) DISCONNECT_CB disconnect_cb;
@property (nonatomic) CONNECT_CB connect_cb;
@property (nonatomic) char * devicename;

@end

#endif
#endif
