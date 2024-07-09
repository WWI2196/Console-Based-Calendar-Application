#include "Exceptions.h"
#include <string>

using namespace std;

Exceptions::Exceptions(int code) {
	this->errorCode = code;
}

const char* Exceptions::what() const noexcept { // Returns the error message
	return errorMessage.c_str();
}
