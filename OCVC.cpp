#include "Arduino.h"
#include "OCVC.h"
#include <Wire.h>

int timeArray[6] = {2010,1,1,12,0,0};
char buffer[30];
int bufferIndex = 0;

//////////////////////////////////////////////////////////////////////
//@I SHARED FUNCTIONS //////////////////////////////////////////////////-
////////////////////////////////////////////////////////////////////
//should be initiated at the beginning of the sketch
void OCVC::begin(){
	Wire.begin(5,4);
}
//@I set an address of an I2C device
void OCVC::setAddress(uint8_t device, uint8_t addr, uint8_t val)
{
    Wire.beginTransmission(device);
    Wire.write(addr);
    Wire.write(val);
    Wire.endTransmission();
}

//get an address of a device
unsigned char OCVC::getAddress(uint8_t device, uint8_t addr)
{
    uint8_t rv;
    Wire.beginTransmission(device);
    Wire.write(addr);
    Wire.endTransmission();
    Wire.requestFrom(int(device), int(1));
    rv = Wire.read();
    return rv;
}

//decimal to hex and reverse
int OCVC::dectobcd(int val)
{
    return ((val / 10 * 16) + (val % 10));
}

int OCVC::bcdtodec(int val)
{
    return ((val / 16 * 10) + (val % 16));
}

//print the contents of the buffer
void OCVC::printBuffer(){
	Serial.print(buffer);
}


/////////////////////////////////////////////////////////////////////////////
//@I CLOCK FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
//@I Set the time by passing values from the array to the clock register
void OCVC::setTime(){
   int sYear;
   unsigned char century;
   int i = 0;
   unsigned char aTime;
    if (timeArray[0] > 2000) {
        century = 0x80;
        sYear = timeArray[0] - 2000;
    } else {
        century = 0x00;
        sYear = timeArray[0] - 1900;
    }

    Wire.beginTransmission(CLOCK_I2C_ADDR);
    Wire.write(CLOCK_TIME_CAL_ADDR);
    while(i<=6) {
		if(i==3){ //put in the addition byte for day of week
			aTime = 0x01;
			Wire.write(aTime);
		}
        aTime = dectobcd(timeArray[5-i]);
		if(i==5) aTime = dectobcd(sYear);
        if (i == 4) aTime += century;
        Wire.write(aTime);
		i++;
    }
    Wire.endTransmission();
}

void OCVC::setTimeArray(int ta[]){
	for(int k = 0; k<6; k++){
		timeArray[k] = ta[k];
	}
}


//loads current time into the timeArray
void OCVC::getTime(){
    unsigned char century = 0;
    int i = 0;
	unsigned char n;
    int year_full;
    Wire.beginTransmission(CLOCK_I2C_ADDR);
    Wire.write(CLOCK_TIME_CAL_ADDR);
    Wire.endTransmission();
    Wire.requestFrom(CLOCK_I2C_ADDR, 7);
    while(i<6){
		if(i==3){ //skip over day of week
			n = Wire.read();
		}
        n = Wire.read();
        if (i == 4) {
		    century = (n & 0x80) >> 7;
            n = n & 0x1F;
		}
		timeArray[5-i] = bcdtodec(n);
		if(i==5){
			if (century == 1) {
				timeArray[0] = 2000 + timeArray[0];
			} else {
				timeArray[0] = 1900 + timeArray[0];
			}
		}
	i++;
	}
	bufferTime(); //load into character buffer
}

//gets current time and loads it into a char array;
//with format YYYY-MM-DD HH:MM:SS;
void OCVC::bufferTime(){
	int spot = 0;
	int needed = 4;
	char spacers[] = "0000-00-00 00:00:00";
	for(int k = 0; k < 6; k++){
		char buff2[4];
		itoa(timeArray[k],buff2,10);
		if(timeArray[k] < 10){
			needed = 1;
			spot++;
		} else{
			needed = 2;
		}
		if(k==0) needed = 4;
		for(int i = 0; i < needed; i++){
			spacers[spot] = buff2[i];
			spot++;
		}
		spot++; //space between each number
	} //end loop through array
	//now load into buffer
	for(int k = 0; k<19; k++){
		buffer[k] = spacers[k];
	}
} //end function

