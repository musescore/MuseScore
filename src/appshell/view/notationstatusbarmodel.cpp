/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "types/translatablestring.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_WORKSPACE
#include "workspace/view/workspacesmenumodel.h"
#endif

#include "defer.h"
#include "log.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace muse;
using namespace muse::actions;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::workspace;

static const QString TITLE_KEY("title");
static const QString ICON_KEY("icon");
static const QString SELECTABLE_KEY("selectable");
static const QString SELECTED_KEY("selected");
static const QString TYPE_KEY("type");
static const QString ID_KEY("id");
static const QString VALUE_KEY("value");

static const ActionCode TOGGLE_CONCERT_PITCH_CODE("concert-pitch");
static const ActionCode SELECT_WORKSPACE_CODE("configure-workspaces");

static constexpr int MIN_DISPLAYED_ZOOM_PERCENTAGE = 25;

static const std::map<ViewMode, ActionCode> ALL_VIEW_MODE_MAP {
    { ViewMode::PAGE, "view-mode-page" },
    { ViewMode::LINE, "view-mode-continuous" },
    { ViewMode::SYSTEM, "view-mode-single" },
    { ViewMode::FLOAT, "view-mode-float" },
};

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
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel = std::make_shared<WorkspacesMenuModel>(this);
#endif
}

QString NotationStatusBarModel::accessibilityInfo() const
{
    return accessibility() ? QString::fromStdString(accessibility()->accessibilityInfo().val) : QString();
}

QVariant NotationStatusBarModel::concertPitchItem()
{
    if (!m_concertPitchItem) {
        m_concertPitchItem = makeMenuItem(TOGGLE_CONCERT_PITCH_CODE);
    }

    UiActionState state;
    state.enabled = notation() ? true : false;
    state.checked = notation() ? notation()->style()->styleValue(StyleId::concertPitch).toBool() : false;
    m_concertPitchItem->setState(state);

    return QVariant::fromValue(m_concertPitchItem);
}

QVariant NotationStatusBarModel::currentWorkspaceItem()
{
    if (!m_currentWorkspaceItem) {
        m_currentWorkspaceItem = makeMenuItem(SELECT_WORKSPACE_CODE);
    }

    UiAction action;
    action.title = muse::TranslatableString::untranslatable("%1 %2")
                   .arg(muse::TranslatableString("workspace", "Workspace:"),
                        String::fromStdString(workspaceConfiguration()->currentWorkspaceName()));

    m_currentWorkspaceItem->setAction(action);

#ifdef MUSE_MODULE_WORKSPACE
    m_currentWorkspaceItem->setSubitems(m_workspacesMenuModel->items());
#endif

    return QVariant::fromValue(m_currentWorkspaceItem);
}

MenuItem* NotationStatusBarModel::makeMenuItem(const ActionCode& actionCode)
{
    MenuItem* item = new MenuItem(actionsRegister()->action(actionCode), this);
    item->setId(QString::fromStdString(item->action().code));
    item->setState(actionsRegister()->actionState(actionCode));

    return item;
}

QVariant NotationStatusBarModel::currentViewMode()
{
    ViewMode viewMode = notation() ? notation()->viewMode() : ViewMode::PAGE;

    for (MenuItem* modeItem : m_availableViewModeList) {
        ViewMode mode = muse::key(ALL_VIEW_MODE_MAP, modeItem->id().toStdString());

        if (mode == viewMode) {
            if (viewMode == ViewMode::LINE || viewMode == ViewMode::SYSTEM) {
                // In continuous view, we don't want to see "horizontal" or "vertical" (those should only be visible in the menu)
                modeItem->setTitle(muse::TranslatableString("notation", "Continuous view"));
            }

            return QVariant::fromValue(modeItem);
        }
    }

    return QVariant();
}

void NotationStatusBarModel::initAvailableViewModeList()
{
    TRACEFUNC;

    qDeleteAll(m_availableViewModeList);
    m_availableViewModeList.clear();

    DEFER {
        emit availableViewModeListChanged();
        emit currentViewModeChanged();
    };

    if (!notation()) {
        return;
    }

    ViewMode currentViewMode = notation()->viewMode();

    for (const auto& pair: ALL_VIEW_MODE_MAP) {
        if (pair.first == ViewMode::FLOAT && !globalConfiguration()->devModeEnabled()) {
            continue;
        }

        const UiAction& action = actionsRegister()->action(pair.second);
        MenuItem* viewModeItem = new MenuItem(action, this);

        UiActionState state;
        state.enabled = true;
        viewModeItem->setState(state);

        viewModeItem->setId(QString::fromStdString(pair.second));
        viewModeItem->setSelectable(true);
        viewModeItem->setSelected(currentViewMode == pair.first);

        m_availableViewModeList << viewModeItem;
    }
}

bool NotationStatusBarModel::zoomEnabled() const
{
    return notation() != nullptr;
}

int NotationStatusBarModel::currentZoomPercentage() const
{
    if (!notation()) {
        return 100;
    }

    return notation()->viewState()->zoomPercentage().val;
}

void NotationStatusBarModel::setCurrentZoomPercentage(int zoomPercentage)
{
    if (zoomPercentage == currentZoomPercentage()) {
        return;
    }

    dispatch(zoomTypeToActionCode(ZoomType::Percentage), ActionData::make_arg1<int>(zoomPercentage));
}

ZoomType NotationStatusBarModel::currentZoomType() const
{
    if (!notation()) {
        return ZoomType::Percentage;
    }

    return notation()->viewState()->zoomType().val;
}

