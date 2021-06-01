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

#include "notationstatusbarmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;
using namespace mu::ui;

NotationStatusBarModel::NotationStatusBarModel(QObject* parent)
    : QObject(parent)
{
}

QString NotationStatusBarModel::accessibilityInfo() const
{
    return accessibility() ? QString::fromStdString(accessibility()->accessibilityInfo().val) : QString();
}

QString NotationStatusBarModel::currentWorkspaceName() const
{
    return QString::fromStdString(workspaceConfiguration()->currentWorkspaceName().val);
}

bool NotationStatusBarModel::concertPitchEnabled() const
{
    return style() ? style()->styleValue(StyleId::concertPitch).toBool() : false;
}

QVariant NotationStatusBarModel::currentViewMode() const
{
    int viewMode = notation() ? static_cast<int>(notation()->viewMode()) : -1;

    for (const QVariant& mode : availableViewModeList()) {
        if (mode.toMap()["type"].toInt() == viewMode) {
            return mode;
        }
    }

    return QVariant();
}

QVariantList NotationStatusBarModel::availableViewModeList() const
{
    static const QMap<ViewMode, ActionCode> allModeMap {
        { ViewMode::PAGE, "view-mode-page" },
        { ViewMode::LINE, "view-mode-continuous" },
        { ViewMode::SYSTEM, "view-mode-single" }
    };

    auto correctedTitle = [](ViewMode viewMode, const QString& title) {
        switch (viewMode) {
        case ViewMode::PAGE:
            return title;
        case ViewMode::LINE:
        case ViewMode::SYSTEM:
            return qtrc("notation", "Continuous View");
        default:
            return title;
        }
    };

    QVariantList result;

    for (const ViewMode& viewMode: allModeMap.keys()) {
        ActionCode code = allModeMap[viewMode];
        UiAction action = actionsRegister()->action(code);

        QVariantMap viewModeObj;
        viewModeObj["type"] = static_cast<int>(viewMode);
        viewModeObj["code"] = QString::fromStdString(code);
        viewModeObj["title"] = correctedTitle(viewMode, action.title);
        viewModeObj["icon"] = static_cast<int>(action.iconCode);

        result << viewModeObj;
    }

    return result;
}

int NotationStatusBarModel::currentZoom() const
{
    return notationConfiguration()->currentZoom().val;
}

void NotationStatusBarModel::load()
{
    TRACEFUNC;

    dispatcher()->reg(this, "concert-pitch", [this](const ActionCode& actionCode, const ActionData& args) {
        UNUSED(actionCode);
        bool enabled = args.count() > 0 ? args.arg<bool>(0) : false;
        setConcertPitchEnabled(enabled);
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        emit currentViewModeChanged();

        notation()->notationChanged().onNotify(this, [this]() {
            emit currentViewModeChanged();
        });

        listenChangesInStyle();
        listenChangesInAccessibility();
    });

    workspaceConfiguration()->currentWorkspaceName().ch.onReceive(this, [this](const std::string&) {
        emit currentWorkspaceNameChanged();
    });

    notationConfiguration()->currentZoom().ch.onReceive(this, [this](int) {
        emit currentZoomChanged();
    });

    emit availableViewModeListChanged();
}

void NotationStatusBarModel::listenChangesInAccessibility()
{
    if (!accessibility()) {
        return;
    }

    emit accessibilityInfoChanged();

    accessibility()->accessibilityInfo().ch.onReceive(this, [this](const std::string&) {
        emit accessibilityInfoChanged();
    });
}

void NotationStatusBarModel::listenChangesInStyle()
{
    if (!style()) {
        return;
    }

    emit concertPitchEnabledChanged();

    style()->styleChanged().onNotify(this, [this]() {
        emit concertPitchEnabledChanged();
    });
}

void NotationStatusBarModel::selectWorkspace()
{
    dispatcher()->dispatch("configure-workspaces");
}

void NotationStatusBarModel::toggleConcertPitch()
{
    setConcertPitchEnabled(!concertPitchEnabled());
}

void NotationStatusBarModel::setCurrentViewMode(const QString& modeCode)
{
    dispatcher()->dispatch(codeFromQString(modeCode));
}

void NotationStatusBarModel::zoomIn()
{
    dispatcher()->dispatch("zoomin");
}

void NotationStatusBarModel::zoomOut()
{
    dispatcher()->dispatch("zoomout");
}

INotationPtr NotationStatusBarModel::notation() const
{
    return context()->currentNotation();
}

INotationAccessibilityPtr NotationStatusBarModel::accessibility() const
{
    return notation() ? notation()->accessibility() : nullptr;
}

INotationStylePtr NotationStatusBarModel::style() const
{
    return notation() ? notation()->style() : nullptr;
}

void NotationStatusBarModel::setConcertPitchEnabled(bool enabled)
{
    if (!style() || concertPitchEnabled() == enabled) {
        return;
    }

    notation()->undoStack()->prepareChanges();
    style()->setStyleValue(StyleId::concertPitch, enabled);
    notation()->undoStack()->commitChanges();
}
