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
#ifndef MU_DOMAIN_NOTATIONSTYLEEDITOR_H
#define MU_DOMAIN_NOTATIONSTYLEEDITOR_H

#include "inotationstyleeditor.h"

namespace mu {
namespace domain {
namespace notation {
class Notation;
class NotationStyleEditor : public INotationStyleEditor
{
public:
    NotationStyleEditor(Notation* notation);

    Style style() const override;
    void changeStyle(ChangeStyleVal* newStyleValue) override;

    void update() override;

    bool isMaster() const override;
    QList<QMap<QString, QString>> metaTags() const override;
    QString textStyleUserName(Tid tid) override;
    void setConcertPitch(bool status) override;

    void startEdit() override;
    void apply() override;
    void applyAllParts() override;
    void cancel() override;

    async::Notification styleChanged() const override;

private:
    Notation* m_notation = nullptr;

    async::Notification m_styleChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATIONSTYLEEDITOR_H
