/*
 * Copyright (C) 2024, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <string>
#include <cstdint>

// Thanks to Deguerre https://github.com/Deguerre for figuring this out

namespace utils {

/** @brief Static class that encapsulates the encoder/decoder for a `score.dat` file taken
 * from a `.musx` file. A `.musx` file is a standard zip archive that contains
 * a directory structure containing all the data Finale uses to render a document.
 * The primary EnigmaXml document is in a file called `score.dat`. This is a Gzip archive that
 * has been encoded using the algorithm provided in this class.
 * 
 * The steps to extract EnigmaXml from a `.musx` document are:
 * - Unzip the `.musx` file.
 * - Read the `score.dat` file into a buffer.
 * - Decode the the buffer using #ScoreFileEncoder::recodeBuffer.
 * - Gunzip the decoded buffer into the EnigmaXml.
 */
class ScoreFileEncoder
{
    // These two values were determined empirically and must not be changed.
    constexpr static uint32_t INITIAL_STATE = 0x28006D45; // arbitrary initial value for algorithm
    constexpr static uint32_t RESET_LIMIT = 0x20000; // reset value corresponding (probably) to an internal Finale buffer size

public:
    /** @brief Encodes or decodes a `score.dat` file extracted
     * from a `.musx` archive. This is a symmetric algorithm that
     * encodes a decoded buffer or decodes an encoded buffer.
     * 
     * @tparam CT the character type (signed or unsigned char). This is usually inferred.
     * @param [in,out] buffer a buffer that is re-coded in place,
     * @param [in] buffSize the number of characters in the buffer.
     * @param [in,out] initialState a value to try for recoding. Must be a specific value to
     */
    template<typename CT>
    static void recodeBuffer(CT* buffer, size_t buffSize, uint32_t initialState = INITIAL_STATE)
    {
        static_assert(std::is_same<CT, uint8_t>::value ||
                      std::is_same<CT, char>::value,
                      "recodeBuffer can only be called with buffers of uint8_t or char.");
        uint32_t state = initialState;
        for (size_t i = 0; i < buffSize; i++) {
            if (i % RESET_LIMIT == 0) {
                state = initialState;
            }
            // this algorithm is BSD rand()!
            state = state * 0x41c64e6d + 0x3039;
            uint16_t upper = state >> 16;
            uint8_t c = uint8_t(upper + upper / 255);
            buffer[i] ^= c;
        }
    }

    /** @brief version of recodeBuffer for containers.
     * 
     * @tparam T the container type (of signed or unsigned chars). This is usually inferred.
     * @param [in,out] buffer a container that is re-coded in place.
     * @param [in,out] initialState a value to try for recoding. Must be a specific value to
     * successfully decode EnigmaXml.
     */
    template <typename T>
    static void recodeBuffer(T& buffer, uint32_t initialState = INITIAL_STATE)
    {
        // Ensure that the value type is either uint8_t or char
        static_assert(std::is_same<typename T::value_type, uint8_t>::value ||
                      std::is_same<typename T::value_type, char>::value,
                      "recodeBuffer can only be called with containers of uint8_t or char.");
        return recodeBuffer(buffer.data(), buffer.size(), initialState);
    }
};

} // namespace utils
