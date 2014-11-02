/*
 * SimplePinger.cpp
 *
 * Created: 11/1/2014 5:37:40 PM
 *  Author: Adam Cooper
 */
/*
 * ***Made available using the The MIT License (MIT)***
 * Copyright (c) 2014, Adam Cooper
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the �Software�), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#include <avr/io.h>

#include "SimplePinger.h"

//never used,except when declaring an object
SimplePinger::SimplePinger(){
}
	
//constructor
SimplePinger::SimplePinger(uint8_t triggerPin, uint8_t echoPin){
	 //until a ping, there is a de-facto last error
	 _lastError= SIMPLEPINGER_ERROR_INVALID;
	 // control parameters - a) not module-specific
	 _speedOfSound=340;
	 _triggerMode = SIMPLEPINGER_TRIGGERMODE_ALWAYS;
	 _waitUntilQuiet = false;
	 
	//initialise for the pins being used
	_triggerBit = digitalPinToBitMask(triggerPin); // Get the port register bitmask for the trigger pin.
	_echoBit = digitalPinToBitMask(echoPin);       // Get the port register bitmask for the echo pin.
	*portModeRegister(digitalPinToPort(triggerPin)) |= _triggerBit; // Set trigger pin to output.
	_triggerOutput = portOutputRegister(digitalPinToPort(triggerPin)); // Get the output port register for the trigger pin.
	_echoInput = portInputRegister(digitalPinToPort(echoPin));         // Get the input port register for the echo pin.
	
	//set-up
	*_triggerOutput &= ~_triggerBit; // Set the trigger pin low
		
	//module-specific default values. PRESENTLY ONLY FOR HC-SR04 (use some compiler directives in future...)
	setMaxRange(4000);// NB this is microseconds
	_minTriggerPeriod_ms=60; //NB this is in milliseconds
	_maxSensorDelay_us=250; //us delay from trigger pin going high to emitter activating
	_lastPingTime = millis()-_minTriggerPeriod_ms;
}
  
  boolean SimplePinger::ping(){
	  //UNless the trigger mode is "always", check when the last ping was
	  if (_triggerMode == SIMPLEPINGER_TRIGGERMODE_BLOCKING){
		  //wait for the min period
		  while(millis()-_lastPingTime < _minTriggerPeriod_ms);
	  }else if(_triggerMode == SIMPLEPINGER_TRIGGERMODE_NONBLOCKING){
		  if(millis()-_lastPingTime < _minTriggerPeriod_ms){
			//return true, (used reads getRange() as normal but gets the last reading)
			_lastError=SIMPLEPINGER_ERROR_SUBSTITUTE;
			return true;
			}
	  }
  
	  //check the echo pin is low, and behave according to _waitUntilQuiet
	  if(_waitUntilQuiet){
		  unsigned long startWait = micros();
		  	while (*_echoInput & _echoBit){
				  if(micros()-startWait > SIMPLEPINGER_MAXTIMEOUT){
					_lastError = SIMPLEPINGER_ERROR_HARDFAIL;
					return false;  
				  }
			  }
	  }else{
		  if(*_echoInput & _echoBit){//quit with error if high
			  _lastError = SIMPLEPINGER_ERROR_UNREADY;
			return false;
		  }
	  }
	  
	  //tell the module to emit a pulse
	  *_triggerOutput |= _triggerBit;  // Set trigger pin high, this tells the sensor to send out a ping.
	  delayMicroseconds(11);           // Wait long enough for the sensor to realize the trigger pin is high. Sensor specs say to wait at least 10uS.
	  *_triggerOutput &= ~_triggerBit; // Set trigger pin back to low.
	  
	  //Calculate a time-out. if no echo pulse has started by this interval then the object is out of range (or not there!)
	  unsigned long timeout_us = min(_maxSensorDelay_us + _maxRange_us, SIMPLEPINGER_MAXTIMEOUT) + micros(); 
	  //wait for the echo to start, signalling an error if timeout
	  while (!(*_echoInput & _echoBit)){
		if (micros() > timeout_us){
			_lastError= SIMPLEPINGER_ERROR_OOR;
			return false;
		}
	  }
	  //time at echo start (give or take a few uS)
	  long startMicros = micros();
	  //wait for the echo to stop and compute the range in mm
	  //in the HIGHLY UNLIKELY event that the echo pin remains high, this will hang. (but that would mean a HW fail anyway)
	  while (*_echoInput & _echoBit);
	  //echo pulse width
	  unsigned long echoLength = micros()-startMicros;
	  //the obstacle was out of range (NB because of uncertainty in the actual maxSensorDelay, an OOR may not
	  // already have been detected
	  if (echoLength>_maxRange_us){
		  _lastError= SIMPLEPINGER_ERROR_OOR;
		  return false;		  
	  }
	  
	  //the echo was fine!
	  _lastRange = echoLength *  long(_speedOfSound ) / 2000L;
	  _lastError = SIMPLEPINGER_ERROR_NONE;
	  return true;	  
  }
  
  int SimplePinger::getRange(){
	  return _lastRange;
  }
  
  int SimplePinger::getLastError(){
	  return _lastError;
  }  

  void SimplePinger::setMaxRange(unsigned int maxRange_mm){
	  _maxRange_mm=maxRange_mm;
	  _maxRange_us = long(maxRange_mm) * 2000L / long(_speedOfSound);
  }  

  void SimplePinger::setSpeedOfSound(unsigned int speedOfSound){
	  _speedOfSound = speedOfSound;
	  //re-compute
	  _maxRange_us = long(_maxRange_mm) * 2000L / long(_speedOfSound);
  }
  
  void SimplePinger::setTriggerMode(uint8_t triggerMode){
	  _triggerMode=triggerMode;
  }
  
  void SimplePinger::setMinTriggerPeriod(unsigned long minTriggerPeriod_ms){
	  _minTriggerPeriod_ms=minTriggerPeriod_ms;
  }
  
  void SimplePinger::setWaitUntilQuiet(boolean waitUntilQuiet){
	  _waitUntilQuiet = waitUntilQuiet;
  }  
  