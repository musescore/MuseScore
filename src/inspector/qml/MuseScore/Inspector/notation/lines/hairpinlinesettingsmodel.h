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

#include "textlinesettingsmodel.h"

namespace mu::inspector {
class HairpinLineSettingsModel : public TextLineSettingsModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * snapBefore READ snapBefore CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * snapAfter READ snapAfter CONSTANT)

public:
    enum HairpinLineType {
        Unknown,
        Crescendo,
        Diminuendo
    };

    explicit HairpinLineSettingsModel(QObject* parent, IElementRepositoryService* repository, HairpinLineType lineType);

    PropertyItem* snapBefore() const;
    PropertyItem* snapAfter() const;

protected:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;
    void requestElements() override;

private:
    engraving::HairpinType m_hairpinType = engraving::HairpinType::CRESC_LINE;

    PropertyItem* m_snapBefore = nullptr;
    PropertyItem* m_snapAfter = nullptr;
};
}
