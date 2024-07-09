#pragma once

#include "Exceptions.h"

class EventExceptions : public Exceptions { // Derived class for the EventExceptions
public:
	EventExceptions(int code);
};