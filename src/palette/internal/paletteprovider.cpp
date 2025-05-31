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

#include "paletteprovider.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QQmlEngine>
#include <QMimeData>
#include <QStandardPaths>

#include "engraving/dom/keysig.h"
#include "engraving/dom/timesig.h"

#include "palettecreator.h"
#include "view/widgets/keyedit.h"
#include "view/widgets/timedialog.h"

#include "io/path.h"
#include "commonscene/commonscenetypes.h"

#include "translation.h"
#include "types/uri.h"

#include "app_config.h"

using namespace muse;
using namespace mu::palette;
using namespace mu::engraving;

// ========================================================
// PaletteElementEditor
// ========================================================

bool PaletteElementEditor::valid() const
{
    using Type = Palette::Type;
    switch (_type) {
    case Type::KeySig:
    case Type::TimeSig:
        return true;
    default:
        break;
    }
    return false;
}

QString PaletteElementEditor::actionName() const
{
    using Type = Palette::Type;
    switch (_type) {
    case Type::KeySig:
        return muse::qtrc("palette", "Create key signature");
    case Type::TimeSig:
        return muse::qtrc("palette", "Create time signature");
    default:
        break;
    }
    return QString();
}

void PaletteElementEditor::setPaletteIndex(QPersistentModelIndex paletteIndex)
{
    _paletteIndex = paletteIndex;
}

void PaletteElementEditor::onElementAdded(const ElementPtr element)
{
    if (!element) {
        return;
    }

    static const QMap<ElementType, Palette::Type> elementTypeToPaletteType {
        { ElementType::TIMESIG, Palette::Type::TimeSig },
        { ElementType::KEYSIG, Palette::Type::KeySig }
    };

    Palette::Type paletteType = elementTypeToPaletteType.value(element->type(), Palette::Type::Unknown);

    if (paletteType != _type) {
        return;
    }

    if (!_paletteIndex.isValid()
        || !_paletteIndex.data(PaletteTreeModel::VisibleRole).toBool()) {
        interactive()->info("", muse::trc("palette", "The palette was hidden or changed"));
        return;
    }

    QVariantMap mimeData;
    mimeData[mu::commonscene::MIME_SYMBOL_FORMAT] = element->mimeData().toQByteArray();

    _controller->insert(_paletteIndex, -1, mimeData, Qt::CopyAction);
}

void PaletteElementEditor::open()
{
    if (!_paletteIndex.isValid()) {
        return;
    }

    muse::UriQuery uri;

    using Type = Palette::Type;
    switch (_type) {
    case Type::KeySig: {
        uri = muse::UriQuery("musescore://notation/keysignatures");
        uri.addParam("showKeyPalette", Val(false));
    }
    break;
    case Type::TimeSig: {
        uri = muse::UriQuery("musescore://notation/timesignatures");
        uri.addParam("showTimePalette", Val(false));
    }
    break;
    default:
        break;
    }

    if (uri.isValid()) {
        uri.addParam("sync", Val(false));

        paletteProvider()->addCustomItemRequested().onReceive(this, [this](ElementPtr item) {
            onElementAdded(item);
        });

        if (interactive()->isOpened(uri).val) {
            interactive()->raise(uri);
        } else {
            interactive()->open(uri);
        }
    }
}

// ========================================================
// Model indices
// ========================================================

static QModelIndex findPaletteIndex(const QAbstractItemModel* model, Palette::Type type)
{
    constexpr int role = PaletteTreeModel::PaletteTypeRole;
    const QModelIndex start = model->index(0, 0);
    const QModelIndexList foundIndexList = model->match(start, role, QVariant::fromValue(type));

    if (!foundIndexList.empty()) {
        return foundIndexList[0];
    }
    return QModelIndex();
}

static QModelIndex convertIndex(const QModelIndex& index, const QAbstractItemModel* targetModel)
{
    if (index.model() == targetModel || !index.isValid()) {
        return index;
    }

    constexpr int typeRole = PaletteTreeModel::PaletteTypeRole;
    const auto type = index.model()->data(index, typeRole).value<Palette::Type>();

    return findPaletteIndex(targetModel, type);
}

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

