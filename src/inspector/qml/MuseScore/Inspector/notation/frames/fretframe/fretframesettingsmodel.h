/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
class FretFrameSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * textScale READ textScale CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * diagramScale READ diagramScale CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * columnGap READ columnGap CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * rowGap READ rowGap CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * chordsPerRow READ chordsPerRow CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * horizontalAlignment READ horizontalAlignment CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * gapAbove READ gapAbove CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * gapBelow READ gapBelow CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameLeftMargin READ frameLeftMargin CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameRightMargin READ frameRightMargin CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameTopMargin READ frameTopMargin CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * frameBottomMargin READ frameBottomMargin CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * isSizeSpatiumDependent READ isSizeSpatiumDependent CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * paddingToNotationAbove READ paddingToNotationAbove CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * paddingToNotationBelow READ paddingToNotationBelow CONSTANT)

public:
    explicit FretFrameSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* textScale() const;
    PropertyItem* diagramScale() const;
    PropertyItem* columnGap() const;
    PropertyItem* rowGap() const;
    PropertyItem* chordsPerRow() const;
    PropertyItem* horizontalAlignment() const;

    PropertyItem* gapAbove() const;
    PropertyItem* gapBelow() const;
    PropertyItem* frameLeftMargin() const;
    PropertyItem* frameRightMargin() const;
    PropertyItem* frameTopMargin() const;
    PropertyItem* frameBottomMargin() const;
    PropertyItem* isSizeSpatiumDependent() const;
    PropertyItem* paddingToNotationAbove() const;
    PropertyItem* paddingToNotationBelow() const;

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_textScale = nullptr;
    PropertyItem* m_diagramScale = nullptr;
    PropertyItem* m_columnGap = nullptr;
    PropertyItem* m_rowGap = nullptr;
    PropertyItem* m_chordsPerRow = nullptr;
    PropertyItem* m_horizontalAlignment = nullptr;

    PropertyItem* m_gapAbove = nullptr;
    PropertyItem* m_gapBelow = nullptr;
    PropertyItem* m_frameLeftMargin = nullptr;
    PropertyItem* m_frameRightMargin = nullptr;
    PropertyItem* m_frameTopMargin = nullptr;
    PropertyItem* m_frameBottomMargin = nullptr;
    PropertyItem* m_isSizeSpatiumDependent = nullptr;
    PropertyItem* m_paddingToNotationAbove = nullptr;
    PropertyItem* m_paddingToNotationBelow = nullptr;
};
}
