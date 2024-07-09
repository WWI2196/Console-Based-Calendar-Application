#pragma once

#include <exception>
#include <string>

using namespace std;

class Exceptions : public exception { // Base class for all exceptions
protected:
    int errorCode;
    string errorMessage;


public:
    Exceptions(int code);

    virtual const char* what() const noexcept override; // Returns the error message

    /*
    * Referred from the GitHub repository: phosphor-pid-control https://github.com/openbmc/phosphor-pid-control/blob/master/errors/exception.hpp
    * Author: OpenBMC
    */
};

