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
#ifndef MU_INSPECTOR_OTTAVASETTINGSMODEL_H
#define MU_INSPECTOR_OTTAVASETTINGSMODEL_H

#include "textlinesettingsmodel.h"

namespace mu::inspector {
class OttavaSettingsModel : public TextLineSettingsModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * ottavaType READ ottavaType CONSTANT)
    Q_PROPERTY(PropertyItem * showNumbersOnly READ showNumbersOnly CONSTANT)

public:
    explicit OttavaSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* ottavaType() const;
    PropertyItem* showNumbersOnly() const;

    Q_INVOKABLE QVariantList possibleOttavaTypes() const;

private:
    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* m_ottavaType = nullptr;
    PropertyItem* m_showNumbersOnly = nullptr;
};
}

#endif // MU_INSPECTOR_OTTAVASETTINGSMODEL_H
