//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "paletteworkspace.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QQmlEngine>
#include <QMimeData>
#include <QStandardPaths>
#include <QMainWindow>

#include "libmscore/keysig.h"
#include "libmscore/timesig.h"

#include "keyedit.h"
#include "palette/palette.h"
#include "timedialog.h"

#include "io/path.h"
#include "commonscene/commonscenetypes.h"

#include "translation.h"

using namespace mu::palette;
using namespace mu::framework;

namespace Ms {
//---------------------------------------------------------
//   PaletteElementEditor::valid
//---------------------------------------------------------

bool PaletteElementEditor::valid() const
{
    using Type = PalettePanel::Type;
    switch (_type) {
    case Type::KeySig:
    case Type::TimeSig:
        return true;
    default:
        break;
    }
    return false;
}

//---------------------------------------------------------
//   PaletteElementEditor::actionName
//---------------------------------------------------------

QString PaletteElementEditor::actionName() const
{
    using Type = PalettePanel::Type;
    switch (_type) {
    case Type::KeySig:
        return mu::qtrc("palette", "Create Key Signature");
    case Type::TimeSig:
        return mu::qtrc("palette", "Create Time Signature");
    default:
        break;
    }
    return QString();
}

//---------------------------------------------------------
//   PaletteElementEditor::onElementAdded
//---------------------------------------------------------

void PaletteElementEditor::onElementAdded(const ElementPtr element)
{
    if (!_paletteIndex.isValid()
        || !_paletteIndex.data(PaletteTreeModel::VisibleRole).toBool()) {
        interactive()->message(IInteractive::Type::Info, "", mu::trc("palette", "The palette was hidden or changed"));
        return;
    }
    QVariantMap mimeData;
    mimeData[mu::commonscene::MIME_SYMBOL_FORMAT] = element->mimeData(QPointF());
    _controller->insert(_paletteIndex, -1, mimeData, Qt::CopyAction);
}

//---------------------------------------------------------
//   PaletteElementEditor::open
//---------------------------------------------------------

void PaletteElementEditor::open()
{
    if (!_paletteIndex.isValid()) {
        return;
    }

    QWidget* editor = nullptr;

    using Type = PalettePanel::Type;
    switch (_type) {
    case Type::KeySig: {
        KeyEditor* keyEditor = new KeyEditor(mainWindow()->qMainWindow());
        keyEditor->showKeyPalette(false);
        connect(keyEditor, &KeyEditor::keySigAdded, this, &PaletteElementEditor::onElementAdded);
        editor = keyEditor;
    }
    break;
    case Type::TimeSig: {
        TimeDialog* timeEditor = new TimeDialog(mainWindow()->qMainWindow());
        timeEditor->showTimePalette(false);
        connect(timeEditor, &TimeDialog::timeSigAdded, this, &PaletteElementEditor::onElementAdded);
        editor = timeEditor;
    }
    break;
    default:
        break;
    }

    if (!editor) {
        return;
    }

    mainWindow()->stackUnder(editor);
    editor->setAttribute(Qt::WA_DeleteOnClose);

    editor->show();
}

//---------------------------------------------------------
//   findPaletteIndex
//---------------------------------------------------------

static QModelIndex findPaletteIndex(const QAbstractItemModel* model, PalettePanel::Type type)
{
    constexpr int role = PaletteTreeModel::PaletteTypeRole;
    const QModelIndex start = model->index(0, 0);
    const QModelIndexList foundIndexList = model->match(start, role, QVariant::fromValue(type));

    if (!foundIndexList.empty()) {
        return foundIndexList[0];
    }
    return QModelIndex();
}

//---------------------------------------------------------
//   convertIndex
//---------------------------------------------------------

static QModelIndex convertIndex(const QModelIndex& index, const QAbstractItemModel* targetModel)
{
    if (index.model() == targetModel || !index.isValid()) {
        return index;
    }

    constexpr int typeRole = PaletteTreeModel::PaletteTypeRole;
    const auto type = index.model()->data(index, typeRole).value<PalettePanel::Type>();

    return findPaletteIndex(targetModel, type);
}

//---------------------------------------------------------
//   convertProxyIndex
//---------------------------------------------------------

static QModelIndex convertProxyIndex(const QModelIndex& srcIndex, const QAbstractItemModel* targetModel)
{
    QModelIndex index = srcIndex;
    while (index.model() != targetModel) {
        if (auto m = qobject_cast<const QAbstractProxyModel*>(index.model())) {
            index = m->mapToSource(index);
        } else {
            break;
        }
    }

    if (targetModel && index.model() != targetModel) {
        return QModelIndex();
    }
    return index;
}

//---------------------------------------------------------
//   AbstractPaletteController::elementEditor
//---------------------------------------------------------

PaletteElementEditor* AbstractPaletteController::elementEditor(const QModelIndex& paletteIndex)
{
    PaletteElementEditor* ed
        = new PaletteElementEditor(this, paletteIndex,
                                   paletteIndex.data(
                                       PaletteTreeModel::PaletteTypeRole).value<PalettePanel::Type>(), this);
    QQmlEngine::setObjectOwnership(ed, QQmlEngine::JavaScriptOwnership);
    return ed;
}

//---------------------------------------------------------
//   UserPaletteController::dropAction
//---------------------------------------------------------

Qt::DropAction UserPaletteController::dropAction(const QVariantMap& mimeData, Qt::DropAction proposedAction,
                                                 const QModelIndex& parent, bool internal) const
{
    if (internal && !userEditable()) {
        return Qt::IgnoreAction;
    }
    const bool parentEditingEnabled = model()->data(parent, PaletteTreeModel::EditableRole).toBool();
    if (!parentEditingEnabled) {
        return Qt::IgnoreAction;
    }

    if (mimeData.contains(PaletteCell::mimeDataFormat) && proposedAction == Qt::MoveAction) {
        const auto cell = PaletteCell::readMimeData(mimeData[PaletteCell::mimeDataFormat].toByteArray());
        if (!cell) {
            return Qt::IgnoreAction;
        }
        if (_filterCustom && cell->custom != _custom) {
            return Qt::IgnoreAction;
        }
        return Qt::MoveAction;
    }
    if (mimeData.contains(mu::commonscene::MIME_SYMBOL_FORMAT) && proposedAction == Qt::CopyAction) {
        if (_filterCustom && !_custom) {
            return Qt::IgnoreAction;
        }
        return Qt::CopyAction;
    }
    return Qt::IgnoreAction;
}

//---------------------------------------------------------
//   UserPaletteController::insert
//---------------------------------------------------------

bool UserPaletteController::insert(const QModelIndex& parent, int row, const QVariantMap& mimeData,
                                   Qt::DropAction action)
{
    if (dropAction(mimeData, action, parent, false) == Qt::IgnoreAction) {
        return false;
    }

    if (row < 0) {
        row = parent.model()->rowCount(parent);
    }

    PaletteCellPtr cell;

    if (mimeData.contains(PaletteCell::mimeDataFormat)) {
        cell = PaletteCell::readMimeData(mimeData[PaletteCell::mimeDataFormat].toByteArray());

        if (!cell) {
            return false;
        }
        if (_filterCustom && cell->custom != _custom) {
            return false;
        }

        if (action == Qt::MoveAction) {
            const QModelIndex visiblePaletteParentIndex = convertIndex(parent, _userPalette);
            const QModelIndex foundIndex(_userPalette->findPaletteCell(*cell, visiblePaletteParentIndex));
            if (foundIndex.isValid()) {
                return _userPalette->setData(foundIndex, _visible, PaletteTreeModel::VisibleRole);
            } else if (!userEditable()) {
                return false;
            }
        }
    } else if (mimeData.contains(mu::commonscene::MIME_SYMBOL_FORMAT) && (action == Qt::CopyAction)) {
        cell = PaletteCell::readElementMimeData(mimeData[mu::commonscene::MIME_SYMBOL_FORMAT].toByteArray());
    }

    if (!cell) {
        return false;
    }

    if (_filterCustom) {
        if (!_custom) {
            return false;       // can only move non-custom cells
        }
        cell->custom = _custom;
    }
    cell->visible = _visible;

    QMimeData data;
    data.setData(PaletteCell::mimeDataFormat, cell->mimeData());
    constexpr int column = 0;
    return model()->dropMimeData(&data, action, row, column, parent);
}

//---------------------------------------------------------
//   UserPaletteController::insertNewItem
//---------------------------------------------------------

bool UserPaletteController::insertNewItem(const QModelIndex& parent, int row, const QString& name)
{
    if (!canEdit(parent)) {
        return false;
    }

    const bool newItemIsPalette = !parent.isValid();

    if (newItemIsPalette) {
        if (!model()->insertRow(row, parent)) {
            return false;
        }
        model()->setData(model()->index(row, 0, parent), name, Qt::DisplayRole);
        return true;
    }

    return model()->insertRow(row, parent);
}

//---------------------------------------------------------
//   UserPaletteController::move
//---------------------------------------------------------

bool UserPaletteController::move(const QModelIndex& sourceParent, int sourceRow, const QModelIndex& destinationParent,
                                 int destinationChild)
{
    if (!canEdit(sourceParent) || !canEdit(destinationParent)) {
        return false;
    }
    if (sourceParent == destinationParent && (sourceParent.model() == model() || !sourceParent.isValid())) {
        const QModelIndex srcIndex = convertProxyIndex(model()->index(sourceRow, 0, sourceParent), _userPalette);
        const QModelIndex destIndex = convertProxyIndex(model()->index(destinationChild, 0,
                                                                       destinationParent), _userPalette);
        return _userPalette->moveRow(srcIndex.parent(), srcIndex.row(), destIndex.parent(), destIndex.row());
    }
    return false;
}

//---------------------------------------------------------
//   UserPaletteController::showHideOrDeleteDialog
//---------------------------------------------------------

void UserPaletteController::showHideOrDeleteDialog(const std::string& question,
                                                   std::function<void(AbstractPaletteController::RemoveAction)> resultHandler)
const
{
    int hideButton = int(IInteractive::Button::CustomButton) + 1;
    int deleteButton = hideButton + 1;

    int button = interactive()->question(std::string(), question, {
            IInteractive::ButtonData(hideButton, mu::trc("palette", "Hide")),
            IInteractive::ButtonData(deleteButton, mu::trc("palette", "Delete permanently")),
            interactive()->buttonData(IInteractive::Button::Cancel)
        });

    RemoveAction action = RemoveAction::NoAction;

    if (button == deleteButton) {
        action = RemoveAction::DeletePermanently;
    } else if (button == hideButton) {
        action = RemoveAction::Hide;
    }

    resultHandler(action);
}

//---------------------------------------------------------
//   UserPaletteController::queryRemove
//---------------------------------------------------------

void UserPaletteController::queryRemove(const QModelIndexList& removeIndices, int customCount)
{
    using RemoveAction = AbstractPaletteController::RemoveAction;

    if (removeIndices.empty() || !canEdit(removeIndices[0].parent())) {
        return;
    }

    if (!customCount) {
        remove(removeIndices, RemoveAction::AutoAction);
        return;
    }

    // these parameters should not depend on exact index as all they should have the same parent in palette tree
    const bool visible = removeIndices[0].data(PaletteTreeModel::VisibleRole).toBool();
    const bool isCell = bool(removeIndices[0].data(PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>());

    RemoveAction action = RemoveAction::AutoAction;

    if (isCell) {
        if (visible) {
            std::string question = customCount == 1
                                   ? mu::trc("palette", "Do you want to hide this custom palette cell or permanently delete it?")
                                   : mu::trc("palette", "Do you want to hide these custom palette cells or permanently delete them?");

            showHideOrDeleteDialog(question,  [=](RemoveAction action) { remove(removeIndices, action); });
            return;
        } else {
            std::string question = customCount == 1
                                   ? mu::trc("palette", "Do you want to permanently delete this custom palette cell?")
                                   : mu::trc("palette", "Do you want to permanently delete these custom palette cells?");

            IInteractive::Button button = interactive()->question(std::string(), question, {
                    IInteractive::Button::Yes,
                    IInteractive::Button::No
                });

            if (button == IInteractive::Button::Yes) {
                remove(removeIndices, RemoveAction::DeletePermanently);
            }

            return;
        }
    } else {
        if (visible) {
            std::string question = customCount == 1
                                   ? mu::trc("palette", "Do you want to hide this custom palette or permanently delete it?")
                                   : mu::trc("palette", "Do you want to hide these custom palettes or permanently delete them?");

            showHideOrDeleteDialog(question,  [=](RemoveAction action) { remove(removeIndices, action); });
            return;
        } else {
            action = RemoveAction::Hide;
        }
    }

    remove(removeIndices, action);
}

//---------------------------------------------------------
//   UserPaletteController::remove
//---------------------------------------------------------

void UserPaletteController::remove(const QModelIndexList& unsortedRemoveIndices,
                                   AbstractPaletteController::RemoveAction action)
{
    using RemoveAction = AbstractPaletteController::RemoveAction;

    if (action == RemoveAction::NoAction) {
        return;
    }

    QModelIndexList removeIndices = unsortedRemoveIndices;
    std::sort(removeIndices.begin(), removeIndices.end(), [](const QModelIndex& a, const QModelIndex& b) {
        return a.row() < b.row();
    });

    // remove in reversed order to leave the previous model indices in the list valid
    for (auto i = removeIndices.rbegin(); i != removeIndices.rend(); ++i) {
        const QModelIndex& index = *i;

        RemoveAction indexAction = action;
        const bool custom = model()->data(index, PaletteTreeModel::CustomRole).toBool();

        if (!custom || indexAction == RemoveAction::AutoAction) {
            const bool isCell
                = bool(model()->data(index, PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>());
            indexAction
                = custom ? RemoveAction::NoAction : (isCell ? RemoveAction::DeletePermanently : RemoveAction::Hide);
        }

        switch (indexAction) {
        case RemoveAction::NoAction:
            break;
        case RemoveAction::Hide:
            model()->setData(index, false, PaletteTreeModel::VisibleRole);
            break;
        case RemoveAction::DeletePermanently:
            model()->removeRow(index.row(), index.parent());
            break;
        case RemoveAction::AutoAction:
            // impossible, we have just assigned another action for that case
            Q_ASSERT(false);
            break;
        }
    }
}

//---------------------------------------------------------
//   UserPaletteController::remove
//---------------------------------------------------------

void UserPaletteController::remove(const QModelIndex& index)
{
    if (!canEdit(index.parent())) {
        return;
    }

    const bool customItem = index.data(PaletteTreeModel::CustomRole).toBool();
    queryRemove({ index }, customItem ? 1 : 0);
}

//---------------------------------------------------------
//   UserPaletteController::removeSelection
//---------------------------------------------------------

void UserPaletteController::removeSelection(const QModelIndexList& selectedIndexes, const QModelIndex& parent)
{
    if (!canEdit(parent)) {
        return;
    }

    QModelIndexList removeIndices;
    int customItemsCount = 0;

    for (const QModelIndex& idx : selectedIndexes) {
        if (idx.parent() == parent) {
            removeIndices.push_back(idx);
            const bool custom = idx.data(PaletteTreeModel::CustomRole).toBool();
            if (custom) {
                ++customItemsCount;
            }
        }
    }

    queryRemove(removeIndices, customItemsCount);
}

//---------------------------------------------------------
//   UserPaletteController::editPaletteProperties
//---------------------------------------------------------

void UserPaletteController::editPaletteProperties(const QModelIndex& index)
{
    if (!canEdit(index)) {
        return;
    }

    QModelIndex srcIndex = convertProxyIndex(index, _userPalette);
    PalettePanel* panel = _userPalette->findPalettePanel(srcIndex);
    if (!panel) {
        return;
    }

    configuration()->paletteConfig(panel->id()).ch.onReceive(this,
                                                             [this, srcIndex, panel](const IPaletteConfiguration::PaletteConfig& config) {
        panel->setName(config.name);
        panel->setGrid(config.size);
        panel->setMag(config.scale);
        panel->setYOffset(config.elementOffset);
        panel->setDrawGrid(config.showGrid);
        _userPalette->itemDataChanged(srcIndex);
    });

    QVariantMap properties;
    properties["paletteId"] = panel->id();
    properties["name"] = panel->translatedName();
    properties["cellWidth"] = panel->gridSize().width();
    properties["cellHeight"] = panel->gridSize().height();
    properties["scale"] = panel->mag();
    properties["elementOffset"] = panel->yOffset();
    properties["showGrid"] = panel->drawGrid();

    QJsonDocument document = QJsonDocument::fromVariant(properties);
    QString uri = QString("musescore://palette/properties?sync=true&properties=%1")
                  .arg(QString(document.toJson()));

    interactive()->open(uri.toStdString());
}

//---------------------------------------------------------
//   UserPaletteController::editCellProperties
//---------------------------------------------------------

void UserPaletteController::editCellProperties(const QModelIndex& index)
{
    if (!canEdit(index)) {
        return;
    }

    QModelIndex srcIndex = convertProxyIndex(index, _userPalette);
    PaletteCellPtr cell = _userPalette->findCell(srcIndex);
    if (!cell) {
        return;
    }

    configuration()->paletteCellConfig(cell->id).ch.onReceive(this,
                                                              [this, srcIndex, cell](
                                                                  const IPaletteConfiguration::PaletteCellConfig& config) {
        cell->name = config.name;
        cell->mag = config.scale;
        cell->drawStaff = config.drawStaff;
        cell->xoffset = config.xOffset;
        cell->yoffset = config.yOffset;
        _userPalette->itemDataChanged(srcIndex);
    });

    QVariantMap properties;
    properties["cellId"] = cell->id;
    properties["name"] = cell->translatedName();
    properties["xOffset"] = cell->xoffset;
    properties["yOffset"] = cell->yoffset;
    properties["scale"] = cell->mag;
    properties["drawStaff"] = cell->drawStaff;

    QJsonDocument document = QJsonDocument::fromVariant(properties);
    QString uri = QString("musescore://palette/cellproperties?sync=true&properties=%1")
                  .arg(QString(document.toJson()));

    interactive()->open(uri.toStdString());
}

//---------------------------------------------------------
//   UserPaletteController::canEdit
//---------------------------------------------------------

bool UserPaletteController::canEdit(const QModelIndex& index) const
{
    if (!userEditable()) {
        return false;
    }

    return model()->data(index, PaletteTreeModel::EditableRole).toBool();
}

//---------------------------------------------------------
//   UserPaletteController::applyPaletteElement
//---------------------------------------------------------

bool UserPaletteController::applyPaletteElement(const QModelIndex& index, Qt::KeyboardModifiers modifiers)
{
    const PaletteCell* cell = model()->data(index, PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>();
    if (cell && cell->element) {
        return Palette::applyPaletteElement(cell->element, modifiers);
    }
    return false;
}

//---------------------------------------------------------
//   PaletteWorkspace
//---------------------------------------------------------

PaletteWorkspace::PaletteWorkspace(PaletteTreeModel* user, PaletteTreeModel* master, QObject* parent)
    : QObject(parent), userPalette(user), masterPalette(master), defaultPalette(nullptr)
{
    if (userPalette) {
        connect(userPalette, &PaletteTreeModel::treeChanged, this, &PaletteWorkspace::userPaletteChanged);
    }

    m_searchFilterModel = new PaletteCellFilterProxyModel(this);
    m_searchFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_searchFilterModel->setSourceModel(userPalette);

    m_visibilityFilterModel = new QSortFilterProxyModel(this);
    m_visibilityFilterModel->setFilterRole(PaletteTreeModel::VisibleRole);
    m_visibilityFilterModel->setFilterFixedString("true");
    m_visibilityFilterModel->setSourceModel(userPalette);
}

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteModel
//---------------------------------------------------------
void PaletteWorkspace::setSearching(bool searching)
{
    if (m_searching == searching) {
        return;
    }

    m_searching = searching;

    mainPalette = nullptr;
    emit mainPaletteChanged();
}

QAbstractItemModel* PaletteWorkspace::mainPaletteModel()
{
    if (m_searching) {
        mainPalette = m_searchFilterModel;
    } else {
        mainPalette = m_visibilityFilterModel;
    }

    return mainPalette;
}

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteModel
//---------------------------------------------------------

AbstractPaletteController* PaletteWorkspace::getMainPaletteController()
{
    if (!mainPaletteController) {
        mainPaletteController = new UserPaletteController(mainPaletteModel(), userPalette, this);
    }
//             mainPaletteController = new PaletteController(mainPaletteModel(), this, this);
    return mainPaletteController;
}

//---------------------------------------------------------
//   PaletteWorkspace::customPaletteIndex
//---------------------------------------------------------

QModelIndex PaletteWorkspace::customElementsPaletteIndex(const QModelIndex& index)
{
    const QAbstractItemModel* model = customElementsPaletteModel();
    if (index.model() == mainPalette && index.parent() == QModelIndex()) {
        const int row = convertProxyIndex(index, userPalette).row();
        return model->index(row, 0);
    }
    return convertIndex(index, model);
}

//---------------------------------------------------------
//   PaletteWorkspace::customElementsPaletteModel
//---------------------------------------------------------

FilterPaletteTreeModel* PaletteWorkspace::customElementsPaletteModel()
{
    if (!customPoolPalette) {
        PaletteCellFilter* filter = new VisibilityCellFilter(/* acceptedValue */ false);
        filter->addChainedFilter(new CustomizedCellFilter(/* acceptedValue */ true));
        customPoolPalette = new FilterPaletteTreeModel(filter, userPalette, this);
    }
    return customPoolPalette;
}

//---------------------------------------------------------
//   PaletteWorkspace::getCustomElementsPaletteController
//---------------------------------------------------------

AbstractPaletteController* PaletteWorkspace::getCustomElementsPaletteController()
{
    if (!customElementsPaletteController) {
        customElementsPaletteController = new UserPaletteController(customElementsPaletteModel(), userPalette, this);
        customElementsPaletteController->setCustom(true);
    }

    return customElementsPaletteController;
}

//---------------------------------------------------------
//   PaletteWorkspace::poolPaletteIndex
//---------------------------------------------------------

QModelIndex PaletteWorkspace::poolPaletteIndex(const QModelIndex& index, Ms::FilterPaletteTreeModel* poolPalette)
{
    const QModelIndex poolPaletteIndex = convertIndex(index, poolPalette);
    if (poolPaletteIndex.isValid()) {
        return poolPaletteIndex;
    }
    const auto contentType = index.data(PaletteTreeModel::PaletteContentTypeRole).value<PalettePanel::Type>();
    return findPaletteIndex(poolPalette, contentType);
}

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteModel
//---------------------------------------------------------

FilterPaletteTreeModel* PaletteWorkspace::poolPaletteModel(const QModelIndex& index)
{
    const QModelIndex filterIndex = convertProxyIndex(index, userPalette);
    PaletteCellFilter* filter = userPalette->getFilter(filterIndex);   // TODO: or mainPalette?
    if (!filter) {
        return nullptr;
    }

    FilterPaletteTreeModel* m = new FilterPaletteTreeModel(filter, masterPalette);
    QQmlEngine::setObjectOwnership(m, QQmlEngine::JavaScriptOwnership);
    return m;
}

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteController
//---------------------------------------------------------

AbstractPaletteController* PaletteWorkspace::poolPaletteController(FilterPaletteTreeModel* poolPaletteModel,
                                                                   const QModelIndex& rootIndex)
{
    Q_UNUSED(rootIndex);
    UserPaletteController* c = new UserPaletteController(poolPaletteModel, userPalette);
    c->setVisible(false);
    c->setCustom(false);
    c->setUserEditable(false);
//       AbstractPaletteController* c = new MasterPaletteController(poolPaletteModel, userPalette, this);
    QQmlEngine::setObjectOwnership(c, QQmlEngine::JavaScriptOwnership);
    return c;
}

//---------------------------------------------------------
//   PaletteWorkspace::availableExtraPalettePanelsModel
//---------------------------------------------------------

QAbstractItemModel* PaletteWorkspace::availableExtraPalettesModel()
{
    QStandardItemModel* m = new QStandardItemModel;

    {
        auto roleNames = m->roleNames();
        roleNames[CustomRole] = "custom";
        roleNames[PaletteIndexRole] = "paletteIndex";
        m->setItemRoleNames(roleNames);
    }

    QStandardItem* root = m->invisibleRootItem();

    const int masterRows = masterPalette->rowCount();
    for (int row = 0; row < masterRows; ++row) {
        const QModelIndex idx = masterPalette->index(row, 0);
        // add everything that cannot be found in user palette
        if (!convertIndex(idx, userPalette).isValid()) {
            const QString name = masterPalette->data(idx, Qt::DisplayRole).toString();
            QStandardItem* item = new QStandardItem(name);
            item->setData(false, CustomRole);       // this palette is from master palette, hence not custom
            item->setData(QPersistentModelIndex(idx), PaletteIndexRole);
            root->appendRow(item);
        }
    }

    const int userRows = userPalette->rowCount();
    for (int row = 0; row < userRows; ++row) {
        const QModelIndex idx = userPalette->index(row, 0);
        // add invisible custom palettes
        const bool visible = userPalette->data(idx, PaletteTreeModel::VisibleRole).toBool();
        const bool custom = userPalette->data(idx, PaletteTreeModel::CustomRole).toBool();
        if (!visible) {
            const QString name = userPalette->data(idx, Qt::DisplayRole).toString();
            QStandardItem* item = new QStandardItem(name);
            item->setData(custom, CustomRole);
            item->setData(QPersistentModelIndex(idx), PaletteIndexRole);
            root->appendRow(item);
        }
    }

    QQmlEngine::setObjectOwnership(m, QQmlEngine::JavaScriptOwnership);
    return m;
}

//---------------------------------------------------------
//   PaletteWorkspace::addPalette
//---------------------------------------------------------

bool PaletteWorkspace::addPalette(const QPersistentModelIndex& index)
{
    if (!index.isValid()) {
        return false;
    }

    if (index.model() == userPalette) {
        const bool ok = userPalette->setData(index, true, PaletteTreeModel::VisibleRole);
        if (!ok) {
            return false;
        }
        const QModelIndex parent = index.parent();
        userPalette->moveRow(parent, index.row(), parent, 0);
        return true;
    }

    if (index.model() == masterPalette) {
        QMimeData* data = masterPalette->mimeData({ QModelIndex(index) });
        const bool success = userPalette->dropMimeData(data, Qt::CopyAction, 0, 0, QModelIndex());
        data->deleteLater();
        return success;
    }

    return false;
}

//---------------------------------------------------------
//   PaletteWorkspace::removeCustomPalette
//---------------------------------------------------------

bool PaletteWorkspace::removeCustomPalette(const QPersistentModelIndex& index)
{
    if (!index.isValid()) {
        return false;
    }

    if (index.model() == userPalette) {
        const bool custom = index.data(PaletteTreeModel::CustomRole).toBool();
        if (!custom) {
            return false;
        }

        IInteractive::Button button
            = interactive()->question("", mu::trc("palette", "Do you want to permanently delete this custom palette?"), {
                IInteractive::Button::Yes, IInteractive::Button::No
            });

        if (button == IInteractive::Button::Yes) {
            return userPalette->removeRow(index.row(), index.parent());
        }

        return false;
    }

    return false;
}

//---------------------------------------------------------
//   PaletteWorkspace::resetPalette
//---------------------------------------------------------

bool PaletteWorkspace::resetPalette(const QModelIndex& index)
{
    if (!index.isValid()) {
        return false;
    }

    IInteractive::Button button
        = interactive()->question("", mu::trc("palette",
                                              "Do you want to restore this palette to its default state? All changes to this palette will be lost."), {
            IInteractive::Button::Yes, IInteractive::Button::No
        });
    if (button != IInteractive::Button::Yes) {
        return false;
    }

    Q_ASSERT(defaultPalette != userPalette);

    QAbstractItemModel* resetModel = nullptr;
    QModelIndex resetIndex;

    if (defaultPalette) {
        resetModel = defaultPalette;
        resetIndex = convertIndex(index, defaultPalette);
    }

    if (!resetIndex.isValid()) {
        resetModel = masterPalette;
        resetIndex = convertIndex(index, masterPalette);
    }

    const QModelIndex userPaletteIndex = convertProxyIndex(index, userPalette);
    const QModelIndex parent = userPaletteIndex.parent();
    const int row = userPaletteIndex.row();
    const int column = userPaletteIndex.column();

    // restore visibility and expanded state of the palette after restoring its state
    const bool wasVisible = index.data(PaletteTreeModel::VisibleRole).toBool();
    const bool wasExpanded = index.data(PaletteTreeModel::PaletteExpandedRole).toBool();

    if (!userPalette->removeRow(row, parent)) {
        return false;
    }

    if (resetIndex.isValid()) {
        QMimeData* data = resetModel->mimeData({ resetIndex });
        userPalette->dropMimeData(data, Qt::CopyAction, row, column, parent);
        data->deleteLater();
    } else {
        userPalette->insertRow(row, parent);
    }

    const QModelIndex newIndex = userPalette->index(row, column, parent);
    userPalette->setData(newIndex, wasVisible, PaletteTreeModel::VisibleRole);
    userPalette->setData(newIndex, wasExpanded, PaletteTreeModel::PaletteExpandedRole);

    return true;
}

QString PaletteWorkspace::getPaletteFilename(bool open, const QString& name)
{
    QString title;
    QString filter;
#if defined(WIN_PORTABLE)
    QString wd = QDir::cleanPath(QString("%1/../../../Data/settings").arg(QCoreApplication::applicationDirPath())
                                 .arg(QCoreApplication::applicationName()));
#else
    QString wd = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
                 .arg(QCoreApplication::applicationName());
#endif
    if (open) {
        title  = mu::qtrc("palette", "Load Palette");
        filter = mu::qtrc("palette", "MuseScore Palette") + " (*.mpal)";
    } else {
        title  = mu::qtrc("palette", "Save Palette");
        filter = mu::qtrc("palette", "MuseScore Palette") + " (*.mpal)";
    }

    QFileInfo myPalettes(wd);
    QString defaultPath = myPalettes.absoluteFilePath();
    if (!name.isEmpty()) {
        QString fname = mu::io::escapeFileName(name).toQString();
        QFileInfo myName(fname);
        if (myName.isRelative()) {
            myName.setFile(defaultPath, fname);
        }
        defaultPath = myName.absoluteFilePath();
    }

    mu::io::path fn;
    if (open) {
        fn = interactive()->selectOpeningFile(title, defaultPath, filter);
    } else {
        fn = interactive()->selectSavingFile(title, defaultPath, filter);
    }
    return fn.toQString();
}

//---------------------------------------------------------
//   PaletteWorkspace::savePalette
//---------------------------------------------------------

bool PaletteWorkspace::savePalette(const QModelIndex& index)
{
    const QModelIndex srcIndex = convertProxyIndex(index, userPalette);
    const PalettePanel* pp = userPalette->findPalettePanel(srcIndex);
    if (!pp) {
        return false;
    }

    const QString path = getPaletteFilename(/* load? */ false, pp->translatedName());

    if (path.isEmpty()) {
        return false;
    }
    return pp->writeToFile(path);
}

//---------------------------------------------------------
//   PaletteWorkspace::loadPalette
//---------------------------------------------------------

bool PaletteWorkspace::loadPalette(const QModelIndex& index)
{
    const QString path = getPaletteFilename(/* load? */ true);
    if (path.isEmpty()) {
        return false;
    }

    PalettePanelPtr pp = std::make_shared<PalettePanel>();
    if (!pp->readFromFile(path)) {
        return false;
    }
    pp->setType(PalettePanel::Type::Custom);   // mark the loaded palette custom

    const QModelIndex srcIndex = convertProxyIndex(index, userPalette);

    const int row = srcIndex.row();
    const QModelIndex parent = srcIndex.parent();

    return userPalette->insertPalettePanel(pp, row, parent);
}

//---------------------------------------------------------
//   PaletteWorkspace::setUserPaletteTree
//---------------------------------------------------------

void PaletteWorkspace::setUserPaletteTree(PaletteTreePtr tree)
{
    if (userPalette) {
        disconnect(userPalette, &PaletteTreeModel::treeChanged, this, &PaletteWorkspace::userPaletteChanged);
        userPalette->setPaletteTree(tree);
        connect(userPalette, &PaletteTreeModel::treeChanged, this, &PaletteWorkspace::userPaletteChanged);
    } else {
        userPalette = new PaletteTreeModel(tree, /* parent */ this);
        connect(userPalette, &PaletteTreeModel::treeChanged, this, &PaletteWorkspace::userPaletteChanged);
    }
}

void PaletteWorkspace::setDefaultPaletteTree(PaletteTreePtr tree)
{
    if (defaultPalette) {
        defaultPalette->setPaletteTree(tree);
    } else {
        defaultPalette = new PaletteTreeModel(tree, /* parent */ this);
    }
}

//---------------------------------------------------------
//   PaletteWorkspace::write
//---------------------------------------------------------

void PaletteWorkspace::write(XmlWriter& xml) const
{
    if (!userPalette) {
        return;
    }
    if (const PaletteTree* tree = userPalette->paletteTree()) {
        tree->write(xml);
    }
}

//---------------------------------------------------------
//   PaletteWorkspace::read
//---------------------------------------------------------

bool PaletteWorkspace::read(XmlReader& e)
{
    PaletteTreePtr tree = std::make_shared<PaletteTree>();
    if (!tree->read(e)) {
        return false;
    }

    setUserPaletteTree(tree);

    return true;
}
} // namespace Ms
