#pragma once

#include <string>
#include "Event.h"
#include "Day.h"
#include "EventExceptions.h"

using namespace std;

class Scheduler {
private:
    Day days[31];
    int currentDay;

    void initializeDays();
    void saveEventsTo_txt();
    void loadEventsFrom_txt();
    void option_list(int index);

public:
    Scheduler(int currentDay);
    ~Scheduler();

    void scheduleEvent(int date, Event& event);
    void cancelEvent(int date, string& title, bool deleteRepeats);
    void shiftEvent(int date, string& title, int newDate);
    void setDayOff(int date);
    void viewDaySchedule(int day) const;
    void viewWeekSchedule(int startDay) const;
    void displayScheduler();
    bool isEventRepeating(int date, const string& title) const;
    void displayScheduler_print(int today);
};
