/* Copyright 2019 Guido Vranken
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
