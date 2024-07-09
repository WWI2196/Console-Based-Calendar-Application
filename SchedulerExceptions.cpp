#include "SchedulerExceptions.h"

SchedulerExceptions::SchedulerExceptions(int code) : Exceptions(code) { // Constructor for the SchedulerExceptions class
	switch (errorCode) {
	case 1:
		errorMessage = "Invalid input. Please enter a valid date.";
		break;
	case 2:
		errorMessage = "The number entered is out of range.";
		break;
	case 3:
		errorMessage = "Invalid input. Please enter a valid option.";
		break;
	case 4:
		errorMessage = "Unable to open file for saving";
		break;
	case 5:
		errorMessage = "Unable to open file for loading";
		break;
	default:
		errorMessage = "Scheduler error";
	}
}