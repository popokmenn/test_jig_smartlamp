/****************************************************************************/
//  Function: Header file for PCF85063TP
//  Hardware: Grove - RTC v2.0
//  Arduino IDE: Arduino-1.6.6
//  Author:   FrankieChu
//  Date:    June 14,2016
//  Version: v1.0
//  by www.seeedstudio.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
//
/****************************************************************************/

#include <Wire.h>
#include "PCF85063TP.h"

uint8_t PCF85063TP::decToBcd(uint8_t val) {
    return ((val / 10 * 16) + (val % 10));
}

//Convert binary coded decimal to normal decimal numbers
uint8_t PCF85063TP::bcdToDec(uint8_t val) {
    return ((val / 16 * 10) + (val % 16));
}

void PCF85063TP::begin(TwoWire *_pWire) {
    // Wire.begin();
    pWire = _pWire;
    pWire->begin();
    cap_sel(CAP_SEL_12_5PF);  // CAP_SEL bit setting 12.5pF

}
/*Function: The clock timing will start */
void PCF85063TP::startClock(void) {      // set the ClockHalt bit low to start the rtc
    uint8_t data;
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write((uint8_t)0x00);                      // Register 0x00 holds the oscillator start/stop bit
    pWire->endTransmission();
    pWire->requestFrom(PCF85063TP_I2C_ADDRESS, 1);
    data = pWire->read() &
           ~0x20;       // save actual control_1 regitser and AND sec with bit 7 (sart/stop bit) = clock started
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write((uint8_t)0x00);
    pWire->write((uint8_t)data);                    // write seconds back and start the clock
    pWire->endTransmission();
}
/*Function: The clock timing will stop */
void PCF85063TP::stopClock(void) {       // set the ClockHalt bit high to stop the rtc
    uint8_t data;
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write((uint8_t)0x00);                      // Register 0x00 holds the oscillator start/stop bit
    pWire->endTransmission();
    pWire->requestFrom(PCF85063TP_I2C_ADDRESS, 1);
    data = pWire->read() |
           0x20;       // save actual control_1 regitser and AND sec with bit 7 (sart/stop bit) = clock started
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write((uint8_t)0x00);
    pWire->write((uint8_t)data);                    // write seconds back and stop the clock
    pWire->endTransmission();
}
/****************************************************************/
/*Function: Read time and date from RTC  */
void PCF85063TP::getTime() {
    // Reset the register pointer
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write((uint8_t)0x04);
    pWire->endTransmission();
    pWire->requestFrom(PCF85063TP_I2C_ADDRESS, 7);
    // A few of these need masks because certain bits are control bits
    second     = bcdToDec(pWire->read() & 0x7f);
    minute     = bcdToDec(pWire->read());
    hour     = bcdToDec(pWire->read() & 0x3f);// Need to change this if 12 hour am/pm
    dayOfMonth = bcdToDec(pWire->read());
    dayOfWeek  = bcdToDec(pWire->read());
    month      = bcdToDec(pWire->read());
    year     = bcdToDec(pWire->read());
}
/*******************************************************************/
/*Frunction: Write the time that includes the date to the RTC chip */
void PCF85063TP::setTime() {
    writeReg(REG_SEC, decToBcd(second));// 0 to bit 7 starts the clock, bit 8 is OS reg
    writeReg(REG_MIN, decToBcd(minute));
    writeReg(REG_HOUR, decToBcd(hour));  // If you want 12 hour am/pm you need to set bit 6
    writeReg(REG_DAY_MONTH, decToBcd(dayOfMonth));
    writeReg(REG_DAY_WEEK, decToBcd(dayOfWeek));
    writeReg(REG_MON, decToBcd(month));
    writeReg(REG_YEAR, decToBcd(year));
}
void PCF85063TP::fillByHMS(uint8_t _hour, uint8_t _minute, uint8_t _second) {
    // assign variables
    hour = _hour;
    minute = _minute;
    second = _second;
}
void PCF85063TP::fillByYMD(uint16_t _year, uint8_t _month, uint8_t _day) {
    year = _year - 2000;
    month = _month;
    dayOfMonth = _day;
}
void PCF85063TP::fillDayOfWeek(uint8_t _dow) {
    dayOfWeek = _dow;
}

void PCF85063TP::reset() {
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write((uint8_t)0x00);
    pWire->write(decToBcd(0x10));// software reset at bit 4
    pWire->endTransmission();
}

