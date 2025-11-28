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
class SectionBreakSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * shouldStartWithLongInstrNames READ shouldStartWithLongInstrNames CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * shouldResetBarNums READ shouldResetBarNums CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * pauseDuration READ pauseDuration CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * firstSystemIndent READ firstSystemIndent CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * showCourtesySignatures READ showCourtesySignatures CONSTANT)

public:
    explicit SectionBreakSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* shouldStartWithLongInstrNames() const;
    PropertyItem* shouldResetBarNums() const;
    PropertyItem* pauseDuration() const;
    PropertyItem* firstSystemIndent() const;
    PropertyItem* showCourtesySignatures() const;

private:
    PropertyItem* m_shouldStartWithLongInstrNames = nullptr;
    PropertyItem* m_shouldResetBarNums = nullptr;
    PropertyItem* m_pauseDuration = nullptr;
    PropertyItem* m_firstSystemIndent = nullptr;
    PropertyItem* m_showCourtesySignatures = nullptr;
};
}
