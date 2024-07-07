#include <iostream>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <limits> // for the numeric_limits of the streamsize in the ignore function

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

    //void sortEvents() {
    //    // Implementing a simple bubble sort to sort events by start time
    //    for (int i = 0; i < eventCount - 1; ++i) {
    //        for (int j = 0; j < eventCount - i - 1; ++j) {
    //            if (events[j].startTime.isGreaterThan(events[j + 1].startTime)) {
    //                Event temp = events[j];
    //                events[j] = events[j + 1];
    //                events[j + 1] = temp;
    //            }
    //        }
    //    }
    //}

    string toString() const {
        if (eventCount == 0 && !isDayOff) return "";

        string result = to_string(date) + " July 2024 (" + dayOfWeek + ")";

        if (isDayOff) {
            result += " (Day Off)";
        }
        result += "\n";

        // Sort the events by start time before printing
        /*const_cast<Day*>(this)->sortEvents();*/

        for (int i = 0; i < eventCount; ++i) {
            result += "  " + events[i].toString() + "\n";
        }

        return result;
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
                char confirmation;
                cout << "The selected day is marked as a day off. Do you want to proceed? (yes/no): ";
                cin >> confirmation;
                if (confirmation != 'yes' && confirmation != 'YES') {
                    return;
                }
                else {
                    days[date - 1].isDayOff = false;  // Remove the day off status
                }

            }

            string dayOfWeek = days[date - 1].dayOfWeek;
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

    void displayScheduler() const {
        cout << "July 2024 Scheduler\n";
        for (int i = 0; i < 31; ++i) {
            string dayStr = days[i].toString();
            if (!dayStr.empty()) {
                cout << dayStr << endl;
            }
        }
    }

    /*void setCurrentDay(int day) {
        if (day < 1 || day > 31) {
            throw invalid_argument("Invalid day for setting current day");
        }
        currentDay = day;
    }*/

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
        for (int i = startDay; i < startDay + 7 && i <= 31; ++i) {
            cout << days[i - 1].toString() << endl;
        }
    }


};

