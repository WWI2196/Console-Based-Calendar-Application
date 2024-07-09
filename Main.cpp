#include "Time.h"
#include "Exceptions.h"
#include "Event.h"
#include "Day.h"
#include "Scheduler.h"
#include "EventExceptions.h"
#include "DayExceptions.h"
#include "SchedulerExceptions.h"
#include "TimeExceptions.h" 

#include <iostream>
#include <sstream>

using namespace std;

int validateInput(int startValue, int endValue, const string& instruct) {
    int input;

    while (true) {
        cout << instruct;
        cin >> input;

        if (!cin.fail() && input >= startValue && input <= endValue) {
            break;
        }
        else {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
            cout << "Invalid input. Please enter a valid number between " << startValue << " and " << endValue << ".\n";
        }
    }
    return input;
}

string validateString(const string& instruct) {
    string input;

    while (true) {
        cout << instruct;
        cin.ignore();
        getline(cin, input);

        if (input.empty()) {
            cout << "You can not keep the title empty. Please enter a name." << endl;
        }
        else {
            break;
        }
    }
    return input;

}

bool validateOption(const  string& instruct) {
    string option;

    cout << instruct;
    cin.ignore();
    getline(cin, option);

    //if the first letter starts with y or Y then return true else false
    if (option[0] == 'y' || option[0] == 'Y') {
        return true;
    }
    else {
        return false;
    }
}

void validateTime(const string& instruct, int& hour, int& minute) {
    string timeInput;
    char colon;

    while (true) {
        cout << instruct;
        cin >> timeInput;

        stringstream timeStream(timeInput);
        if (timeStream >> hour >> colon >> minute && (hour >= 0 && hour < 24) && (minute >= 0 && minute < 60)) {
            break;
        }
        else {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer

            /*
         * Referred from : https://stackoverflow.com/questions/25020129/cin-ignorenumeric-limitsstreamsizemax-n
         * Referred from : https://stackoverflow.com/questions/20446373/cin-ignorenumeric-limitsstreamsizemax-n-max-not-recognize-it
         */

            cout << "Invalid time format. Please enter time in HH:MM format(24 hour).\n";
        }
    }
}

int main() {

    int currentDay;

    currentDay = validateInput(1, 31, "\nEnter the current day (1-31): ");


    Scheduler scheduler(currentDay);

    while (true) {

        scheduler.displayScheduler_print(currentDay);

        int option = validateInput(1, 8, "\nChoose an option: ");

        if (option == 8) {
            cout << "You have exited the program.\n";
            break;
        }


        switch (option) {
        case 1: {

            int startHour, startMinute, endHour, endMinute;

            int date = validateInput(currentDay, 31, "Enter the date (" + to_string(currentDay) + "-31): ");
            string title = validateString("Enter event title: ");
            validateTime("Enter start time (HH:MM): ", startHour, startMinute);
            validateTime("Enter end time (HH:MM): ", endHour, endMinute);
            string repeatType = validateString("Enter repeat type (none, daily, weekly): ");


            try {
                Time start(startHour, startMinute);
                Time end(endHour, endMinute);
                Event event(title, start, end, repeatType);
                scheduler.scheduleEvent(date, event);
            }
            catch (const exception& exception) {
                cout << "Error: " << exception.what() << endl;
            }
            break;
        }
        case 2: {

            int date = validateInput(currentDay, 31, "Enter the date (1-31): ");
            string title = validateString("Enter event title: ");

            if (scheduler.isEventRepeating(date, title)) {
                bool deleteRepeats = validateOption("\nThis is a repeating event.\nDelete all repeating events with the same title? (yes/no): ");
                scheduler.cancelEvent(date, title, deleteRepeats);
            }
            else {
                scheduler.cancelEvent(date, title, false);
            }
            break;
        }
        case 3: {

            int date = validateInput(currentDay, 31, "Enter the shifting date (" + to_string(currentDay) + "-31): ");
            string title = validateString("Enter event title: ");
            int newDate = validateInput(currentDay, 31, "Enter the new date (" + to_string(currentDay) + "-31): ");

            scheduler.shiftEvent(date, title, newDate);
            break;
        }
        case 4: {

            int date = validateInput(currentDay, 31, "Enter date (" + to_string(currentDay) + "-31): ");

            scheduler.setDayOff(date);
            break;
        }
        case 5: {

            int date = validateInput(currentDay, 31, "Enter date (" + to_string(currentDay) + "-31): ");

            try {
                scheduler.viewDaySchedule(date);

            }
            catch (const exception& exception) {
                cout << "Error: " << exception.what() << endl;
            }
            break;
        }

        case 6: {

            int startDate = validateInput(currentDay, 31, "Enter date (" + to_string(currentDay) + "-31): ");

            try {
                scheduler.viewWeekSchedule(startDate);
            }
            catch (const exception& exception) {
                cout << "Error: " << exception.what() << endl;
            }
            break;
        }
        case 7: {

            scheduler.displayScheduler();
            break;
        }

        }

    }

    return 0;
}