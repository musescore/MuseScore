/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#pragma once

#include <qqmlintegration.h>

#include "abstractinspectormodel.h"

namespace mu::inspector {
class ChordBracketSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(PropertyItem * bracketRightSide READ bracketRightSide CONSTANT)
    Q_PROPERTY(PropertyItem * hookPos READ hookPos CONSTANT)
    Q_PROPERTY(PropertyItem * hookLen READ hookLen CONSTANT)

    Q_PROPERTY(bool isBracket READ isBracket NOTIFY isBracketChanged)

public:
    explicit ChordBracketSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    PropertyItem* bracketRightSide() const { return m_bracketRightSide; }
    PropertyItem* hookPos() const { return m_hookPos; }
    PropertyItem* hookLen() const { return m_hookLen; }

    bool isBracket() const { return m_isBracket; }

signals:
    void isBracketChanged(bool isBracket);

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* m_bracketRightSide = nullptr;
    PropertyItem* m_hookPos = nullptr;
    PropertyItem* m_hookLen = nullptr;

    void updateIsBracket();
    bool m_isBracket;
};
}
