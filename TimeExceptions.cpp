#include "TimeExceptions.h"

TimeExceptions::TimeExceptions(int code) : Exceptions(code) {
    switch (errorCode) {
    case 1:
        errorMessage = "Invalid time format";
        break;
    case 2:
        errorMessage = "";
        break;
    default:
        errorMessage = "Time error";
    }
}