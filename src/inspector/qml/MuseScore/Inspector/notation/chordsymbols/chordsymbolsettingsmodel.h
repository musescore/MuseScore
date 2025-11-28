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
class ChordSymbolSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * isLiteral READ isLiteral CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * voicingType READ voicingType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * durationType READ durationType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * verticalAlign READ verticalAlign CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * bassScale READ bassScale CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * doNotStackModifiers READ doNotStackModifiers CONSTANT)

    Q_PROPERTY(bool hasLinkedFretboardDiagram READ hasLinkedFretboardDiagram NOTIFY hasLinkedFretboardDiagramChanged FINAL)
    Q_PROPERTY(bool insideFretBox READ insideFretBox NOTIFY insideFretBoxChanged FINAL)
    Q_PROPERTY(bool showStackModifiers READ showStackModifiers NOTIFY showStackModifiersChanged FINAL)

public:
    explicit ChordSymbolSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* isLiteral() const;
    PropertyItem* voicingType() const;
    PropertyItem* durationType() const;
    PropertyItem* verticalAlign() const;
    PropertyItem* bassScale() const;
    PropertyItem* doNotStackModifiers() const;

    bool hasLinkedFretboardDiagram() const;
    bool insideFretBox() const;
    bool showStackModifiers() const;

    Q_INVOKABLE void addFretboardDiagram();

signals:
    void hasLinkedFretboardDiagramChanged();
    void insideFretBoxChanged();
    void showStackModifiersChanged();

private:
    void setHasLinkedFretboardDiagram(bool has);
    void updateHasLinkedFretboardDiagram();
    void setInsideFretBox(bool has);
    void updateInsideFretBox();
    void setShowStackModifiers(bool val);
    void updateShowStackModifiers();

    void updateIsDurationAvailable();

    PropertyItem* m_isLiteral = nullptr;
    PropertyItem* m_voicingType = nullptr;
    PropertyItem* m_durationType = nullptr;
    bool m_hasLinkedFretboardDiagram = false;
    bool m_insideFretBox = false;
    bool m_showStackModifiers = false;
    PropertyItem* m_verticalAlign = nullptr;
    PropertyItem* m_bassScale = nullptr;
    PropertyItem* m_doNotStackModifiers = nullptr;
};
}