/*
    @brief  clock calibration setting
    @Parameter:
     mode = 0, calibrate every 2 hours
     mode = 1, calibrate every 4 minutes
     offset_sec, offset value of one second.
     If the RTC time too fast: offset_sec < 0
     If the RTC time too slow: offset_sec > 0
*/
uint8_t PCF85063TP::calibratBySeconds(int mode, float offset_sec) {
    float Fmeas = 32768.0 + offset_sec * 32768.0;
    setcalibration(mode, Fmeas);
    return readCalibrationReg();
}
/*
    @brief: Clock calibrate setting
    @parm:
          mode: calibration cycle, mode 0 -> every 2 hours, mode 1 -> every 4 minutes
          Fmeas: Real frequency you detect
*/

void PCF85063TP::setcalibration(int mode, float Fmeas) {
    float offset = 0;
    float Tmeas = 1.0 / Fmeas;
    float Dmeas = 1.0 / 32768 - Tmeas;
    float Eppm = 1000000.0 * Dmeas / Tmeas;
    if (mode == 0) {
        offset = Eppm / 4.34;
    } else if (mode == 1) {
        offset = Eppm / 4.069;
    }

    uint8_t data = (mode << 7) & 0x80 | ((int)(offset + 0.5) & 0x7f);
    writeReg(PCF85063TP_OFFSET, data);
}


uint8_t PCF85063TP::readCalibrationReg(void) {
    return readReg(PCF85063TP_OFFSET);
}


void PCF85063TP::setRam(uint8_t value) {
    writeReg(REG_RAM, value);
}

uint8_t PCF85063TP::readRamReg(void) {
    return readReg(REG_RAM);
}


/*
    @brief: internal oscillator capacitor selection for
           quartz crystals with a corresponding load capacitance
    @parm:
         value(0 or 1): 0 - 7 pF
                        1 - 12.5 pF
    @return: value of CAP_SEL bit
*/
uint8_t PCF85063TP::cap_sel(uint8_t value) {
    uint8_t control_1 = readReg(REG_CTRL1);
    control_1 = (control_1 & 0xFE) | (0x01 & value);
    writeReg(REG_CTRL1, control_1);

    return readReg(REG_CTRL1) & 0x01;
}


uint8_t PCF85063TP::readReg(uint8_t reg) {
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write(reg & 0xFF);
    pWire->endTransmission();
    pWire->requestFrom(PCF85063TP_I2C_ADDRESS, 1);

    return pWire->read();

}

void PCF85063TP::writeReg(uint8_t reg, uint8_t data) {
    pWire->beginTransmission(PCF85063TP_I2C_ADDRESS);
    pWire->write(reg & 0xFF);
    pWire->write(data & 0xFF);
    pWire->endTransmission();

}

#ifdef PCF85063TP_USE_STRINGS
void PCF85063TP::timeOrDateStringsToInts(const char time_or_date[],
                                         int time_or_date_parts[]) {
    int time_or_date_index = 0;
    char mutable_time_or_date[12];
    strcpy(mutable_time_or_date, time_or_date);
    char *token;

    token = strtok(mutable_time_or_date, " :");
    while (token != NULL) {
        char *token_end;
        int value = strtol(token, &token_end, 10);
        if (token == token_end) {
            // If this occurs, there was no number to be parsed. Assume month.
            if (strncmp(token, "Jan", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 1;
            } else if (strncmp(token, "Feb", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 2;
            } else if (strncmp(token, "Mar", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 3;
            } else if (strncmp(token, "Apr", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 4;
            } else if (strncmp(token, "May", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 5;
            } else if (strncmp(token, "Jun", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 6;
            } else if (strncmp(token, "Jul", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 7;
            } else if (strncmp(token, "Aug", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 8;
            } else if (strncmp(token, "Sep", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 9;
            } else if (strncmp(token, "Oct", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 10;
            } else if (strncmp(token, "Nov", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 11;
            } else if (strncmp(token, "Dec", 3) == 0) {
                time_or_date_parts[time_or_date_index++] = 12;
            }
        } else {
            time_or_date_parts[time_or_date_index++] = value;
        }
        token = strtok(NULL, " :");
    }
}

void PCF85063TP::fillDateByString(const char date_string[]) {
    int date_parts[3] = {0, 0, 0};
    timeOrDateStringsToInts(date_string, date_parts);
    fillByYMD(date_parts[2], date_parts[0], date_parts[1]);
}

void PCF85063TP::fillTimeByString(const char time_string[]) {
    int time_parts[3] = {0, 0, 0};
    timeOrDateStringsToInts(time_string, time_parts);
    fillByHMS(time_parts[0], time_parts[1], time_parts[2]);
}
#endif