// ========================================================
// AbstractPaletteController
// ========================================================
PaletteElementEditor* AbstractPaletteController::elementEditor(const QModelIndex& paletteIndex)
{
    Palette::Type paletteType = paletteIndex.data(PaletteTreeModel::PaletteTypeRole).value<Palette::Type>();

    if (m_paletteElementEditorMap.contains(paletteType)) {
        PaletteElementEditor* ed = m_paletteElementEditorMap[paletteType];
        ed->setPaletteIndex(paletteIndex);
        return ed;
    }

    PaletteElementEditor* ed = new PaletteElementEditor(this, paletteIndex,
                                                        paletteIndex.data(PaletteTreeModel::PaletteTypeRole).value<Palette::Type>(), this);

    m_paletteElementEditorMap.insert(paletteType, ed);
    return ed;
}

// ========================================================
//   UserPaletteController
// ========================================================

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
        const auto cell = PaletteCell::fromMimeData(mimeData[PaletteCell::mimeDataFormat].toByteArray());
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
        cell = PaletteCell::fromMimeData(mimeData[PaletteCell::mimeDataFormat].toByteArray());

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
        cell = PaletteCell::fromElementMimeData(mimeData[mu::commonscene::MIME_SYMBOL_FORMAT].toByteArray());
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
    data.setData(PaletteCell::mimeDataFormat, cell->toMimeData());
    constexpr int column = 0;
    return model()->dropMimeData(&data, action, row, column, parent);
}

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

void UserPaletteController::showHideOrDeleteDialog(const std::string& question,
                                                   std::function<void(AbstractPaletteController::RemoveAction)> resultHandler) const
{
    int hideButton = int(IInteractive::Button::CustomButton) + 1;
    int deleteButton = hideButton + 1;

    auto result = interactive()->question(std::string(), question, {
        IInteractive::ButtonData(hideButton, muse::trc("palette", "Hide")),
        IInteractive::ButtonData(deleteButton, muse::trc("palette", "Delete permanently")),
        interactive()->buttonData(IInteractive::Button::Cancel)
    });

    result.onResolve(this, [deleteButton, hideButton, resultHandler](const IInteractive::Result& res) {
        RemoveAction action = RemoveAction::NoAction;
        if (res.isButton(deleteButton)) {
            action = RemoveAction::DeletePermanently;
        } else if (res.isButton(hideButton)) {
            action = RemoveAction::Hide;
        }

        resultHandler(action);
    });
}

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
                                   ? muse::trc("palette", "Do you want to hide this custom palette cell or permanently delete it?")
                                   : muse::trc("palette", "Do you want to hide these custom palette cells or permanently delete them?");

            showHideOrDeleteDialog(question,  [=](RemoveAction action) { remove(removeIndices, action); });
            return;
        } else {
            std::string question = customCount == 1
                                   ? muse::trc("palette", "Do you want to permanently delete this custom palette cell?")
                                   : muse::trc("palette", "Do you want to permanently delete these custom palette cells?");

            interactive()->question(std::string(), question, {
                IInteractive::Button::Yes,
                IInteractive::Button::No
            })
            .onResolve(this, [this, removeIndices](const IInteractive::Result& res) {
                if (res.isButton(IInteractive::Button::Yes)) {
                    remove(removeIndices, RemoveAction::DeletePermanently);
                }
            });
        }
    } else {
        if (visible) {
            std::string question = customCount == 1
                                   ? muse::trc("palette", "Do you want to hide this custom palette or permanently delete it?")
                                   : muse::trc("palette", "Do you want to hide these custom palettes or permanently delete them?");

            showHideOrDeleteDialog(question,  [=](RemoveAction action) { remove(removeIndices, action); });
            return;
        } else {
            action = RemoveAction::Hide;
        }

        remove(removeIndices, action);
    }
}

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

void UserPaletteController::remove(const QModelIndex& index)
{
    if (!canEdit(index.parent())) {
        return;
    }

    const bool customItem = index.data(PaletteTreeModel::CustomRole).toBool();
    queryRemove({ index }, customItem ? 1 : 0);
}

void UserPaletteController::removeSelection(const QModelIndexList& selectedIndexes, const QModelIndex& parent)
{
    if (!canEdit(parent)) {
        return;
    }

    QModelIndexList removeIndices;
    int customItemsCount = 0;

    for (const QModelIndex& idx : selectedIndexes) {
        if (idx.parent() == parent || !parent.isValid()) {
            removeIndices.push_back(idx);
            const bool custom = idx.data(PaletteTreeModel::CustomRole).toBool();
            if (custom) {
                ++customItemsCount;
            }
        }
    }

    queryRemove(removeIndices, customItemsCount);
}

