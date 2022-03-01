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
#include "appmenumodel.h"

#include <QApplication>
#include <QWindow>
#include <QKeyEvent>

#include "translation.h"

#include "log.h"
#include "config.h"

using namespace mu::appshell;
using namespace mu::ui;
using namespace mu::uicomponents;
using namespace mu::project;
using namespace mu::workspace;
using namespace mu::actions;
using namespace mu::plugins;

static QString makeId(const ActionCode& actionCode, int itemIndex)
{
    return QString::fromStdString(actionCode) + QString::number(itemIndex);
}

AppMenuModel::AppMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void AppMenuModel::load()
{
    TRACEFUNC;

    AbstractMenuModel::load();

    MenuItemList items {
        makeFileMenu(),
        makeEditMenu(),
        makeViewMenu(),
        makeAddMenu(),
        makeFormatMenu(),
        makeToolsMenu(),
        makePluginsMenu(),
        makeHelpMenu(),
#ifdef BUILD_DIAGNOSTICS
        makeDiagnosticMenu()
#endif
    };

    setItems(items);

    setupConnections();

    //! NOTE: removes some undesired platform-specific items
    //! (such as "Start Dictation" and "Special Characters" on macOS)
    appMenuModelHook()->onAppMenuInited();
}

QWindow* AppMenuModel::appWindow() const
{
    return m_appWindow;
}

void AppMenuModel::setHighlightedMenuId(QString highlightedMenuId)
{
    if (m_highlightedMenuId == highlightedMenuId) {
        return;
    }

    m_highlightedMenuId = highlightedMenuId;
    emit highlightedMenuIdChanged(m_highlightedMenuId);
}

void AppMenuModel::setAppWindow(QWindow* appWindow)
{
    m_appWindow = appWindow;
}

void AppMenuModel::setupConnections()
{
    recentProjectsProvider()->recentProjectListChanged().onNotify(this, [this]() {
        MenuItem& recentScoreListItem = findMenu("menu-file-open");

        MenuItemList recentScoresList = makeRecentScoresItems();
        bool openRecentEnabled = !recentScoresList.empty();

        if (!recentScoresList.empty()) {
            recentScoresList = appendClearRecentSection(recentScoresList);
        }

        UiActionState state = recentScoreListItem.state();
        state.enabled = openRecentEnabled;
        recentScoreListItem.setState(state);

        recentScoreListItem.setSubitems(recentScoresList);
    });

    workspacesManager()->currentWorkspaceChanged().onNotify(this, [this]() {
        MenuItem& workspacesItem = findMenu("menu-workspaces");
        workspacesItem.setSubitems(makeWorkspacesItems());
    });

    workspacesManager()->workspacesListChanged().onNotify(this, [this]() {
        MenuItem& workspacesItem = findMenu("menu-workspaces");
        workspacesItem.setSubitems(makeWorkspacesItems());
    });

    connect(qApp, &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state){
        if (state != Qt::ApplicationActive) {
            resetNavigation();
        }
    });

    pluginsService()->pluginsChanged().onNotify(this, [this]() {
        MenuItem& pluginsMenu = findMenu("menu-plugins");
        pluginsMenu.setSubitems(makePluginsMenuSubitems());
    });

    pluginsService()->pluginChanged().onReceive(this, [this](const PluginInfo&) {
        MenuItem& pluginsItem = findMenu("menu-plugins");
        pluginsItem.setSubitems(makePluginsMenuSubitems());
    });

    qApp->installEventFilter(this);
}

MenuItem* AppMenuModel::makeMenuItem(const actions::ActionCode& actionCode, MenuItemRole menuRole)
{
    MenuItem* item = makeMenuItem(actionCode);
    item->setRole(menuRole);
    return item;
}

