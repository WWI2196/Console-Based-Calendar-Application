#include "TimeExceptions.h"

TimeExceptions::TimeExceptions(int code) : Exceptions(code) { // Constructor for the TimeExceptions class
    switch (errorCode) {
    case 1:
        errorMessage = "Invalid time format";
        break;
    default:
        errorMessage = "Time error";
    }
}