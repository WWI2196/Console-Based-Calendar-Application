
// to access colors in the command prompt 
#include<windows.h>
#undef max

#include <iostream>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <fstream>
#include <sstream>

//just 1 funciton used in displayCalender_print function setw(2)
#include <iomanip>


using namespace std;

// this global object of HANDLE class from windows.h header file to allow command prompt colors 
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);


class Time{
public:
    int hour;
    int minute;

    //these two are for derived class
    string title;
    string repeatType; // "none", "daily", "weekly" 

    Time() : hour(0), minute(0),title(""), repeatType("none") {
     }

    Time(int h, int min) : hour(h), minute(min) {
        if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
            throw invalid_argument("Invalid time format");
        }
    }

    Time(string t , string r ) :title(t), repeatType(r) {
    }

    bool isLessThan(Time& other) {
        return (hour < other.hour) || (hour == other.hour && minute < other.minute);
    }

    bool isGreaterThan(Time& other) {
        return (hour > other.hour) || (hour == other.hour && minute > other.minute);
    }

    bool isEqualTo(Time& other) {
        return hour == other.hour && minute == other.minute;
    }

    string toString() {
        return (hour < 10 ? "0" : "") + to_string(hour) + ":" + (minute < 10 ? "0" : "") + to_string(minute);
    }

    //different code check this 
    void fromString( string& timeStr) {
        stringstream ss(timeStr);
        char delim;
        ss >> hour >> delim >> minute;
        if (delim != ':' || hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
            throw invalid_argument("Invalid time format");
        }
    }

    string serialize()  {
        return toString();
    }

    void deserialize( string& timeStr) {
        fromString(timeStr);
    }
};

//made Event a member of Time
class Event: public Time {
public:
    
    Time start;
    Time end;
    

    Event( Time s = Time(), Time e = Time()) :start(s), end(e) {
        if (end.isLessThan(start)) {
            throw invalid_argument("Event end time must be after start time");
        }
    }
    Event(string t, Time s = Time(), Time e = Time(), string r = "none") : start(s), end(e), Time(t, r) {
        if (end.isLessThan(start)) {
            throw invalid_argument("Event end time must be after start time");
        }
    }

    bool overlaps(Event& other) {
        return (start.isLessThan(other.end) && end.isGreaterThan(other.start));
    }

    string toString()  {
        return title + " from " + start.toString() + " to " + end.toString() + " (" + repeatType + ")";
    }

    string serialize() {
        return title + "|" + start.serialize() + "|" + end.serialize() + "|" + repeatType;
    }

    void deserialize( const string& eventStr) {
        stringstream ss(eventStr);
        getline(ss, title, '|');
        string startTimeStr, endTimeStr;
        getline(ss, startTimeStr, '|');
        getline(ss, endTimeStr, '|');
        getline(ss, repeatType);
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

    void addEvent( Event& event) {
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

    void deleteEvent( string& title) {
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
            throw invalid_argument("No such event exists");
        }
    }

    //just a different parameter used 
    void shiftEvent(string& title, int newDate, Day* days) {
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
                deleteEvent(title);
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

    //const cast is used
    string toString() {
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

    bool toString_print() {
        if (isDayOff) return true;
        else return false;
    }

    string serialize() {
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
            if (line.empty()) continue;
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

    void scheduleEvent(int date, Event& event) {
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
    void cancelEvent(int date, string& title, bool deleteRepeats) {
        try {
            if (date < currentDay || date > 31) {
                throw invalid_argument("Cannot cancel events in the past or beyond July 2024");
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
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }



    void shiftEvent(int date, string& title, int newDate) {
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

    void displayCalendar() {
        cout << "July 2024 Calendar\n";
        for (int i = 0; i < 31; ++i) {
            string dayStr = days[i].toString();
            if (!dayStr.empty()) {
                cout << dayStr << endl;
            }
        }
    }

    void option_list(int i) {
        string option_list[8] = { "       1. Schedule an Event","      2. Cancel an Event","      3. Shift an Event","      4. Set a Day Off","      5. View Day Schedule","               6. View Week Schedule","\t\t\t      7. View Month Schedule","\t      8. Exit"};
        SetConsoleTextAttribute(h, 14);
        cout << option_list[i];
        SetConsoleTextAttribute(h, 11);
        cout << endl;
    }

    void displayCalendar_print(int today) {
        int option_increment = 0;
        cout << endl;
        SetConsoleTextAttribute(h, 11);
        cout << "======================================================" << endl;
        SetConsoleTextAttribute(h, 14);
        cout << "                     2024 > July" << endl;
        SetConsoleTextAttribute(h, 11);
        cout << "======================================================" << endl << endl;
        SetConsoleTextAttribute(h, 14);
        cout << "   Su Mo Tu We Th Fr Sa" ;
        SetConsoleTextAttribute(h, 11);
        option_list(option_increment);
        
        int startDay = 1; // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
        
        for (int i = 0 ; i <= 31; ++i) {
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
        cout <<"   XX";
        SetConsoleTextAttribute(h, 14);
        cout << " > Off Days";
        option_list(7);
        cout << "\n";
        SetConsoleTextAttribute(h, 7);
        
    }

    void setCurrentDay(int day) {
        if (day < 1 || day > 31) {
            throw invalid_argument("Invalid day for setting current day");
        }
        currentDay = day;
    }

    void viewDaySchedule(int day)  {
        if (day < 1 || day > 31) {
            throw invalid_argument("Invalid day for viewing schedule");
        }
        cout << days[day - 1].toString() << endl;
    }

    void viewWeekSchedule(int startDay) {
        if (startDay < 1 || startDay > 31) {
            throw invalid_argument("Invalid start day for viewing week schedule");
        }
        for (int i = startDay; i < startDay + 7 && i <= 31; ++i) {
            cout << days[i - 1].toString() << endl;
        }
    }

private:
    void initializeDays() {
        string daysOfWeek[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

        for (int i = 0; i < 31; ++i) {
            days[i] = Day(i + 1, daysOfWeek[i % 7]);
        }
    }

    void saveToFile() {
        ofstream file("calendar.txt");
        if (!file.is_open()) {
            throw runtime_error("Unable to open file for saving");
        }

        for (int i = 0; i < 31; ++i) {
            file << days[i].serialize();
        }
        file.close();
    }

    //new thing used 
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
    }


    Calendar calendar(currentDay);

    while (true) {

        calendar.displayCalendar_print(currentDay);

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
                //had to change constructor mechanics to work with these 4 parameters. 
                Event event(title, start, end, repeatType);
                calendar.scheduleEvent(date, event);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
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

            calendar.cancelEvent(date, title, deleteRepeats);
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

            calendar.shiftEvent(date, title, newDate);
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

            calendar.setDayOff(date);
            break;
        }
        case 5: {
            int day;
            cout << "Enter day (1-31): ";
            cin >> day;
            try {
                calendar.viewDaySchedule(day);
                
                


            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            break;
        }

        case 6: {
            int startDay;
            cout << "Enter start day (1-31): ";
            cin >> startDay;
            try {
                calendar.viewWeekSchedule(startDay);
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
            }
            break;
        }
        case 7: {
            calendar.displayCalendar();
            break;
        }

        }
    }

    return 0;
}
