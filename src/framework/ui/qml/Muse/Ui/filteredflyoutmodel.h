/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include <QObject>
#include <qqmlintegration.h>

namespace muse::ui {
class FilteredFlyoutModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariant rawModel READ rawModel WRITE setRawModel NOTIFY modelChanged)
    Q_PROPERTY(QVariant filteredModel READ filteredModel NOTIFY modelChanged)

public:
    explicit FilteredFlyoutModel(QObject* parent = nullptr);

    QVariant rawModel() const;
    void setRawModel(const QVariant& model);

    QVariant filteredModel() const;

    Q_INVOKABLE void setFilterText(const QString& filterText);

signals:
    void modelChanged();

private:
    QString m_filterText;

    QVariant m_rawModel;

    //! NOTE: We use separate models here so that we don't need to re-build filtered lists every time the
    //! filter text changes. The flattened model is built every time setModel is called, and the filtered
    //! model is set every time the text changes...
    QVariant m_flattenedModel;
    QVariant m_filteredModel;

    QVariant m_alwaysAppend;
};
}