void UserPaletteController::editPaletteProperties(const QModelIndex& index)
{
    if (!canEdit(index)) {
        return;
    }

    QModelIndex srcIndex = convertProxyIndex(index, _userPalette);
    Palette* palette = _userPalette->findPalette(srcIndex);
    if (!palette) {
        return;
    }

    configuration()->paletteConfig(palette->id()).ch.onReceive(this,
                                                               [this, srcIndex, palette](
                                                                   const IPaletteConfiguration::PaletteConfig& config) {
        palette->setName(config.name);
        palette->setGridSize(config.size);
        palette->setMag(config.scale);
        palette->setYOffset(config.elementOffset);
        palette->setDrawGrid(config.showGrid);
        _userPalette->itemDataChanged(srcIndex);
    });

    QVariantMap properties;
    properties["paletteId"] = palette->id();
    properties["name"] = QString(palette->translatedName().toUtf8().toPercentEncoding());
    properties["cellWidth"] = palette->gridSize().width();
    properties["cellHeight"] = palette->gridSize().height();
    properties["scale"] = palette->mag();
    properties["elementOffset"] = palette->yOffset();
    properties["showGrid"] = palette->drawGrid();

    QJsonDocument document = QJsonDocument::fromVariant(properties);
    QString uri = QString("musescore://palette/properties?sync=true&properties=%1")
                  .arg(QString(document.toJson()));

    interactive()->open(uri.toStdString());
}

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

bool UserPaletteController::canEdit(const QModelIndex& index) const
{
    if (!userEditable()) {
        return false;
    }

    return model()->data(index, PaletteTreeModel::EditableRole).toBool();
}

