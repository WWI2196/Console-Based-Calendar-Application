#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>

using namespace std;

class Time {
public:
    int hour;
    int minute;

    Time(int h = 0, int m = 0) : hour(h), minute(m) {
        if (h < 0 || h >= 24 || m < 0 || m >= 60 || (m % 30 != 0)) {
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
    vector<Event> events;
    string dayOfWeek;

    Day(int d = 0, string day = "") : date(d), isDayOff(false), dayOfWeek(day) {}

    void addEvent(const Event& event) {
        if (isDayOff) {
            throw invalid_argument("Cannot schedule events on a day off");
        }
        for (const auto& e : events) {
            if (event.overlaps(e)) {
                throw invalid_argument("Event overlaps with an existing event");
            }
        }
        events.push_back(event);
    }

    void deleteEvent(const string& title, bool deleteRepeats) {
        if (deleteRepeats) {
            events.erase(remove_if(events.begin(), events.end(),
                [&title](const Event& e) { return e.title == title; }),
                events.end());
        }
        else {
            auto it = find_if(events.begin(), events.end(),
                [&title](const Event& e) { return e.title == title; });
            if (it != events.end()) {
                events.erase(it);
            }
        }
    }

    void clearEvents() {
        events.clear();
    }

    string toString() const {
        if (events.empty() && !isDayOff) return "";

        string result = to_string(date) + " July 2024 (" + dayOfWeek + ")";
        if (isDayOff) result += " (Day Off)";
        result += "\n";
        for (const auto& event : events) {
            result += "  " + event.toString() + "\n";
        }
        return result;
    }

    friend ostream& operator<<(ostream& os, const Day& day) {
        if (day.isDayOff) {
            os << day.date << "|off|\n";
        }
        for (const auto& event : day.events) {
            os << day.date << "|" << event << "\n";
        }
        return os;
    }

    friend istream& operator>>(istream& is, Day& day) {
        string line;
        getline(is, line);
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
        return is;
    }
};

class Calendar {
public:
    map<int, Day> days;
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
        if (days[date].isDayOff) {
            days[date].isDayOff = false; // Remove day off if it exists
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
        days[date].date = date;
        days[date].addEvent(event);
        handleRepeatingEvents(date, event);
    }

    void markDayOff(int date) {
        if (date < currentDay || date > 31) {
            throw invalid_argument("Cannot mark past days or beyond July 2024");
        }
        days[date].date = date;
        days[date].isDayOff = true;
        days[date].clearEvents();
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
            for (auto& pair : days) {
                pair.second.deleteEvent(title, deleteRepeats);
            }
        }
        else {
            days[date].deleteEvent(title, deleteRepeats);
        }
    }

    void viewDay(int date) const {
        if (date < currentDay || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        cout << days.at(date).toString() << endl;
    }

    void viewWeek(int startDate) const {
        if (startDate < currentDay || startDate > 25) {
            throw invalid_argument("Start date must be within range");
        }
        for (int i = startDate; i < startDate + 7; ++i) {
            cout << days.at(i).toString() << endl;
        }
    }

    void viewMonth() const {
        for (const auto& pair : days) {
            const auto& day = pair.second;
            string dayStr = day.toString();
            if (!dayStr.empty()) {
                cout << dayStr << endl;
            }
        }
    }

    void shiftEvent(int fromDate, int toDate, const string& title) {
        if (fromDate < currentDay || fromDate > 31 || toDate < currentDay || toDate > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        auto& fromDay = days[fromDate];
        auto it = find_if(fromDay.events.begin(), fromDay.events.end(),
            [&title](const Event& e) { return e.title == title; });
        if (it == fromDay.events.end()) {
            throw invalid_argument("Event not found");
        }
        Event event = *it;
        fromDay.deleteEvent(title, false);
        days[toDate].addEvent(event);
    }

private:
    void initializeDays() {
        string daysOfWeek[7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
        for (int i = 1; i <= 31; ++i) {
            days[i] = Day(i, daysOfWeek[(i - 1) % 7]);
        }
    }

    bool isWeekend(int date) const {
        string day = days.at(date).dayOfWeek;
        return day == "Saturday" || day == "Sunday";
    }

    void handleRepeatingEvents(int startDate, const Event& event) {
        if (event.repeatType == "daily") {
            for (int i = startDate + 1; i <= 31; ++i) {
                days[i].date = i;
                days[i].addEvent(Event(event.title, event.start, event.end, "daily"));
            }
        }
        else if (event.repeatType == "weekly") {
            for (int i = startDate + 7; i <= 31; i += 7) {
                days[i].date = i;
                days[i].addEvent(Event(event.title, event.start, event.end, "weekly"));
            }
        }
    }

    void saveToFile() const {
        ofstream outFile("calendar_data.txt");
        for (const auto& pair : days) {
            const auto& day = pair.second;
            outFile << day;
        }
    }

    void loadFromFile() {
        ifstream inFile("calendar_data.txt");
        if (inFile) {
            string line;
            while (getline(inFile, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                int date;
                ss >> date;
                ss.ignore(1); // ignore the '|'

                string token;
                getline(ss, token, '|');
                if (token == "off") {
                    days[date] = Day(date, getDayOfWeek(date));
                    days[date].isDayOff = true;
                }
                else {
                    ss.clear();
                    ss.str(line);
                    ss >> date;
                    ss.ignore(1); // ignore the '|'

                    Event event;
                    ss >> event;
                    if (days.find(date) == days.end()) {
                        days[date] = Day(date, getDayOfWeek(date));
                    }
                    days[date].addEvent(event);
                }
            }
        }
    }

    string getDayOfWeek(int date) const {
        string daysOfWeek[7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
        return daysOfWeek[(date + 0) % 7];
    }
};

void displayMenu() {
    cout << "1. Schedule an Event\n";
    cout << "2. Mark a Day Off\n";
    cout << "3. Delete an Event\n";
    cout << "4. View Day Schedule\n";
    cout << "5. View Week Schedule\n";
    cout << "6. View Month Schedule\n";
    cout << "7. Shift an Event\n";
    cout << "8. Exit\n";
    cout << "Choose an option: ";
}

int getCurrentDay() {
    int day;
    cout << "Enter the current day in July (1-31): ";
    cin >> day;
    if (day < 1 || day > 31) {
        throw invalid_argument("Day must be between 1 and 31");
    }
    return day;
}

int main() {
    int currentDay = getCurrentDay();
    Calendar cal(currentDay);
    int choice;

    while (true) {
        displayMenu();
        cin >> choice;

        if (choice == 8) {
            break;
        }

        int date, startDate, fromDate, toDate;
        string title, repeatType;
        Time start, end;

        try {
            switch (choice) {
            case 1:
                cout << "Enter date (1-31): ";
                cin >> date;
                if (date < currentDay) {
                    cout << "Cannot schedule events on past days.\n";
                    break;
                }
                cout << "Enter title: ";
                cin.ignore();
                getline(cin, title);
                cout << "Enter start time (hour minute): ";
                cin >> start;
                cout << "Enter end time (hour minute): ";
                cin >> end;
                cout << "Enter repeat type (none, daily, weekly): ";
                cin >> repeatType;
                cal.scheduleEvent(date, Event(title, start, end, repeatType));
                break;

            case 2:
                cout << "Enter date (1-31) to mark as day off: ";
                cin >> date;
                if (date < currentDay) {
                    cout << "Cannot mark past days as day off.\n";
                    break;
                }
                cal.markDayOff(date);
                break;

            case 3:
                cout << "Enter date (1-31): ";
                cin >> date;
                if (date < currentDay) {
                    cout << "Cannot delete events from past days.\n";
                    break;
                }
                cout << "Enter title of the event to delete: ";
                cin.ignore();
                getline(cin, title);
                cal.deleteEvent(date, title);
                break;

            case 4:
                cout << "Enter date (1-31): ";
                cin >> date;
                cal.viewDay(date);
                break;

            case 5:
                cout << "Enter start date (1-25) of the week: ";
                cin >> startDate;
                cal.viewWeek(startDate);
                break;

            case 6:
                cal.viewMonth();
                break;

            case 7:
                cout << "Enter current date of the event (1-31): ";
                cin >> fromDate;
                if (fromDate < currentDay) {
                    cout << "Cannot shift events from past days.\n";
                    break;
                }
                cout << "Enter new date for the event (1-31): ";
                cin >> toDate;
                if (toDate < currentDay) {
                    cout << "Cannot shift events to past days.\n";
                    break;
                }
                cout << "Enter title of the event to shift: ";
                cin.ignore();
                getline(cin, title);
                cal.shiftEvent(fromDate, toDate, title);
                break;

            default:
                cout << "Invalid option. Please try again.\n";
            }
        }
        catch (const exception& e) {
            cerr << e.what() << endl;
        }
    }

    return 0;
}
