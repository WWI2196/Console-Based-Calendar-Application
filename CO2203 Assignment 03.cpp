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

        if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60 || (minute % 30 != 0)) {
            throw invalid_argument("Invalid time format");
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

    string toString() const {
        return (hour < 10 ? "0" : "") + to_string(hour) + ":" + (minute == 0 ? "00" : to_string(minute));
    }

    friend ostream& operator<<(ostream& os, const Time& time) {
        os << time.hour << " " << time.minute;
        return os;
    }

    friend istream& operator>>(istream& is, Time& time) {
        is >> time.hour;
        is.ignore(1); // ignore space
        is >> time.minute;
        return is;
    }
};

class Event {
public:
    string title;
    Time start;
    Time end;
    string repeatType; // "none", "daily", "weekly"

    Event(string t = "", Time s = Time(), Time e = Time(), string r = "none")
        : title(t), start(s), end(e), repeatType(r) {
        if (end < start) {
            throw invalid_argument("Event end time must be after start time");
        }
    }

    bool overlaps(const Event& other) const {
        return (start < other.end && end > other.start);
    }

    string toString() const {
        return title + " from " + start.toString() + " to " + end.toString() + " (" + repeatType + ")";
    }

    friend ostream& operator<<(ostream& os, const Event& event) {
        os << event.title << "|" << event.start << "|" << event.end << "|" << event.repeatType;
        return os;
    }

    friend istream& operator>>(istream& is, Event& event) {
        string title, repeatType;
        Time start, end;
        getline(is, title, '|');
        is >> start;
        is.ignore(1); // ignore the '|'
        is >> end;
        is.ignore(1); // ignore the '|'
        getline(is, repeatType);

        event.title = title;
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
        initializeDays();
        loadFromFile();
    }

    ~Calendar() {
        saveToFile();
    }

    void scheduleEvent(int date, const Event& event) {
        if (date < currentDay || date > 31) {
            throw invalid_argument("Cannot schedule events in the past or beyond July 2024");
        }
        if (days[date - 1].isDayOff) {
            days[date - 1].isDayOff = false; // Remove day off if it exists
        }
        if (isWeekend(date)) {
            cout << "You are scheduling an event on a weekend. Do you want to proceed? (yes/no): ";
            string response;
            cin >> response;
            if (response != "yes") {
                cout << "Event not scheduled.\n";
                return;
            }
        }
        days[date - 1].addEvent(event);
        handleRepeatingEvents(date, event);
    }

    void markDayOff(int date) {
        if (date < currentDay || date > 31) {
            throw invalid_argument("Cannot mark past days or beyond July 2024");
        }
        days[date - 1].date = date;
        days[date - 1].isDayOff = true;
        days[date - 1].clearEvents();
    }

    void deleteEvent(int date, const string& title) {
        if (date < currentDay || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        bool deleteRepeats = false;
        cout << "Do you want to delete all repeating events with this title? (yes/no): ";
        string response;
        cin >> response;
        if (response == "yes") {
            deleteRepeats = true;
            for (int i = 0; i < 31; ++i) {
                days[i].deleteEvent(title, deleteRepeats);
            }
        }
        else {
            days[date - 1].deleteEvent(title, deleteRepeats);
        }
    }

    void viewDay(int date) const {
        if (date < currentDay || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        cout << days[date - 1].toString() << endl;
    }

    void viewWeek(int startDate) const {
        if (startDate < currentDay || startDate > 25) {
            throw invalid_argument("Start date must be within range");
        }
        for (int i = startDate; i < startDate + 7; ++i) {
            cout << days[i - 1].toString() << endl;
        }
    }

    void viewMonth() const {
        for (int i = currentDay; i <= 31; ++i) {
            cout << days[i - 1].toString() << endl;
        }
    }

private:
    void initializeDays() {
        string daysOfWeek[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
        for (int i = 0; i < 31; ++i) {
            days[i].date = i + 1;
            days[i].dayOfWeek = daysOfWeek[(i + 1) % 7];
        }
    }

    void loadFromFile() {
        ifstream file("events.txt");
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
        ofstream file("calender_date.txt");
        for (int i = 0; i < 31; ++i) {
            file << days[i];
        }
        file.close();
    }

    void handleRepeatingEvents(int date, const Event& event) {
        if (event.repeatType == "daily") {
            for (int i = date; i <= 31; ++i) {
                if (i != date) {
                    days[i - 1].addEvent(event);
                }
            }
        }
        else if (event.repeatType == "weekly") {
            for (int i = date; i <= 31; i += 7) {
                if (i != date) {
                    days[i - 1].addEvent(event);
                }
            }
        }
    }

    bool isWeekend(int date) const {
        string dayOfWeek = days[date - 1].dayOfWeek;
        return dayOfWeek == "Saturday" || dayOfWeek == "Sunday";
    }
};

int main() {
    Calendar calendar(1);
    char choice;
    do {
        cout << "1. Schedule an event\n";
        cout << "2. Mark a day off\n";
        cout << "3. Delete an event\n";
        cout << "4. View a day\n";
        cout << "5. View a week\n";
        cout << "6. View the month\n";
        cout << "7. Exit\n";
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
            calendar.scheduleEvent(date, Event(title, Time(sh, sm), Time(eh, em), repeatType));
            break;
        }
        case '2': {
            int date;
            cout << "Enter date (1-31): ";
            cin >> date;
            calendar.markDayOff(date);
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
            calendar.deleteEvent(date, title);
            break;
        }
        case '4': {
            int date;
            cout << "Enter date (1-31): ";
            cin >> date;
            calendar.viewDay(date);
            break;
        }
        case '5': {
            int date;
            cout << "Enter start date (1-25): ";
            cin >> date;
            calendar.viewWeek(date);
            break;
        }
        case '6': {
            calendar.viewMonth();
            break;
        }
        case '7': {
            cout << "Exiting...\n";
            break;
        }
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != '7');

    return 0;
}
