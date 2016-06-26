/*! \mainpage The SODA library for datalogging with Olympia Circuit's SODA HE board.
 *
 * \section intro_sec Introduction
 *
 * SODA is an Arduino library for data logging. It provides functions to simplify datalogging tasks using Olympia Circuit's SODA HE 1.0 board.
 * SODA stands for Simple Open Data Acquisition.  The goal of the project is to provide simple, high quality tools for the collection and analysis of environmental data.
 * The library was built by Peter Gould (peter@olympiacircuits.com). Some code was adapted from Petre Rodan's DS3231 library for Arduino.
 * Additional thanks go to William Greiman for his SD_FAT library.
 *
 * The SODA library consists of a single class SODA. 
 * \section dependencies_sec Dependencies
 * SdFat: library for SD card functions. This library needs to be added to your Arduino library along with SODA.
 *
 * EEPROM: standard Arduino library for EEPROM functions (comes with your Arduino installation).
 *
 * Wire: standard Arduino library for I2C communication (comes with your Arduino installation).
 * \section install_sec Installation
 * The contents of the SODA folder should be added to the library folder of your Arduino installation (e.g., C:\\Program Files (x86)\\Arduino\\libraries). 
 * Arduino must be restarted after the library has been added.
 */


#include <Arduino.h>
#include <Wire.h>

//clock values
#define CLOCK_I2C_ADDR   0x68
#define CLOCK_CONTROL_ADDR 0x0E
#define CLOCK_TEMPERATURE_ADDR     0x11
#define CLOCK_TIME_CAL_ADDR        0x00
#define CLOCK_SETUP 0x5 //allow interrupts on alarm1
#define CLOCK_ALARM1_ADDR          0x07
#define CLOCK_ALARM_STATUS 0x0F

/** 
* A class to control the Olympia Circuits Valve Controller
*/
class OCVC
{
	private:
		void setAddress(uint8_t,uint8_t,uint8_t);
		unsigned char getAddress(uint8_t,uint8_t);
		int dectobcd(int val);
		int bcdtodec(int val);
		//variables 
	//	static char buffer[];
	//	static int timeArray[];
	public:
		/**
       * Initializes an instance of the SODA class. Should be called in each sketch before any other SODA functions.
       */
		void begin();
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//@I Clock Functions
		/**
       * Updates the time array. Need to run setTime to send time array to clock.
       * @param val an int time values.
       * @param place an int specifying the place in the time array (0 = year, 1 = month, 2 = day, 3 = hour, 4 = minute, 5 = second).
       * @see setTime()
	   * @see getTIme()
       */
		void updateTime(int val,int place);
		/**
       * Returns an int value from the timeArray.
       * @param place an int specifying the place in the time array (0 = year, 1 = month, 2 = day, 3 = hour, 4 = minute, 5 = second).
       * @see setTime()
	   * @see updateTime()
       */
		int checkTime(int place);
		/**
		* Resets the time in the clock to the values from timeArray.
		* @see checkTime()
		* @see getTime()
	   * @see updateTime()
		*/
		void setTime();
		
		/**
		*Loads an external array into timeArray
		*/
		void setTimeArray(int ta[]);
		
		/**
		*return time as String
		*/
		String getTimeString(int check_time = 1);
		/**
		* Loads the time from the clock to the timeArray.
		* (0 = year, 1 = month, 2 = day, 3 = hour, 4 = minute, 5 = second).
	   * @see setTime()
		*/
		void getTime();
		/**
		* Loads the timeArray into a formatted character buffer.
		* Format = YYYY-MM-DD HH:MM:SS
		* @see getTime()
		*/
		void bufferTime();
		/**
		*Set the clock based on input from the Serial connection.
		*Serial data are first saved to the buffer[] array and then loaded to the timeArray before being sent to the clock.
		*Serial data format = 'YYYY-MM-DD HH:MM:SS'.
		* @see setTime()
		*/
		void serialSetTime();
		/**
		*Returns the value from the internal temperature sensor in the DS3231 real time clock.
		*@return temperature in Celsius as float
		*/
		float getClockTemp();
		/**
		*Sets the clock alarm.  Used to wake up the logger and begin a new measurement.
		*Example: setWake(10,2); sets the alarm to the next 10 minute interval.
		*@param val an int time value.
		*@param valType an int indicating the units of time 1= secs, 2 = mins, 3=hours.
		*@see turnOff
		*/
		void printBuffer();
		/**
		*Returns a text representation of the time with minutes added.
		*@param offset minutes added to current time
		*/
		String getTimeOffset(int offset);
		/**
		*return UNIX time as a long
		*/
		long getUnixTime();

};


