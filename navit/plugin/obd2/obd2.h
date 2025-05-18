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

#ifndef OBD2_H
#define OBD2_H

typedef void(*DISCONNECT_CB)(void*);
typedef void(*CONNECT_CB)(void*);
typedef void(*RECEIVE_CB)(void*, const char*);
void obd2btcontroller_init(void* pbth_arg, DISCONNECT_CB disconnect_cb, CONNECT_CB connect_cb, RECEIVE_CB recv_cb, char *devicename);
void obd2btcontroller_send(char* data, unsigned long length);

#ifdef OBD2_OBJC
#import <CoreBluetooth/CoreBluetooth.h>

// Nordic NUS defines
//#define OBD2_SERVICE_UUID      @"0000fff0-0000-1000-8000-00805f9b34fb"
//#define OBD2_CHARACTERISTIC_RX @"0000fff1-0000-1000-8000-00805f9b34fb"
//#define OBD2_CHARACTERISTIC_TX @"0000fff2-0000-1000-8000-00805f9b34fb"
#define OBD2_SERVICE_UUID      @"fff0"
#define OBD2_CHARACTERISTIC_RX @"fff1"
#define OBD2_CHARACTERISTIC_TX @"fff2"
#define DIS_SVC_UUID          @"180a"
#define DIS_CHARACTERISTIC_SERIAL @"2A25"

@interface OBD2 : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate> {

@public
    DISCONNECT_CB disconnect_cb;
    CONNECT_CB connect_cb;
    RECEIVE_CB recv_cb;
    void * pbth_arg;
    char * devicename;
    NSTimer *recv_timeout;
}

@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) CBPeripheral *discoveredPeripheral;
@property (strong, nonatomic) NSMutableData *data;
@property (strong, nonatomic) CBCharacteristic *characteristic;
@property (strong, nonatomic) CBCharacteristic *characteristic_tx;
@property (nonatomic) void * pbth_arg;
@property (nonatomic) DISCONNECT_CB disconnect_cb;
@property (nonatomic) CONNECT_CB connect_cb;
@property (nonatomic) RECEIVE_CB recv_cb;
@property (nonatomic) char * devicename;

@end

#endif
#endif