void NotationStatusBarModel::load()
{
    TRACEFUNC;

    onCurrentNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });

#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel->load();
    connect(m_workspacesMenuModel.get(), &WorkspacesMenuModel::itemsChanged, this, [this](){
        emit currentWorkspaceActionChanged();
    });
#endif
}

void NotationStatusBarModel::onCurrentNotationChanged()
{
    emit zoomEnabledChanged();
    emit concertPitchActionChanged();

    initAvailableViewModeList();
    initAvailableZoomList();

    if (!notation()) {
        return;
    }

    notation()->undoStack()->changesChannel().onReceive(this, [this](const mu::engraving::ScoreChangesRange& range) {
        if (muse::contains(range.changedStyleIdSet, mu::engraving::Sid::concertPitch)) {
            emit concertPitchActionChanged();
        }
    });

    notation()->viewModeChanged().onNotify(this, [this]() {
        initAvailableViewModeList();
    });

    notation()->viewState()->zoomPercentage().ch.onReceive(this, [this](int) {
        initAvailableZoomList();
    });

    notation()->viewState()->zoomType().ch.onReceive(this, [this](ZoomType) {
        initAvailableZoomList();
    });

    listenChangesInAccessibility();
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

void NotationStatusBarModel::toggleConcertPitch()
{
    dispatch(TOGGLE_CONCERT_PITCH_CODE);
}

void NotationStatusBarModel::setCurrentViewMode(const QString& modeCode)
{
    dispatch(codeFromQString(modeCode));
}

void NotationStatusBarModel::initAvailableZoomList()
{
    TRACEFUNC;

    qDeleteAll(m_availableZoomList);
    m_availableZoomList.clear();

    DEFER {
        emit availableZoomListChanged();
        emit currentZoomPercentageChanged();
    };

    if (!notation()) {
        return;
    }

    int currZoomPercentage = currentZoomPercentage();
    ZoomType currZoomType = currentZoomType();

    auto zoomPercentageTitle = [](int percentage) {
        return muse::TranslatableString::untranslatable("%1%").arg(percentage);
    };

    auto buildZoomItem = [=](ZoomType type, const muse::TranslatableString& title = {}, int value = 0) {
        MenuItem* menuItem = new MenuItem(this);
        menuItem->setId(QString::number(static_cast<int>(type)) + QString::number(value));

        UiAction action;
        action.title = title.isEmpty() ? zoomTypeTitle(type) : title;
        menuItem->setAction(action);

        UiActionState state;
        state.enabled = true;
        menuItem->setState(state);

        menuItem->setSelectable(true);
        if (currZoomType == type) {
            menuItem->setSelected(type == ZoomType::Percentage ? value == currZoomPercentage : true);
        }

        menuItem->setArgs(ActionData::make_arg2<ZoomType, int>(type, value));

        return menuItem;
    };

    QList<int> possibleZoomList = possibleZoomPercentageList();

    for (int zoom : possibleZoomList) {
        m_availableZoomList << buildZoomItem(ZoomType::Percentage, zoomPercentageTitle(zoom), zoom);
    }

    m_availableZoomList << buildZoomItem(ZoomType::PageWidth);
    m_availableZoomList << buildZoomItem(ZoomType::WholePage);
    m_availableZoomList << buildZoomItem(ZoomType::TwoPages);

    bool isCustomZoom = currZoomType == ZoomType::Percentage && !possibleZoomList.contains(currZoomPercentage);
    if (isCustomZoom) {
        MenuItem* customZoom = buildZoomItem(ZoomType::Percentage, zoomPercentageTitle(currZoomPercentage), currZoomPercentage);
        customZoom->setSelected(true);
        m_availableZoomList << customZoom;
    }
}

void NotationStatusBarModel::setCurrentZoom(const QString& zoomId)
{
    int zoomIndex = -1;
    for (int i = 0; i < m_availableZoomList.size(); ++i) {
        const MenuItem* zoomItem = m_availableZoomList[i];
        if (zoomItem->id() == zoomId) {
            zoomIndex = i;
            break;
        }
    }

    if (zoomIndex < 0 || zoomIndex >= m_availableZoomList.size()) {
        return;
    }

    const MenuItem* zoom = m_availableZoomList[zoomIndex];
    ZoomType type = zoom->args().arg<ZoomType>(0);
    int value = zoom->args().arg<int>(1);

    emit availableZoomListChanged();

    dispatch(zoomTypeToActionCode(type), ActionData::make_arg1<int>(value));
}

int NotationStatusBarModel::minZoomPercentage() const
{
    return 5;
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

void NotationStatusBarModel::handleWorkspacesMenuItem(const QString& itemId)
{
#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel->handleMenuItem(itemId);
#endif
}

INotationPtr NotationStatusBarModel::notation() const
{
    return context()->currentNotation();
}

INotationAccessibilityPtr NotationStatusBarModel::accessibility() const
{
    return notation() ? notation()->accessibility() : nullptr;
}

void NotationStatusBarModel::dispatch(const ActionCode& code, const ActionData& args)
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

QVariantList NotationStatusBarModel::availableViewModeList_property()
{
    return menuItemListToVariantList(m_availableViewModeList);
}

QVariantList NotationStatusBarModel::availableZoomList_property()
{
    return menuItemListToVariantList(m_availableZoomList);
}

QVariantList NotationStatusBarModel::menuItemListToVariantList(const MenuItemList& list) const
{
    QVariantList result;
    for (MenuItem* item: list) {
        result << QVariant::fromValue(item);
    }

    return result;
}
