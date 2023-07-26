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
