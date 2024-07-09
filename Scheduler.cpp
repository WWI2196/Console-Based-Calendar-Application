#include "Scheduler.h"

#include "SchedulerExceptions.h"
#include "DayExceptions.h"
#include "Event.h"
#include "Day.h"


#include <windows.h> // to access colors in the command instruct 
#undef max          // max() function is defined in both limits and windows.h and limits, therefore to avoid clash between definitions max function 
                    // defined in windows.h is removed

#include <iostream>
#include <fstream>
#include <sstream> // Stringstream function is used to convert the strings to stream which will be usefull throughout the code. 
#include <limits> // for the numeric_limits of the streamsize in the ignore function
#include <string>

#include <iomanip> //just 1 function used in displayCalendar_print function setw(2)

using namespace std;

HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); // this global object of HANDLE class from windows.h header file to allow command instruct colors

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
    cout << setColor(option_list[index], 14);
    cout << endl;
}


Scheduler::Scheduler(int currentDay) {
    this->currentDay = currentDay;
    try {
        initializeDays();
        loadEventsFrom_txt();
    }
    catch (const exception& exception) {
        cout << setColor("   Error : ", 12) << setColor(exception.what(), 12) << endl;
    }
}


Scheduler::~Scheduler() {
    try {
        saveEventsTo_txt();
    }
    catch (const exception& exception) {
        cout << setColor("   Error ", 12) << setColor(exception.what(), 12) << endl;
    }
}

void Scheduler::scheduleEvent(int date, Event& event) {
    try {
        if (date < currentDay || date > 31) {
            throw DayExceptions(4);
        }

        if (days[date - 1].isDayOff) {
            string confirmation;
            cout << setColor("      The selected day is marked as a day off. Do you want to proceed? (", 15);
            cout << setColor("yes", 10);
            cout << setColor(" / ", 15);
            cout << setColor("no", 12);
            cout << setColor(") ", 15);

            getline(cin, confirmation);

            if (confirmation != "yes" && confirmation != "YES") {
                cout << setColor("Event not scheduled as the day is marked as a day off.\n", 12);
                return;
            }

            // Remove the day off status if user confirms
            days[date - 1].isDayOff = false;
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

        cout << setColor("   Event scheduled successfully.\n", 10);
    }
    catch (const exception& exception) {
        cout << setColor("   Error: ", 12) << setColor(exception.what(), 12) << endl;
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

        cout << setColor("   Event cancelled successfully.\n", 12);
    }
    catch (const exception& exception) {
        cout << setColor("   Error: ", 12) << setColor(exception.what(), 12) << endl;
    }
}

void Scheduler::shiftEvent(int date, string& title, int newDate) {
    try {
        if (date < currentDay || date > 31 || newDate < currentDay || newDate > 31) {
            throw EventExceptions(5);
        }
        days[date - 1].shiftEvent(title, newDate, days);
        cout << setColor("   Event shifted successfully.\n", 10);
    }
    catch (const exception& exception) {
        cout << setColor("   Error: ", 12) << setColor(exception.what(), 12) << endl;
    }
}

void Scheduler::setDayOff(int date) {
    try {
        if (date < currentDay || date > 31) {
            throw DayExceptions(4);
        }
        days[date - 1].isDayOff = true;
        days[date - 1].clearEvents();
        cout << setColor("   Day off set for ", 10) << setColor(to_string(date), 10) << setColor(" July 2024.\n", 10);
    }
    catch (const exception& exception) {
        cout << setColor("   Error: ", 12) << setColor(exception.what(), 12) << endl;
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
   
        if (!output.empty()) {
            cout << "   " << setColor(output, 9);
        }
    }
}

void Scheduler::viewDaySchedule(int day) const {
    if (day < 1 || day > 31) {
        throw DayExceptions(3);
    }
    cout << "   " << setColor(days[day - 1].toString(), 9) << endl;
}

void Scheduler::displayScheduler() {
    cout << setColor("\n\t\t\tSchedule - July 2024\n", 9);
    for (int i = 0; i < 31; ++i) {
        string dayStr = days[i].toString();

        if (!dayStr.empty()) {
            cout << "   " << setColor(dayStr, 9);
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
    cout << setColor("======================================================", 11) << endl;
    cout << setColor("                     2024 > July", 14) << endl;
    cout << setColor("======================================================", 11) << endl << endl;
    cout << setColor("   Su Mo Tu We Th Fr Sa", 14);
    option_list(option_increment);

    int startDay = 1; // 0 = Sunday, 1 = Monday, ..., 6 = Saturday

    for (int i = 0; i <= 31; ++i) {
        if (i == 0) {
            cout << "      ";
        }
        else if (i == today) {
            SetConsoleTextAttribute(h, 176);
            cout << setw(2) << setColor(to_string(i), 16) << " ";
            SetConsoleTextAttribute(h, 11);
        }
        else if (days[i - 1].toString_print()) {
            SetConsoleTextAttribute(h, 12);
            cout << setw(2) << setColor(to_string(i), 12) << " ";
            SetConsoleTextAttribute(h, 11);
        }
        else {
            cout << setw(2) << setColor(to_string(i), 11) << " ";
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
    cout << setColor("   XX", 12);
    cout << setColor(" > Off Days", 14);
    option_list(7);
    cout << "\n";

}

string setColor(const string& txt, const int& color) {

    switch (color) {
    case 8:
        SetConsoleTextAttribute(h, 8); // Light Ash
        break;
    case 9:
        SetConsoleTextAttribute(h, 9); // Light Blue
        break;
    case 10:
        SetConsoleTextAttribute(h, 10); // Light Green
        break;
    case 11:
        SetConsoleTextAttribute(h, 11); // Light Aqua
        break;
    case 12:
        SetConsoleTextAttribute(h, 12); // Light Red
        break;
    case 13:
        SetConsoleTextAttribute(h, 13); // Light Purple
        break;
    case 14:
        SetConsoleTextAttribute(h, 14); // Light Yello
        break;
    case 15:
        SetConsoleTextAttribute(h, 15); // Light White
        break;
    case 16:
        SetConsoleTextAttribute(h, 176); // Light White
        break;
    default:
        SetConsoleTextAttribute(h, 8);  // Default color
        break;
    }

    return txt;
}