int main() {

    int currentDay;

    while (true) {

        cout << "Enter the current day (1-31): ";
        cin >> currentDay;

        try {

            if (!cin.fail() || currentDay >= 1 && currentDay <= 31) {
                break; // Valid input, break out of the loop
            }
            else {
                cout << "Invalid input. Please enter a valid date.\n";
            }
        }
        catch (invalid_argument& e) {
            cout << "Invalid input. Please enter a valid date.\n";
        }
        catch (out_of_range& e) {
            cout << "The number entered is out of range.\n";
        }

        // Clear input stream and ignore remaining characters
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        /*
         * Referred from : https://stackoverflow.com/questions/25020129/cin-ignorenumeric-limitsstreamsizemax-n
         * Referred from : https://stackoverflow.com/questions/20446373/cin-ignorenumeric-limitsstreamsizemax-n-max-not-recognize-it
         */
    }


    Scheduler Scheduler(currentDay);

    while (true) {

        cout << "\n1. Schedule an Event\n"
            "2. Cancel an Event\n"
            "3. Shift an Event\n"
            "4. Set a Day Off\n"
            "5. View Day Schedule\n"
            "6. View Week Schedule\n"
            "7. View Month Schedule\n"
            "8. Exit\n";

        int option;
        cout << "\nChoose an option: ";
        cin >> option;

        if (option == 8) {
            cout << "You have exited the program.\n";
            break;
        }

        while (true) {

            try {

                if (!cin.fail() || currentDay >= 1 && currentDay <= 8) {
                    break; // Valid input, break out of the loop
                }
                else {
                    cout << "Invalid input. Please enter a valid option.\n";
                }
            }
            catch (invalid_argument& e) {
                cout << "Invalid input. Please enter a valid option.\n";
            }
            catch (out_of_range& e) {
                cout << "Please enter a valid option.\n";
            }

            // Clear input stream and ignore remaining characters
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }


        switch (option) {
        case 1: {
            int date, startHour, startMinute, endHour, endMinute;
            string title, repeatType;

            while (true) {
                printf("Enter date (%d-31): ", currentDay);
                fflush(stdout);
                cin >> date;

                if (cin.fail() || date < currentDay || date > 31) {
                    cin.clear(); // Clear the error flag
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
                    cout << "Invalid date. Please enter a valid date between " << currentDay << " and 31.\n" << endl;
                }
                else {
                    break;
                }
            }

            cout << "Enter event title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Enter start time (HH MM): ";
            cin >> startHour >> startMinute;
            cout << "Enter end time (HH MM): ";
            cin >> endHour >> endMinute;
            cout << "Enter repeat type (none, daily, weekly): ";
            cin >> repeatType;

            try {
                Time start(startHour, startMinute);
                Time end(endHour, endMinute);
                Event event(title, start, end, repeatType);
                Scheduler.scheduleEvent(date, event);
            }
            catch (const exception& exception) {
                cout << "Error: " << exception.what() << endl;
            }
            break;
        }
        case 2: {
            int date;
            string title;
            char repeatChoice;
            bool deleteRepeats = false;

            while (true) {
                cout << "Enter date (1-31): ";
                cin >> date;

                if (cin.fail() || date < currentDay || date > 31) {
                    cin.clear(); // Clear the error flag
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
                    cout << "Invalid date. Please enter a valid date between " << currentDay << " and 31.\n" << endl;
                }
                else {
                    break;
                }
            }

            cout << "Enter event title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Delete all repeating events with the same title? (y/n): "; // check this only for the repeating events
            cin >> repeatChoice;
            deleteRepeats = (repeatChoice == 'yes' || repeatChoice == 'YES');

            Scheduler.cancelEvent(date, title, deleteRepeats);
            break;
        }
        case 3: {
            int date, newDate;
            string title;

            while (true) {
                printf("Enter date (%d-31): ", currentDay);
                fflush(stdout);
                cin >> date;

                if (cin.fail() || date < currentDay || date > 31) {
                    cin.clear(); // Clear the error flag
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
                    cout << "Invalid date. Please enter a valid date between " << currentDay << " and 31.\n" << endl;
                }
                else {
                    break;
                }
            }

            cout << "Enter event title: ";
            cin.ignore();
            getline(cin, title);

            while (true) {
                printf("Enter new date (%d-31): ", currentDay);
                fflush(stdout);
                cin >> newDate;

                if (cin.fail() || date < currentDay || date > 31) {
                    cin.clear(); // Clear the error flag
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
                    cout << "Invalid date. Please enter a valid date between " << currentDay << " and 31.\n" << endl;
                }
                else {
                    break;
                }
            }

            Scheduler.shiftEvent(date, title, newDate);
            break;
        }
        case 4: {
            int date;

            while (true) {
                printf("Enter date (%d-31): ", currentDay);
                fflush(stdout);
                cin >> date;

                if (cin.fail() || date < currentDay || date > 31) {
                    cin.clear(); // Clear the error flag
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
                    cout << "Invalid date. Please enter a valid date between " << currentDay << " and 31.\n" << endl;
                }
                else {
                    break;
                }
            }

            Scheduler.setDayOff(date);
            break;
        }
        case 5: {
            int day;
            cout << "Enter day (1-31): ";
            cin >> day;
            try {
                Scheduler.viewDaySchedule(day);
            }
            catch (const exception& exception) {
                cout << "Error: " << exception.what() << endl;
            }
            break;
        }

        case 6: {
            int startDay;
            cout << "Enter start day (1-31): ";
            cin >> startDay;
            try {
                Scheduler.viewWeekSchedule(startDay);
            }
            catch (const exception& exception) {
                cout << "Error: " << exception.what() << endl;
            }
            break;
        }
        case 7: {
            Scheduler.displayScheduler();
            break;
        }

        }
    }

    return 0;
}