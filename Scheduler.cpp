#include "Scheduler.h"

#include "SchedulerExceptions.h"
#include "DayExceptions.h"
#include "Event.h"
#include "Day.h"

// to access colors in the command instruct 
#include <windows.h>
#undef max

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>// for the numeric_limits of the streamsize in the ignore function
#include <string>

//just 1 function used in displayCalendar_print function setw(2)
#include <iomanip>

using namespace std;

// this global object of HANDLE class from windows.h header file to allow command instruct colors
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

void Scheduler::initializeDays() {
    string daysOfWeek[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

    for (int i = 0; i < 31; ++i) {
        days[i] = Day(i + 1, daysOfWeek[i % 7]);
    }
}

void Scheduler::saveEventsTo_txt() {
    ofstream file("EventFile.txt");
    if (!file.is_open()) {
        throw SchedulerExceptions(4);
    }

    for (int i = 0; i < 31; ++i) {
        file << days[i].formatDayDataToString();
    }
    file.close();
}

void Scheduler::loadEventsFrom_txt() {
    ifstream file("EventFile.txt");
    if (!file.is_open()) {
        throw SchedulerExceptions(5);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        int date;
        stringstream lineStream(line);
        string dateStr;
        getline(lineStream, dateStr, '|');
        date = stoi(dateStr);
        days[date - 1].extractDayData(line);
    }
    file.close();

    /*
     * Referred from https://www.digitalocean.com/community/tutorials/getline-in-c-plus-plus
     */
}

void Scheduler::option_list(int index) {
    string option_list[8] = { "       1. Schedule an Event","      2. Cancel an Event","      3. Shift an Event","      4. Set a Day Off","      5. View Day Schedule","               6. View Week Schedule","\t\t\t      7. View Month Schedule","\t      8. Exit" };
    SetConsoleTextAttribute(h, 14);
    cout << option_list[index];
    SetConsoleTextAttribute(h, 11);
    cout << endl;
}


Scheduler::Scheduler(int currentDay) : currentDay(currentDay) {
    try {
        initializeDays();
        loadEventsFrom_txt();
    }
    catch (const exception& exception) {
        cout << "Error : " << exception.what() << endl;
    }
}


Scheduler::~Scheduler() {
    try {
        saveEventsTo_txt();
    }
    catch (const exception& exception) {
        cout << "Error " << exception.what() << endl;
    }
}


void Scheduler::scheduleEvent(int date, Event& event) {
    try {
        if (date < currentDay || date > 31) {
            throw DayExceptions(4);
        }

        if (days[date - 1].isDayOff) {
            string confirmation;
            cout << "The selected day is marked as a day off. Do you want to proceed? (yes/no): ";
            cin.ignore();
            getline(cin, confirmation);

            if (confirmation != "yes" && confirmation != "YES") {
                // Remove the day off status
                days[date - 1].isDayOff = false;
            }
            else {
                return;
            }
        }

        Event newEvent = event;
        newEvent.repeatType = event.repeatType;

        if (event.repeatType == "daily") {
            for (int i = date; i <= 31; ++i) {
                if (!days[i - 1].isDayOff) {
                    days[i - 1].addEvent(newEvent);
                }
            }
        }
        else if (event.repeatType == "weekly") {
            for (int i = date; i <= 31; i += 7) {
                if (!days[i - 1].isDayOff) {
                    days[i - 1].addEvent(newEvent);
                }
            }
        }
        else {
            days[date - 1].addEvent(newEvent);
        }

        cout << "Event scheduled successfully.\n";
    }
    catch (const exception& exception) {
        cout << "Error: " << exception.what() << endl;
    }
}

void Scheduler::cancelEvent(int date, string& title, bool deleteRepeats) {
    try {
        if (date < currentDay || date > 31) {
            throw DayExceptions(4);
        }

        // If deleteRepeats is true, delete all occurrences of the event in the future
        if (!deleteRepeats) {
            for (int i = date - 1; i < 31; ++i) {
                bool eventFound = true;
                while (eventFound) {
                    eventFound = false;
                    for (int j = 0; j < days[i].eventCount; ++j) {
                        if (days[i].events[j].title == title) {
                            days[i].deleteEvent(title);
                            eventFound = true; // Continue to check for more instances of the event on this day
                            break; // Exit the for loop to check the updated list
                        }
                    }
                }
            }
        }
        else {
            days[date - 1].deleteEvent(title);
        }

        cout << "Event cancelled successfully.\n";
    }
    catch (const exception& exception) {
        cout << "Error: " << exception.what() << endl;
    }
}

void Scheduler::shiftEvent(int date, string& title, int newDate) {
    try {
        if (date < currentDay || date > 31 || newDate < currentDay || newDate > 31) {
            throw EventExceptions(5);
        }
        days[date - 1].shiftEvent(title, newDate, days);
        cout << "Event shifted successfully.\n";
    }
    catch (const exception& exception) {
        cout << "Error: " << exception.what() << endl;
    }
}

void Scheduler::setDayOff(int date) {
    try {
        if (date < currentDay || date > 31) {
            throw DayExceptions(4);
        }
        days[date - 1].isDayOff = true;
        days[date - 1].clearEvents();
        cout << "Day off set for " << date << " July 2024.\n";
    }
    catch (const exception& exception) {
        cout << "Error: " << exception.what() << endl;
    }
}

void Scheduler::viewWeekSchedule(int startDay) const {
    if (startDay < 1 || startDay > 31) {
        throw DayExceptions(5);
    }

    int startIndex = ((startDay) / 7) * 7;
    int endIndex = startIndex + 7;
    if (startIndex >= 28) endIndex = 31;

    for (int i = (startIndex == 0 ? 1 : startIndex); i < endIndex; ++i) {
        string output = days[i - 1].toString();
        /*cout << days[i - 1].toString() << endl;*/
        if (!output.empty()) {
            cout << output;
        }
    }
}

void Scheduler::viewDaySchedule(int day) const {
    if (day < 1 || day > 31) {
        throw DayExceptions(3);
    }
    cout << days[day - 1].toString() << endl;
}

void Scheduler::displayScheduler() {
    cout << "\n\t\t\tSchedule - July 2024\n";
    for (int i = 0; i < 31; ++i) {
        string dayStr = days[i].toString();

        if (!dayStr.empty()) {
            cout << dayStr;
        }
    }
}

bool Scheduler::isEventRepeating(int date, const string& title) const {
    const Day& day = days[date - 1];
    for (int i = 0; i < day.eventCount; ++i) {
        if (day.events[i].title == title && day.events[i].repeatType != "none") {
            return true;
        }
    }
    return false;
}

void Scheduler::displayScheduler_print(int today) {
    int option_increment = 0;
    cout << endl;
    SetConsoleTextAttribute(h, 11);
    cout << "======================================================" << endl;
    SetConsoleTextAttribute(h, 14);
    cout << "                     2024 > July" << endl;
    SetConsoleTextAttribute(h, 11);
    cout << "======================================================" << endl << endl;
    SetConsoleTextAttribute(h, 14);
    cout << "   Su Mo Tu We Th Fr Sa";
    SetConsoleTextAttribute(h, 11);
    option_list(option_increment);

    int startDay = 1; // 0 = Sunday, 1 = Monday, ..., 6 = Saturday

    for (int i = 0; i <= 31; ++i) {
        if (i == 0) {
            cout << "      ";
        }
        else if (i == today) {
            SetConsoleTextAttribute(h, 176);
            cout << setw(2) << i << " ";
            SetConsoleTextAttribute(h, 11);
        }
        else if (days[i - 1].toString_print()) {
            SetConsoleTextAttribute(h, 12);
            cout << setw(2) << i << " ";
            SetConsoleTextAttribute(h, 11);
        }
        else {
            cout << setw(2) << i << " ";
        }
        if ((i + startDay) % 7 == 0) {
            option_increment++;
            option_list(option_increment);
            //cout << endl;
            cout << "   ";
        }
    }
    option_list(5);
    option_list(6);
    SetConsoleTextAttribute(h, 12);
    cout << "   XX";
    SetConsoleTextAttribute(h, 14);
    cout << " > Off Days";
    option_list(7);
    cout << "\n";
    SetConsoleTextAttribute(h, 7);

}

