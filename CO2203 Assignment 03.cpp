#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>

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

    bool operator<(const Time& other) const {
        return (hour < other.hour) || (hour == other.hour && minute < other.minute);
    }

    bool operator>(const Time& other) const {
        return (hour > other.hour) || (hour == other.hour && minute > other.minute);
    }

    bool operator==(const Time& other) const {
        return hour == other.hour && minute == other.minute;
    }

    std::string toString() const {
        return (hour < 10 ? "0" : "") + std::to_string(hour) + ":" + (minute < 10 ? "0" : "") + std::to_string(minute);
    }

    friend std::ostream& operator<<(std::ostream& os, const Time& time) {
        os << (time.hour < 10 ? "0" : "") << time.hour << ":" << (time.minute < 10 ? "0" : "") << time.minute;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Time& time) {
        char delim;
        is >> time.hour >> delim >> time.minute;
        if (delim != ':' || time.hour < 0 || time.hour >= 24 || time.minute < 0 || time.minute >= 60) {
            throw std::invalid_argument("Invalid time format");
        }
        return is;
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
        if (end < start) {
            throw std::invalid_argument("Event end time must be after start time");
        }
    }

    bool overlaps(const Event& other) const {
        return (start < other.end && end > other.start);
    }

    std::string toString() const {
        return title + " from " + start.toString() + " to " + end.toString() + " (" + repeatType + ")";
    }

    friend std::ostream& operator<<(std::ostream& os, const Event& event) {
        os << event.title << "|" << event.start << "|" << event.end << "|" << event.repeatType;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Event& event) {
        std::string title, repeatType;
        Time start, end;
        std::getline(is, title, '|');
        if (title.empty()) return is;
        event.title = title;
        char delim;
        is >> start >> delim >> end >> delim;
        if (delim != '|') {
            throw std::invalid_argument("Invalid event format");
        }
        std::getline(is, repeatType);
        event.start = start;
        event.end = end;
        event.repeatType = repeatType;

        return is;
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
        if (deleteRepeats) {
            for (int i = 0; i < eventCount; ) {
                if (events[i].title == title) {
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
                    for (int j = i; j < eventCount - 1; ++j) {
                        events[j] = events[j + 1];
                    }
                    --eventCount;
                    break;
                }
            }
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

    string toString() const {
        if (eventCount == 0 && !isDayOff) return "";

        string result = to_string(date) + " July 2024 (" + dayOfWeek + ")";
        if (isDayOff) result += " (Day Off)";
        result += "\n";

        for (int i = 0; i < eventCount; ++i) {
            result += "  " + events[i].toString() + "\n";
        }

        return result;
    }

    friend ostream& operator<<(ostream& os, const Day& day) {
        if (day.isDayOff) {
            os << day.date << "|off|\n";
        }
        for (int i = 0; i < day.eventCount; ++i) {
            os << day.date << "|" << day.events[i] << "\n";
        }
        return os;
    }

    friend istream& operator>>(istream& is, Day& day) {
        string line;
        while (getline(is, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;
            getline(ss, token, '|');
            day.date = stoi(token);
            getline(ss, token, '|');
            if (token == "off") {
                day.isDayOff = true;
                day.clearEvents();
            }
            else {
                Event event;
                ss.str(line);
                ss.clear();
                ss >> event;
                day.isDayOff = false;
                day.addEvent(event);
            }
        }
        return is;
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
                days[date - 1].isDayOff = false;
            }

            if (event.repeatType == "none") {
                days[date - 1].addEvent(event);
            }

            
            if (event.repeatType == "daily") {
                for (int i = date; i < 31; ++i) {
                    days[i-1].addEvent(event);
                }
            }
            else if (event.repeatType == "weekly") {
                for (int i = date; i <= 31; i= i+ 7) {
                    days[i-1].addEvent(event);
                }
            }
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void markDayOff(int date) {
        if (date < currentDay || date > 31) {
            throw invalid_argument("Cannot mark past days or beyond July 2024 as day off");
        }
        days[date - 1].isDayOff = true;
        days[date - 1].clearEvents();
    }

    void deleteEvent(int date, const string& title) {
        try {
            if (date < currentDay || date > 31) {
                throw invalid_argument("Cannot delete events in the past or beyond July 2024");
            }
            days[date - 1].deleteEvent(title, false);
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void shiftEvent(int date, const string& title, int newDate) {
        try {
            if (date < currentDay || date > 31 || newDate < currentDay || newDate > 31) {
                throw invalid_argument("Cannot shift events in the past or beyond July 2024");
            }
            days[date - 1].shiftEvent(title, newDate, days);
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void displayDay(int date) const {
        if (date < 1 || date > 31) {
            cout << "Invalid date" << endl;
            return;
        }
        string dayStr = days[date - 1].toString();
        if (!dayStr.empty()) {
            cout << dayStr << endl;
        }
    }

    void displayWeek(int startDate) const {
        if (startDate < 1 || startDate > 25) {
            cout << "Invalid start date" << endl;
            return;
        }
        for (int i = startDate; i < startDate + 7 && i <= 31; ++i) {
            string dayStr = days[i - 1].toString();
            if (!dayStr.empty()) {
                cout << dayStr << endl;
            }
        }
    }

    void displayMonth() const {
        for (int i = 0; i < 31; ++i) {
            string dayStr = days[i].toString();
            if (!dayStr.empty()) {
                cout << dayStr << endl;
            }
        }
    }

private:
    void initializeDays() {
        string dayOfWeek[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
        for (int i = 0; i < 31; ++i) {
            days[i] = Day(i + 1, dayOfWeek[i % 7]);  // July 1, 2024 is a Monday
        }
    }

    void loadFromFile() {
        ifstream file("calendar.txt");
 
        if (!file) return;
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            int date;
            ss >> date;
            ss.ignore(1); // ignore the '|'
            if (line.find("|off|") != string::npos) {
                days[date - 1].date = date;
                days[date - 1].isDayOff = true;
                days[date - 1].clearEvents();
            }
            else {
                Event event;
                ss >> event;
                days[date - 1].addEvent(event);
            }
        }
        file.close();
    }

    void saveToFile() const {
        ofstream file("calendar.txt");
        for (int i = 0; i < 31; ++i) {
            file << days[i];
        }
    }
};

int main() {
    int currentDay;
    cout << "Enter the current day of July 2024 (1-31): ";
    while (!(cin >> currentDay) || currentDay < 1 || currentDay > 31) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Enter the current day of July 2024 (1-31): ";
    }

    Calendar calendar(currentDay);
    char choice;
    do {
        cout << "1. Schedule an event\n";
        cout << "2. Mark a day off\n";
        cout << "3. Delete an event\n";
        cout << "4. Shift an event\n";
        cout << "5. View a day\n";
        cout << "6. View a week\n";
        cout << "7. View the month\n";
        cout << "8. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        switch (choice) {
        case '1': {
            int date, sh, sm, eh, em;
            string title, repeatType;
            cout << "Enter date (1-31): ";
            cin >> date;
            cout << "Enter title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Enter start time (HH MM): ";
            cin >> sh >> sm;
            cout << "Enter end time (HH MM): ";
            cin >> eh >> em;
            cout << "Enter repeat type (none/daily/weekly): ";
            cin >> repeatType;
            try {
                calendar.scheduleEvent(date, Event(title, Time(sh, sm), Time(eh, em), repeatType));
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            break;
        }
        case '2': {
            int date;
            cout << "Enter date (1-31): ";
            cin >> date;
            try {
                calendar.markDayOff(date);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            break;
        }
        case '3': {
            int date;
            string title;
            cout << "Enter date (1-31): ";
            cin >> date;
            cout << "Enter title: ";
            cin.ignore();
            getline(cin, title);
            try {
                calendar.deleteEvent(date, title);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            break;
        }
        case '4': {
            int date, newDate;
            string title;
            cout << "Enter date (1-31): ";
            cin >> date;
            cout << "Enter title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Enter new date (1-31): ";
            cin >> newDate;
            try {
                calendar.shiftEvent(date, title, newDate);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            break;
        }
        case '5': {
            int date;
            cout << "Enter date (1-31): ";
            cin >> date;
            calendar.displayDay(date);
            break;
        }
        case '6': {
            int date;
            cout << "Enter start date (1-25): ";
            cin >> date;
            calendar.displayWeek(date);
            break;
        }
        case '7': {
            calendar.displayMonth();
            break;
        }
        case '8': {
            cout << "Exiting...\n";
            break;
        }
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != '8');

    return 0;
}
