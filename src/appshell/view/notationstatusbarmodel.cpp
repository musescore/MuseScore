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

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;
using namespace mu::ui;

static const QString TITLE_KEY("title");
static const QString ICON_KEY("icon");
static const QString SELECTABLE_KEY("selectable");
static const QString SELECTED_KEY("selected");
static const QString TYPE_KEY("type");
static const QString CODE_KEY("code");
static const QString VALUE_KEY("value");

static const ActionCode TOGGLE_CONCERT_PITCH_CODE("concert-pitch");
static const ActionCode SELECT_WORKSPACE_CODE("configure-workspaces");

static constexpr int MIN_DISPLAYED_ZOOM_PERCENTAGE = 25;

static ActionCode zoomTypeToActionCode(ZoomType type)
{
    switch (type) {
    case ZoomType::Percentage: return "zoom-x-percent";
    case ZoomType::PageWidth: return "zoom-page-width";
    case ZoomType::WholePage: return "zoom-whole-page";
    case ZoomType::TwoPages: return "zoom-two-pages";
    }

    return "";
}

NotationStatusBarModel::NotationStatusBarModel(QObject* parent)
    : QObject(parent)
{
}

QString NotationStatusBarModel::accessibilityInfo() const
{
    return accessibility() ? QString::fromStdString(accessibility()->accessibilityInfo().val) : QString();
}

QVariant NotationStatusBarModel::concertPitchAction() const
{
    return menuItem(TOGGLE_CONCERT_PITCH_CODE).toMap();
}

QVariant NotationStatusBarModel::currentWorkspaceAction() const
{
    MenuItem item = menuItem(SELECT_WORKSPACE_CODE);
    item.title = qtrc("appshell", "Workspace: ") + QString::fromStdString(workspaceConfiguration()->currentWorkspaceName());
    return item.toMap();
}

MenuItem NotationStatusBarModel::menuItem(const actions::ActionCode& actionCode) const
{
    MenuItem item = actionsRegister()->action(actionCode);
    item.state = actionsRegister()->actionState(actionCode);

    return item;
}

QVariant NotationStatusBarModel::currentViewMode() const
{
    int viewMode = notation() ? static_cast<int>(notation()->viewMode()) : -1;

    for (const QVariant& mode : availableViewModeList()) {
        if (mode.toMap()[TYPE_KEY].toInt() == viewMode) {
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
            return qtrc("appshell", "Continuous View");
        default:
            return title;
        }
    };

    QVariantList result;

    for (const ViewMode& viewMode: allModeMap.keys()) {
        ActionCode code = allModeMap[viewMode];
        UiAction action = actionsRegister()->action(code);

        QVariantMap viewModeObj;
        viewModeObj[TYPE_KEY] = static_cast<int>(viewMode);
        viewModeObj[CODE_KEY] = QString::fromStdString(code);
        viewModeObj[TITLE_KEY] = correctedTitle(viewMode, action.title);
        viewModeObj[ICON_KEY] = static_cast<int>(action.iconCode);

        result << viewModeObj;
    }

    return result;
}

bool NotationStatusBarModel::zoomEnabled() const
{
    return notation() != nullptr;
}

int NotationStatusBarModel::currentZoomPercentage() const
{
    return notationConfiguration()->currentZoom().val;
}

void NotationStatusBarModel::setCurrentZoomPercentage(int zoomPercentage)
{
    if (zoomPercentage == currentZoomPercentage()) {
        return;
    }

    m_currentZoomType = ZoomType::Percentage;
    dispatch(zoomTypeToActionCode(m_currentZoomType), ActionData::make_arg1<int>(zoomPercentage));
}

