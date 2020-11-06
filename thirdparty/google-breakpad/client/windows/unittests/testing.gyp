# Copyright 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{
  'includes': [
    '../../../build/common.gypi',
  ],
  'target_defaults': {
  },
  'targets': [
    {
      'target_name': 'gtest',
      'type': 'static_library',
      'include_dirs': [
        '<(DEPTH)/testing/include',
        '<(DEPTH)/testing/googletest/include',
        '<(DEPTH)/testing/googletest',
        '<(DEPTH)/testing',
      ],
      'sources': [
        '<(DEPTH)/testing/googletest/src/gtest-all.cc',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)/testing/include',
          '<(DEPTH)/testing/gtest/include',
        ],
        # Visual C++ implements variadic templates strangely, and
        # VC++2012 broke Google Test by lowering this value. See
        # http://stackoverflow.com/questions/12558327/google-test-in-visual-studio-2012
        'defines': ['_VARIADIC_MAX=10', '_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING'],
      },
      'defines': ['_VARIADIC_MAX=10', '_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING'],
    },
    {
      'target_name': 'gmock',
      'type': 'static_library',
      'include_dirs': [
        '<(DEPTH)/testing/include',
        '<(DEPTH)/testing/googletest/include',
        '<(DEPTH)/testing/googletest',
        '<(DEPTH)/testing/googlemock/include',
        '<(DEPTH)/testing/googlemock',
        '<(DEPTH)/testing',
      ],
      'sources': [
        '<(DEPTH)/testing/googlemock/src/gmock-all.cc',
        '<(DEPTH)/testing/googletest/src/gtest_main.cc',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)/testing/include',
          '<(DEPTH)/testing/googletest/include',
          '<(DEPTH)/testing/googletest',
          '<(DEPTH)/testing/googlemock/include',
          '<(DEPTH)/testing/googlemock',
          '<(DEPTH)/testing',
        ],
        'defines': ['_VARIADIC_MAX=10', '_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING'],
      },
      'defines': ['_VARIADIC_MAX=10', '_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING'],
    },

  ],
}
