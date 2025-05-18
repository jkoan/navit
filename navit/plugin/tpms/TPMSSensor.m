//
//  TPMSSensor.m
//  BLE TPMS Warner
//
//  Created by olf on 09.07.23.
//

#import <Foundation/Foundation.h>
#import "TPMSSensor.h"

@implementation TPMSSensor

- (id) initWithTimer {
    self=[super init];
    [self startTimer];
    
    return self;
}

- (bool)getAlarmStatus {
    return self.alarm;
}

- (double)getBatteryPercentage {
    return self.batt;
}

- (double)getPressure {
    return self.pressure;
}

- (double)getTemperature {
    return self.temp;
}

- (void)setAlarmStatus: (bool) alarm {
    self.alarm=alarm;
}

- (void)setBatteryPercentage: (double) batt {
    self.batt=batt;
}

- (void)setPressureValue: (double) pressure {
    self.pressure=pressure;
}

- (void)setTemperature: (double) temp {
    self.temp=temp; 
}

-(void) noSignal: (NSTimer *)timer {
    NSLog(@"Nothing received within 10 seconds!");
    self.alarm=TRUE;
    self.batt=0.0;
    self.temp=0.0;
    self.pressure=0.0;
}

- (void) startTimer {
    if(_signalTimer==nil) {
        _signalTimer = [NSTimer scheduledTimerWithTimeInterval:10.0
                                                        target:self
                                                      selector:@selector(noSignal:)
                                                      userInfo:nil
                                                       repeats:NO];
    }
}

-(void) removeTimer {
    [_signalTimer invalidate];
    _signalTimer = nil;
}



@end
