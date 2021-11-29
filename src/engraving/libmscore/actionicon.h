/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_LIBMSCORE_ACTIONICON_H
#define MU_LIBMSCORE_ACTIONICON_H

#include "engravingitem.h"

namespace Ms {
enum class ActionIconType {
    UNDEFINED = -1,

    ACCIACCATURA,
    APPOGGIATURA,
    GRACE4,
    GRACE16,
    GRACE32,
    GRACE8_AFTER,
    GRACE16_AFTER,
    GRACE32_AFTER,

    BEAM_START,
    BEAM_MID,
    BEAM_NONE,
    BEAM_BEGIN_32,
    BEAM_BEGIN_64,
    BEAM_AUTO,

    BEAM_FEATHERED_SLOWER,
    BEAM_FEATHERED_FASTER,

    VFRAME,
    HFRAME,
    TFRAME,
    FFRAME,
    MEASURE,

    PARENTHESES,
    BRACKETS,
};

//! Dummy element, used for drag&drop
class ActionIcon final : public EngravingItem
{
public:
    ActionIcon(EngravingItem* score);
    ~ActionIcon() override = default;

    ActionIcon* clone() const override;

    ActionIconType actionType() const;
    const std::string& actionCode() const;

    void setActionType(ActionIconType val);
    void setAction(const std::string& actionCode, char16_t icon);

    qreal extent() const;
    void setExtent(qreal extent);

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;
    void draw(mu::draw::Painter*) const override;
    void layout() override;

    mu::engraving::PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const mu::engraving::PropertyValue&) override;

private:
    mu::RectF boundingBox() const;

    ActionIconType m_actionType { ActionIconType::UNDEFINED };
    std::string m_actionCode;
    char16_t m_icon = 0;
    qreal m_extent { 40 };
    mu::draw::Font m_iconFont;
};
}

#endif // MU_LIBMSCORE_ACTIONICON_H