MenuItem* AppMenuModel::makeFileMenu()
{
    MenuItemList recentScoresList = makeRecentScoresItems();
    bool openRecentEnabled = !recentScoresList.isEmpty();

    if (!recentScoresList.empty()) {
        recentScoresList = appendClearRecentSection(recentScoresList);
    }

    MenuItemList fileItems {
        makeMenuItem("file-new"),
        makeMenuItem("file-open"),
        makeMenu(qtrc("appshell", "Open &recent"), recentScoresList, "menu-file-open", openRecentEnabled),
        makeSeparator(),
        makeMenuItem("file-close"),
        makeMenuItem("file-save"),
        makeMenuItem("file-save-as"),
        makeMenuItem("file-save-a-copy"),
        makeMenuItem("file-save-selection"),
        makeMenuItem("file-save-to-cloud"),
        makeMenuItem("file-publish"),
        makeSeparator(),
        makeMenuItem("file-import-pdf"),
        makeMenuItem("file-export"),
        makeSeparator(),
        makeMenuItem("edit-info"),
        makeMenuItem("parts"),
        makeSeparator(),
        makeMenuItem("print"),
        makeSeparator(),
        makeMenuItem("quit", MenuItemRole::QuitRole)
    };

    return makeMenu(qtrc("appshell", "&File"), fileItems, "menu-file");
}

MenuItem* AppMenuModel::makeEditMenu()
{
    MenuItemList editItems {
        makeMenuItem("undo"),
        makeMenuItem("redo"),
        makeSeparator(),
        makeMenuItem("notation-cut"),
        makeMenuItem("notation-copy"),
        makeMenuItem("notation-paste"),
        makeMenuItem("notation-paste-half"),
        makeMenuItem("notation-paste-double"),
        makeMenuItem("notation-swap"),
        makeMenuItem("notation-delete"),
        makeSeparator(),
        makeMenuItem("notation-select-all"),
        makeMenuItem("notation-select-section"),
        makeMenuItem("find"),
        makeSeparator(),
        makeMenuItem("preference-dialog", MenuItemRole::PreferencesRole)
    };

    return makeMenu(qtrc("appshell", "&Edit"), editItems, "menu-edit");
}

MenuItem* AppMenuModel::makeViewMenu()
{
    MenuItemList viewItems {
#ifndef Q_OS_MAC
        makeMenuItem("fullscreen"),
#endif
        makeMenuItem("toggle-palettes"),
        makeMenuItem("masterpalette"),
        makeMenuItem("toggle-instruments"),
        makeMenuItem("inspector"),
        makeMenuItem("toggle-selection-filter"),
        makeMenuItem("toggle-navigator"),
        makeMenuItem("toggle-timeline"),
        makeMenuItem("toggle-mixer"),
        // TODO: https://github.com/musescore/MuseScore/issues/9168
        // makeMenuItem("toggle-piano"), // need implement
        makeSeparator(),
        makeMenu(qtrc("appshell", "&Toolbars"), makeToolbarsItems(), "menu-toolbars"),
        makeMenu(qtrc("appshell", "W&orkspaces"), makeWorkspacesItems(), "menu-workspaces"),
        makeSeparator(),
        makeMenu(qtrc("appshell", "Show"), makeShowItems(), "menu-show"),
        makeSeparator(),
        makeMenuItem("dock-restore-default-layout")
    };

    return makeMenu(qtrc("appshell", "&View"), viewItems, "menu-view");
}

MenuItem* AppMenuModel::makeAddMenu()
{
    MenuItemList addItems {
        makeMenu(qtrc("appshell", "N&otes"), makeNotesItems()),
        makeMenu(qtrc("appshell", "&Intervals"), makeIntervalsItems()),
        makeMenu(qtrc("appshell", "T&uplets"), makeTupletsItems()),
        makeSeparator(),
        makeMenu(qtrc("appshell", "&Measures"), makeMeasuresItems()),
        makeMenu(qtrc("appshell", "&Frames"), makeFramesItems()),
        makeMenu(qtrc("appshell", "&Text"), makeTextItems()),
        makeMenu(qtrc("appshell", "&Lines"), makeLinesItems()),
    };

    return makeMenu(qtrc("appshell", "&Add"), addItems, "menu-add");
}

MenuItem* AppMenuModel::makeFormatMenu()
{
    MenuItemList stretchItems {
        makeMenuItem("stretch+"),
        makeMenuItem("stretch-"),
        makeMenuItem("reset-stretch")
    };

    MenuItemList formatItems {
        makeMenuItem("edit-style"),
        makeMenuItem("page-settings"),
        makeSeparator(),
        makeMenuItem("add-remove-breaks"),
        makeMenu(qtrc("appshell", "&Stretch"), stretchItems),
        makeSeparator(),
        makeMenuItem("reset-text-style-overrides"),
        makeMenuItem("reset-beammode"),
        makeMenuItem("reset"),
        makeSeparator(),
        makeMenuItem("load-style"),
        makeMenuItem("save-style")
    };

    return makeMenu(qtrc("appshell", "F&ormat"), formatItems, "menu-format");
}