bool UserPaletteController::applyPaletteElement(const QModelIndex& index, Qt::KeyboardModifiers modifiers)
{
    const PaletteCell* cell = model()->data(index, PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>();
    if (!cell || !cell->element) {
        return false;
    }

    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    return notation->interaction()->applyPaletteElement(cell->element.get(), modifiers);
}

// ========================================================
// PaletteProvider
// ========================================================

void PaletteProvider::init()
{
    m_userPaletteModel = new PaletteTreeModel(std::make_shared<PaletteTree>(), this);
    connect(m_userPaletteModel, &PaletteTreeModel::treeChanged, this, &PaletteProvider::notifyAboutUserPaletteChanged);

    m_masterPaletteModel = new PaletteTreeModel(PaletteCreator::newMasterPaletteTree());
    m_masterPaletteModel->setParent(this);

    m_searchFilterModel = new PaletteCellFilterProxyModel(this);
    m_searchFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_visibilityFilterModel = new QSortFilterProxyModel(this);
    m_visibilityFilterModel->setFilterRole(PaletteTreeModel::VisibleRole);
    m_visibilityFilterModel->setFilterFixedString("true");
    m_visibilityFilterModel->setSourceModel(m_userPaletteModel);

    configuration()->isSinglePalette().ch.onReceive(this, [this](bool) {
        emit isSinglePaletteChanged();
    });

    configuration()->isSingleClickToOpenPalette().ch.onReceive(this, [this](bool) {
        emit isSingleClickToOpenPaletteChanged();
    });

    configuration()->isPaletteDragEnabled().ch.onReceive(this, [this](bool) {
        emit isPaletteDragEnabledChanged();
    });
}

void PaletteProvider::setFilter(const QString& filter)
{
    // Unbind the model when there is no search text so as to return no results
    // and thus speed up the opening of the palette search. Rebind the model
    // as soon as the search text is non-empty.
    // Doing this *trick* also when the search text is non-empty helps
    // speed up the search in certain other scenarios,
    // e.g. when deleting search characters (going from fewer to more search results).
    m_searchFilterModel->setSourceModel(nullptr);
    m_searchFilterModel->setFilterFixedString(filter);
    if (!filter.isEmpty()) {
        m_searchFilterModel->setSourceModel(m_masterPaletteModel);
    }
}

void PaletteProvider::setSearching(bool searching)
{
    if (m_isSearching == searching) {
        return;
    }

    m_isSearching = searching;

    m_mainPalette = nullptr;
    m_mainPaletteController = nullptr;
    emit mainPaletteChanged();
}

bool PaletteProvider::isSinglePalette() const
{
    return configuration()->isSinglePalette().val;
}

bool PaletteProvider::isSingleClickToOpenPalette() const
{
    return configuration()->isSingleClickToOpenPalette().val;
}

bool PaletteProvider::isPaletteDragEnabled() const
{
    return configuration()->isPaletteDragEnabled().val;
}

QAbstractItemModel* PaletteProvider::mainPaletteModel()
{
    if (m_isSearching) {
        m_mainPalette = m_searchFilterModel;
    } else {
        m_mainPalette = m_visibilityFilterModel;
    }

    return m_mainPalette;
}

AbstractPaletteController* PaletteProvider::mainPaletteController()
{
    if (!m_mainPaletteController) {
        m_mainPaletteController = new UserPaletteController(mainPaletteModel(), m_userPaletteModel, this);
    }
    return m_mainPaletteController;
}

QModelIndex PaletteProvider::customElementsPaletteIndex(const QModelIndex& index)
{
    const QAbstractItemModel* model = customElementsPaletteModel();
    if (index.model() == m_mainPalette && index.parent() == QModelIndex()) {
        const int row = convertProxyIndex(index, m_userPaletteModel).row();
        return model->index(row, 0);
    }
    return convertIndex(index, model);
}

FilterPaletteTreeModel* PaletteProvider::customElementsPaletteModel()
{
    if (!m_customPoolPalette) {
        PaletteCellFilter* filter = new VisibilityCellFilter(/* acceptedValue */ false);
        filter->addChainedFilter(new CustomizedCellFilter(/* acceptedValue */ true));
        m_customPoolPalette = new FilterPaletteTreeModel(filter, m_userPaletteModel, this);
    }
    return m_customPoolPalette;
}

AbstractPaletteController* PaletteProvider::customElementsPaletteController()
{
    if (!m_customElementsPaletteController) {
        m_customElementsPaletteController = new UserPaletteController(customElementsPaletteModel(), m_userPaletteModel, this);
        m_customElementsPaletteController->setCustom(true);
    }

    return m_customElementsPaletteController;
}

QModelIndex PaletteProvider::poolPaletteIndex(const QModelIndex& index, FilterPaletteTreeModel* poolPalette) const
{
    const QModelIndex poolPaletteIndex = convertIndex(index, poolPalette);
    if (poolPaletteIndex.isValid()) {
        return poolPaletteIndex;
    }
    const auto contentType = index.data(PaletteTreeModel::PaletteContentTypeRole).value<Palette::Type>();
    return findPaletteIndex(poolPalette, contentType);
}

FilterPaletteTreeModel* PaletteProvider::poolPaletteModel(const QModelIndex& index) const
{
    const QModelIndex filterIndex = convertProxyIndex(index, m_userPaletteModel);
    PaletteCellFilter* filter = m_userPaletteModel->getFilter(filterIndex);
    if (!filter) {
        return nullptr;
    }

    FilterPaletteTreeModel* m = new FilterPaletteTreeModel(filter, m_masterPaletteModel);
    QQmlEngine::setObjectOwnership(m, QQmlEngine::JavaScriptOwnership);
    return m;
}

AbstractPaletteController* PaletteProvider::poolPaletteController(FilterPaletteTreeModel* poolPaletteModel,
                                                                  const QModelIndex& rootIndex) const
{
    Q_UNUSED(rootIndex);
    UserPaletteController* c = new UserPaletteController(poolPaletteModel, m_userPaletteModel);
    c->setVisible(false);
    c->setCustom(false);
    c->setUserEditable(false);
//       AbstractPaletteController* c = new MasterPaletteController(poolPaletteModel, userPalette, this);
    QQmlEngine::setObjectOwnership(c, QQmlEngine::JavaScriptOwnership);
    return c;
}

QAbstractItemModel* PaletteProvider::availableExtraPalettesModel() const
{
    QStandardItemModel* m = new QStandardItemModel;

    {
        auto roleNames = m->roleNames();
        roleNames[CustomRole] = "custom";
        roleNames[PaletteIndexRole] = "paletteIndex";
        m->setItemRoleNames(roleNames);
    }

    QStandardItem* root = m->invisibleRootItem();

    const int masterRows = m_masterPaletteModel->rowCount();
    for (int row = 0; row < masterRows; ++row) {
        const QModelIndex idx = m_masterPaletteModel->index(row, 0);
        // add everything that cannot be found in user palette
        if (!convertIndex(idx, m_userPaletteModel).isValid()) {
            const QString name = m_masterPaletteModel->data(idx, Qt::DisplayRole).toString();
            QStandardItem* item = new QStandardItem(name);
            item->setData(false, CustomRole);       // this palette is from master palette, hence not custom
            item->setData(QPersistentModelIndex(idx), PaletteIndexRole);
            root->appendRow(item);
        }
    }

    const int userRows = m_userPaletteModel->rowCount();
    for (int row = 0; row < userRows; ++row) {
        const QModelIndex idx = m_userPaletteModel->index(row, 0);
        // add invisible custom palettes
        const bool visible = m_userPaletteModel->data(idx, PaletteTreeModel::VisibleRole).toBool();
        const bool custom = m_userPaletteModel->data(idx, PaletteTreeModel::CustomRole).toBool();
        if (!visible) {
            const QString name = m_userPaletteModel->data(idx, Qt::DisplayRole).toString();
            QStandardItem* item = new QStandardItem(name);
            item->setData(custom, CustomRole);
            item->setData(QPersistentModelIndex(idx), PaletteIndexRole);
            root->appendRow(item);
        }
    }

    QQmlEngine::setObjectOwnership(m, QQmlEngine::JavaScriptOwnership);
    return m;
}

bool PaletteProvider::addPalette(const QPersistentModelIndex& index)
{
    if (!index.isValid()) {
        return false;
    }

    if (index.model() == m_userPaletteModel) {
        const bool ok = m_userPaletteModel->setData(index, true, PaletteTreeModel::VisibleRole);
        if (!ok) {
            return false;
        }
        const QModelIndex parent = index.parent();
        m_userPaletteModel->moveRow(parent, index.row(), parent, 0);
        return true;
    }

    if (index.model() == m_masterPaletteModel) {
        QMimeData* data = m_masterPaletteModel->mimeData({ QModelIndex(index) });
        const bool success = m_userPaletteModel->dropMimeData(data, Qt::CopyAction, 0, 0, QModelIndex());
        data->deleteLater();
        return success;
    }

    return false;
}

void PaletteProvider::resetPalette(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    std::string title = muse::trc("palette",
                                  "Do you want to restore this palette to its default state? All changes to this palette will be lost.");

    interactive()->question("", title, {
        IInteractive::Button::No, IInteractive::Button::Yes
    })
    .onResolve(this, [this, index](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::Yes)) {
            doResetPalette(index);
        }
    });
}

