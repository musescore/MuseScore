#include "dataformatter.h"

#include <QString>

double DataFormatter::formatDouble(const double& val, const int decimals)
{
    return QString::number(val, 'f', decimals).toDouble();
}
