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
        emit currentZoomPercentageChanged();
        emit availableZoomListChanged();
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
    dispatch("configure-workspaces");
}

void NotationStatusBarModel::toggleConcertPitch()
{
    setConcertPitchEnabled(!concertPitchEnabled());
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

void NotationStatusBarModel::setConcertPitchEnabled(bool enabled)
{
    if (!style() || concertPitchEnabled() == enabled) {
        return;
    }

    notation()->undoStack()->prepareChanges();
    style()->setStyleValue(StyleId::concertPitch, enabled);
    notation()->undoStack()->commitChanges();
}