void PaletteProvider::doResetPalette(const QModelIndex& index)
{
    Q_ASSERT(m_defaultPaletteModel != m_userPaletteModel);

    QAbstractItemModel* resetModel = nullptr;
    QModelIndex resetIndex;

    if (m_defaultPaletteModel) {
        resetModel = m_defaultPaletteModel;
        resetIndex = convertIndex(index, m_defaultPaletteModel);
    }

    if (!resetIndex.isValid()) {
        resetModel = m_masterPaletteModel;
        resetIndex = convertIndex(index, m_masterPaletteModel);
    }

    const QModelIndex userPaletteIndex = convertProxyIndex(index, m_userPaletteModel);
    const QModelIndex parent = userPaletteIndex.parent();
    const int row = userPaletteIndex.row();
    const int column = userPaletteIndex.column();

    // restore visibility and expanded state of the palette after restoring its state
    const bool wasVisible = index.data(PaletteTreeModel::VisibleRole).toBool();
    const bool wasExpanded = index.data(PaletteTreeModel::PaletteExpandedRole).toBool();

    if (!m_userPaletteModel->removeRow(row, parent)) {
        return;
    }

    if (resetIndex.isValid()) {
        QMimeData* data = resetModel->mimeData({ resetIndex });
        m_userPaletteModel->dropMimeData(data, Qt::CopyAction, row, column, parent);
        data->deleteLater();
    } else {
        m_userPaletteModel->insertRow(row, parent);
    }

    const QModelIndex newIndex = m_userPaletteModel->index(row, column, parent);
    m_userPaletteModel->setData(newIndex, wasVisible, PaletteTreeModel::VisibleRole);
    m_userPaletteModel->setData(newIndex, wasExpanded, PaletteTreeModel::PaletteExpandedRole);
}

