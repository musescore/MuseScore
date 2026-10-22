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
#include "keysignaturesettingsmodel.h"

#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/staff.h"

#include "translation.h"

using namespace mu::propertiespanel;

KeySignatureSettingsModel::KeySignatureSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                     IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setModelType(PropertiesPanelModelType::TYPE_KEYSIGNATURE);
    setTitle(muse::qtrc("propertiespanel", "Key signature"));
    setIcon(muse::ui::IconCode::Code::KEY_SIGNATURE);
    createProperties();
}

void KeySignatureSettingsModel::createProperties()
{
    m_hasToShowCourtesy = buildPropertyItem(mu::engraving::Pid::SHOW_COURTESY);
    m_mode = buildPropertyItem(mu::engraving::Pid::KEYSIG_MODE);
    m_jianpuNumbering = buildPropertyItem(mu::engraving::Pid::JIANPU_NUMBERING);
    m_jianpuTonicKey = buildPropertyItem(mu::engraving::Pid::JIANPU_TONIC_KEY);
    m_jianpuTonicMode = buildPropertyItem(mu::engraving::Pid::JIANPU_TONIC_MODE);
}

void KeySignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::KEYSIG);
}

void KeySignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_hasToShowCourtesy);
    loadPropertyItem(m_mode);
    loadPropertyItem(m_jianpuNumbering);
    loadPropertyItem(m_jianpuTonicKey);
    loadPropertyItem(m_jianpuTonicMode);

    bool enableMode = true;
    bool enableCourtesy = true;

    for (const mu::engraving::EngravingItem* element : m_elementList) {
        if (toKeySig(element)->isCourtesy()) {
            enableMode = false;
        }

        const engraving::Measure* measure = element->findMeasure();
        const engraving::Measure* prevMeasure = measure ? measure->prevMeasure() : nullptr;
        const engraving::LayoutBreak* sectionBreak = prevMeasure ? prevMeasure->sectionBreakElement() : nullptr;
        if (sectionBreak && !sectionBreak->showCourtesy()) {
            enableCourtesy = false;
        }
    }

    m_hasToShowCourtesy->setIsEnabled(enableCourtesy);
    m_mode->setIsEnabled(enableMode);
    m_jianpuNumbering->setIsEnabled(enableMode);

    const bool customJianpuMapping = m_jianpuNumbering->value().toInt() == int(mu::engraving::JianpuTonicMode::CUSTOM);
    m_jianpuTonicKey->setIsEnabled(enableMode && customJianpuMapping);
    m_jianpuTonicMode->setIsEnabled(enableMode && customJianpuMapping);

    updateIsJianpuStaff();
}

PropertyItem* KeySignatureSettingsModel::hasToShowCourtesy() const
{
    return m_hasToShowCourtesy;
}

PropertyItem* KeySignatureSettingsModel::mode() const
{
    return m_mode;
}

PropertyItem* KeySignatureSettingsModel::jianpuNumbering() const
{
    return m_jianpuNumbering;
}

PropertyItem* KeySignatureSettingsModel::jianpuTonicKey() const
{
    return m_jianpuTonicKey;
}

PropertyItem* KeySignatureSettingsModel::jianpuTonicMode() const
{
    return m_jianpuTonicMode;
}

bool KeySignatureSettingsModel::isJianpuStaff() const
{
    return m_isJianpuStaff;
}

void KeySignatureSettingsModel::updateIsJianpuStaff()
{
    bool isJianpuStaff = false;
    for (const mu::engraving::EngravingItem* element : m_elementList) {
        if (element->staff() && element->staff()->isJianpuStaff(element->tick())) {
            isJianpuStaff = true;
            break;
        }
    }

    if (m_isJianpuStaff != isJianpuStaff) {
        m_isJianpuStaff = isJianpuStaff;
        emit isJianpuStaffChanged(m_isJianpuStaff);
    }
}
