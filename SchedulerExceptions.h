#pragma once

#include "Exceptions.h"

class SchedulerExceptions : public Exceptions {
public:
	SchedulerExceptions(int code);
};