QString PaletteProvider::getPaletteFilename(bool open, const QString& name) const
{
    QString title;
    std::vector<std::string> filter;
#ifdef WIN_PORTABLE
    QString wd = QDir::cleanPath(QString("%1/../../../Data/settings").arg(QCoreApplication::applicationDirPath()));
#else
    QString wd = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
                 .arg(QCoreApplication::applicationName());
#endif
    if (open) {
        title  = muse::qtrc("palette", "Load palette");
        filter = { muse::trc("palette", "MuseScore Studio palette") + " (*.mpal)" };
    } else {
        title  = muse::qtrc("palette", "Save palette");
        filter = { muse::trc("palette", "MuseScore Studio palette") + " (*.mpal)" };
    }

    QFileInfo myPalettes(wd);
    QString defaultPath = myPalettes.absoluteFilePath();
    if (!name.isEmpty()) {
        QString fname = muse::io::escapeFileName(name).toQString();
        QFileInfo myName(fname);
        if (myName.isRelative()) {
            myName.setFile(defaultPath, fname);
        }
        defaultPath = myName.absoluteFilePath();
    }

    muse::io::path_t fn;
    if (open) {
        fn = interactive()->selectOpeningFile(title, defaultPath, filter);
    } else {
        fn = interactive()->selectSavingFile(title, defaultPath, filter);
    }
    return fn.toQString();
}

bool PaletteProvider::savePalette(const QModelIndex& index)
{
    const QModelIndex srcIndex = convertProxyIndex(index, m_userPaletteModel);
    const Palette* pp = m_userPaletteModel->findPalette(srcIndex);
    if (!pp) {
        return false;
    }

    const QString path = getPaletteFilename(/* load? */ false, pp->translatedName());

    if (path.isEmpty()) {
        return false;
    }
    return pp->writeToFile(path);
}

bool PaletteProvider::loadPalette(const QModelIndex& index)
{
    const QString path = getPaletteFilename(/* load? */ true);
    if (path.isEmpty()) {
        return false;
    }

    PalettePtr pp = std::make_shared<Palette>();
    if (!pp->readFromFile(path)) {
        return false;
    }
    pp->setType(Palette::Type::Custom);   // mark the loaded palette custom

    const QModelIndex srcIndex = convertProxyIndex(index, m_userPaletteModel);

    const int row = srcIndex.row();
    const QModelIndex parent = srcIndex.parent();

    return m_userPaletteModel->insertPalette(pp, row, parent);
}

void PaletteProvider::setUserPaletteTree(PaletteTreePtr tree)
{
    if (m_userPaletteModel) {
        disconnect(m_userPaletteModel, &PaletteTreeModel::treeChanged, this, &PaletteProvider::notifyAboutUserPaletteChanged);
        m_userPaletteModel->setPaletteTree(tree);
        connect(m_userPaletteModel, &PaletteTreeModel::treeChanged, this, &PaletteProvider::notifyAboutUserPaletteChanged);
    } else {
        m_userPaletteModel = new PaletteTreeModel(tree, /* parent */ this);
        connect(m_userPaletteModel, &PaletteTreeModel::treeChanged, this, &PaletteProvider::notifyAboutUserPaletteChanged);
    }
}

void PaletteProvider::setDefaultPaletteTree(PaletteTreePtr tree)
{
    if (m_defaultPaletteModel) {
        m_defaultPaletteModel->setPaletteTree(tree);
    } else {
        m_defaultPaletteModel = new PaletteTreeModel(tree, /* parent */ this);
    }
}

muse::async::Channel<ElementPtr> PaletteProvider::addCustomItemRequested() const
{
    return m_addCustomItemRequested;
}

void PaletteProvider::write(XmlWriter& xml, bool pasteMode) const
{
    if (!m_userPaletteModel) {
        return;
    }
    if (const PaletteTree* tree = m_userPaletteModel->paletteTree()) {
        tree->write(xml, pasteMode);
    }
}

bool PaletteProvider::read(XmlReader& e, bool pasteMode)
{
    PaletteTreePtr tree = std::make_shared<PaletteTree>();
    if (!tree->read(e, pasteMode)) {
        return false;
    }

    setUserPaletteTree(tree);

    return true;
}
