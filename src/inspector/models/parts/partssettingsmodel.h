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
#ifndef MU_INSPECTOR_PARTSSETTINGSMODEL_H
#define MU_INSPECTOR_PARTSSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class PartsSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * positionLinkedToMaster READ positionLinkedToMaster CONSTANT)
    Q_PROPERTY(PropertyItem * appearanceLinkedToMaster READ appearanceLinkedToMaster CONSTANT)
    Q_PROPERTY(PropertyItem * textLinkedToMaster READ textLinkedToMaster CONSTANT)
    Q_PROPERTY(PropertyItem * excludeFromOtherParts READ excludeFromOtherParts CONSTANT)

    Q_PROPERTY(bool showPartLinkingOption READ showPartLinkingOption NOTIFY showPartLinkingOptionChanged)
    Q_PROPERTY(bool showExcludeOption READ showExcludeOption NOTIFY showExcludeOptionChanged)
    Q_PROPERTY(bool showTextLinkingOption READ showTextLinkingOption NOTIFY showTextLinkingOptionChanged)

public:
    explicit PartsSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* positionLinkedToMaster() const;
    PropertyItem* appearanceLinkedToMaster() const;
    PropertyItem* excludeFromOtherParts() const;
    PropertyItem* textLinkedToMaster() const;

    bool showPartLinkingOption() const;
    bool showExcludeOption() const;
    bool showTextLinkingOption() const;

signals:
    void showPartLinkingOptionChanged(bool showPartsOption);
    void showExcludeOptionChanged(bool excludeOption);
    void showTextLinkingOptionChanged(bool showTextLink);

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;

    void updateShowPartLinkingOption();
    void updateShowExcludeOption();
    void updateShowTextLinkingOption();

private:
    PropertyItem* m_positionLinkedToMaster;
    PropertyItem* m_appearanceLinkedToMaster;
    PropertyItem* m_textLinkedToMaster;
    PropertyItem* m_excludeFromOtherParts;

    QList<mu::engraving::EngravingItem*> m_elementsForPartLinkingOption;
    QList<mu::engraving::EngravingItem*> m_elementsForExcludeOption;
    QList<mu::engraving::EngravingItem*> m_elementsForTextLinkingOption;

    bool m_showPartLinkingOption = false;
    bool m_showExcludeOption = false;
    bool m_showTextLinkingOption = false;
};
}

#endif // MU_INSPECTOR_PARTSSETTINGSMODEL_H
