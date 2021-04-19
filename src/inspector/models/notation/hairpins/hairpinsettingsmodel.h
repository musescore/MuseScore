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
#ifndef MU_INSPECTOR_HAIRPINSETTINGSMODEL_H
#define MU_INSPECTOR_HAIRPINSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class HairpinSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * lineStyle READ lineStyle CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(PropertyItem * dashGapLength READ dashGapLength CONSTANT)
    Q_PROPERTY(PropertyItem * height READ height CONSTANT)
    Q_PROPERTY(PropertyItem * continiousHeight READ continiousHeight CONSTANT)

    Q_PROPERTY(PropertyItem * isDiagonalLocked READ isDiagonalLocked CONSTANT)
    Q_PROPERTY(PropertyItem * isNienteCircleVisible READ isNienteCircleVisible CONSTANT)

    Q_PROPERTY(PropertyItem * beginingText READ beginingText CONSTANT)
    Q_PROPERTY(PropertyItem * beginingTextHorizontalOffset READ beginingTextHorizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * beginingTextVerticalOffset READ beginingTextVerticalOffset CONSTANT)

    Q_PROPERTY(PropertyItem * continiousText READ continiousText CONSTANT)
    Q_PROPERTY(PropertyItem * continiousTextHorizontalOffset READ continiousTextHorizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * continiousTextVerticalOffset READ continiousTextVerticalOffset CONSTANT)

public:
    explicit HairpinSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* lineStyle() const;
    PropertyItem* placement() const;

    PropertyItem* thickness() const;
    PropertyItem* dashLineLength() const;
    PropertyItem* dashGapLength() const;
    PropertyItem* height() const;
    PropertyItem* continiousHeight() const;

    PropertyItem* isDiagonalLocked() const;
    PropertyItem* isNienteCircleVisible() const;

    PropertyItem* beginingText() const;
    PropertyItem* beginingTextHorizontalOffset() const;
    PropertyItem* beginingTextVerticalOffset() const;

    PropertyItem* continiousText() const;
    PropertyItem* continiousTextHorizontalOffset() const;
    PropertyItem* continiousTextVerticalOffset() const;

private:
    void updateLinePropertiesAvailability();

    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_placement = nullptr;

    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_dashLineLength = nullptr;
    PropertyItem* m_dashGapLength = nullptr;
    PropertyItem* m_height = nullptr;
    PropertyItem* m_continiousHeight = nullptr;

    PropertyItem* m_isDiagonalLocked = nullptr;
    PropertyItem* m_isNienteCircleVisible = nullptr;

    PropertyItem* m_beginingText = nullptr;
    PropertyItem* m_beginingTextHorizontalOffset = nullptr;
    PropertyItem* m_beginingTextVerticalOffset = nullptr;

    PropertyItem* m_continiousText = nullptr;
    PropertyItem* m_continiousTextHorizontalOffset = nullptr;
    PropertyItem* m_continiousTextVerticalOffset = nullptr;
};
}

#endif // MU_INSPECTOR_HAIRPINSETTINGSMODEL_H
