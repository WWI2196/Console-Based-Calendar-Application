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

    Day(int d = 0) : date(d), isDayOff(false) {}

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

    void deleteEvent(const string& title) {
        events.erase(remove_if(events.begin(), events.end(),
            [&title](const Event& e) { return e.title == title; }),
            events.end());
    }

    void clearEvents() {
        events.clear();
    }

    string toString() const {
        if (events.empty() && !isDayOff) return "";

        string result = to_string(date) + " July 2024";
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

    Calendar() {
        loadFromFile();
    }

    ~Calendar() {
        saveToFile();
    }

    void scheduleEvent(int date, const Event& event) {
        if (date < 1 || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        if (days[date].isDayOff) {
            days[date].isDayOff = false; // Remove day off if it exists
        }
        days[date].date = date;
        days[date].addEvent(event);
        handleRepeatingEvents(date, event);
    }

    void markDayOff(int date) {
        if (date < 1 || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        days[date].date = date;
        days[date].isDayOff = true;
        days[date].clearEvents();
    }

    void deleteEvent(int date, const string& title) {
        if (date < 1 || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        days[date].deleteEvent(title);
    }

    void viewDay(int date) const {
        if (date < 1 || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        cout << days.at(date).toString() << endl;
    }

    void viewWeek(int startDate) const {
        if (startDate < 1 || startDate > 25) {
            throw invalid_argument("Start date must be within range 1-25");
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
                    days[date] = Day(date);
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
                        days[date] = Day(date);
                    }
                    days[date].addEvent(event);
                }
            }
        }
    }
};

void displayMenu() {
    cout << "1. Schedule an Event\n";
    cout << "2. Mark a Day Off\n";
    cout << "3. Delete an Event\n";
    cout << "4. View Day Schedule\n";
    cout << "5. View Week Schedule\n";
    cout << "6. View Month Schedule\n";
    cout << "7. Exit\n";
    cout << "Choose an option: ";
}

int main() {
    Calendar cal;
    int choice;

    while (true) {
        displayMenu();
        cin >> choice;

        if (choice == 7) {
            break;
        }

        int date, startDate;
        string title, repeatType;
        Time start, end;

        try {
            switch (choice) {
            case 1:
                cout << "Enter date (1-31): ";
                cin >> date;
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
                cal.markDayOff(date);
                break;

            case 3:
                cout << "Enter date (1-31): ";
                cin >> date;
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
