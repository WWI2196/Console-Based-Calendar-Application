#pragma once

#include <string>
#include "Event.h"
#include "EventExceptions.h"

using namespace std;

class Day {
private:
    void sortEvents();

public:
    int date;
    bool isDayOff;
    Event events[10];
    int eventCount;
    string dayOfWeek;

    Day(int date = 0, string dayOfWeek = "");

    void addEvent(Event& event);
    void deleteEvent(string& title);
    void shiftEvent(string& title, int newDate, Day* days);
    void clearEvents();
    string toString() const;
    bool toString_print() const;
    string formatDayDataToString() const;
    void extractDayData(string& dayStr);
};