MenuItem* AppMenuModel::makeToolsMenu()
{
    MenuItemList voicesItems {
        makeMenuItem("voice-x12"),
        makeMenuItem("voice-x13"),
        makeMenuItem("voice-x14"),
        makeMenuItem("voice-x23"),
        makeMenuItem("voice-x24"),
        makeMenuItem("voice-x34")
    };

    MenuItemList measuresItems {
        makeMenuItem("split-measure"),
        makeMenuItem("join-measures")
    };

    MenuItemList toolsItems {
        makeMenuItem("transpose"),
        makeSeparator(),
        makeMenuItem("explode"),
        makeMenuItem("implode"),
        makeMenuItem("realize-chord-symbols"),
        makeMenu(qtrc("appshell", "&Voices"), voicesItems),
        makeMenu(qtrc("appshell", "&Measures"), measuresItems),
        makeMenuItem("time-delete"),
        makeSeparator(),
        makeMenuItem("slash-fill"),
        makeMenuItem("slash-rhythm"),
        makeSeparator(),
        makeMenuItem("pitch-spell"),
        makeMenuItem("reset-groupings"),
        makeMenuItem("resequence-rehearsal-marks"),
        /*
         * TODO: https://github.com/musescore/MuseScore/issues/9670
        makeMenuItem("unroll-repeats"),
         */
        makeSeparator(),
        makeMenuItem("copy-lyrics-to-clipboard"),
        makeMenuItem("del-empty-measures"),
    };

    return makeMenu(qtrc("appshell", "&Tools"), toolsItems, "menu-tools");
}

MenuItem* AppMenuModel::makePluginsMenu()
{
    return makeMenu(qtrc("appshell", "&Plugins"), makePluginsMenuSubitems(), "menu-plugins");
}

MenuItemList AppMenuModel::makePluginsMenuSubitems()
{
    MenuItemList subitems {
        makeMenuItem("manage-plugins"),
    };

    MenuItemList enabledPlugins = makePluginsItems();

    if (!enabledPlugins.empty()) {
        subitems << makeSeparator();
    }

    subitems << enabledPlugins;

    return subitems;
}

MenuItem* AppMenuModel::makeHelpMenu()
{
    MenuItemList helpItems {
        makeMenuItem("online-handbook"),
        makeSeparator(),
        makeMenuItem("about", MenuItemRole::AboutRole),
        makeMenuItem("about-qt", MenuItemRole::AboutQtRole),
        makeMenuItem("about-musicxml")
    };

    if (configuration()->isAppUpdatable()) {
        helpItems << makeMenuItem("check-update");
    }

    helpItems << makeSeparator()
              << makeMenuItem("ask-help")
              << makeMenuItem("report-bug")
              << makeMenuItem("leave-feedback")
              << makeSeparator()
              << makeMenuItem("revert-factory");

    return makeMenu(qtrc("appshell", "&Help"), helpItems, "menu-help");
}

MenuItem* AppMenuModel::makeDiagnosticMenu()
{
    MenuItemList systemItems {
        makeMenuItem("diagnostic-show-paths"),
        makeMenuItem("diagnostic-show-profiler"),
    };

    MenuItemList accessibilityItems {
        makeMenuItem("diagnostic-show-navigation-tree"),
        makeMenuItem("diagnostic-show-accessible-tree"),
        makeMenuItem("diagnostic-accessible-tree-dump"),
    };

    MenuItemList engravingItems {
        makeMenuItem("diagnostic-show-engraving-elements"),
    };

    MenuItemList autobotItems {
        makeMenuItem("autobot-show-scripts"),
    };

    MenuItemList items {
        makeMenu(qtrc("appshell", "System"), systemItems),
        makeMenu(qtrc("appshell", "Accessibility"), accessibilityItems),
        makeMenu(qtrc("appshell", "Engraving"), engravingItems),
        makeMenu(qtrc("appshell", "Autobot"), autobotItems),
        makeMenuItem("multiinstances-dev-show-info")
    };

    return makeMenu(qtrc("appshell", "&Diagnostic"), items, "menu-diagnostic");
}

