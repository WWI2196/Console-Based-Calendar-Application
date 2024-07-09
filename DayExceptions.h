#pragma once

#include "Exceptions.h"

class DayExceptions : public Exceptions { // Derived class for the DayExceptions
public:
	DayExceptions(int code);
};