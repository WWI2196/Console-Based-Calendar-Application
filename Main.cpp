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
#include <limits>

using namespace std;


int validateInput_currentDay(int startValue, int endValue, const string& instruct) { // Validate the input for the current day
    int input;

    while (true) {
        cout << setColor(instruct, 8);
        cin >> input;

        if (!cin.fail() && input >= startValue && input <= endValue) {
            break;
        }
        else {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
            cout << setColor("Invalid input. Please enter a valid number between ", 12) << setColor(to_string(startValue), 12) << setColor(" and ", 12) << setColor(to_string(endValue), 12) << ".\n";
        }
    }
    return input;
}
int validateInput(int startValue, int endValue, const string& instruct) { // Validate the input for the options such as dates
    int input;

    while (true) {
        cout << setColor(instruct, 15);
        cin >> input;

        if (!cin.fail() && input >= startValue && input <= endValue) {
            break;
        }
        else {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
            cout << setColor("   Invalid input. Please enter a valid number between ", 12) << setColor(to_string(startValue), 12) << setColor(" and ", 12) << setColor(to_string(endValue), 12) << ".\n";
        }
    }
    return input;
}

string validateString(const string& instruct) { // Validate the input for the strings
    string input;

    while (true) {
        cout << setColor(instruct, 15);
        cin.ignore();
        getline(cin, input);

        if (input.empty()) {
            cout << setColor("      You can not keep the title empty. Please enter a name.", 12) << endl;
        }
        else {
            break;
        }
    }
    return input;

}

bool validateOption(const  string& instruct) { // Validate the input for the menu options
    string option;

    cout << setColor(instruct, 15);
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

void validateTime(const string& instruct, int& hour, int& minute) { // Validate the input for the time
    string timeInput;
    char colon;

    while (true) {
        cout << setColor(instruct, 15);
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

            cout << setColor("      Invalid time format. Please enter time in HH:MM format(24 hour).\n", 12);
        }
    }
}

int main() {

    int currentDay = validateInput_currentDay(1, 31, setColor("\nEnter the current day (1-31): ", 8)); // set the current day


    Scheduler scheduler(currentDay);

    while (true) {

        scheduler.displayScheduler_print(currentDay);

        int option = validateInput(1, 8, setColor("\n   Choose an option: ", 15));

        if (option == 8) {
            cout << setColor("You have exited the program.\n", 12);
            cout << setColor("", 8) << endl;
            break;
        }


        switch (option) {
        case 1: {

            int startHour, startMinute, endHour, endMinute;

            int date = validateInput(currentDay, 31, "      Enter date (" + to_string(currentDay) + "-31): ");
            string title = validateString("      Enter event title: ");
            validateTime("      Enter start time (HH:MM): ", startHour, startMinute);
            validateTime("      Enter end time (HH:MM): ", endHour, endMinute);
            string repeatType = validateString("      Enter repeat type (none, daily, weekly): ");


            try {
                Time start(startHour, startMinute);
                Time end(endHour, endMinute);
                Event event(title, start, end, repeatType);
                scheduler.scheduleEvent(date, event);
            }
            catch (const exception& exception) {
                cout << setColor("   Error: ", 12) << exception.what() << endl;
            }
            break;
        }
        case 2: {

            int date = validateInput(currentDay, 31, "      Enter date (1-31): ");
            string title = validateString("      Enter event title: ");

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

            int date = validateInput(currentDay, 31, "      Enter date (" + to_string(currentDay) + "-31): ");
            string title = validateString("      Enter event title: ");
            int newDate = validateInput(currentDay, 31, "      Enter new date (" + to_string(currentDay) + "-31): ");

            scheduler.shiftEvent(date, title, newDate);
            break;
        }
        case 4: {

            int date = validateInput(currentDay, 31, "      Enter date (" + to_string(currentDay) + "-31): ");

            scheduler.setDayOff(date);
            break;
        }
        case 5: {

            int date = validateInput(currentDay, 31, "      Enter date (" + to_string(currentDay) + "-31): ");

            try {
                scheduler.viewDaySchedule(date);

            }
            catch (const exception& exception) {
                cout << setColor("   Error: ", 12) << exception.what() << endl;
            }
            break;
        }

        case 6: {

            int startDate = validateInput(currentDay, 31, "      Enter date (" + to_string(currentDay) + "-31): ");

            try {
                scheduler.viewWeekSchedule(startDate);
            }
            catch (const exception& exception) {
                cout << setColor("   Error: ", 12) << exception.what() << endl;
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