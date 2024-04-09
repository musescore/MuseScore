#ifndef MU_ENGRAVING_DIAGNOSTICSERRORS_H
#define MU_ENGRAVING_DIAGNOSTICSERRORS_H

#include "global/types/ret.h"

namespace mu::engraving {
enum class Err {
    Undefined       = int(muse::Ret::Code::Undefined),
    Ok              = int(muse::Ret::Code::Ok),
    UnknownError    = int(muse::Ret::Code::DiagnosticsFirst),

    // Draw Data
    DDiff   = 3101
};

inline muse::Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return muse::Ret(retCode);
    case Err::Ok: return muse::Ret(retCode);
    case Err::UnknownError: return muse::Ret(retCode);
    case Err::DDiff: return muse::Ret(retCode, "drawdata is different");
    }

    return retCode;
}
}

#endif // MU_ENGRAVING_DIAGNOSTICSERRORS_H
