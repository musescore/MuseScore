#ifndef MU_DIAGNOSTICS_DIAGNOSTICSERRORS_H
#define MU_DIAGNOSTICS_DIAGNOSTICSERRORS_H

#include "global/types/ret.h"

namespace mu::diagnostics {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    Ok              = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::DiagnosticsFirst),

    // Draw Data
    DDiff   = 3101
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::Ok: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::DDiff: return Ret(retCode, "drawdata is different");
    }

    return retCode;
}
}

#endif // MU_DIAGNOSTICS_DIAGNOSTICSERRORS_H
