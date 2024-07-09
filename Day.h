#pragma once

#include <string>
#include "Event.h"
#include "EventExceptions.h"

using namespace std;

class Day { // Class for the Days
private:
    void sortEvents(); // function to sort the events

public:
    int date;
    bool isDayOff;
    Event events[10]; // maximum of 10 events per day
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