MenuItemList AppMenuModel::makeRecentScoresItems()
{
    MenuItemList items;
    ProjectMetaList recentProjects = recentProjectsProvider()->recentProjectList();

    int index = 0;
    for (const ProjectMeta& meta : recentProjects) {
        MenuItem* item = new MenuItem(this);

        UiAction action;
        action.code = "file-open";
        action.title = meta.fileName().toQString();
        item->setAction(action);

        item->setId(makeId(item->action().code, index++));

        UiActionState state;
        state.enabled = true;
        item->setState(state);

        item->setSelectable(true);
        item->setArgs(ActionData::make_arg1<io::path>(meta.filePath));

        items << item;
    }

    return items;
}

MenuItemList AppMenuModel::appendClearRecentSection(const uicomponents::MenuItemList& recentScores)
{
    MenuItemList result = recentScores;
    result << makeSeparator()
           << makeMenuItem("clear-recent");

    return result;
}

MenuItemList AppMenuModel::makeNotesItems()
{
    MenuItemList items {
        makeMenuItem("note-input"),
        makeSeparator(),
        makeMenuItem("note-c"),
        makeMenuItem("note-d"),
        makeMenuItem("note-e"),
        makeMenuItem("note-f"),
        makeMenuItem("note-g"),
        makeMenuItem("note-a"),
        makeMenuItem("note-b"),
        makeSeparator(),
        makeMenuItem("chord-c"),
        makeMenuItem("chord-d"),
        makeMenuItem("chord-e"),
        makeMenuItem("chord-f"),
        makeMenuItem("chord-g"),
        makeMenuItem("chord-a"),
        makeMenuItem("chord-b")
    };

    return items;
}

MenuItemList AppMenuModel::makeIntervalsItems()
{
    MenuItemList items {
        makeMenuItem("interval1"),
        makeMenuItem("interval2"),
        makeMenuItem("interval3"),
        makeMenuItem("interval4"),
        makeMenuItem("interval5"),
        makeMenuItem("interval6"),
        makeMenuItem("interval7"),
        makeMenuItem("interval8"),
        makeMenuItem("interval9"),
        makeSeparator(),
        makeMenuItem("interval-2"),
        makeMenuItem("interval-3"),
        makeMenuItem("interval-4"),
        makeMenuItem("interval-5"),
        makeMenuItem("interval-6"),
        makeMenuItem("interval-7"),
        makeMenuItem("interval-8"),
        makeMenuItem("interval-9")
    };

    return items;
}

MenuItemList AppMenuModel::makeTupletsItems()
{
    MenuItemList items {
        makeMenuItem("duplet"),
        makeMenuItem("triplet"),
        makeMenuItem("quadruplet"),
        makeMenuItem("quintuplet"),
        makeMenuItem("sextuplet"),
        makeMenuItem("septuplet"),
        makeMenuItem("octuplet"),
        makeMenuItem("nonuplet"),
        makeMenuItem("tuplet-dialog")
    };

    return items;
}

MenuItemList AppMenuModel::makeMeasuresItems()
{
    MenuItemList items {
        makeMenuItem("insert-measures-after-selection", qtrc("notation", "Insert after selection…")),
        makeMenuItem("insert-measures", qtrc("notation", "Insert before selection…")),
        makeSeparator(),
        makeMenuItem("insert-measures-at-start-of-score", qtrc("notation", "Insert at start of score…")),
        makeMenuItem("append-measures", qtrc("notation", "Insert at end of score…"))
    };

    return items;
}

MenuItemList AppMenuModel::makeFramesItems()
{
    MenuItemList items {
        makeMenuItem("insert-hbox"),
        makeMenuItem("insert-vbox"),
        makeMenuItem("insert-textframe"),
        makeSeparator(),
        makeMenuItem("append-hbox"),
        makeMenuItem("append-vbox"),
        makeMenuItem("append-textframe")
    };

    return items;
}

