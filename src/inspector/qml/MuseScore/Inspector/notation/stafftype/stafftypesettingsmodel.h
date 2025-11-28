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
class StaffTypeSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * isSmall READ isSmall CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * verticalOffset READ verticalOffset CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * scale READ scale CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * lineCount READ lineCount CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * lineDistance READ lineDistance CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * stepOffset READ stepOffset CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * isInvisible READ isInvisible CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * color READ color CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * noteheadSchemeType READ noteheadSchemeType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * isStemless READ isStemless CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * shouldShowBarlines READ shouldShowBarlines CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * shouldShowLedgerLines READ shouldShowLedgerLines CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * shouldGenerateClefs READ shouldGenerateClefs CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * shouldGenerateTimeSignatures READ shouldGenerateTimeSignatures CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * shouldGenerateKeySignatures READ shouldGenerateKeySignatures CONSTANT)
public:
    explicit StaffTypeSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* isSmall() const;
    PropertyItem* verticalOffset() const;
    PropertyItem* scale() const;

    PropertyItem* lineCount() const;
    PropertyItem* lineDistance() const;
    PropertyItem* stepOffset() const;
    PropertyItem* isInvisible() const;
    PropertyItem* color() const;

    PropertyItem* noteheadSchemeType() const;
    PropertyItem* isStemless() const;
    PropertyItem* shouldShowBarlines() const;
    PropertyItem* shouldShowLedgerLines() const;
    PropertyItem* shouldGenerateClefs() const;
    PropertyItem* shouldGenerateTimeSignatures() const;
    PropertyItem* shouldGenerateKeySignatures() const;

private:
    PropertyItem* m_isSmall = nullptr;
    PropertyItem* m_verticalOffset = nullptr;
    PropertyItem* m_scale = nullptr;

    PropertyItem* m_lineCount = nullptr;
    PropertyItem* m_lineDistance = nullptr;
    PropertyItem* m_stepOffset = nullptr;
    PropertyItem* m_isInvisible = nullptr;
    PropertyItem* m_color = nullptr;

    PropertyItem* m_noteheadSchemeType = nullptr;
    PropertyItem* m_isStemless = nullptr;
    PropertyItem* m_shouldShowBarlines = nullptr;
    PropertyItem* m_shouldShowLedgerLines = nullptr;
    PropertyItem* m_shouldGenerateClefs = nullptr;
    PropertyItem* m_shouldGenerateTimeSignatures = nullptr;
    PropertyItem* m_shouldGenerateKeySignatures = nullptr;
};
}
