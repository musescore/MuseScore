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

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fuzzing/memory.hpp>
#include <vector>
#include <string>

namespace fuzzing {
namespace types {

template <typename CoreType, bool NullTerminated, bool UseMSAN = false>
class Container {
    private:
        CoreType* InvalidAddress = (CoreType*)0x12;

        CoreType* _data = InvalidAddress;
        size_t _size = 0;

#ifndef FUZZING_HEADERS_NO_IMPL
        void copy(const void* data, size_t size) {
            if ( size > 0 ) {
                std::memcpy(_data, data, size);
            }
        }

        void allocate(size_t size) {
            if ( size > 0 ) {
                _data = static_cast<CoreType*>(malloc(size * sizeof(CoreType)));
            } else {
                _data = InvalidAddress;
            }
        };

        void allocate_and_copy(const void* data, size_t size) {
            allocate(size);
            copy(data, size);
        }

        void allocate_plus_1_and_copy(const void* data, size_t size) {
            allocate(size+1);
            copy(data, size);
        }

        void access_hook(void) const {
            if ( UseMSAN == true ) {
                memory::memory_test_msan(_data, _size);
            }
        }

        void free(void) {
            access_hook();

            if ( _data != InvalidAddress ) {
                std::free(_data);
                _data = InvalidAddress;
                _size = 0;
            }
        }

#endif

    public:
#ifndef FUZZING_HEADERS_NO_IMPL
        CoreType* data(void) {
            access_hook();
            return _data;
        }

        size_t size(void) const {
            access_hook();
            return _size;
        }
#endif

        Container(void)
#ifndef FUZZING_HEADERS_NO_IMPL
        = default
#endif
        ;

        Container(const void* data, const size_t size)
#ifndef FUZZING_HEADERS_NO_IMPL
        {
            if ( NullTerminated == false ) {
                allocate_and_copy(data, size);
            } else {
                allocate_plus_1_and_copy(data, size);
                _data[size] = 0;
            }

            access_hook();
        }
#endif
        ;

        template<class T>
        Container(const T& t)
#ifndef FUZZING_HEADERS_NO_IMPL
        {
            Container(t.data(), t.size());
        }
#endif
        ;

        ~Container(void)
#ifndef FUZZING_HEADERS_NO_IMPL
        {
            this->free();
        }
#endif
        ;



        // The copy constructor was not originally explicitly supplied
        // so it must have been incorrectly just copying the pointers.
        Container(const Container &c) {
          InvalidAddress = c.InvalidAddress;
          allocate_and_copy(c._data, c._size);
        }

        Container& operator=(Container &c) {
          InvalidAddress = c.InvalidAddress;
          allocate_and_copy(c._data, c._size);
        }

};

template <bool UseMSAN = false> using String = Container<char, true, UseMSAN>;
template <bool UseMSAN = false> using Data = Container<uint8_t, false, UseMSAN>;

} /* namespace types */
} /* namespace fuzzing */
