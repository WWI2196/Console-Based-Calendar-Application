#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std;

class Time {
public:
    int hour;
    int minute;

    Time(int hour = 0, int minute = 0) {
        this->hour = hour;
        this->minute = minute;

        if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
            throw std::invalid_argument("Invalid time format");
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

    std::string toString() const {
        return (hour < 10 ? "0" : "") + std::to_string(hour) + ":" + (minute < 10 ? "0" : "") + std::to_string(minute);
    }

    void fromString(const std::string& timeStr) {
        std::stringstream ss(timeStr);
        char delim;
        ss >> hour >> delim >> minute;
        if (delim != ':' || hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
            throw std::invalid_argument("Invalid time format");
        }
    }

    std::string serialize() const {
        return toString();
    }

    void deserialize(const std::string& timeStr) {
        fromString(timeStr);
    }
};

class Event {
public:
    std::string title;
    Time start;
    Time end;
    std::string repeatType; // "none", "daily", "weekly"

    Event(std::string t = "", Time s = Time(), Time e = Time(), std::string r = "none")
        : title(t), start(s), end(e), repeatType(r) {
        if (end.isLessThan(start)) {
            throw std::invalid_argument("Event end time must be after start time");
        }
    }

    bool overlaps(const Event& other) const {
        return (start.isLessThan(other.end) && end.isGreaterThan(other.start));
    }

    std::string toString() const {
        return title + " from " + start.toString() + " to " + end.toString() + " (" + repeatType + ")";
    }

    std::string serialize() const {
        return title + "|" + start.serialize() + "|" + end.serialize() + "|" + repeatType;
    }

    void deserialize(const std::string& eventStr) {
        std::stringstream ss(eventStr);
        std::getline(ss, title, '|');
        std::string startTimeStr, endTimeStr;
        std::getline(ss, startTimeStr, '|');
        std::getline(ss, endTimeStr, '|');
        std::getline(ss, repeatType);
        start.deserialize(startTimeStr);
        end.deserialize(endTimeStr);
    }
};

class Day {
public:
    int date;
    bool isDayOff;
    Event events[10];  // Fixed size array to store events
    int eventCount;
    string dayOfWeek;

    Day(int d = 0, string day = "") : date(d), isDayOff(false), dayOfWeek(day), eventCount(0) {}

    void addEvent(const Event& event) {
        if (isDayOff) {
            throw invalid_argument("Cannot schedule events on a day off");
        }
        for (int i = 0; i < eventCount; ++i) {
            if (event.overlaps(events[i])) {
                throw invalid_argument("Event overlaps with an existing event");
            }
        }
        if (eventCount >= 10) {
            throw overflow_error("Maximum number of events reached");
        }
        events[eventCount++] = event;
    }