MenuItemList AppMenuModel::makeTextItems()
{
    MenuItemList items {
        makeMenuItem("title-text"),
        makeMenuItem("subtitle-text"),
        makeMenuItem("composer-text"),
        makeMenuItem("poet-text"),
        makeMenuItem("part-text"),
        makeSeparator(),
        makeMenuItem("system-text"),
        makeMenuItem("staff-text"),
        makeMenuItem("expression-text"),
        makeMenuItem("rehearsalmark-text"),
        makeMenuItem("instrument-change-text"),
        makeMenuItem("fingering-text"),
        makeSeparator(),
        makeMenuItem("sticking-text"),
        makeMenuItem("chord-text"),
        makeMenuItem("roman-numeral-text"),
        makeMenuItem("nashville-number-text"),
        makeMenuItem("lyrics"),
        makeMenuItem("figured-bass"),
        makeMenuItem("tempo")
    };

    return items;
}

MenuItemList AppMenuModel::makeLinesItems()
{
    MenuItemList items {
        makeMenuItem("add-slur"),
        makeMenuItem("add-hairpin"),
        makeMenuItem("add-hairpin-reverse"),
        makeMenuItem("add-8va"),
        makeMenuItem("add-8vb"),
        makeMenuItem("add-noteline")
    };

    return items;
}

MenuItemList AppMenuModel::makeToolbarsItems()
{
    MenuItemList items {
        makeMenuItem("toggle-transport"),
        makeMenuItem("toggle-noteinput"),
        makeMenuItem("toggle-statusbar")
    };

    return items;
}

MenuItemList AppMenuModel::makeWorkspacesItems()
{
    MenuItemList items;

    IWorkspacePtrList workspaces = workspacesManager()->workspaces();
    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace();

    std::sort(workspaces.begin(), workspaces.end(), [](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    int index = 0;
    for (const IWorkspacePtr& workspace : workspaces) {
        MenuItem* item = new MenuItem(uiActionsRegister()->action("select-workspace"), this);
        item->setId(makeId(item->action().code, index++));

        UiAction action = item->action();
        action.title = QString::fromStdString(workspace->title());

        item->setAction(action);
        item->setArgs(ActionData::make_arg1<std::string>(workspace->name()));
        item->setSelectable(true);
        item->setSelected(workspace == currentWorkspace);

        UiActionState state;
        state.enabled = true;
        state.checked = item->selected();
        item->setState(state);

        items << item;
    }

    items << makeSeparator()
          << makeMenuItem("configure-workspaces");

    return items;
}

MenuItemList AppMenuModel::makeShowItems()
{
    MenuItemList items {
        makeMenuItem("show-invisible"),
        makeMenuItem("show-unprintable"),
        makeMenuItem("show-frames"),
        makeMenuItem("show-pageborders"),
        makeMenuItem("show-irregular")
    };

    return items;
}

MenuItemList AppMenuModel::makePluginsItems()
{
    MenuItemList result;

    for (const PluginInfo& plugin : pluginsService()->plugins(IPluginsService::Enabled).val) {
        MenuItem* pluginItem = makeMenuItem(plugin.codeKey.toStdString(), plugin.name);
        if (!pluginItem) {
            continue;
        }

        result << pluginItem;
    }

    return result;
}

bool AppMenuModel::eventFilter(QObject* watched, QEvent* event)
{
#ifdef Q_OS_MACOS
    return AbstractMenuModel::eventFilter(watched, event);
#endif

    if (watched != m_appWindow) {
        return AbstractMenuModel::eventFilter(watched, event);
    }

    auto isAltKey = [](const QKeyEvent* keyEvent){
        return keyEvent->key() == Qt::Key_Alt && keyEvent->key() != Qt::Key_Shift && !(keyEvent->modifiers() & Qt::ShiftModifier);
    };

    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (isAltKey(keyEvent)) {
            m_needActivateHighlight = true;
        }

        break;
    }
    case QEvent::KeyRelease: {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (!isAltKey(keyEvent)) {
            break;
        }

        if (isNavigationStarted()) {
            resetNavigation();
            restoreMUNavigationSystemState();
        } else {
            if (m_needActivateHighlight) {
                saveMUNavigationSystemState();
                navigateToFirstMenu();
            } else {
                m_needActivateHighlight = true;
            }
        }

        break;
    }
    case QEvent::ShortcutOverride: {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        bool isNavigationStarted = this->isNavigationStarted();
        if (isNavigationStarted && isNavigateKey(keyEvent->key())) {
            navigate(keyEvent->key());
            m_needActivateHighlight = false;

            event->accept();
        } else if ((!keyEvent->modifiers() && isNavigationStarted)
                   || ((keyEvent->modifiers() & Qt::AltModifier) && !(keyEvent->modifiers() & Qt::ShiftModifier)
                       && keyEvent->text().length() == 1)) {
            if (hasItemByActivateKey(keyEvent->text())) {
                navigate(keyEvent->text());
                m_needActivateHighlight = true;

                event->accept();
            } else {
                m_needActivateHighlight = false;
                resetNavigation();
                restoreMUNavigationSystemState();

                event->ignore();
            }
        } else {
            m_needActivateHighlight = false;

            event->ignore();
        }

        break;
    }
    case QEvent::MouseButtonPress: {
        resetNavigation();
        break;
    }
    default:
        break;
    }

    return AbstractMenuModel::eventFilter(watched, event);
}

