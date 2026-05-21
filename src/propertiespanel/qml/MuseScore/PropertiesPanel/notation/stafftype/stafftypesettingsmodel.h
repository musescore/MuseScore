/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "propertiespanelabstractmodel.h"

namespace mu::propertiespanel {
class StaffTypeSettingsModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * isSmall READ isSmall CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * verticalOffset READ verticalOffset CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * scale READ scale CONSTANT)

    Q_PROPERTY(mu::propertiespanel::PropertyItem * lineCount READ lineCount CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * lineDistance READ lineDistance CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * stepOffset READ stepOffset CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * isInvisible READ isInvisible CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * color READ color CONSTANT)

    Q_PROPERTY(mu::propertiespanel::PropertyItem * noteheadSchemeType READ noteheadSchemeType CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * isStemless READ isStemless CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * shouldShowBarlines READ shouldShowBarlines CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * shouldShowLedgerLines READ shouldShowLedgerLines CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * shouldGenerateClefs READ shouldGenerateClefs CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * shouldGenerateTimeSignatures READ shouldGenerateTimeSignatures CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * shouldGenerateKeySignatures READ shouldGenerateKeySignatures CONSTANT)

    Q_PROPERTY(mu::propertiespanel::PropertyItem * staffLongName READ staffLongName CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * staffShortName READ staffShortName CONSTANT)

public:
    explicit StaffTypeSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

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

    PropertyItem* staffLongName() const;
    PropertyItem* staffShortName() const;

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

    PropertyItem* m_staffLongName = nullptr;
    PropertyItem* m_staffShortName = nullptr;
};
}
