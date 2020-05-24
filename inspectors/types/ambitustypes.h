#ifndef AMBITUSTYPES_H
#define AMBITUSTYPES_H

#include "qobjectdefs.h"

class AmbitusTypes
{
    Q_GADGET

public:
    enum class TpcType {
        TPC_INVALID = -2,
        TPC_F_BB,
        TPC_C_BB,
        TPC_G_BB,
        TPC_D_BB,
        TPC_A_BB,
        TPC_E_BB,
        TPC_B_BB,
        TPC_F_B,
        TPC_C_B,
        TPC_G_B,
        TPC_D_B,
        TPC_A_B,
        TPC_E_B,
        TPC_B_B,
        TPC_F,
        TPC_C,
        TPC_G,
        TPC_D,
        TPC_A,
        TPC_E,
        TPC_B,
        TPC_F_S,
        TPC_C_S,
        TPC_G_S,
        TPC_D_S,
        TPC_A_S,
        TPC_E_S,
        TPC_B_S,
        TPC_F_SS,
        TPC_C_SS,
        TPC_G_SS,
        TPC_D_SS,
        TPC_A_SS,
        TPC_E_SS,
        TPC_B_SS
    };

    Q_ENUM(TpcType)
};

#endif // AMBITUSTYPES_H
