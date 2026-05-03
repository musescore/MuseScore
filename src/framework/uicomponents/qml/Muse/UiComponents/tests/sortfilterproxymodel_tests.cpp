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

#include <memory>

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>
#include <QStringListModel>

#include "uicomponents/qml/Muse/UiComponents/filtervalue.h"
#include "uicomponents/qml/Muse/UiComponents/sortervalue.h"
#include "uicomponents/qml/Muse/UiComponents/sortfilterproxymodel.h"

using namespace Qt::StringLiterals;

namespace muse::uicomponents {
class UiComponents_SortFilterProxyModelTests : public ::testing::Test
{
public:
    UiComponents_SortFilterProxyModelTests()
    {
        QStringList modelData;
        modelData << u"hay"_s;
        modelData << u"needle"_s;
        modelData << u"hay needle hay"_s;
        modelData << u"needle"_s;
        modelData << u"hay hay"_s;
        m_model->setStringList(modelData);
    }

    QAbstractListModel* listModel() const
    {
        return m_model.get();
    }

private:
    std::unique_ptr<QStringListModel> m_model = std::make_unique<QStringListModel>();
};

TEST_F(UiComponents_SortFilterProxyModelTests, testFilterValue)
{
    QAbstractListModel* model = this->listModel();
    auto proxyModel = std::make_unique<SortFilterProxyModel>();
    proxyModel->setSourceModel(model);

    FilterValue* filter = new FilterValue(proxyModel.get());
    filter->setCompareType(CompareType::Equal);
    filter->setRoleName(u"display"_s);
    filter->setRoleValue(u"needle"_s);

    QQmlListProperty<Filter> filters = proxyModel->filters();
    ASSERT_TRUE(filters.append);
    filters.append(&filters, filter);

    EXPECT_EQ(proxyModel->rowCount(), 2);
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(0, 0)), model->index(1));
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(1, 0)), model->index(3));

    filter->setCompareType(CompareType::NotEqual);
    EXPECT_EQ(proxyModel->rowCount(), 3);
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(0, 0)), model->index(0));
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(1, 0)), model->index(2));
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(2, 0)), model->index(4));

    filter->setCompareType(CompareType::Contains);
    EXPECT_EQ(proxyModel->rowCount(), 3);
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(0, 0)), model->index(1));
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(1, 0)), model->index(2));
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(2, 0)), model->index(3));

    filter->setEnabled(false);
    EXPECT_EQ(proxyModel->rowCount(), model->rowCount());
}

TEST_F(UiComponents_SortFilterProxyModelTests, testFilterValueMultiple)
{
    QAbstractListModel* model = this->listModel();
    auto proxyModel = std::make_unique<SortFilterProxyModel>();
    proxyModel->setSourceModel(model);

    FilterValue* hayFilter = new FilterValue(proxyModel.get());
    hayFilter->setCompareType(CompareType::Contains);
    hayFilter->setRoleName(u"display"_s);
    hayFilter->setRoleValue(u"hay"_s);

    FilterValue* needleFilter = new FilterValue(proxyModel.get());
    needleFilter->setCompareType(CompareType::Contains);
    needleFilter->setRoleName(u"display"_s);
    needleFilter->setRoleValue(u"needle"_s);

    QQmlListProperty<Filter> filters = proxyModel->filters();
    ASSERT_TRUE(filters.append);
    filters.append(&filters, hayFilter);
    filters.append(&filters, needleFilter);

    EXPECT_EQ(proxyModel->rowCount(), 1);
    EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(0, 0)), model->index(2));
}

TEST_F(UiComponents_SortFilterProxyModelTests, testSorterValue)
{
    QAbstractListModel* model = this->listModel();
    auto proxyModel = std::make_unique<SortFilterProxyModel>();
    proxyModel->setSourceModel(model);

    SorterValue* sorter = new SorterValue(proxyModel.get());
    sorter->setRoleName(u"display"_s);

    QQmlListProperty<Sorter> sorters = proxyModel->sorters();
    ASSERT_TRUE(sorters.append);
    sorters.append(&sorters, sorter);

    EXPECT_EQ(proxyModel->rowCount(), model->rowCount());
    for (int row = 0; row < proxyModel->rowCount(); ++row) {
        SCOPED_TRACE(row);
        EXPECT_EQ(proxyModel->mapToSource(proxyModel->index(row, 0)), model->index(row));
    }

    sorter->setEnabled(true);
    for (int row = 1; row < proxyModel->rowCount(); ++row) {
        SCOPED_TRACE(row);

        const QPartialOrdering ordering = QVariant::compare(proxyModel->data(proxyModel->index(row - 1, 0)),
                                                            proxyModel->data(proxyModel->index(row, 0)));
        EXPECT_TRUE(ordering == QPartialOrdering::Less || ordering == QPartialOrdering::Equivalent);
    }

    sorter->setSortOrder(Qt::DescendingOrder);
    for (int row = 1; row < proxyModel->rowCount(); ++row) {
        SCOPED_TRACE(row);

        const QPartialOrdering ordering = QVariant::compare(proxyModel->data(proxyModel->index(row - 1, 0)),
                                                            proxyModel->data(proxyModel->index(row, 0)));
        EXPECT_TRUE(ordering == QPartialOrdering::Greater || ordering == QPartialOrdering::Equivalent);
    }
}
}
