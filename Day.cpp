#include "Day.h"

#include <string>
#include <sstream>

#include "Event.h"
#include "EventExceptions.h"
#include "DayExceptions.h"

using namespace std;

void Day::sortEvents() {
    for (int i = 0; i < eventCount - 1; ++i) {
        for (int j = 0; j < eventCount - i - 1; ++j) {
            if (events[j].startTime.isLargerComparedTo(events[j + 1].startTime)) {
                Event temp = events[j];
                events[j] = events[j + 1];
                events[j + 1] = temp;
            }
        }
    }
}


Day::Day(int date, string dayOfWeek) {
    this->date = date;
    this->isDayOff = false;
    this->dayOfWeek = dayOfWeek;
    this->eventCount = 0;
}

void Day::addEvent(Event& event) {
    if (isDayOff) {
        throw DayExceptions(1);
    }
    for (int i = 0; i < eventCount; ++i) {
        if (event.overlaps(events[i])) {
            throw EventExceptions(1);
        }
    }
    if (eventCount >= 10) {
        throw EventExceptions(2);
    }
    events[eventCount++] = event;
    sortEvents();
}

void Day::deleteEvent(string& title) {
    bool eventFound = false;
    for (int i = 0; i < eventCount; ++i) {
        if (events[i].title == title) {
            eventFound = true;
            for (int j = i; j < eventCount - 1; ++j) {
                events[j] = events[j + 1];
            }
            --eventCount;
            break;
        }
    }
    if (!eventFound) {
        throw EventExceptions(3);
    }
}

void Day::shiftEvent(string& title, int newDate, Day* days) {
    bool eventFound = false;
    for (int i = 0; i < eventCount; ++i) {
        if (events[i].title == title) {
            Event eventToShift = events[i];
            // Check for conflicts in the new date
            for (int j = 0; j < days[newDate - 1].eventCount; ++j) {
                if (eventToShift.overlaps(days[newDate - 1].events[j])) {
                    throw EventExceptions(7);
                }
            }
            // Remove the event from the current date
            deleteEvent(title);
            // Add the event to the new date
            days[newDate - 1].addEvent(eventToShift);
            eventFound = true;
            break;
        }
    }
    if (!eventFound) {
        throw EventExceptions(3);
    }
}
void Day::clearEvents() {
    eventCount = 0;
}

string Day::toString() const {
    if (eventCount == 0 && !isDayOff) return "";

    stringstream EventStream;
    EventStream << "\n" << date << " July 2024 (" << dayOfWeek << ")";

    if (isDayOff) {
        EventStream << " (Day Off)";
    }
    EventStream << "\n";

    for (int i = 0; i < eventCount; ++i) {
        EventStream << "  " << events[i].toString() << "\n";
    }

    return EventStream.str();

}

bool Day::toString_print() const {
    if (isDayOff) return true;
    else return false;
}

string Day::formatDayDataToString() const {
    string dayString;
    if (isDayOff) {
        dayString = dayString + to_string(date) + "|off|\n";
    }
    for (int i = 0; i < eventCount; ++i) {
        dayString = dayString + to_string(date) + "|" + events[i].formatEventDataToString() + "\n";
    }
    return dayString;
}

void Day::extractDayData(string& dayStr) {
    stringstream dayStream(dayStr);
    string line;
    while (getline(dayStream, line)) {
        if (line.empty()) {
            continue;
        }
        stringstream lineStream(line);
        string data;
        getline(lineStream, data, '|');
        date = stoi(data);
        getline(lineStream, data, '|');
        if (data == "off") {
            isDayOff = true;
            clearEvents();
        }
        else {
            Event event;
            event.extractEventData(line.substr(line.find('|') + 1));
            isDayOff = false;
            addEvent(event);
        }
    }
}
