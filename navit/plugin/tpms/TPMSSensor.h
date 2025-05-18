//
//  TPMSSensor.h
//  BLE TPMS Warner
//
//  Created by olf on 09.07.23.
//

#ifndef TPMSSensor_h
#define TPMSSensor_h

@interface TPMSSensor : NSObject

@property double pressure; // the pressure in bar
@property double temp; // the temperature in °C
@property double pressure_corr; // the pressure correction in bar
@property double temp_corr; // the temperature correction in °C
@property int batt; // the battery level in percent
@property bool alarm; // the alarm status
@property NSString *advDataLocalName; // The complete name in Advertising
@property NSString *barcodeValue; // The scanned barcode value for detection of the sensor
@property NSString *advDataLocalNamePrefix; // The common prefix for all Sensors
@property NSTimer *signalTimer;

-(id)   initWithTimer;
-(double) getPressure;
-(double) getTemperature;
-(double) getBatteryPercentage;
-(bool)   getAlarmStatus;
-(void)   setPressure:(double) pressure;
-(void)   setAlarm:(bool) alarm;
-(void)   setBatteryPercentage:(double) batt;
-(void)   setTemperature:(double) temp;

@end

#endif /* TPMSSensor_h */
