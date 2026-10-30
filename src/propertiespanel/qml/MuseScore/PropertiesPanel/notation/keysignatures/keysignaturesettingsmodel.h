/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

namespace mu::propertiespanel {
class KeySignatureSettingsModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * hasToShowCourtesy READ hasToShowCourtesy CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * mode READ mode CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * jianpuNumbering READ jianpuNumbering CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * jianpuTonicKey READ jianpuTonicKey CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * jianpuTonicMode READ jianpuTonicMode CONSTANT)
    Q_PROPERTY(bool isJianpuStaff READ isJianpuStaff NOTIFY isJianpuStaffChanged)
public:
    explicit KeySignatureSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;

    PropertyItem* hasToShowCourtesy() const;
    PropertyItem* mode() const;
    PropertyItem* jianpuNumbering() const;
    PropertyItem* jianpuTonicKey() const;
    PropertyItem* jianpuTonicMode() const;
    bool isJianpuStaff() const;

signals:
    void isJianpuStaffChanged(bool isJianpuStaff);

private:
    void updateIsJianpuStaff();

    PropertyItem* m_hasToShowCourtesy = nullptr;
    PropertyItem* m_mode = nullptr;
    PropertyItem* m_jianpuNumbering = nullptr;
    PropertyItem* m_jianpuTonicKey = nullptr;
    PropertyItem* m_jianpuTonicMode = nullptr;
    bool m_isJianpuStaff = false;
};
}
