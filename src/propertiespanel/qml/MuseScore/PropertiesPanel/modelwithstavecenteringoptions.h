/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

using namespace mu::engraving;

namespace mu::propertiespanel {
//! Base for models whose items can be centered in the gap between two staves.
class ModelWithStaveCenteringOptions : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * centerBetweenStaves READ centerBetweenStaves CONSTANT)
    Q_PROPERTY(
        bool isStaveCenteringApplicable READ isStaveCenteringApplicable WRITE setIsStaveCenteringApplicable NOTIFY
        isStaveCenteringApplicableChanged)
    Q_PROPERTY(
        bool isStaveCenteringAvailable READ isStaveCenteringAvailable WRITE setIsStaveCenteringAvailable NOTIFY
        isStaveCenteringAvailableChanged)

public:
    explicit ModelWithStaveCenteringOptions(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                            IElementRepositoryService* repository, ElementType elementType = ElementType::INVALID);

    void createProperties() override;
    void loadProperties() override;
    void onNotationChanged(const PropertyIdSet&, const StyleIdSet&) override;

    PropertyItem* centerBetweenStaves() const;
    bool isStaveCenteringApplicable() const;
    bool isStaveCenteringAvailable() const;

public slots:
    void setIsStaveCenteringApplicable(bool v);
    void setIsStaveCenteringAvailable(bool v);

signals:
    void isStaveCenteringApplicableChanged(bool isStaveCenteringApplicable);
    void isStaveCenteringAvailableChanged(bool isStaveCenteringAvailable);

protected:
    /** Whether the staff on the given side is one the item could actually be centered against.
     * Items whose placement is definite only care about the side they are placed on; subclasses
     * whose items can be placed automatically may count both sides. */
    virtual bool centeringSideIsRelevant(const EngravingItem* item, bool above) const;

    void updateStaveCenteringFlags();

private:
    PropertyItem* m_centerBetweenStaves = nullptr;
    bool m_isStaveCenteringApplicable = false;
    bool m_isStaveCenteringAvailable = false;
};
}
