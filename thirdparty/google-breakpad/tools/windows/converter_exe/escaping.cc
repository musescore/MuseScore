// Copyright 2019 Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "tools/windows/converter_exe/escaping.h"

#include <assert.h>

#define kApb kAsciiPropertyBits

const unsigned char kAsciiPropertyBits[256] = {
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  // 0x00
  0x40, 0x68, 0x48, 0x48, 0x48, 0x48, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,  // 0x10
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,  // 0x20
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x84, 0x84, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x05,  // 0x40
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,  // 0x50
  0x05, 0x05, 0x05, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x05,  // 0x60
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,  // 0x70
  0x05, 0x05, 0x05, 0x10, 0x10, 0x10, 0x10, 0x40,
};

// Use !! to suppress the warning C4800 of forcing 'int' to 'bool'.
static inline bool ascii_isspace(unsigned char c) { return !!(kApb[c] & 0x08); }

///////////////////////////////////
// scoped_array
///////////////////////////////////
// scoped_array<C> is like scoped_ptr<C>, except that the caller must allocate
// with new [] and the destructor deletes objects with delete [].
//
// As with scoped_ptr<C>, a scoped_array<C> either points to an object
// or is NULL.  A scoped_array<C> owns the object that it points to.
// scoped_array<T> is thread-compatible, and once you index into it,
// the returned objects have only the threadsafety guarantees of T.
//
// Size: sizeof(scoped_array<C>) == sizeof(C*)
template <class C>
class scoped_array {
 public:

  // The element type
  typedef C element_type;

  // Constructor.  Defaults to intializing with NULL.
  // There is no way to create an uninitialized scoped_array.
  // The input parameter must be allocated with new [].
  explicit scoped_array(C* p = NULL) : array_(p) { }

  // Destructor.  If there is a C object, delete it.
  // We don't need to test ptr_ == NULL because C++ does that for us.
  ~scoped_array() {
    enum { type_must_be_complete = sizeof(C) };
    delete[] array_;
  }

  // Reset.  Deletes the current owned object, if any.
  // Then takes ownership of a new object, if given.
  // this->reset(this->get()) works.
  void reset(C* p = NULL) {
    if (p != array_) {
      enum { type_must_be_complete = sizeof(C) };
      delete[] array_;
      array_ = p;
    }
  }

  // Get one element of the current object.
  // Will assert() if there is no current object, or index i is negative.
  C& operator[](std::ptrdiff_t i) const {
    assert(i >= 0);
    assert(array_ != NULL);
    return array_[i];
  }

  // Get a pointer to the zeroth element of the current object.
  // If there is no current object, return NULL.
  C* get() const {
    return array_;
  }

  // Comparison operators.
  // These return whether a scoped_array and a raw pointer refer to
  // the same array, not just to two different but equal arrays.
  bool operator==(const C* p) const { return array_ == p; }
  bool operator!=(const C* p) const { return array_ != p; }

  // Swap two scoped arrays.
  void swap(scoped_array& p2) {
    C* tmp = array_;
    array_ = p2.array_;
    p2.array_ = tmp;
  }

  // Release an array.
  // The return value is the current pointer held by this object.
  // If this object holds a NULL pointer, the return value is NULL.
  // After this operation, this object will hold a NULL pointer,
  // and will not own the object any more.
  C* release() {
    C* retVal = array_;
    array_ = NULL;
    return retVal;
  }

 private:
  C* array_;

  // Forbid comparison of different scoped_array types.
  template <class C2> bool operator==(scoped_array<C2> const& p2) const;
  template <class C2> bool operator!=(scoped_array<C2> const& p2) const;

  // Disallow evil constructors
  scoped_array(const scoped_array&);
  void operator=(const scoped_array&);
};


///////////////////////////////////
// Escape methods
///////////////////////////////////

