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
        is >> time.hour >> time.minute;
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
        os << event.title << "\n" << event.start << "\n" << event.end << "\n" << event.repeatType;
        return os;
    }

    friend istream& operator>>(istream& is, Event& event) {
        is.ignore();
        getline(is, event.title);
        is >> event.start >> event.end >> event.repeatType;
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
        os << day.date << " " << day.isDayOff << " " << day.events.size() << "\n";
        for (const auto& event : day.events) {
            os << event << "\n";
        }
        return os;
    }

    friend istream& operator>>(istream& is, Day& day) {
        int eventsSize;
        is >> day.date >> day.isDayOff >> eventsSize;
        for (int i = 0; i < eventsSize; ++i) {
            Event event;
            is >> event;
            day.events.push_back(event);
        }
        return is;
    }
};

class Calendar {
public:
    map<int, Day> days;

    Calendar() {
        for (int i = 1; i <= 31; ++i) {
            days[i] = Day(i);
        }
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
            throw invalid_argument("Cannot schedule events on a day off");
        }
        days[date].addEvent(event);
        handleRepeatingEvents(date, event);
    }

    void markDayOff(int date) {
        if (date < 1 || date > 31) {
            throw invalid_argument("Date must be within July 2024");
        }
        days[date].isDayOff = true;
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
                if (!days[i].isDayOff) {
                    days[i].addEvent(Event(event.title, event.start, event.end, "daily"));
                }
            }
        }
        else if (event.repeatType == "weekly") {
            for (int i = startDate + 7; i <= 31; i += 7) {
                if (!days[i].isDayOff) {
                    days[i].addEvent(Event(event.title, event.start, event.end, "weekly"));
                }
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
            Day day;
            while (inFile >> day) {
                days[day.date] = day;
            }
        }
    }
};

int main() {
    Calendar cal;
    try {
        cal.scheduleEvent(2, Event("Meeting A", Time(8, 0), Time(10, 0), "none"));
        cal.scheduleEvent(2, Event("Meeting B", Time(13, 0), Time(14, 30), "none"));
        cal.markDayOff(3);
        cal.scheduleEvent(4, Event("Meeting C", Time(10, 0), Time(13, 0), "none"));
        cal.scheduleEvent(6, Event("Meeting D", Time(9, 30), Time(12, 0), "none"));
        cal.scheduleEvent(5, Event("Daily Meeting", Time(11, 0), Time(11, 30), "daily"));
        cal.scheduleEvent(7, Event("Weekly Meeting", Time(14, 0), Time(15, 0), "weekly"));

        cout << "View Day 2 July 2024:\n";
        cal.viewDay(2);

        cout << "View Week from 1 July 2024:\n";
        cal.viewWeek(1);

        cout << "View Month of July 2024:\n";
        cal.viewMonth();

        cal.deleteEvent(2, "Meeting A");

        cout << "View Day 2 July 2024 after deleting Meeting A:\n";
        cal.viewDay(2);
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}