bool AppMenuModel::isNavigationStarted() const
{
    return !m_highlightedMenuId.isEmpty();
}

bool AppMenuModel::isNavigateKey(int key) const
{
    static QList<Qt::Key> keys {
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Down,
        Qt::Key_Space,
        Qt::Key_Escape,
        Qt::Key_Return
    };

    return keys.contains(static_cast<Qt::Key>(key));
}

void AppMenuModel::navigate(int key)
{
    Qt::Key _key = static_cast<Qt::Key>(key);
    switch (_key) {
    case Qt::Key_Left: {
        int newIndex = itemIndex(m_highlightedMenuId) - 1;
        if (newIndex < 0) {
            newIndex = rowCount() - 1;
        }

        setHighlightedMenuId(item(newIndex).id());
        break;
    }
    case Qt::Key_Right: {
        int newIndex = itemIndex(m_highlightedMenuId) + 1;
        if (newIndex > rowCount() - 1) {
            newIndex = 0;
        }

        setHighlightedMenuId(item(newIndex).id());
        break;
    }
    case Qt::Key_Down:
    case Qt::Key_Space:
    case Qt::Key_Return:
        activateHighlightedMenu();
        break;
    case Qt::Key_Escape:
        resetNavigation();
        restoreMUNavigationSystemState();
        break;
    default:
        break;
    }
}

bool AppMenuModel::hasItemByActivateKey(const QString& keySymbol)
{
    return !menuIdByActivateSymbol(keySymbol).isEmpty();
}

void AppMenuModel::navigate(const QString& keySymbol)
{
    saveMUNavigationSystemState();

    setHighlightedMenuId(menuIdByActivateSymbol(keySymbol));
    activateHighlightedMenu();
}

void AppMenuModel::resetNavigation()
{
    setHighlightedMenuId("");
}

void AppMenuModel::navigateToFirstMenu()
{
    setHighlightedMenuId(item(0).id());
}

void AppMenuModel::saveMUNavigationSystemState()
{
    if (!navigationController()->isHighlight()) {
        return;
    }

    ui::INavigationControl* activeControl = navigationController()->activeControl();
    if (activeControl) {
        m_lastActiveNavigationControl = activeControl;
        activeControl->setActive(false);
    }
}

void AppMenuModel::restoreMUNavigationSystemState()
{
    if (m_lastActiveNavigationControl) {
        m_lastActiveNavigationControl->requestActive();
    }
}

void AppMenuModel::activateHighlightedMenu()
{
    emit openMenu(m_highlightedMenuId);
    actionsDispatcher()->dispatch("nav-first-control");
}

QString AppMenuModel::highlightedMenuId() const
{
    return m_highlightedMenuId;
}

QString AppMenuModel::menuIdByActivateSymbol(const QString& symbol)
{
    for (int i = 0; i < rowCount(); i++) {
        MenuItem& menuItem = item(i);
        QString title = menuItem.action().title;

        int activateKeyIndex = title.indexOf('&');
        if (activateKeyIndex == -1) {
            continue;
        }

        if (title[activateKeyIndex + 1].toLower() == symbol.toLower()) {
            return menuItem.id();
        }
    }

    return QString();
}
