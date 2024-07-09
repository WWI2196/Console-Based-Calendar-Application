#include "EventExceptions.h"

EventExceptions::EventExceptions(int code) : Exceptions(code) {
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