#include "DayExceptions.h"

DayExceptions::DayExceptions(int code) : Exceptions(code) {
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