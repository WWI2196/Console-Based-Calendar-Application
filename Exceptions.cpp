#include "Exceptions.h"
#include <string>

using namespace std;

Exceptions::Exceptions(int code) {
	this->errorCode = code;
}

const char* Exceptions::what() const noexcept {
	return errorMessage.c_str();
}
