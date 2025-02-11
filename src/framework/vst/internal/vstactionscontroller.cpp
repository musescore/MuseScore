/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "vstactionscontroller.h"

#include "../view/vstviewdialog_qwidget.h"

#include "log.h"

using namespace muse;
using namespace muse::vst;

static const char16_t* VST_EDITOR_URI = u"muse://vst/editor?instanceId=%1&sync=false&modal=false&floating=true";

void VstActionsController::init()
{
    dispatcher()->reg(this, "vst-use-oldview", [this]() { useView(false); });
    dispatcher()->reg(this, "vst-use-newview", [this]() { useView(true); });

    dispatcher()->reg(this, actions::ActionQuery("action://vst/fx_editor"), this, &VstActionsController::fxEditor);
    dispatcher()->reg(this, actions::ActionQuery("action://vst/instrument_editor"), this, &VstActionsController::instEditor);
}

void VstActionsController::fxEditor(const actions::ActionQuery& actionQuery)
{
    LOGD() << actionQuery.toString();

#ifdef Q_OS_LINUX
    if (!isUsedNewView()) {
        LOGW() << "Old (QWidget) VST View not support Linux";
        return;
    }
#endif

    std::string resourceId = actionQuery.param("resourceId").toString();
    IF_ASSERT_FAILED(!resourceId.empty()) {
        LOGE() << "not set resourceId";
        return;
    }

    int trackId = actionQuery.param("trackId", Val(-1)).toInt();
    int chainOrder = actionQuery.param("chainOrder", Val(0)).toInt();
    std::string operation = actionQuery.param("operation", Val("open")).toString();

    auto instance = instancesRegister()->fxPlugin(resourceId, trackId, chainOrder);

    if (operation == "close" && !instance) {
        return;
    }

    IF_ASSERT_FAILED(instance) {
        LOGE() << "not found instance, resourceId: " << resourceId
               << ", trackId: " << trackId << ", chainOrder: " << chainOrder;
        return;
    }

    editorOperation(operation, instance->id());
}

void VstActionsController::instEditor(const actions::ActionQuery& actionQuery)
{
    LOGD() << actionQuery.toString();

#ifdef Q_OS_LINUX
    if (!isUsedNewView()) {
        LOGW() << "Old (QWidget) VST View not support Linux";
        return;
    }
#endif

    std::string resourceId = actionQuery.param("resourceId").toString();
    IF_ASSERT_FAILED(!resourceId.empty()) {
        LOGE() << "not set resourceId";
        return;
    }

    int trackId = actionQuery.param("trackId", Val(-1)).toInt();

    auto instance = instancesRegister()->instrumentPlugin(resourceId, trackId);

    std::string operation = actionQuery.param("operation", Val("open")).toString();

    if (operation == "close" && !instance) {
        return;
    }

    IF_ASSERT_FAILED(instance) {
        LOGE() << "not found instance, resourceId: " << resourceId
               << ", trackId: " << trackId;
        return;
    }

    editorOperation(operation, instance->id());
}

void VstActionsController::editorOperation(const std::string& operation, int instanceId)
{
    UriQuery editorUri = UriQuery(String(VST_EDITOR_URI).arg(instanceId));

    if (operation == "close") {
        interactive()->close(editorUri);
    } else {
        if (interactive()->isOpened(editorUri).val) {
            interactive()->raise(editorUri);
        } else {
            interactive()->open(editorUri);
        }
    }
}

void VstActionsController::setupUsedView()
{
    useView(configuration()->usedVstView() == "newview");
}

void VstActionsController::useView(bool isNew)
{
    interactiveUriRegister()->unregisterUri(Uri("muse://vst/editor"));

    if (isNew) {
        configuration()->setUsedVstView("newview");
        interactiveUriRegister()->registerQmlUri(Uri("muse://vst/editor"), "Muse/Vst/VstEditorDialog.qml");
    } else {
        configuration()->setUsedVstView("oldview");
        interactiveUriRegister()->registerWidgetUri<VstViewDialog>(Uri("muse://vst/editor"));
    }

    m_actionCheckedChanged.send({ "vst-use-oldview", "vst-use-newview" });
}

bool VstActionsController::isUsedNewView() const
{
    return configuration()->usedVstView() == "newview";
}

bool VstActionsController::actionChecked(const actions::ActionCode& act) const
{
    if (act == "vst-use-oldview") {
        return !isUsedNewView();
    } else if (act == "vst-use-newview") {
        return isUsedNewView();
    }

    return false;
}

async::Channel<actions::ActionCodeList> VstActionsController::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
