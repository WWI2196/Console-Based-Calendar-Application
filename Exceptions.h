#pragma once

#include <exception>
#include <string>

using namespace std;

class Exceptions : public exception {
protected:
    int errorCode;
    string errorMessage;


public:
    Exceptions(int code);

    virtual const char* what() const noexcept override;

    /*
    * Referred from the GitHub repository: phosphor-pid-control https://github.com/openbmc/phosphor-pid-control/blob/master/errors/exception.hpp
    * Author: OpenBMC
    */
};

