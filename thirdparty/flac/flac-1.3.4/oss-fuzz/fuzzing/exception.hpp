#pragma once

#include <exception>
#include <string>

namespace fuzzing {
namespace exception {

class ExceptionBase : public std::exception {
    public:
        ExceptionBase(void) = default;
        /* typeid(T).name */
};

/* Recoverable exception */
class FlowException : public ExceptionBase {
    public:
        FlowException(void) : ExceptionBase() { }
};

/* Error in this library, should never happen */
class LogicException : public ExceptionBase {
    private:
        std::string reason;
    public:
        LogicException(const std::string r) : ExceptionBase(), reason(r) { }
        virtual const char* what(void) const throw() {
            return reason.c_str();
        }
};

/* Error in target application */
class TargetException : public ExceptionBase {
    private:
        std::string reason;
    public:
        TargetException(const std::string r) : ExceptionBase(), reason(r) { }
        virtual const char* what(void) const throw() {
            return reason.c_str();
        }
};

} /* namespace exception */
} /* namespace fuzzing */
