#ifndef DATAFORMATTER_H
#define DATAFORMATTER_H

class DataFormatter
{
public:
    DataFormatter() = default;

    static double formatDouble(const double& val, const int decimals = 2);
};

#endif // DATAFORMATTER_H
