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

#include "notationstatusbarmodel.h"

#include "defer.h"
#include "log.h"
#include "thirdparty/kors_logger/src/log_base.h"
#include "types/translatablestring.h"

#include "muse_framework_config.h"
#include <algorithm>

#ifdef MUSE_MODULE_WORKSPACE
#include "workspace/qml/Muse/Workspace/workspacesmenumodel.h"
#endif

#include "engraving/dom/score.h"

#include "notation/inotation.h"
#include "notation/inotationaccessibility.h"
#include "notation/inotationstyle.h" // IWYU pragma: keep
#include "notation/inotationundostack.h" // IWYU pragma: keep
#include "notation/inotationviewstate.h" // IWYU pragma: keep

#include "notationscene/notationcommands.h"
#include "workspace/workspacecommands.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::workspace;

static const QString ICON_KEY("icon");
static const QString SELECTABLE_KEY("selectable");
static const QString SELECTED_KEY("selected");
static const QString TYPE_KEY("type");
static const QString ID_KEY("id");
static const QString VALUE_KEY("value");

static constexpr int MIN_DISPLAYED_ZOOM_PERCENTAGE = 25;

static const std::map<ViewMode, Command> ALL_VIEW_MODE_MAP {
    { ViewMode::PAGE, VIEW_MODE_PAGE_COMMAND },
    { ViewMode::LINE, VIEW_MODE_CONTINUOUS_COMMAND },
    { ViewMode::SYSTEM, VIEW_MODE_SINGLE_COMMAND },
    { ViewMode::FLOAT, VIEW_MODE_FLOAT_COMMAND },
};

static Command zoomTypeToCommand(ZoomType type)
{
    switch (type) {
    case ZoomType::Percentage: return ZOOM_TO_PERCENT_COMMAND;
    case ZoomType::PageWidth: return ZOOM_TO_PAGE_WIDTH_COMMAND;
    case ZoomType::WholePage: return ZOOM_TO_WHOLE_PAGE_COMMAND;
    case ZoomType::TwoPages: return ZOOM_TO_TWO_PAGES_COMMAND;
    }

    return {};
}

NotationStatusBarModel::NotationStatusBarModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel = std::make_shared<WorkspacesMenuModel>(this);
#endif
}

void NotationStatusBarModel::classBegin()
{
    init();
}

void NotationStatusBarModel::init()
{
    TRACEFUNC;

    m_concertPitchItem = makeMenuItem(TOGGLE_CONCERT_PITCH_COMMAND);
    m_currentWorkspaceItem = makeMenuItem(WORKSPACES_CONFIGURE_COMMAND);

    setNotation(context()->currentNotation());

    context()->currentNotationChanged().onNotify(this, [this]() {
        setNotation(context()->currentNotation());
    });

#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel->load();
    connect(m_workspacesMenuModel.get(), &WorkspacesMenuModel::itemsChanged, this, [this]() {
        updateCurrentWorkspaceItem();
    });
    workspaceConfiguration()->currentWorkspaceNameChanged().onReceive(this, [this](const std::string&){
        updateCurrentWorkspaceItem();
    });
#endif

    updateCurrentWorkspaceItem();
}

QString NotationStatusBarModel::accessibilityInfo() const
{
    return accessibility() ? QString::fromStdString(accessibility()->accessibilityInfo().val) : QString();
}

MenuItem* NotationStatusBarModel::concertPitchItem()
{
    return m_concertPitchItem;
}

void NotationStatusBarModel::updateConcertPitchItem()
{
    m_concertPitchItem->setEnabled(m_notation ? true : false);
    m_concertPitchItem->setChecked(m_notation ? m_notation->style()->styleValue(StyleId::concertPitch).toBool() : false);
}

MenuItem* NotationStatusBarModel::currentWorkspaceItem()
{
    return m_currentWorkspaceItem;
}

void NotationStatusBarModel::updateCurrentWorkspaceItem()
{
    m_currentWorkspaceItem->setTitle(
        muse::TranslatableString::untranslatable("%1 %2")
        .arg(muse::TranslatableString("workspace", "Workspace:"),
             String::fromStdString(workspaceConfiguration()->currentWorkspaceName())));

#ifdef MUSE_MODULE_WORKSPACE
    m_currentWorkspaceItem->setSubitems(m_workspacesMenuModel->items());
#endif
}

MenuItem* NotationStatusBarModel::makeMenuItem(const Command& command)
{
    MenuItem* item = new MenuItem(commandsRegister()->commandInfo(command), this);
    item->setCommandState(commandsState()->commandState(command));
    return item;
}

MenuItem* NotationStatusBarModel::currentViewMode()
{
    ViewMode viewMode = m_notation ? m_notation->viewMode() : ViewMode::PAGE;

    for (MenuItem* modeItem : m_availableViewModeList) {
        ViewMode mode = muse::key(ALL_VIEW_MODE_MAP, modeItem->command());

        if (mode == viewMode) {
            if (viewMode == ViewMode::LINE || viewMode == ViewMode::SYSTEM) {
                // In continuous view, we don't want to see "horizontal" or "vertical" (those should only be visible in the menu)
                modeItem->setTitle(muse::TranslatableString("notation", "Continuous view"));
            }

            return modeItem;
        }
    }

    return nullptr;
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

    if (!m_notation) {
        return;
    }

    ViewMode currentViewMode = m_notation->viewMode();

    for (const auto& pair: ALL_VIEW_MODE_MAP) {
        if (pair.first == ViewMode::FLOAT && !globalConfiguration()->devModeEnabled()) {
            continue;
        }

        MenuItem* viewModeItem = makeMenuItem(pair.second);
        viewModeItem->setEnabled(true);
        viewModeItem->setSelectable(true);
        viewModeItem->setSelected(currentViewMode == pair.first);

        m_availableViewModeList << viewModeItem;
    }
}

