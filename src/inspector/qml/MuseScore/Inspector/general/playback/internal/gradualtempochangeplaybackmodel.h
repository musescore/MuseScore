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
class GradualTempoChangePlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * tempoChangeFactor READ tempoChangeFactor CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * tempoEasingMethod READ tempoEasingMethod CONSTANT)

public:
    explicit GradualTempoChangePlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* tempoChangeFactor() const;
    PropertyItem* tempoEasingMethod() const;

    Q_INVOKABLE QVariantList possibleEasingMethods() const;

protected:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_tempoChangeFactor = nullptr;
    PropertyItem* m_tempoEasingMethod = nullptr;
};
}
