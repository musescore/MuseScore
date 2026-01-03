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
class DynamicsSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(PropertyItem * avoidBarLines READ avoidBarLines CONSTANT)
    Q_PROPERTY(PropertyItem * dynamicScale READ dynamicScale CONSTANT)
    Q_PROPERTY(PropertyItem * centerOnNotehead READ centerOnNotehead CONSTANT)

    // Frame-related settings
    Q_PROPERTY(mu::inspector::PropertyItem * frameType READ frameType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameBorderColor READ frameBorderColor CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameFillColor READ frameFillColor CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameThickness READ frameThickness CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameMargin READ frameMargin CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameCornerRadius READ frameCornerRadius CONSTANT)

public:
    explicit DynamicsSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* avoidBarLines() const;
    PropertyItem* dynamicScale() const;
    PropertyItem* centerOnNotehead() const;

    PropertyItem* frameType() const;
    PropertyItem* frameBorderColor() const;
    PropertyItem* frameFillColor() const;
    PropertyItem* frameThickness() const;
    PropertyItem* frameMargin() const;
    PropertyItem* frameCornerRadius() const;

private:
    void updateFramePropertiesAvailability();

private:
    PropertyItem* m_avoidBarLines = nullptr;
    PropertyItem* m_dynamicScale = nullptr;
    PropertyItem* m_centerOnNotehead = nullptr;

    PropertyItem* m_frameType = nullptr;
    PropertyItem* m_frameBorderColor = nullptr;
    PropertyItem* m_frameFillColor = nullptr;
    PropertyItem* m_frameThickness = nullptr;
    PropertyItem* m_frameMargin = nullptr;
    PropertyItem* m_frameCornerRadius = nullptr;
};
}
