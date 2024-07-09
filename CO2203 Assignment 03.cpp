
#include<windows.h>     // to access colors in the command instructions 
 
#undef max              // max() function is defined in both limits and windows.h and limits, therefore to avoid clash between definitions max function 
                       // defined in windows.h is removed

#include <iostream>
#include <string>
#include <cstdio>      // this is to add printf function, but that is not anymore used in the code 
#include <stdexcept>
#include <fstream>  
#include <sstream>     // Stringstream function is used to convert the strings to stream which will be usefull throughout the code. 
#include <limits>     // for the numeric_limits of the streamsize in the ignore function

#include <iomanip>    //just 1 function used in displayCalendar_print function setw(2)

HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);   // this global object of HANDLE class from windows.h header file to allow command instruct colors 

using namespace std;

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

class Exceptions : public exception {
protected:
    int errorCode;
    string errorMessage;

public:
    Exceptions(int code) {
        this->errorCode = code;
    }

    virtual const char* what() const noexcept override {
        return errorMessage.c_str();
    }
    /*
     * Referred from the GitHub repository: phosphor-pid-control https://github.com/openbmc/phosphor-pid-control/blob/master/errors/exception.hpp
     * Author: OpenBMC
     */
};

class TimeExceptions : public Exceptions {
public:
    TimeExceptions(int code) : Exceptions(code) {
        switch (errorCode) {
        case 1:
            errorMessage = "Invalid time format";
            break;
        case 2:
            errorMessage = "";
            break;
        default:
            errorMessage = "Time error";
        }
    }
};

class EventExceptions : public Exceptions {
public:
    EventExceptions(int code) : Exceptions(code) {
        switch (errorCode) {
        case 1:
            errorMessage = "Event overlaps with an existing event";
            break;
        case 2:
            errorMessage = "Maximum number of events reached for a day";
            break;
        case 3:
            errorMessage = "No such event exists";
            break;
        case 4:
            errorMessage = "Cannot schedule events on a day off";
            break;
        case 5:
            errorMessage = "Invalid date for shifting events";
            break;
        case 6:
            errorMessage = "Event end time must be after start time";
            break;
        case 7:
            errorMessage = "Event overlaps with an existing event on the new date";
            break;
        default:
            errorMessage = "Event error";
        }
    }
};

class DayExceptions : public Exceptions {
public:
    DayExceptions(int code) : Exceptions(code) {
        switch (errorCode) {
        case 1:
            errorMessage = "Cannot schedule events on a day off";
            break;
        case 2:
            errorMessage = "Unable to open file for loading";
            break;
        case 3:
            errorMessage = "Invalid day for viewing schedule";
            break;
        case 4:
            errorMessage = "Cannot schedule events in the past or beyond July 2024";
            break;
        case 5:
            errorMessage = "Invalid start day for viewing week schedule";
            break;
        default:
            errorMessage = "Day error";
        }
    }
};

class SchedulerExceptions : public Exceptions {
public:
    SchedulerExceptions(int code) : Exceptions(code) {
        switch (errorCode) {
        case 1:
            errorMessage = "Invalid input. Please enter a valid date.";
            break;
        case 2:
            errorMessage = "The number entered is out of range.";
            break;
        case 3:
            errorMessage = "Invalid input. Please enter a valid option.";
            break;
        case 4:
            errorMessage = "Unable to open file for saving";
            break;
        case 5:
            errorMessage = "Unable to open file for loading";
            break;
        default:
            errorMessage = "Scheduler error";
        }
    }
};

class Time {
private:
    int hour;
    int minute;
public:

