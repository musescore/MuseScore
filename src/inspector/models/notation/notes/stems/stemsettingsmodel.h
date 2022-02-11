/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_INSPECTOR_NOTATIONINSPECTORMODEL_H
#define MU_INSPECTOR_NOTATIONINSPECTORMODEL_H

#include "models/abstractinspectormodel.h"

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::inspector {
class StemSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isStemHidden READ isStemHidden CONSTANT)
    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * length READ length CONSTANT)
    Q_PROPERTY(PropertyItem * horizontalOffset READ horizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * verticalOffset READ verticalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * stemDirection READ stemDirection CONSTANT)

    Q_PROPERTY(bool useStraightNoteFlags READ useStraightNoteFlags WRITE setUseStraightNoteFlags NOTIFY useStraightNoteFlagsChanged)

    INJECT(inspector, context::IGlobalContext, context)

public:
    explicit StemSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isStemHidden() const;
    PropertyItem* thickness() const;
    PropertyItem* length() const;

    PropertyItem* horizontalOffset() const;
    PropertyItem* verticalOffset() const;
    PropertyItem* stemDirection() const;

    bool useStraightNoteFlags() const;
    void setUseStraightNoteFlags(bool use);

signals:
    void useStraightNoteFlagsChanged();

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    void onStemDirectionChanged(Ms::DirectionV newDirection);

    PropertyItem* m_isStemHidden = nullptr;
    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_length = nullptr;
    PropertyItem* m_horizontalOffset = nullptr;
    PropertyItem* m_verticalOffset = nullptr;
    PropertyItem* m_stemDirection = nullptr;
};
}

#endif // MU_INSPECTOR_NOTATIONINSPECTORMODEL_H