namespace strings {

// Return a mutable char* pointing to a string's internal buffer,
// which may not be null-terminated. Writing through this pointer will
// modify the string.
//
// string_as_array(&str)[i] is valid for 0 <= i < str.size() until the
// next call to a string method that invalidates iterators.
//
// As of 2006-04, there is no standard-blessed way of getting a
// mutable reference to a string's internal buffer. However, issue 530
// (http://www.open-std.org/JTC1/SC22/WG21/docs/lwg-active.html#530)
// proposes this as the method. According to Matt Austern, this should
// already work on all current implementations.
inline char* string_as_array(string* str) {
  // DO NOT USE const_cast<char*>(str->data())! See the unittest for why.
  return str->empty() ? NULL : &*str->begin();
}

int CalculateBase64EscapedLen(int input_len, bool do_padding) {
  // these formulae were copied from comments that used to go with the base64
  // encoding functions
  int intermediate_result = 8 * input_len + 5;
  assert(intermediate_result > 0);     // make sure we didn't overflow
  int len = intermediate_result / 6;
  if (do_padding) len = ((len + 3) / 4) * 4;
  return len;
}

// Base64Escape does padding, so this calculation includes padding.
int CalculateBase64EscapedLen(int input_len) {
  return CalculateBase64EscapedLen(input_len, true);
}

// ----------------------------------------------------------------------
// int Base64Unescape() - base64 decoder
// int Base64Escape() - base64 encoder
// int WebSafeBase64Unescape() - Google's variation of base64 decoder
// int WebSafeBase64Escape() - Google's variation of base64 encoder
//
// Check out
// http://www.cis.ohio-state.edu/htbin/rfc/rfc2045.html for formal
// description, but what we care about is that...
//   Take the encoded stuff in groups of 4 characters and turn each
//   character into a code 0 to 63 thus:
//           A-Z map to 0 to 25
//           a-z map to 26 to 51
//           0-9 map to 52 to 61
//           +(- for WebSafe) maps to 62
//           /(_ for WebSafe) maps to 63
//   There will be four numbers, all less than 64 which can be represented
//   by a 6 digit binary number (aaaaaa, bbbbbb, cccccc, dddddd respectively).
//   Arrange the 6 digit binary numbers into three bytes as such:
//   aaaaaabb bbbbcccc ccdddddd
//   Equals signs (one or two) are used at the end of the encoded block to
//   indicate that the text was not an integer multiple of three bytes long.
// ----------------------------------------------------------------------

int Base64UnescapeInternal(const char *src, int szsrc,
                           char *dest, int szdest,
                           const signed char* unbase64) {
  static const char kPad64 = '=';

  int decode = 0;
  int destidx = 0;
  int state = 0;
  unsigned int ch = 0;
  unsigned int temp = 0;

  // The GET_INPUT macro gets the next input character, skipping
  // over any whitespace, and stopping when we reach the end of the
  // string or when we read any non-data character.  The arguments are
  // an arbitrary identifier (used as a label for goto) and the number
  // of data bytes that must remain in the input to avoid aborting the
  // loop.
#define GET_INPUT(label, remain)                 \
  label:                                         \
    --szsrc;                                     \
    ch = *src++;                                 \
    decode = unbase64[ch];                       \
    if (decode < 0) {                            \
      if (ascii_isspace((char)ch) && szsrc >= remain)  \
        goto label;                              \
      state = 4 - remain;                        \
      break;                                     \
    }

  // if dest is null, we're just checking to see if it's legal input
  // rather than producing output.  (I suspect this could just be done
  // with a regexp...).  We duplicate the loop so this test can be
  // outside it instead of in every iteration.

  if (dest) {
    // This loop consumes 4 input bytes and produces 3 output bytes
    // per iteration.  We can't know at the start that there is enough
    // data left in the string for a full iteration, so the loop may
    // break out in the middle; if so 'state' will be set to the
    // number of input bytes read.

    while (szsrc >= 4)  {
      // We'll start by optimistically assuming that the next four
      // bytes of the string (src[0..3]) are four good data bytes
      // (that is, no nulls, whitespace, padding chars, or illegal
      // chars).  We need to test src[0..2] for nulls individually
      // before constructing temp to preserve the property that we
      // never read past a null in the string (no matter how long
      // szsrc claims the string is).

      if (!src[0] || !src[1] || !src[2] ||
          (temp = ((unbase64[static_cast<int>(src[0])] << 18) |
                   (unbase64[static_cast<int>(src[1])] << 12) |
                   (unbase64[static_cast<int>(src[2])] << 6) |
                   (unbase64[static_cast<int>(src[3])]))) & 0x80000000) {
        // Iff any of those four characters was bad (null, illegal,
        // whitespace, padding), then temp's high bit will be set
        // (because unbase64[] is -1 for all bad characters).
        //
        // We'll back up and resort to the slower decoder, which knows
        // how to handle those cases.

        GET_INPUT(first, 4);
        temp = decode;
        GET_INPUT(second, 3);
        temp = (temp << 6) | decode;
        GET_INPUT(third, 2);
        temp = (temp << 6) | decode;
        GET_INPUT(fourth, 1);
        temp = (temp << 6) | decode;
      } else {
        // We really did have four good data bytes, so advance four
        // characters in the string.

        szsrc -= 4;
        src += 4;
        decode = -1;
        ch = '\0';
      }

      // temp has 24 bits of input, so write that out as three bytes.

      if (destidx+3 > szdest) return -1;
      dest[destidx+2] = (char)temp;
      temp >>= 8;
      dest[destidx+1] = (char)temp;
      temp >>= 8;
      dest[destidx] = (char)temp;
      destidx += 3;
    }
  } else {
    while (szsrc >= 4)  {
      if (!src[0] || !src[1] || !src[2] ||
          (temp = ((unbase64[static_cast<int>(src[0])] << 18) |
                   (unbase64[static_cast<int>(src[1])] << 12) |
                   (unbase64[static_cast<int>(src[2])] << 6) |
                   (unbase64[static_cast<int>(src[3])]))) & 0x80000000) {
        GET_INPUT(first_no_dest, 4);
        GET_INPUT(second_no_dest, 3);
        GET_INPUT(third_no_dest, 2);
        GET_INPUT(fourth_no_dest, 1);
      } else {
        szsrc -= 4;
        src += 4;
        decode = -1;
        ch = '\0';
      }
      destidx += 3;
    }
  }

#undef GET_INPUT

  // if the loop terminated because we read a bad character, return
  // now.
  if (decode < 0 && ch != '\0' && ch != kPad64 && !ascii_isspace((char)ch))
    return -1;

  if (ch == kPad64) {
    // if we stopped by hitting an '=', un-read that character -- we'll
    // look at it again when we count to check for the proper number of
    // equals signs at the end.
    ++szsrc;
    --src;
  } else {
    // This loop consumes 1 input byte per iteration.  It's used to
    // clean up the 0-3 input bytes remaining when the first, faster
    // loop finishes.  'temp' contains the data from 'state' input
    // characters read by the first loop.
    while (szsrc > 0)  {
      --szsrc;
      ch = *src++;
      decode = unbase64[ch];
      if (decode < 0) {
        if (ascii_isspace((char)ch)) {
          continue;
        } else if (ch == '\0') {
          break;
        } else if (ch == kPad64) {
          // back up one character; we'll read it again when we check
          // for the correct number of equals signs at the end.
          ++szsrc;
          --src;
          break;
        } else {
          return -1;
        }
      }

      // Each input character gives us six bits of output.
      temp = (temp << 6) | decode;
      ++state;
      if (state == 4) {
        // If we've accumulated 24 bits of output, write that out as
        // three bytes.
        if (dest) {
          if (destidx+3 > szdest) return -1;
          dest[destidx+2] = (char)temp;
          temp >>= 8;
          dest[destidx+1] = (char)temp;
          temp >>= 8;
          dest[destidx] = (char)temp;
        }
        destidx += 3;
        state = 0;
        temp = 0;
      }
    }
  }

  // Process the leftover data contained in 'temp' at the end of the input.
  int expected_equals = 0;
  switch (state) {
    case 0:
      // Nothing left over; output is a multiple of 3 bytes.
      break;

    case 1:
      // Bad input; we have 6 bits left over.
      return -1;

    case 2:
      // Produce one more output byte from the 12 input bits we have left.
      if (dest) {
        if (destidx+1 > szdest) return -1;
        temp >>= 4;
        dest[destidx] = (char)temp;
      }
      ++destidx;
      expected_equals = 2;
      break;

    case 3:
      // Produce two more output bytes from the 18 input bits we have left.
      if (dest) {
        if (destidx+2 > szdest) return -1;
        temp >>= 2;
        dest[destidx+1] = (char)temp;
        temp >>= 8;
        dest[destidx] = (char)temp;
      }
      destidx += 2;
      expected_equals = 1;
      break;

    default:
      // state should have no other values at this point.
      fprintf(stdout, "This can't happen; base64 decoder state = %d", state);
  }

  // The remainder of the string should be all whitespace, mixed with
  // exactly 0 equals signs, or exactly 'expected_equals' equals
  // signs.  (Always accepting 0 equals signs is a google extension
  // not covered in the RFC.)

  int equals = 0;
  while (szsrc > 0 && *src) {
    if (*src == kPad64)
      ++equals;
    else if (!ascii_isspace(*src))
      return -1;
    --szsrc;
    ++src;
  }

  return (equals == 0 || equals == expected_equals) ? destidx : -1;
}

int Base64Unescape(const char *src, int szsrc, char *dest, int szdest) {
  static const signed char UnBase64[] = {
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      62/*+*/, -1,      -1,      -1,      63/*/ */,
     52/*0*/, 53/*1*/, 54/*2*/, 55/*3*/, 56/*4*/, 57/*5*/, 58/*6*/, 59/*7*/,
     60/*8*/, 61/*9*/, -1,      -1,      -1,      -1,      -1,      -1,
     -1,       0/*A*/,  1/*B*/,  2/*C*/,  3/*D*/,  4/*E*/,  5/*F*/,  6/*G*/,
      7/*H*/,  8/*I*/,  9/*J*/, 10/*K*/, 11/*L*/, 12/*M*/, 13/*N*/, 14/*O*/,
     15/*P*/, 16/*Q*/, 17/*R*/, 18/*S*/, 19/*T*/, 20/*U*/, 21/*V*/, 22/*W*/,
     23/*X*/, 24/*Y*/, 25/*Z*/, -1,      -1,      -1,      -1,      -1,
     -1,      26/*a*/, 27/*b*/, 28/*c*/, 29/*d*/, 30/*e*/, 31/*f*/, 32/*g*/,
     33/*h*/, 34/*i*/, 35/*j*/, 36/*k*/, 37/*l*/, 38/*m*/, 39/*n*/, 40/*o*/,
     41/*p*/, 42/*q*/, 43/*r*/, 44/*s*/, 45/*t*/, 46/*u*/, 47/*v*/, 48/*w*/,
     49/*x*/, 50/*y*/, 51/*z*/, -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1
  };
  // The above array was generated by the following code
  // #include <sys/time.h>
  // #include <stdlib.h>
  // #include <string.h>
  // main()
  // {
  //   static const char Base64[] =
  //     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  //   char *pos;
  //   int idx, i, j;
  //   printf("    ");
  //   for (i = 0; i < 255; i += 8) {
  //     for (j = i; j < i + 8; j++) {
  //       pos = strchr(Base64, j);
  //       if ((pos == NULL) || (j == 0))
  //         idx = -1;
  //       else
  //         idx = pos - Base64;
  //       if (idx == -1)
  //         printf(" %2d,     ", idx);
  //       else
  //         printf(" %2d/*%c*/,", idx, j);
  //     }
  //     printf("\n    ");
  //   }
  // }

  return Base64UnescapeInternal(src, szsrc, dest, szdest, UnBase64);
}

bool Base64Unescape(const char *src, int slen, string* dest) {
  // Determine the size of the output string.  Base64 encodes every 3 bytes into
  // 4 characters.  any leftover chars are added directly for good measure.
  // This is documented in the base64 RFC: http://www.ietf.org/rfc/rfc3548.txt
  const int dest_len = 3 * (slen / 4) + (slen % 4);

  dest->resize(dest_len);

  // We are getting the destination buffer by getting the beginning of the
  // string and converting it into a char *.
  const int len = Base64Unescape(src, slen,
                                 string_as_array(dest), dest->size());
  if (len < 0) {
    return false;
  }

  // could be shorter if there was padding
  assert(len <= dest_len);
  dest->resize(len);

  return true;
}

// Base64Escape
//
// NOTE: We have to use an unsigned type for src because code built
// in the the /google tree treats characters as signed unless
// otherwised specified.
//
// TODO(who?): Move this function to use the char* type for "src"
int Base64EscapeInternal(const unsigned char *src, int szsrc,
                         char *dest, int szdest, const char *base64,
                         bool do_padding) {
  static const char kPad64 = '=';

  if (szsrc <= 0) return 0;

  char *cur_dest = dest;
  const unsigned char *cur_src = src;

  // Three bytes of data encodes to four characters of cyphertext.
  // So we can pump through three-byte chunks atomically.
  while (szsrc > 2) { /* keep going until we have less than 24 bits */
    if ((szdest -= 4) < 0) return 0;
    cur_dest[0] = base64[cur_src[0] >> 2];
    cur_dest[1] = base64[((cur_src[0] & 0x03) << 4) + (cur_src[1] >> 4)];
    cur_dest[2] = base64[((cur_src[1] & 0x0f) << 2) + (cur_src[2] >> 6)];
    cur_dest[3] = base64[cur_src[2] & 0x3f];

    cur_dest += 4;
    cur_src += 3;
    szsrc -= 3;
  }

  /* now deal with the tail (<=2 bytes) */
  switch (szsrc) {
    case 0:
      // Nothing left; nothing more to do.
      break;
    case 1:
      // One byte left: this encodes to two characters, and (optionally)
      // two pad characters to round out the four-character cypherblock.
      if ((szdest -= 2) < 0) return 0;
      cur_dest[0] = base64[cur_src[0] >> 2];
      cur_dest[1] = base64[(cur_src[0] & 0x03) << 4];
      cur_dest += 2;
      if (do_padding) {
        if ((szdest -= 2) < 0) return 0;
        cur_dest[0] = kPad64;
        cur_dest[1] = kPad64;
        cur_dest += 2;
      }
      break;
    case 2:
      // Two bytes left: this encodes to three characters, and (optionally)
      // one pad character to round out the four-character cypherblock.
      if ((szdest -= 3) < 0) return 0;
      cur_dest[0] = base64[cur_src[0] >> 2];
      cur_dest[1] = base64[((cur_src[0] & 0x03) << 4) + (cur_src[1] >> 4)];
      cur_dest[2] = base64[(cur_src[1] & 0x0f) << 2];
      cur_dest += 3;
      if (do_padding) {
        if ((szdest -= 1) < 0) return 0;
        cur_dest[0] = kPad64;
        cur_dest += 1;
      }
      break;
    default:
      // Should not be reached: blocks of 3 bytes are handled
      // in the while loop before this switch statement.
      fprintf(stderr, "Logic problem? szsrc = %d",  szsrc);
      assert(false);
      break;
  }
  return (cur_dest - dest);
}

static const char kBase64Chars[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char kWebSafeBase64Chars[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int Base64Escape(const unsigned char *src, int szsrc, char *dest, int szdest) {
  return Base64EscapeInternal(src, szsrc, dest, szdest, kBase64Chars, true);
}

void Base64Escape(const unsigned char *src, int szsrc,
                  string* dest, bool do_padding) {
  const int max_escaped_size =
    CalculateBase64EscapedLen(szsrc, do_padding);
  dest->clear();
  dest->resize(max_escaped_size + 1, '\0');
  const int escaped_len = Base64EscapeInternal(src, szsrc,
                                               &*dest->begin(), dest->size(),
                                               kBase64Chars,
                                               do_padding);
  assert(max_escaped_size <= escaped_len);
  dest->resize(escaped_len);
}

void Base64Escape(const string& src, string* dest) {
  Base64Escape(reinterpret_cast<const unsigned char*>(src.c_str()),
               src.size(), dest, true);
}

////////////////////////////////////////////////////
// WebSafe methods
////////////////////////////////////////////////////

int WebSafeBase64Unescape(const char *src, int szsrc, char *dest, int szdest) {
  static const signed char UnBase64[] = {
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      62/*-*/, -1,      -1,
     52/*0*/, 53/*1*/, 54/*2*/, 55/*3*/, 56/*4*/, 57/*5*/, 58/*6*/, 59/*7*/,
     60/*8*/, 61/*9*/, -1,      -1,      -1,      -1,      -1,      -1,
     -1,       0/*A*/,  1/*B*/,  2/*C*/,  3/*D*/,  4/*E*/,  5/*F*/,  6/*G*/,
      7/*H*/,  8/*I*/,  9/*J*/, 10/*K*/, 11/*L*/, 12/*M*/, 13/*N*/, 14/*O*/,
     15/*P*/, 16/*Q*/, 17/*R*/, 18/*S*/, 19/*T*/, 20/*U*/, 21/*V*/, 22/*W*/,
     23/*X*/, 24/*Y*/, 25/*Z*/, -1,      -1,      -1,      -1,      63/*_*/,
     -1,      26/*a*/, 27/*b*/, 28/*c*/, 29/*d*/, 30/*e*/, 31/*f*/, 32/*g*/,
     33/*h*/, 34/*i*/, 35/*j*/, 36/*k*/, 37/*l*/, 38/*m*/, 39/*n*/, 40/*o*/,
     41/*p*/, 42/*q*/, 43/*r*/, 44/*s*/, 45/*t*/, 46/*u*/, 47/*v*/, 48/*w*/,
     49/*x*/, 50/*y*/, 51/*z*/, -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1
  };
  // The above array was generated by the following code
  // #include <sys/time.h>
  // #include <stdlib.h>
  // #include <string.h>
  // main()
  // {
  //   static const char Base64[] =
  //     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
  //   char *pos;
  //   int idx, i, j;
  //   printf("    ");
  //   for (i = 0; i < 255; i += 8) {
  //     for (j = i; j < i + 8; j++) {
  //       pos = strchr(Base64, j);
  //       if ((pos == NULL) || (j == 0))
  //         idx = -1;
  //       else
  //         idx = pos - Base64;
  //       if (idx == -1)
  //         printf(" %2d,     ", idx);
  //       else
  //         printf(" %2d/*%c*/,", idx, j);
  //     }
  //     printf("\n    ");
  //   }
  // }

  return Base64UnescapeInternal(src, szsrc, dest, szdest, UnBase64);
}

bool WebSafeBase64Unescape(const char *src, int slen, string* dest) {
  int dest_len = 3 * (slen / 4) + (slen % 4);
  dest->clear();
  dest->resize(dest_len);
  int len = WebSafeBase64Unescape(src, slen, &*dest->begin(), dest->size());
  if (len < 0) {
    dest->clear();
    return false;
  }
  // could be shorter if there was padding
  assert(len <= dest_len);
  dest->resize(len);
  return true;
}

bool WebSafeBase64Unescape(const string& src, string* dest) {
  return WebSafeBase64Unescape(src.data(), src.size(), dest);
}

int WebSafeBase64Escape(const unsigned char *src, int szsrc, char *dest,
                        int szdest, bool do_padding) {
  return Base64EscapeInternal(src, szsrc, dest, szdest,
                              kWebSafeBase64Chars, do_padding);
}

void WebSafeBase64Escape(const unsigned char *src, int szsrc,
                         string *dest, bool do_padding) {
  const int max_escaped_size =
    CalculateBase64EscapedLen(szsrc, do_padding);
  dest->clear();
  dest->resize(max_escaped_size + 1, '\0');
  const int escaped_len = Base64EscapeInternal(src, szsrc,
                                               &*dest->begin(), dest->size(),
                                               kWebSafeBase64Chars,
                                               do_padding);
  assert(max_escaped_size <= escaped_len);
  dest->resize(escaped_len);
}

void WebSafeBase64EscapeInternal(const string& src,
                                 string* dest,
                                 bool do_padding) {
  int encoded_len = CalculateBase64EscapedLen(src.size());
  scoped_array<char> buf(new char[encoded_len]);
  int len = WebSafeBase64Escape(reinterpret_cast<const unsigned char*>(src.c_str()),
                                src.size(), buf.get(),
                                encoded_len, do_padding);
  dest->assign(buf.get(), len);
}

void WebSafeBase64Escape(const string& src, string* dest) {
  WebSafeBase64EscapeInternal(src, dest, false);
}

void WebSafeBase64EscapeWithPadding(const string& src, string* dest) {
  WebSafeBase64EscapeInternal(src, dest, true);
}

}  // namespace strings