    Time(int hour = 0, int minute = 0) {
        this->hour = hour;
        this->minute = minute;

        if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
            throw TimeExceptions(1);
        }
    }

    bool isLessThan(Time& comparisonTime) const {
        return (hour < comparisonTime.hour) || (hour == comparisonTime.hour && minute < comparisonTime.minute);
    }

    bool isGreaterThan(Time& comparisonTime) const {
        return (hour > comparisonTime.hour) || (hour == comparisonTime.hour && minute > comparisonTime.minute);
    }

    bool isEqualTo(Time& comparisonTime) const {
        return hour == comparisonTime.hour && minute == comparisonTime.minute;
    }

    string toString() const {
        return (hour < 10 ? "0" : "") + to_string(hour) + ":" + (minute < 10 ? "0" : "") + to_string(minute);
    }

    void fromString(string& timeString) {
        stringstream timeStream(timeString);  // Here is one way that we usd stringstream where to convert stream which enable us to use stream managing tools in cpp such as >>
        char separator;

        timeStream >> hour >> separator >> minute;
        if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
            throw TimeExceptions(1);
        }
    }
    /*
     * Referred from the GitHub repository: Appointment-Booking https://github.com/pgagliano/Appointment-Booking/blob/master/myTime.cpp
     * Author: Patrick Gagliano
     */
};

class Event {
public:
    string title;
    Time startTime;
    Time endTime;
    string repeatType; // "none", "daily", "weekly"

    Event(string title = "EVENT", Time startTime = Time(), Time endTime = Time(), string repeatType = "none") {
        this->title = title;
        this->startTime = startTime;
        this->endTime = endTime;
        this->repeatType = repeatType;

        if (endTime.isLessThan(startTime)) {
            throw EventExceptions(6);
        }
    }

    bool overlaps(Event& comparisonEvent) const {
        return (startTime.isLessThan(comparisonEvent.endTime) && endTime.isGreaterThan(comparisonEvent.startTime));
    }

    string toString() const {
        return "         " + title + " from " + startTime.toString() + " to " + endTime.toString() + " (" + repeatType + ")";
    }

    string formatEventDataToString() const {
        return title + "|" + startTime.toString() + "|" + endTime.toString() + "|" + repeatType;
    }

    void extractEventData(const string& eventString) {
        stringstream eventStream(eventString);
        getline(eventStream, title, '|');      // Here is another use of stringstream, where we can use string handling functions like getline(), where the first parameter should insert as stream.

        string startTimeString, endTimeString;
        getline(eventStream, startTimeString, '|');
        getline(eventStream, endTimeString, '|');
        getline(eventStream, repeatType);

        startTime.fromString(startTimeString);
        endTime.fromString(endTimeString);
    }
};

class Day {
private:
    void sortEvents() {
        for (int i = 0; i < eventCount - 1; ++i) {
            for (int j = 0; j < eventCount - i - 1; ++j) {
                if (events[j].startTime.isGreaterThan(events[j + 1].startTime)) {
                    Event temp = events[j];
                    events[j] = events[j + 1];
                    events[j + 1] = temp;
                }
            }
        }
    }

public:
    int date;
    bool isDayOff;
    Event events[10];  // Fixed size array to store events
    int eventCount;
    string dayOfWeek;

    Day(int date = 0, string dayOfWeek = "") {
        this->date = date;
        this->isDayOff = false;
        this->dayOfWeek = dayOfWeek;
        this->eventCount = 0;
    }

