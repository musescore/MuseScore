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

#include <fuzzing/exception.hpp>
#include <fuzzing/types.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace fuzzing {
namespace datasource  {

class Base
{
    protected:
        virtual std::vector<uint8_t> get(const size_t min, const size_t max, const uint64_t id = 0) = 0;
    public:
        Base(void) = default;
        virtual ~Base(void) = default;

        template<class T> T Get(const uint64_t id = 0);
        uint16_t GetChoice(const uint64_t id = 0);
        std::vector<uint8_t> GetData(const uint64_t id, const size_t min = 0, const size_t max = 0);
        template <class T> std::vector<T> GetVector(const uint64_t id = 0);

        class OutOfData : public fuzzing::exception::FlowException {
            public:
                OutOfData() = default;
        };

        class DeserializationFailure : public fuzzing::exception::FlowException {
            public:
                DeserializationFailure() = default;
        };
};

#ifndef FUZZING_HEADERS_NO_IMPL
template<class T> T Base::Get(const uint64_t id)
{
    T ret;
    const auto v = get(sizeof(ret), sizeof(ret), id);
    memcpy(&ret, v.data(), sizeof(ret));
    return ret;
}

template <> bool Base::Get<bool>(const uint64_t id)
{
    uint8_t ret;
    const auto v = get(sizeof(ret), sizeof(ret), id);
    memcpy(&ret, v.data(), sizeof(ret));
    return (ret % 2) ? true : false;
}

template <> std::string Base::Get<std::string>(const uint64_t id)
{
    auto data = GetData(id);
    return std::string(data.data(), data.data() + data.size());
}

template <> std::vector<std::string> Base::Get<std::vector<std::string>>(const uint64_t id)
{
    std::vector<std::string> ret;
    while ( true ) {
        auto data = GetData(id);
        ret.push_back( std::string(data.data(), data.data() + data.size()) );
        if ( Get<bool>(id) == false ) {
            break;
        }
    }
    return ret;
}

uint16_t Base::GetChoice(const uint64_t id)
{
    return Get<uint16_t>(id);
}

std::vector<uint8_t> Base::GetData(const uint64_t id, const size_t min, const size_t max)
{
    return get(min, max, id);
}


template <> types::String<> Base::Get<types::String<>>(const uint64_t id) {
    const auto data = GetData(id);
    types::String<> ret(data.data(), data.size());
    return ret;
}

template <> types::Data<> Base::Get<types::Data<>>(const uint64_t id) {
    const auto data = GetData(id);
    types::Data<> ret(data.data(), data.size());
    return ret;
}

template <class T>
std::vector<T> Base::GetVector(const uint64_t id) {
    std::vector<T> ret;

    while ( Get<bool>(id) == true ) {
        ret.push_back( Get<T>(id) );
    }

    return ret;
}
#endif

class Datasource : public Base
{
    private:
        const uint8_t* data;
        const size_t size;
        size_t idx;
        size_t left;
        std::vector<uint8_t> get(const size_t min, const size_t max, const uint64_t id = 0) override;

		// Make copy constructor and assignment operator private.
        Datasource(const Datasource &) : data(0), size(0), idx(0), left(0) {}
        Datasource& operator=(const Datasource &) { return *this; }
    public:
        Datasource(const uint8_t* _data, const size_t _size);
};

#ifndef FUZZING_HEADERS_NO_IMPL
Datasource::Datasource(const uint8_t* _data, const size_t _size) :
    Base(), data(_data), size(_size), idx(0), left(size)
{
}

std::vector<uint8_t> Datasource::get(const size_t min, const size_t max, const uint64_t id) {
    (void)id;

    uint32_t getSize;
    if ( left < sizeof(getSize) ) {
        throw OutOfData();
    }
    memcpy(&getSize, data + idx, sizeof(getSize));
    idx += sizeof(getSize);
    left -= sizeof(getSize);

    if ( getSize < min ) {
        getSize = min;
    }
    if ( max && getSize > max ) {
        getSize = max;
    }

    if ( left < getSize ) {
        throw OutOfData();
    }

    std::vector<uint8_t> ret(getSize);

    if ( getSize > 0 ) {
        memcpy(ret.data(), data + idx, getSize);
    }
    idx += getSize;
    left -= getSize;

    return ret;
}
#endif

} /* namespace datasource */
} /* namespace fuzzing */
