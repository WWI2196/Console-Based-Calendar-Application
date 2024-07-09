#pragma once

#include "Exceptions.h"

class TimeExceptions : public Exceptions { // Derived class for the TimeExceptions
public:
	TimeExceptions(int code);
};