    void addEvent(Event& event) {
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

    void deleteEvent(string& title) {
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

    void shiftEvent(string& title, int newDate, Day* days) {
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

    void clearEvents() {
        eventCount = 0;
    }

    string toString() const {
        if (eventCount == 0 && !isDayOff) return "";

        stringstream ss;
        ss << "\n      " << date << " July 2024 (" << dayOfWeek << ")";

        if (isDayOff) {
            ss << " (Day Off)";
        }
        ss << "\n";

        for (int i = 0; i < eventCount; ++i) {
            ss << "  " << events[i].toString() << "\n";
        }

        return ss.str();   //pass stringstream as string from the function

    }

    bool toString_print() const {
        if (isDayOff) return true;
        else return false;
    }

    string formatDayDataToString() const {
        string dayString;
        if (isDayOff) {
            dayString = dayString + to_string(date) + "|off|\n";
        }
        for (int i = 0; i < eventCount; ++i) {
            dayString = dayString + to_string(date) + "|" + events[i].formatEventDataToString() + "\n";
        }
        return dayString;
    }

    void extractDayData(string& dayStr) {
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
};

class Scheduler {
private:
    Day days[31];
    int currentDay;

    void initializeDays() {
        string daysOfWeek[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

        for (int i = 0; i < 31; ++i) {
            days[i] = Day(i + 1, daysOfWeek[i % 7]);
        }
    }

    void saveEventsTo_txt() {
        ofstream file("EventFile.txt");
        if (!file.is_open()) {
            throw SchedulerExceptions(4);
        }
        
        for (int i = 0; i < 31; ++i) {
            file << days[i].formatDayDataToString();
        }
        file.close();
    }

    void loadEventsFrom_txt() {
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

    void option_list(int index) {
        string option_list[8] = { "       1. Schedule an Event","      2. Cancel an Event","      3. Shift an Event","      4. Set a Day Off","      5. View Day Schedule","               6. View Week Schedule","\t\t\t      7. View Month Schedule","\t      8. Exit" };
        cout << setColor(option_list[index], 14);
        cout << endl;
    }

public:

    Scheduler(int currentDay) : currentDay(currentDay) {
        try {
            initializeDays();
            loadEventsFrom_txt();
        }
        catch (const exception& exception) {
            cout << setColor("   Error : ", 12) << setColor(exception.what(), 12) << endl;
        }
    }

    ~Scheduler() {
        try {
            saveEventsTo_txt();
        }
        catch (const exception& exception) {
            cout << setColor("   Error ", 12) << setColor(exception.what(), 12) << endl;
        }
    }

    void scheduleEvent(int date, Event& event) {
        try {
            if (date < currentDay || date > 31) {
                throw DayExceptions(4);
            }

            if (days[date - 1].isDayOff) {
                string confirmation;
                cout << setColor("      The selected day is marked as a day off. Do you want to proceed? (",15);
                cout << setColor("yes", 10);
                cout << setColor(" / ", 15);
                cout << setColor("no", 12);
                cout << setColor(") ", 15);

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

            cout << setColor("   Event scheduled successfully.\n", 10);
        }
        catch (const exception& exception) {
            cout << setColor("   Error: ", 12) << setColor(exception.what(), 12) << endl;
        }
    }
    void cancelEvent(int date, string& title, bool deleteRepeats) {
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


    void shiftEvent(int date, string& title, int newDate) {
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

    void setDayOff(int date) {
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

    void viewDaySchedule(int day) const {
        if (day < 1 || day > 31) {
            throw DayExceptions(3);
        }
        cout << "   " << setColor(days[day - 1].toString(), 9) << endl;
    }

    void viewWeekSchedule(int startDay) const {
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
                cout << "   " << setColor(output, 9);
            }
        }
    }

    void displayScheduler() {
        cout << setColor("\n\t\t\tJuly 2024 Calendar\n", 9);
        for (int i = 0; i < 31; ++i) {
            string dayStr = days[i].toString();

            if (!dayStr.empty()) {
                cout << "   " << setColor(dayStr, 9);
            }
        }
    }

    bool isEventRepeating(int date, const string& title) const {
        const Day& day = days[date - 1];
        for (int i = 0; i < day.eventCount; ++i) {
            if (day.events[i].title == title && day.events[i].repeatType != "none") {
                return true;
            }
        }
        return false;
    }

    void displayScheduler_print(int today) {
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
                cout << setw(2) << setColor(to_string(i), 16) << " ";
            }
            else if (days[i - 1].toString_print()) {
                cout << setw(2) << setColor(to_string(i), 12) << " ";
            }
            else {
                cout << setw(2) << setColor(to_string(i), 11) << " ";
            }
            if ((i + startDay) % 7 == 0) {
                option_increment++;
                option_list(option_increment);
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
};

int validateInput(int startValue, int endValue, const string& instruct, bool value) {
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

int validateInput(int startValue, int endValue, const string& instruct) {
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

string validateString(const string& instruct) {
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

bool validateOption(const  string& instruct) {
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

void validateTime(const string& instruct, int& hour, int& minute) {
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

    int currentDay;
    currentDay = validateInput(1, 31, setColor("\nEnter the current day (1-31): ", 8), true);


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
                bool deleteRepeats = validateOption("      Delete all repeating events with the same title? (yes/no): ");
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
