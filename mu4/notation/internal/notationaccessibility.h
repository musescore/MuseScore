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

#ifndef MU_NOTATION_NOTATIONACCESSIBILITY_H
#define MU_NOTATION_NOTATIONACCESSIBILITY_H

#include "inotationaccessibility.h"
#include "notationtypes.h"

#include "async/asyncable.h"
#include "async/notification.h"

namespace Ms {
class Score;
class Selection;
}

namespace mu {
namespace notation {
class IGetScore;
class NotationAccessibility : public INotationAccessibility, public async::Asyncable
{
public:
    NotationAccessibility(const IGetScore* getScore, async::Notification selectionChangedNotification);

    ValCh<std::string> accessibilityInfo() const override;

private:
    const Ms::Score* score() const;
    const Ms::Selection* selection() const;

    void updateAccessibilityInfo();
    void setAccessibilityInfo(const QString& info);

    QString rangeAccessibilityInfo() const;
    QString singleElementAccessibilityInfo() const;

    std::pair<int, float> barbeat(const Element* element) const;
    QString formatSingleElementBarsAndBeats(const Element* element) const;
    QString formatStartBarsAndBeats(const Element* element) const;
    QString formatEndBarsAndBeats(const Element* element) const;

    const IGetScore* m_getScore = nullptr;
    ValCh<std::string> m_accessibilityInfo;
};
}
}

#endif // MU_NOTATION_NOTATIONACCESSIBILITY_H
