#pragma once

#include "Exceptions.h"

class SchedulerExceptions : public Exceptions { // Derived class for the SchedulerExceptions
public:
	SchedulerExceptions(int code);
};