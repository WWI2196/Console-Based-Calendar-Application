#include "Time.h"
#include "TimeExceptions.h"

#include <sstream>
#include <iostream>


Time::Time(int hour, int minute) {
    this->hour = hour;
    this->minute = minute;

    if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
        throw TimeExceptions(1);
    }
}

bool Time::isSmallerComparedTo(Time& comparisonTime) const { // check if the time is smaller than another time
    return (hour < comparisonTime.hour) || (hour == comparisonTime.hour && minute < comparisonTime.minute);
}

bool Time::isLargerComparedTo(Time& comparisonTime) const { // check if the time is larger than another time
    return (hour > comparisonTime.hour) || (hour == comparisonTime.hour && minute > comparisonTime.minute);
}

string Time::toString() const { // return the time as a string
    return (hour < 10 ? "0" : "") + to_string(hour) + ":" + (minute < 10 ? "0" : "") + to_string(minute);
}

void Time::fromString(string& timeString) {
    stringstream timeStream(timeString);  // Here is one way that we usd stringstream where to convert stream which enable us to use stream managing tools in cpp such as >>
    char separator;

    timeStream >> hour >> separator >> minute;
    if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
        throw TimeExceptions(1);
    }

    /*
     * Referred from the GitHub repository: Appointment-Booking https://github.com/pgagliano/Appointment-Booking/blob/master/myTime.cpp
     * Author: Patrick Gagliano
     */
}