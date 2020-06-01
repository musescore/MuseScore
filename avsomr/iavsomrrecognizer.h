//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef AVS_IAVSOMRRECOGNIZER_H
#define AVS_IAVSOMRRECOGNIZER_H

#include <functional>
#include <QString>
#include <QByteArray>

#include "ret.h"

namespace Ms {
namespace Avs {
class IAvsOmrRecognizer
{
public:
    virtual ~IAvsOmrRecognizer() {}

    struct Step {
        enum Type {
            Undefined = 0,
            PrepareStart,
            PrepareFinish,
            ProcessingStart,
            ProcessingFinish,
            LoadStart,
            LoadFinish,
        };

        Type type{ Undefined };
        uint16_t percent{ 0 };      //! NOTE Estimated percent of completion
        uint16_t percentMax{ 0 };   //! NOTE Max percent of current step
        Ret error{ Ret::Ok };

        Step() {}
        Step(Type t, uint16_t perc, uint16_t percMax, Ret err)
            : type(t), percent(perc), percentMax(percMax), error(err) {}

        bool success() const { return error.success(); }
    };

    using OnStep = std::function<void (const Step& step)>;

    virtual QString type() const = 0;
    virtual bool isAvailable() const = 0;
    virtual bool recognize(const QString& filePath, QByteArray* avsFileData, const OnStep& onStep) = 0;
};
} // Avs
} // Ms

#endif // AVS_IAVSOMRRECOGNIZER_H