bool NotationStatusBarModel::zoomEnabled() const
{
    return m_notation != nullptr;
}

int NotationStatusBarModel::currentZoomPercentage() const
{
    if (!m_notation) {
        return 100;
    }

    return m_notation->viewState()->zoomPercentage().val;
}

void NotationStatusBarModel::setCurrentZoomPercentage(int zoomPercentage)
{
    if (zoomPercentage == currentZoomPercentage()) {
        return;
    }

    dispatch(zoomTypeToCommand(ZoomType::Percentage), { { "percent", Val(zoomPercentage) } });
}

ZoomType NotationStatusBarModel::currentZoomType() const
{
    if (!m_notation) {
        return ZoomType::Percentage;
    }

    return m_notation->viewState()->zoomType().val;
}

void NotationStatusBarModel::setNotation(const INotationPtr& notation)
{
    if (m_notation == notation) {
        return;
    }

    if (m_notation) {
        m_notation->undoStack()->changesChannel().disconnect(this);
        m_notation->viewModeChanged().disconnect(this);
        m_notation->viewState()->zoomPercentage().ch.disconnect(this);
        m_notation->viewState()->zoomType().ch.disconnect(this);
        m_notation->accessibility()->accessibilityInfo().ch.disconnect(this);
    }

    m_notation = notation;

    emit zoomEnabledChanged();
    updateConcertPitchItem();

    initAvailableViewModeList();
    initAvailableZoomList();

    if (!notation) {
        return;
    }

    notation->undoStack()->changesChannel().onReceive(this, [this](const mu::engraving::ScoreChanges& changes) {
        if (muse::contains(changes.changedStyleIdSet, mu::engraving::Sid::concertPitch)) {
            updateConcertPitchItem();
        }
    });

    notation->viewModeChanged().onNotify(this, [this]() {
        initAvailableViewModeList();
    });

    notation->viewState()->zoomPercentage().ch.onReceive(this, [this](int) {
        initAvailableZoomList();
    });

    notation->viewState()->zoomType().ch.onReceive(this, [this](ZoomType) {
        initAvailableZoomList();
    });

    emit accessibilityInfoChanged();

    notation->accessibility()->accessibilityInfo().ch.onReceive(this, [this](const std::string&) {
        emit accessibilityInfoChanged();
    });
}

void NotationStatusBarModel::toggleConcertPitch()
{
    dispatch(TOGGLE_CONCERT_PITCH_COMMAND);
}

void NotationStatusBarModel::setCurrentViewMode(const QString& modeCode)
{
    dispatch(Command(modeCode.toStdString()));
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

    if (!m_notation) {
        return;
    }

    int currZoomPercentage = currentZoomPercentage();
    ZoomType currZoomType = currentZoomType();

    auto zoomPercentageTitle = [](int percentage) {
        return muse::TranslatableString::untranslatable("%1%").arg(percentage);
    };

    auto buildZoomItem
        = [this, currZoomType, currZoomPercentage](ZoomType type, const muse::TranslatableString& title = {}, int value = 0) {
        MenuItem* menuItem = new MenuItem(this);
        menuItem->setId(QString::number(static_cast<int>(type)) + QString::number(value));
        menuItem->setTitle(title.isEmpty() ? zoomTypeTitle(type) : title);
        menuItem->setSelectable(true);
        if (currZoomType == type) {
            menuItem->setSelected(type == ZoomType::Percentage ? value == currZoomPercentage : true);
        }

        menuItem->setParams({ { "type", Val(type) }, { "percent", Val(value) } });

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
    auto it = std::find_if(m_availableZoomList.begin(), m_availableZoomList.end(), [zoomId](const MenuItem* item) {
        return item->id() == zoomId;
    });

    if (it == m_availableZoomList.end()) {
        return;
    }

    const Params& params = (*it)->params();
    ZoomType type = params.at("type").toEnum<ZoomType>();
    int percent = params.at("percent").toInt();

    dispatch(zoomTypeToCommand(type), { { "percent", Val(percent) } });

    emit availableZoomListChanged();
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
    dispatch(ZOOM_IN_COMMAND);
}

void NotationStatusBarModel::zoomOut()
{
    dispatch(ZOOM_OUT_COMMAND);
}

void NotationStatusBarModel::handleMenuItem(const QString& itemId)
{
    LOGDA() << itemId;
    if (m_concertPitchItem->id() == itemId) {
        dispatch(m_concertPitchItem->command());
    } else if (m_currentWorkspaceItem->id() == itemId) {
        dispatch(m_currentWorkspaceItem->command());
    } else {
        UNREACHABLE;
    }
}

void NotationStatusBarModel::handleWorkspacesMenuItem(const QString& itemId)
{
#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel->handleMenuItem(itemId);
#else
    UNUSED(itemId);
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

void NotationStatusBarModel::dispatch(const muse::rcommand::Command& command, const muse::rcommand::Params& params)
{
    dispatcher()->dispatch(command, params);
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
