// to access colors in the command instruct 
#include<windows.h>
#undef max

#include <iostream>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <limits> // for the numeric_limits of the streamsize in the ignore function

//just 1 function used in displayCalendar_print function setw(2)
#include <iomanip>

// this global object of HANDLE class from windows.h header file to allow command instruct colors 
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);


using namespace std;

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

    bool isLessThan(const Time& other) const {
        return (hour < other.hour) || (hour == other.hour && minute < other.minute);
    }

    bool isGreaterThan(const Time& other) const {
        return (hour > other.hour) || (hour == other.hour && minute > other.minute);
    }

    bool isEqualTo(const Time& other) const {
        return hour == other.hour && minute == other.minute;
    }

    string toString() const {
        return (hour < 10 ? "0" : "") + to_string(hour) + ":" + (minute < 10 ? "0" : "") + to_string(minute);
    }

    void fromString(const string& timeStr) {
        stringstream ss(timeStr);
        char delim;
        ss >> hour >> delim >> minute;
        if (delim != ':' || hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
            throw TimeExceptions(1);
        }
    }

    string serialize() const {
        return toString();
    }
    
    void deserialize(const string& timeStr) {
        fromString(timeStr);
    }
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

    bool overlaps(const Event& other) const {
        return (startTime.isLessThan(other.endTime) && endTime.isGreaterThan(other.startTime));
    }

    string toString() const {
        return title + " from " + startTime.toString() + " to " + endTime.toString() + " (" + repeatType + ")";
    }

    string serialize() const {
        return title + "|" + startTime.serialize() + "|" + endTime.serialize() + "|" + repeatType;
    }

    void deserialize(const string& eventStr) {
        stringstream ss(eventStr);
        getline(ss, title, '|');
        string startTimeStr, endTimeStr;
        getline(ss, startTimeStr, '|');
        getline(ss, endTimeStr, '|');
        getline(ss, repeatType);
        startTime.deserialize(startTimeStr);
        endTime.deserialize(endTimeStr);
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

    void addEvent(const Event& event) {
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

    void deleteEvent(const string& title) {
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


    void shiftEvent(const string& title, int newDate, Day* days) {
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

        string result = to_string(date) + " July 2024 (" + dayOfWeek + ")";

        if (isDayOff) {
            result += " (Day Off)";
        }
        result += "\n";

        for (int i = 0; i < eventCount; ++i) {
            result += "  " + events[i].toString() + "\n";
        }

        return result;
    }

    bool toString_print() const {
        if (isDayOff) return true;
        else return false;
    }

    string serialize() const {
        string serializedDay;
        if (isDayOff) {
            serializedDay += to_string(date) + "|off|\n";
        }
        for (int i = 0; i < eventCount; ++i) {
            serializedDay += to_string(date) + "|" + events[i].serialize() + "\n";
        }
        return serializedDay;
    }

    void deserialize(const string& dayStr) {
        stringstream ss(dayStr);
        string line;
        while (getline(ss, line)) {
            if (line.empty()) {
                continue;
            }
            stringstream ssLine(line);
            string token;
            getline(ssLine, token, '|');
            date = stoi(token);
            getline(ssLine, token, '|');
            if (token == "off") {
                isDayOff = true;
                clearEvents();
            }
            else {
                Event event;
                event.deserialize(line.substr(line.find('|') + 1));
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
            file << days[i].serialize();
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
            stringstream ss(line);
            string dateStr;
            getline(ss, dateStr, '|');
            date = stoi(dateStr);
            days[date - 1].deserialize(line);
        }
        file.close();

        /*
         * Referred from https://www.digitalocean.com/community/tutorials/getline-in-c-plus-plus
         */
    }

    


public:

    Scheduler(int currentDay) : currentDay(currentDay) {
        try {
            initializeDays();
            loadEventsFrom_txt();
        }
        catch (const exception& exception) {
            cout << "Error : " << exception.what() << endl;
        }
    }

    ~Scheduler() {
        try {
            saveEventsTo_txt();
        }
        catch (const exception& exception) {
            cout << "Error " << exception.what() << endl;
        }
    }

    void scheduleEvent(int date, const Event& event) {
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
    void cancelEvent(int date, const string& title, bool deleteRepeats) {
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



    void shiftEvent(int date, const string& title, int newDate) {
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

    void setDayOff(int date) {
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

    void displayScheduler() {
        cout << "\n\t\t\tJuly 2024 Calendar\n";
        for (int i = 0; i < 31; ++i) {
            string dayStr = days[i].toString();
            if (!dayStr.empty()) {
                cout << dayStr << endl;
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

    void option_list(int i) {
        string option_list[8] = { "       1. Schedule an Event","      2. Cancel an Event","      3. Shift an Event","      4. Set a Day Off","      5. View Day Schedule","               6. View Week Schedule","\t\t\t      7. View Month Schedule","\t      8. Exit" };
        SetConsoleTextAttribute(h, 14);
        cout << option_list[i];
        SetConsoleTextAttribute(h, 11);
        cout << endl;
    }

    void displayScheduler_print(int today) {
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

    void viewDaySchedule(int day) const {
        if (day < 1 || day > 31) {
            throw DayExceptions(3);
        }
        cout << days[day - 1].toString() << endl;
    }

    void viewWeekSchedule(int startDay) const {
        if (startDay < 1 || startDay > 31) {
            throw DayExceptions(5);
        }

        int startIndex = ((startDay - 1) / 7) * 7 + 1;
        int endIndex = startIndex + 7;
        if (startIndex >= 28) endIndex = 31;

        for (int i = startIndex; i < endIndex; ++i) {
            cout << days[i - 1].toString() << endl;
        }
    }
};

int validateInput(int startValue, int endValue,const string& instruct) {
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
        if (timeStream >> hour >> colon >> minute && colon == ':' && (hour >= 0 && hour < 24) && (minute >= 0 && minute < 60)) {
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

            int date = validateInput(currentDay, 31, "Enter date (" + to_string(currentDay) + "-31): ");
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

            int date = validateInput(currentDay, 31, "Enter date (1-31): ");
            string title = validateString("Enter event title: ");

            if (scheduler.isEventRepeating(date, title)) {
                bool deleteRepeats = validateOption("Delete all repeating events with the same title? (yes/no): ");
                scheduler.cancelEvent(date, title, deleteRepeats);
            }
            else {
                scheduler.cancelEvent(date, title, false);
            }
            break;
        }
        case 3: {

            int date = validateInput(currentDay, 31, "Enter date (" + to_string(currentDay) + "-31): ");
            string title = validateString("Enter event title: ");
            int newDate = validateInput(currentDay, 31, "Enter new date (" + to_string(currentDay) + "-31): ");

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

            scheduler.setDayOff(date);
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