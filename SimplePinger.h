/*
 * SimplePinger.h
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

/*

*/

#ifndef SIMPLEPINGER_H
#define SIMPLEPINGER_H

#include <Arduino.h>

/*! Doxygen documentation about class SimplePinger
 @brief This is a fairly minimal library for ultrasonic range-finder modules.
 Currently it only supports HC-SR04 modules, but should be easily adaptable.

 It borrows code from NewPing v1.5 https://code.google.com/p/arduino-new-ping/
 but makes a number of changes, including removal of stuff that I think should be
 in other libraries for clean software component design (e.g. low pass filtering,
 event-timing). There are also some functional changes and API differences to suit me.
 I have also removed the support for modules other than HC-SR04 since I do not
 have them so cannot test.

 "Simple" refers to the fact that it is not using interrupts. */

#define SIMPLEPINGER_MAXTIMEOUT 1000000L //!< An over-riding timeout in us. User-set values cannot give more waiting time than this. Also used for "total-fail" checks.

/** @name SIMPLEPINGER_TRIGGERMODE_*
 Control what should happen if ping() is called sooner than the time specified by setMinTriggerPeriod() after the previous ping() */
//!@{
#define SIMPLEPINGER_TRIGGERMODE_ALWAYS 0 //!< Trigger always (calling code responsible for controlling interval sensibly)
#define SIMPLEPINGER_TRIGGERMODE_NONBLOCKING 1 //!< Trigger only if time since last ping() is >= the min trigger period, non-blocking. A subsequent getRange() will return the previous value.
#define SIMPLEPINGER_TRIGGERMODE_BLOCKING 2 //!< Trigger only if time since last ping() is >= the min trigger period, blocking. This will cause a wait before emitting, and getRange() will return a new value.
//!@}

/** @name SIMPLEPINGER_ERROR_*
 MODE2 register settings for various stereotype LED driving situations. Note that the /OE pin is not under the control of this library. */
//!@{
#define SIMPLEPINGER_ERROR_NONE 0 //!< There was no error. The last ping() led to a valid echo
#define SIMPLEPINGER_ERROR_SUBSTITUTE -1  //!< There was no error, but the trigger mode is SIMPLEPINGER_TRIGGERMODE_NONBLOCKING and the value returned by getRange() is from the previous error-free echo event.
#define SIMPLEPINGER_ERROR_UNREADY -2 //!< The echo pin was high when ping() was called.
#define SIMPLEPINGER_ERROR_OOR -3 //!< No obstacle detected within max range (see setMaxRange() ).
#define SIMPLEPINGER_ERROR_HARDFAIL -10 //!< Something is causing a fail, e.g. a blocking trigger mode and the echo pin stays high to SIMPLEPINGER_MAXTIMEOUT
#define SIMPLEPINGER_ERROR_INVALID -100 //!< getRange() will not return a valid value for some other reason, currently only if no previous successful ping()
//!@}


class SimplePinger{
public:
  /** @name Constructors */
  //!@{
  /*! Never used */
  SimplePinger();
  /*! Basic constructor. */
  SimplePinger(uint8_t triggerPin, uint8_t echoPin);
  //!@}
  
  /** @name Normal operating methods
  @brief In normal operation, call ping() followed by either getRange() or getLastError(), according to the return value from ping(). */
  //!@{
  /*! Emit an ultrasound pulse and wait for the echo. Behaviour is controlled by setTriggerMode(),
  setMaxRange(), setWaitForClear() 
  @returns true if getRange() is valid, false if an error (call getLastError()) */
  boolean ping();
  
  /*! If Ping() returns true then this will contain a range in millimeters. The value will only change when
  a subsequent Ping() returns true.
  @returns the range of the last object detected in mm */
  int getRange();
  
    /*! If Ping() returns false, this will show the reason why a range is not available. 
  @returns a value from SIMPLEPINGER_TRIGGERMODE_* */
  int getLastError();
  
  /*! Gets the time of the last ping trigger. NB this is the time when the module trigger pin was set high,
  which will not have occurred on calls to ping() that return SIMPLEPINGER_ERROR_SUBSTITUTE, 
  SIMPLEPINGER_ERROR_UNREADY, or SIMPLEPINGER_ERROR_HARDFAIL . Does not give the actual time when the sound
  bounced off the obstacle.
  @returns Value of millis() just before rising edge of trigger pin. */
  unsigned long getLastPingTime();

//!@}  
  
   /** @name Change Operating Parameters
  @brief Change the values of various operating parameters. These all have module-specific defaults and need not
  be called if the default value is appropriate. */
  //!@{
  /*! The maximum range in mm that should be sensed. If the echo is not detected in the time equivalent to this
  range, or the echo pulse length indicates over-range then ping() will return false.
  @param maxRange_mm Maximum range to detect in mm */  
  void setMaxRange(unsigned int maxRange_mm);
  
  /*! Set the speed of sound used to calculate the range in mm from the echo time.
  @param speedOfSound the speed of sound in mm/s */
  void setSpeedOfSound(unsigned int speedOfSound);
  
  /*! Determine how to behave if within the specified min trigger period - see setMinTriggerPeriod().
  The default value for trigger mode is SIMPLEPINGER_TRIGGERMODE_ALWAYS
  @param triggerMode a value from SIMPLEPINGER_TRIGGERMODE_* */
  void setTriggerMode(uint8_t triggerMode);
  
    /*! Set the minimum allowed period between triggering pulse emission. See also setTriggerMode()
  @param minTriggerPeriod_ms The minimum period in milliseconds */
  void setMinTriggerPeriod(unsigned long minTriggerPeriod_ms);
  
  /*! Determine whether to wait if ping() is called and the echo pin is still high. The default is
  for ping() to return immediately with a return value of false, if the pin is high, and for
  a subsequent getLastError() to return SIMPLEPINGER_ERROR_UNREADY. 
  @param waitUntilQuiet Pass true to cause a wait until the echo pin goes low. Pass false for default behaviour. */
  void setWaitUntilQuiet(boolean waitUntilQuiet);
  
  
//!@}

private:
  //private member variables
  //last ping values
  unsigned int _lastRange;
  int _lastError;
  unsigned long _lastPingTime;
  // control parameters - a) not module-specific
  unsigned long _speedOfSound;
  uint8_t _triggerMode;
  boolean _waitUntilQuiet;
  // control parameters - b) with module-specific defaults set in constructor
  unsigned int _maxRange_mm;
  unsigned long _maxRange_us;// NB this is microseconds and is a ROUND TRIP time for the max range
  unsigned long _minTriggerPeriod_ms; //NB this is in milliseconds
  unsigned long _maxSensorDelay_us; //us delay from trigger going high to emitter activating
  
  //hardware
		uint8_t _triggerBit;
		uint8_t _echoBit;
		volatile uint8_t *_triggerOutput;
		volatile uint8_t *_echoInput;
  
  //private methods
};



#endif //include guard