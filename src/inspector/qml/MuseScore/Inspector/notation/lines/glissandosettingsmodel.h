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
class GlissandoSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * lineType READ lineType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * showText READ showText CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * text READ text CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * thickness READ thickness CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * lineStyle READ lineStyle CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * dashGapLength READ dashGapLength CONSTANT)

public:
    explicit GlissandoSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* lineType() const;
    PropertyItem* showText() const;
    PropertyItem* text() const;

    PropertyItem* thickness() const;

    PropertyItem* lineStyle() const;
    PropertyItem* dashLineLength() const;
    PropertyItem* dashGapLength() const;

    Q_INVOKABLE QVariantList possibleLineTypes() const;

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;
    void onUpdateGlissPropertiesAvailability();

    PropertyItem* m_lineType = nullptr;
    PropertyItem* m_showText = nullptr;
    PropertyItem* m_text = nullptr;

    PropertyItem* m_thickness = nullptr;

    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_dashLineLength = nullptr;
    PropertyItem* m_dashGapLength = nullptr;
};
}