    void deleteEvent(const string& title, bool deleteRepeats) {
        bool eventFound = false;
        if (deleteRepeats) {
            for (int i = 0; i < eventCount; ) {
                if (events[i].title == title) {
                    eventFound = true;
                    for (int j = i; j < eventCount - 1; ++j) {
                        events[j] = events[j + 1];
                    }
                    --eventCount;
                }
                else {
                    ++i;
                }
            }
        }
        else {
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
        }
        if (!eventFound) {
            throw invalid_argument("No such event exists");
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
                        throw invalid_argument("Event overlaps with an existing event on the new date");
                    }
                }
                // Remove the event from the current date
                deleteEvent(title, false);
                // Add the event to the new date
                days[newDate - 1].addEvent(eventToShift);
                eventFound = true;
                break;
            }
        }
        if (!eventFound) {
            throw invalid_argument("Event not found");
        }
    }

    void clearEvents() {
        eventCount = 0;
    }

    void sortEvents() {
        // Implementing a simple bubble sort to sort events by start time
        for (int i = 0; i < eventCount - 1; ++i) {
            for (int j = 0; j < eventCount - i - 1; ++j) {
                if (events[j].start.isGreaterThan(events[j + 1].start)) {
                    Event temp = events[j];
                    events[j] = events[j + 1];
                    events[j + 1] = temp;
                }
            }
        }
    }

    string toString() const {
        if (eventCount == 0 && !isDayOff) return "";

        string result = to_string(date) + " July 2024 (" + dayOfWeek + ")";
        if (isDayOff) result += " (Day Off)";
        result += "\n";

        // Sort the events by start time before printing
        const_cast<Day*>(this)->sortEvents();

        for (int i = 0; i < eventCount; ++i) {
            result += "  " + events[i].toString() + "\n";
        }

        return result;
    }

    std::string serialize() const {
        std::string serializedDay;
        if (isDayOff) {
            serializedDay += std::to_string(date) + "|off|\n";
        }
        for (int i = 0; i < eventCount; ++i) {
            serializedDay += std::to_string(date) + "|" + events[i].serialize() + "\n";
        }
        return serializedDay;
    }

    void deserialize(const std::string& dayStr) {
        std::stringstream ss(dayStr);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            std::stringstream ssLine(line);
            std::string token;
            std::getline(ssLine, token, '|');
            date = std::stoi(token);
            std::getline(ssLine, token, '|');
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

class Calendar {
public:
    Day days[31];
    int currentDay;

    Calendar(int currentDay) : currentDay(currentDay) {
        try {
            initializeDays();
            loadFromFile();
        }
        catch (const exception& e) {
            cout << "Error during initialization: " << e.what() << endl;
        }
    }

    ~Calendar() {
        try {
            saveToFile();
        }
        catch (const exception& e) {
            cout << "Error saving to file: " << e.what() << endl;
        }
    }

    void scheduleEvent(int date, const Event& event) {
        try {
            if (date < currentDay || date > 31) {
                throw invalid_argument("Cannot schedule events in the past or beyond July 2024");
            }

            if (days[date - 1].isDayOff) {
                char confirmation;
                cout << "The selected day is marked as a day off. Do you want to proceed? (y/n): ";
                cin >> confirmation;
                if (confirmation != 'y' && confirmation != 'Y') {
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
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void cancelEvent(int date, const string& title, bool deleteRepeats) {
        try {
            if (date < currentDay || date > 31) {
                throw invalid_argument("Cannot cancel events in the past or beyond July 2024");
            }
            days[date - 1].deleteEvent(title, deleteRepeats);
            cout << "Event cancelled successfully.\n";
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void shiftEvent(int date, const string& title, int newDate) {
        try {
            if (date < currentDay || date > 31 || newDate < currentDay || newDate > 31) {
                throw invalid_argument("Invalid date for shifting events");
            }
            days[date - 1].shiftEvent(title, newDate, days);
            cout << "Event shifted successfully.\n";
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void setDayOff(int date) {
        try {
            if (date < currentDay || date > 31) {
                throw invalid_argument("Cannot set days off in the past or beyond July 2024");
            }
            days[date - 1].isDayOff = true;
            days[date - 1].clearEvents();
            cout << "Day off set for " << date << " July 2024.\n";
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void displayCalendar() const {
        cout << "July 2024 Calendar\n";
        for (int i = 0; i < 31; ++i) {
            string dayStr = days[i].toString();
            if (!dayStr.empty()) {
                cout << dayStr << endl;
            }
        }
    }

    void setCurrentDay(int day) {
        if (day < 1 || day > 31) {
            throw invalid_argument("Invalid day for setting current day");
        }
        currentDay = day;
    }

    void viewDaySchedule(int day) const {
        if (day < 1 || day > 31) {
            throw invalid_argument("Invalid day for viewing schedule");
        }
        cout << days[day - 1].toString() << endl;
    }

    void viewWeekSchedule(int startDay) const {
        if (startDay < 1 || startDay > 31) {
            throw invalid_argument("Invalid start day for viewing week schedule");
        }
        for (int i = startDay; i < startDay + 7 && i <= 31; ++i) {
            cout << days[i - 1].toString() << endl;
        }
    }

private:
    void initializeDays() {
        std::string dayOfWeek = "Monday";
        for (int i = 0; i < 31; ++i) {
            days[i] = Day(i + 1, dayOfWeek);

            // Determine the next day of the week
            if (dayOfWeek == "Monday") dayOfWeek = "Tuesday";
            else if (dayOfWeek == "Tuesday") dayOfWeek = "Wednesday";
            else if (dayOfWeek == "Wednesday") dayOfWeek = "Thursday";
            else if (dayOfWeek == "Thursday") dayOfWeek = "Friday";
            else if (dayOfWeek == "Friday") dayOfWeek = "Saturday";
            else if (dayOfWeek == "Saturday") dayOfWeek = "Sunday";
            else if (dayOfWeek == "Sunday") dayOfWeek = "Monday";
        }
    }

    void saveToFile() const {
        ofstream file("calendar.txt");
        if (!file.is_open()) {
            throw runtime_error("Unable to open file for saving");
        }

        for (int i = 0; i < 31; ++i) {
            file << days[i].serialize();
        }
        file.close();
    }

    void loadFromFile() {
        ifstream file("calendar.txt");
        if (!file.is_open()) {
            throw runtime_error("Unable to open file for loading");
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
    }
};

int main() {

    string currentDay_s;
    int currentDay;

    while (true) {

        cout << "Enter the current day (1-31): ";
        cin >> currentDay_s;

        try {
            currentDay = stoi(currentDay_s);

            if (currentDay >= 1 && currentDay <= 31) {
                break; // Valid input, break out of the loop
            }
            else {
                std::cout << "Invalid input. Please enter a valid date.\n";
            }
        }
        catch (invalid_argument& e) {
            std::cout << "Invalid input. Please enter a valid date.\n";
        }
        catch (out_of_range& e) {
            std::cout << "The number entered is out of range.\n";
        }

        // Clear input stream and ignore remaining characters
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }


    Calendar calendar(currentDay);

    while (true) {

        cout << "\n1. Schedule Event\n"
            "2. Cancel Event\n"
            "3. Shift Event\n"
            "4. Set Day Off\n"
            "5. Display Calendar\n"
            "7. View Day Schedule\n"
            "8. View Week Schedule\n"
            "9. Exit\n"
            "Choose an option: " ;

        int option;
        cin >> option;

        if (option == 1) {
            int date, startHour, startMinute, endHour, endMinute;
            string title, repeatType;

            cout << "Enter date (1-31): ";
            cin >> date;
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
                calendar.scheduleEvent(date, event);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
        else if (option == 2) {
            int date;
            string title;
            char repeatChoice;
            bool deleteRepeats = false;

            cout << "Enter date (1-31): ";
            cin >> date;
            cout << "Enter event title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Delete all repeating events with the same title? (y/n): ";
            cin >> repeatChoice;
            deleteRepeats = (repeatChoice == 'y' || repeatChoice == 'Y');

            calendar.cancelEvent(date, title, deleteRepeats);
        }
        else if (option == 3) {
            int date, newDate;
            string title;

            cout << "Enter current date (1-31): ";
            cin >> date;
            cout << "Enter event title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Enter new date (1-31): ";
            cin >> newDate;

            calendar.shiftEvent(date, title, newDate);
        }
        else if (option == 4) {
            int date;

            cout << "Enter date (1-31): ";
            cin >> date;

            calendar.setDayOff(date);
        }
        else if (option == 5) {
            calendar.displayCalendar();
        }
        /*else if (option == 6) {
            int day;
            cout << "Enter current day (1-31): ";
            cin >> day;
            try {
                calendar.setCurrentDay(day);
                cout << "Current day set to " << day << " July 2024.\n";
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }*/
        else if (option == 7) {
            int day;
            cout << "Enter day (1-31): ";
            cin >> day;
            try {
                calendar.viewDaySchedule(day);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
        else if (option == 8) {
            int startDay;
            cout << "Enter start day (1-31): ";
            cin >> startDay;
            try {
                calendar.viewWeekSchedule(startDay);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
        else if (option == 9) {
            break;
        }
        else {
            cout << "Invalid option. Please try again.\n";
        }
    }

    return 0;
}
