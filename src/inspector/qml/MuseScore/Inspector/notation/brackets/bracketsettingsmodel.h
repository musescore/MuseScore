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
class BracketSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * bracketColumnPosition READ bracketColumnPosition CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * bracketSpanStaves READ bracketSpanStaves CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * longName READ longName CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * shortName READ shortName CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * showText READ showText CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * showBracket READ showBracket CONSTANT)

    Q_PROPERTY(bool areSettingsAvailable READ areSettingsAvailable NOTIFY selectionChanged)
    Q_PROPERTY(int maxBracketColumnPosition READ maxBracketColumnPosition NOTIFY maxBracketColumnPositionChanged)
    Q_PROPERTY(int maxBracketSpanStaves READ maxBracketSpanStaves NOTIFY selectionChanged)
    Q_PROPERTY(bool isGroupBracket READ isGroupBracket NOTIFY isGroupBracketChanged)

public:

    explicit BracketSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* bracketColumnPosition() const;
    PropertyItem* bracketSpanStaves() const;

    bool isGroupBracket() const;
    PropertyItem* longName() const;
    PropertyItem* shortName() const;
    PropertyItem* showText() const;
    PropertyItem* showBracket() const;

    bool areSettingsAvailable() const;
    int maxBracketColumnPosition() const;
    int maxBracketSpanStaves() const;

signals:
    void selectionChanged();
    void maxBracketColumnPositionChanged();
    void isGroupBracketChanged(bool isGroupBracket);

private:
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);
    void updateIsGroupBracket();

    PropertyItem* m_bracketColumnPosition = nullptr;
    PropertyItem* m_bracketSpanStaves = nullptr;

    bool m_isGroupBracket = false;
    PropertyItem* m_longName = nullptr;
    PropertyItem* m_shortName = nullptr;
    PropertyItem* m_showText = nullptr;
    PropertyItem* m_showBracket = nullptr;
};
}