void NotationStatusBarModel::load()
{
    TRACEFUNC;

    m_currentZoomType = notationConfiguration()->defaultZoomType();

    context()->currentNotationChanged().onNotify(this, [this]() {
        if (!notation()) {
            return;
        }

        emit currentViewModeChanged();
        emit zoomEnabledChanged();

        notation()->notationChanged().onNotify(this, [this]() {
            emit currentViewModeChanged();
        });

        listenChangesInAccessibility();
    });

    workspaceConfiguration()->currentWorkspaceNameChanged().onReceive(this, [this](const std::string&) {
        emit currentWorkspaceActionChanged();
    });

    notationConfiguration()->currentZoom().ch.onReceive(this, [this](int) {
        emit currentZoomPercentageChanged();
        emit availableZoomListChanged();
    });

    actionsRegister()->actionStateChanged().onReceive(this, [this](const ActionCodeList& codeList) {
        for (const ActionCode& code : codeList) {
            if (code == SELECT_WORKSPACE_CODE) {
                emit currentWorkspaceActionChanged();
            }

            if (code == TOGGLE_CONCERT_PITCH_CODE) {
                emit concertPitchActionChanged();
            }
        }
    });

    emit availableViewModeListChanged();
    emit availableZoomListChanged();
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

void NotationStatusBarModel::selectWorkspace()
{
    dispatch(SELECT_WORKSPACE_CODE);
}

void NotationStatusBarModel::toggleConcertPitch()
{
    dispatch(TOGGLE_CONCERT_PITCH_CODE);
}

void NotationStatusBarModel::setCurrentViewMode(const QString& modeCode)
{
    dispatch(codeFromQString(modeCode));
}

QVariantList NotationStatusBarModel::availableZoomList() const
{
    QVariantList result;

    int currZoomPercentage = currentZoomPercentage();

    auto zoomPercentageTitle = [](int percentage) {
        return QString::number(percentage) + "%";
    };

    auto buildZoomObj = [=](ZoomType type, const QString& title = QString(), int value = 0) {
        QVariantMap obj;
        obj[TYPE_KEY] = static_cast<int>(type);
        obj[TITLE_KEY] = title.isEmpty() ? zoomTypeTitle(type) : title;
        obj[SELECTABLE_KEY] = true;
        obj[SELECTED_KEY] = false;

        if (m_currentZoomType == type) {
            obj[SELECTED_KEY] = type == ZoomType::Percentage ? value == currZoomPercentage : true;
        }

        obj[VALUE_KEY] = value;

        return obj;
    };

    QList<int> possibleZoomList = possibleZoomPercentageList();

    for (int zoom : possibleZoomList) {
        result << buildZoomObj(ZoomType::Percentage, zoomPercentageTitle(zoom), zoom);
    }

    result << buildZoomObj(ZoomType::PageWidth);
    result << buildZoomObj(ZoomType::WholePage);
    result << buildZoomObj(ZoomType::TwoPages);

    bool isCustomZoom = m_currentZoomType == ZoomType::Percentage && !possibleZoomList.contains(currZoomPercentage);
    if (isCustomZoom) {
        QVariantMap customZoom = buildZoomObj(ZoomType::Percentage, zoomPercentageTitle(currZoomPercentage), currZoomPercentage);
        customZoom[SELECTED_KEY] = true;
        result << customZoom;
    }

    return result;
}

void NotationStatusBarModel::setCurrentZoomIndex(int zoomIndex)
{
    QVariantList zoomList = availableZoomList();

    if (zoomIndex < 0 || zoomIndex >= zoomList.size()) {
        return;
    }

    QVariantMap zoom = zoomList[zoomIndex].toMap();
    ZoomType type = static_cast<ZoomType>(zoom[TYPE_KEY].toInt());
    int value = zoom[VALUE_KEY].toInt();

    m_currentZoomType = type;
    emit availableZoomListChanged();

    dispatch(zoomTypeToActionCode(type), ActionData::make_arg1<int>(value));
}

int NotationStatusBarModel::minZoomPercentage() const
{
    return possibleZoomPercentageList().first();
}

int NotationStatusBarModel::maxZoomPercentage() const
{
    return possibleZoomPercentageList().last();
}

void NotationStatusBarModel::zoomIn()
{
    dispatch("zoomin");
}

void NotationStatusBarModel::zoomOut()
{
    dispatch("zoomout");
}

void NotationStatusBarModel::handleAction(const QString& actionCode)
{
    dispatch(codeFromQString(actionCode));
}

INotationPtr NotationStatusBarModel::notation() const
{
    return context()->currentNotation();
}

INotationAccessibilityPtr NotationStatusBarModel::accessibility() const
{
    return notation() ? notation()->accessibility() : nullptr;
}

void NotationStatusBarModel::dispatch(const actions::ActionCode& code, const actions::ActionData& args)
{
    dispatcher()->dispatch(code, args);
}

QList<int> NotationStatusBarModel::possibleZoomPercentageList() const
{
    QList<int> result;

    for (int zoom : notationConfiguration()->possibleZoomPercentageList()) {
        if (zoom >= MIN_DISPLAYED_ZOOM_PERCENTAGE) {
            result << zoom;
        }
    }

    return result;
}