String OCVC::getTimeString(int check_time){
	if(check_time !=0) getTime(); //don't always want to update time
	String st_time = String(buffer);
	return(st_time);
}

//Set clock using input from the serial monitor
void OCVC::serialSetTime(){
	//expects 19=character format YYYY-MM-DD HH:MM:SS
	if(bufferIndex<19) return; //coarse filter for malformed data
	bufferIndex = 0;
	for(int k = 0; k < 6; k++){
		int need = 2;
		if(k==0) need = 4;
		char temp[need];
		for(int i = 0; i < need; i++){ //read value
			temp[i] = buffer[bufferIndex];
			bufferIndex++;
		}
		timeArray[k] = atoi(temp);
		if(k < 5) temp[0] = bufferIndex++; //read spacing characters
		} //end loop through time characters
		setTime(); //set the time using the current time array;
		getTime(); //reload the time
		printBuffer();
}


//set the values in the clock array
void OCVC::updateTime(int val,int place){
	if(place > 5) return;  //only 6 places
	timeArray[place] = val;
}

//retrieve values from clock array
int OCVC::checkTime(int place){
	if(place>5) return(0);
	return(timeArray[place]);
}

String OCVC::getTimeOffset(int offset){
	int valType = 0;   //legacy code. Sets the time type to seconds
	getTime(); //load current timese
	int times[3] = {timeArray[5],timeArray[4],timeArray[3]}; //load current time
	for(int i = 0; i < 3; i++){
		if(i<valType) times[i] = 0;
		if(i==valType){
			times[i]+= offset;
		}
			if(i<2 && times[i] >= 60){ //push into next increment if needed
				while(times[i]>=60){
					times[i+1] = times[i+1] + 1;
					times[i]-=60;
				}
			}
			if(i==2 & times[i] >= 24) times[i]-= 24;
		}
	//push back into time array
	for(int k = 5; k>2;k--){
		timeArray[k] = times[5-k];
	}
	bufferTime();
	return(getTimeString(0));
}

long OCVC::getUnixTime(){
	// returns the number of seconds since 01.01.1970 00:00:00 UTC, valid for 2000..FIXME
	long SECONDS_FROM_1970_TO_2000 = 946684800;
	uint8_t days_in_month[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    uint8_t i;
    uint16_t d;
    int16_t y;
    uint32_t rv;

    if (timeArray[0] >= 2000) {
        y = timeArray[0] - 2000;
    } else {
        return 0;
    }

    d = timeArray[2]  - 1;
    for (i=1; i < timeArray[1] ; i++) {
        d += days_in_month[i-1];
    }
    if (timeArray[1]> 2 && y % 4 == 0) {
        d++;
    }
    // count leap days
    d += (365 * y + (y + 3) / 4);
    rv = ((d * 24UL + timeArray[3]) * 60 + timeArray[4]) * 60 + timeArray[5] + SECONDS_FROM_1970_TO_2000;
    return rv;
}



//get temperature value
float OCVC::getClockTemp()
{
    float rv;
    uint8_t temp_msb, temp_lsb;
    int8_t nint;
	setAddress(CLOCK_I2C_ADDR,CLOCK_CONTROL_ADDR,CLOCK_SETUP | 0x20); //updates temp
    Wire.beginTransmission(CLOCK_I2C_ADDR);
    Wire.write(CLOCK_TEMPERATURE_ADDR);
    Wire.endTransmission();
    Wire.requestFrom(CLOCK_I2C_ADDR, 2);
    temp_msb = Wire.read();
    temp_lsb = Wire.read() >> 6;
    if ((temp_msb & 0x80) != 0)
        nint = temp_msb | ~((1 << 8) - 1);      // if negative get two's complement
    else
        nint = temp_msb;
    rv = 0.25 * temp_lsb + nint;
    return rv;
}
