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
#ifndef MU_INSPECTOR_NOTEHEADSETTINGSMODEL_H
#define MU_INSPECTOR_NOTEHEADSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class NoteheadSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isHeadHidden READ isHeadHidden CONSTANT)
    Q_PROPERTY(PropertyItem * isHeadSmall READ isHeadSmall CONSTANT)
    Q_PROPERTY(PropertyItem * hasHeadParentheses READ hasHeadParentheses CONSTANT)
    Q_PROPERTY(PropertyItem * headDirection READ headDirection CONSTANT)
    Q_PROPERTY(PropertyItem * headGroup READ headGroup CONSTANT)
    Q_PROPERTY(PropertyItem * headType READ headType CONSTANT)
    Q_PROPERTY(PropertyItem * headSystem READ headSystem CONSTANT)
    Q_PROPERTY(PropertyItem * dotPosition READ dotPosition CONSTANT)
    Q_PROPERTY(PropertyItem * offset READ offset CONSTANT)
    Q_PROPERTY(bool isTrillCueNote READ isTrillCueNote NOTIFY isTrillCueNoteChanged)

public:
    explicit NoteheadSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isHeadHidden() const;
    PropertyItem* isHeadSmall() const;
    PropertyItem* hasHeadParentheses() const;
    PropertyItem* headDirection() const;
    PropertyItem* headGroup() const;
    PropertyItem* headType() const;
    PropertyItem* headSystem() const;
    PropertyItem* dotPosition() const;
    PropertyItem* offset() const;
    bool isTrillCueNote() const;

    Q_INVOKABLE QVariantList possibleHeadSystemTypes() const;

public slots:
    void setIsTrillCueNote(bool v);

signals:
    void isTrillCueNoteChanged(bool isTrillCueNote);

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);
    void updateIsTrillCueNote();

    PropertyItem* m_isHeadHidden = nullptr;
    PropertyItem* m_isHeadSmall = nullptr;
    PropertyItem* m_hasHeadParentheses = nullptr;
    PropertyItem* m_headDirection = nullptr;
    PropertyItem* m_headGroup = nullptr;
    PropertyItem* m_headType = nullptr;
    PropertyItem* m_headSystem = nullptr;
    PropertyItem* m_dotPosition = nullptr;
    PointFPropertyItem* m_offset = nullptr;
    bool m_isTrillCueNote = false;
};
}

#endif // MU_INSPECTOR_NOTEHEADSETTINGSMODEL_H
