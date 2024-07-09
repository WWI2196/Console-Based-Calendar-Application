#pragma once

#include <string>
#include "Time.h"
#include "EventExceptions.h"

using namespace std;

class Event { // Class for the Events
public:
    string title;
    Time startTime;
    Time endTime;
    string repeatType; // "none", "daily", "weekly"

    Event(string title = "EVENT", Time startTime = Time(), Time endTime = Time(), string repeatType = "none");

    bool overlaps(Event& comparisonEvent) const;

    string toString() const;

    string formatEventDataToString() const;

    void extractEventData(const string& eventString);
};
