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
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_NOTATION_NOTATIONSTYLE_H
#define MU_NOTATION_NOTATIONSTYLE_H

#include "inotationstyle.h"

#include "igetscore.h"

namespace mu {
namespace notation {
class NotationStyle : public INotationStyle
{
public:
    NotationStyle(IGetScore* getScore);

    QVariant styleValue(const StyleId& styleId) const override;
    QVariant defaultStyleValue(const StyleId& styleId) const override;
    void setStyleValue(const StyleId& styleId, const QVariant& newValue) override;

    bool canApplyToAllParts() const override;
    void applyToAllParts() override;

    async::Notification styleChanged() const override;

private:
    IGetScore* m_getScore = nullptr;
    async::Notification m_styleChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONSTYLE_H
