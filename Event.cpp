#include "Event.h"
#include "EventExceptions.h"

#include <sstream>
#include <iostream>

using namespace std;

Event::Event(string title, Time startTime, Time endTime, string repeatType) {
    this->title = title;
    this->startTime = startTime;
    this->endTime = endTime;
    this->repeatType = repeatType;

    if (endTime.isSmallerComparedTo(startTime)) {
        throw EventExceptions(6);
    }
}

bool Event::overlaps(Event& comparisonEvent) const {
    return (startTime.isSmallerComparedTo(comparisonEvent.endTime) && endTime.isLargerComparedTo(comparisonEvent.startTime));
}

string Event::toString() const {
    return title + " from " + startTime.toString() + " to " + endTime.toString() + " (" + repeatType + ")";
}

string Event::formatEventDataToString() const {
    return title + "|" + startTime.toString() + "|" + endTime.toString() + "|" + repeatType;
}

void Event::extractEventData(const string& eventString) {
    stringstream eventStream(eventString);
    getline(eventStream, title, '|');

    string startTimeString, endTimeString;
    getline(eventStream, startTimeString, '|');
    getline(eventStream, endTimeString, '|');
    getline(eventStream, repeatType);

    startTime.fromString(startTimeString);
    endTime.fromString(endTimeString);
}
