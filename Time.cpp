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

bool Time::isSmallerComparedTo(Time& comparisonTime) const {
    return (hour < comparisonTime.hour) || (hour == comparisonTime.hour && minute < comparisonTime.minute);
}

bool Time::isLargerComparedTo(Time& comparisonTime) const {
    return (hour > comparisonTime.hour) || (hour == comparisonTime.hour && minute > comparisonTime.minute);
}

string Time::toString() const {
    return (hour < 10 ? "0" : "") + to_string(hour) + ":" + (minute < 10 ? "0" : "") + to_string(minute);
}

void Time::fromString(string& timeString) {
    stringstream timeStream(timeString);
    char separator;

    timeStream >> hour >> separator >> minute;
    if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
        throw TimeExceptions(1);
    }
}