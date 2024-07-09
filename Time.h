#pragma once

#include <iostream>
#include <string>

#include "TimeExceptions.h"

using namespace std;

class Time {
private:
    int hour;
    int minute;
public:

    Time(int hour = 0, int minute = 0);

    bool isSmallerComparedTo(Time& comparisonTime) const;

    bool isLargerComparedTo(Time& comparisonTime) const;

    string toString() const;

    void fromString(string& timeString);
    /*
     * Referred from the GitHub repository: Appointment-Booking https://github.com/pgagliano/Appointment-Booking/blob/master/myTime.cpp
     * Author: Patrick Gagliano
     */
};