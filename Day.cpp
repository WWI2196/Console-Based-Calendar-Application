#include "Day.h"

#include <string>
#include <sstream>

#include "Event.h"
#include "EventExceptions.h"
#include "DayExceptions.h"

using namespace std;

void Day::sortEvents() { // Bubble sort of the events in the day according to the start time
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

void Day::addEvent(Event& event) { // Add an event to the day
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


void Day::deleteEvent(string& title) { // Delete an event from the day
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

void Day::shiftEvent(string& title, int newDate, Day* days) { // Shift an event to another day
    bool eventFound = false;

    for (int i = 0; i < eventCount; ++i) {
        if (events[i].title == title) {
            Event eventToShift = events[i];
            
            for (int j = 0; j < days[newDate - 1].eventCount; ++j) {
                if (eventToShift.overlaps(days[newDate - 1].events[j])) { // Check if the event overlaps with any other events on the new date
                    throw EventExceptions(7);
                }
            }
           
            deleteEvent(title);  // Remove the event from the current date
            
            days[newDate - 1].addEvent(eventToShift);// Add the event to the new date
            eventFound = true;
            break;
        }
    }
    if (!eventFound) {
        throw EventExceptions(3);
    }
}
void Day::clearEvents() { // Clear all events from the day
    eventCount = 0;
}

string Day::toString() const { // Convert the day data to a string
    if (eventCount == 0 && !isDayOff) return "";

    stringstream EventStream;
    EventStream << "\n      " << date << " July 2024 (" << dayOfWeek << ")";

    if (isDayOff) {
        EventStream << " (Day Off)";
    }
    EventStream << "\n";

    for (int i = 0; i < eventCount; ++i) {
        EventStream << "  " << events[i].toString() << "\n";
    }

    return EventStream.str(); // pass stringstream as string from the function

}

bool Day::toString_print() const { 
    if (isDayOff) return true;
    else return false;
}

string Day::formatDayDataToString() const { 
    string dayString;
    if (isDayOff) {
        dayString = dayString + to_string(date) + "|off|\n"; // If the day is off, only the date and "off" are stored
    }
    for (int i = 0; i < eventCount; ++i) { // If the day is not off, the date and all the events are stored
        dayString = dayString + to_string(date) + "|" + events[i].formatEventDataToString() + "\n";
    }
    return dayString; 
}

void Day::extractDayData(string& dayStr) {
    stringstream dayStream(dayStr);
    string line;
    while (getline(dayStream, line)) {  // Extract the day data from the string
        if (line.empty()) { // If the line is empty, continue to the next line
            continue;
        }
        stringstream lineStream(line); 
        string data;

        getline(lineStream, data, '|'); // Extract the date 
        date = stoi(data);

        getline(lineStream, data, '|'); // Extract the day off status
        if (data == "off") { // If the day is off, set the day off status to true
            isDayOff = true;
            clearEvents();
        }
        else { // If the day is not off, extract the event data
            Event event;
            event.extractEventData(line.substr(line.find('|') + 1));
            isDayOff = false;
            addEvent(event);
        }
    }
}
