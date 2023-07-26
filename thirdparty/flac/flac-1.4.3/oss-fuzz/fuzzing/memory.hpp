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

#include <stdio.h>
#include <optional>

#ifndef ASAN
#define ASAN 0
#endif

#ifndef MSAN
#define MSAN 0
#endif

namespace fuzzing {
namespace memory {

#ifndef FUZZING_HEADERS_NO_IMPL
#if ASAN == 1
extern "C" void *__asan_region_is_poisoned(const void *beg, size_t size);
#endif

#if MSAN == 1
extern "C" void __msan_check_mem_is_initialized(const volatile void *x, size_t size);
#endif

void memory_test_asan(const void* data, const size_t size)
{
    (void)data;
    (void)size;

#if ASAN == 1
    if ( __asan_region_is_poisoned(data, size) != NULL ) {
        abort();
    }
#endif
}

void memory_test_msan(const void* data, const size_t size)
{
    (void)data;
    (void)size;

#if MSAN == 1
    __msan_check_mem_is_initialized(data, size);
#endif
}

void memory_test(const void* data, const size_t size)
{
    memory_test_asan(data, size);
    memory_test_msan(data, size);
}

template <class T>
void memory_test(const T& t)
{
    (void)t;
}

template <>
void memory_test(const std::string& s)
{
    (void)s;

#if MSAN == 1
    memory_test(s.data(), s.size());
#endif
}

#endif

} /* namespace memory */
} /* namespace fuzzing */
