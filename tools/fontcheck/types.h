#ifndef TYPES_H
#define TYPES_H

#include <vector>

enum FontCode {
    Bravura,
    BravuraText,
    Leland,
    LelandText
};

static const std::vector<FontCode> fonts = { Bravura, BravuraText, Leland, LelandText };

#endif // TYPES_H
