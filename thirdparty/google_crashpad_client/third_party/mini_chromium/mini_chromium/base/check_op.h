// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_BASE_CHECK_OP_H_
#define MINI_CHROMIUM_BASE_CHECK_OP_H_

#include "base/logging.h"

namespace logging {

template<typename t1, typename t2>
std::string* MakeCheckOpString(const t1& v1, const t2& v2, const char* names) {
  std::ostringstream ss;
  ss << names << " (" << v1 << " vs. " << v2 << ")";
  std::string* msg = new std::string(ss.str());
  return msg;
}

#define DEFINE_CHECK_OP_IMPL(name, op) \
    template <typename t1, typename t2> \
    inline std::string* Check ## name ## Impl(const t1& v1, const t2& v2, \
                                              const char* names) { \
      if (v1 op v2) { \
        return NULL; \
      } else { \
        return MakeCheckOpString(v1, v2, names); \
      } \
    } \
    inline std::string* Check ## name ## Impl(int v1, int v2, \
                                              const char* names) { \
      if (v1 op v2) { \
        return NULL; \
      } else { \
        return MakeCheckOpString(v1, v2, names); \
      } \
    }

DEFINE_CHECK_OP_IMPL(EQ, ==)
DEFINE_CHECK_OP_IMPL(NE, !=)
DEFINE_CHECK_OP_IMPL(LE, <=)
DEFINE_CHECK_OP_IMPL(LT, <)
DEFINE_CHECK_OP_IMPL(GE, >=)
DEFINE_CHECK_OP_IMPL(GT, >)

#undef DEFINE_CHECK_OP_IMPL

}  // namespace logging

#define CHECK_OP(name, op, val1, val2) \
    if (std::string* _result = \
          logging::Check ## name ## Impl((val1), (val2), \
                                         # val1 " " # op " " # val2)) \
      logging::LogMessage(FUNCTION_SIGNATURE, __FILE__, __LINE__, \
                          _result).stream()

#define CHECK_EQ(val1, val2) CHECK_OP(EQ, ==, val1, val2)
#define CHECK_NE(val1, val2) CHECK_OP(NE, !=, val1, val2)
#define CHECK_LE(val1, val2) CHECK_OP(LE, <=, val1, val2)
#define CHECK_LT(val1, val2) CHECK_OP(LT, <, val1, val2)
#define CHECK_GE(val1, val2) CHECK_OP(GE, >=, val1, val2)
#define CHECK_GT(val1, val2) CHECK_OP(GT, >, val1, val2)


#define DCHECK_OP(name, op, val1, val2) \
    if (DCHECK_IS_ON()) \
      if (std::string* _result = \
          logging::Check ## name ## Impl((val1), (val2), \
                                         # val1 " " # op " " # val2)) \
        logging::LogMessage(FUNCTION_SIGNATURE, __FILE__, __LINE__, \
                            _result).stream()

#define DCHECK_EQ(val1, val2) DCHECK_OP(EQ, ==, val1, val2)
#define DCHECK_NE(val1, val2) DCHECK_OP(NE, !=, val1, val2)
#define DCHECK_LE(val1, val2) DCHECK_OP(LE, <=, val1, val2)
#define DCHECK_LT(val1, val2) DCHECK_OP(LT, <, val1, val2)
#define DCHECK_GE(val1, val2) DCHECK_OP(GE, >=, val1, val2)
#define DCHECK_GT(val1, val2) DCHECK_OP(GT, >, val1, val2)

#endif  // MINI_CHROMIUM_BASE_CHECK_OP_H_
