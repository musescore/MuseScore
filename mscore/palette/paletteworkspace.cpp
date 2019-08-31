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

#include "musescore.h"
#include "palette.h" // applyPaletteElement
#include "palettedialogs.h"

namespace Ms {

//---------------------------------------------------------
//   findPaletteIndex
//---------------------------------------------------------

static QModelIndex findPaletteIndex(const QAbstractItemModel* model, PalettePanel::Type type)
      {
      constexpr int role = PaletteTreeModel::PaletteTypeRole;
      const QModelIndex start = model->index(0, 0);
      const QModelIndexList foundIndexList = model->match(start, role, QVariant::fromValue(type));

      if (!foundIndexList.empty())
            return foundIndexList[0];
      return QModelIndex();
      }

//---------------------------------------------------------
//   convertIndex
//---------------------------------------------------------

static QModelIndex convertIndex(const QModelIndex& index, const QAbstractItemModel* targetModel)
      {
      if (index.model() == targetModel || !index.isValid())
            return index;

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
            if (auto m = qobject_cast<const QAbstractProxyModel*>(index.model()))
                  index = m->mapToSource(index);
            else
                  break;
            }

      if (targetModel && index.model() != targetModel)
            return QModelIndex();
      return index;
      }

//---------------------------------------------------------
//   UserPaletteController::dropAction
//---------------------------------------------------------

Qt::DropAction UserPaletteController::dropAction(const QVariantMap& mimeData, Qt::DropActions supportedActions, bool internal) const
      {
      if (internal && readOnly())
            return Qt::IgnoreAction;

      if (mimeData.contains(PaletteCell::mimeDataFormat) && (supportedActions & Qt::MoveAction)) {
            const auto cell = PaletteCell::readMimeData(mimeData[PaletteCell::mimeDataFormat].toByteArray());
            if (!cell)
                  return Qt::IgnoreAction;
            if (_filterCustom && cell->custom != _custom)
                  return Qt::IgnoreAction;
            return Qt::MoveAction;
            }
      if (mimeData.contains(mimeSymbolFormat) && (supportedActions & Qt::CopyAction)) {
            if (_filterCustom && !_custom)
                  return Qt::IgnoreAction;
            return Qt::CopyAction;
            }
      return Qt::IgnoreAction;
      }

//---------------------------------------------------------
//   UserPaletteController::insert
//---------------------------------------------------------

bool UserPaletteController::insert(const QModelIndex& parent, int row, const QVariantMap& mimeData)
      {
      const Qt::DropAction action = dropAction(mimeData, Qt::DropActions(Qt::CopyAction | Qt::MoveAction), false); // TODO: make it an argument, then there won't be a need in this override

      std::unique_ptr<PaletteCell> cell;

      if (mimeData.contains(PaletteCell::mimeDataFormat)) {
            cell = PaletteCell::readMimeData(mimeData[PaletteCell::mimeDataFormat].toByteArray());

            if (!cell)
                  return false;
            if (_filterCustom && cell->custom != _custom)
                  return false;

            if (action == Qt::MoveAction) {
                  const QModelIndex visiblePaletteParentIndex = convertIndex(parent, _userPalette);
                  const QModelIndex foundIndex(_userPalette->findPaletteCell(*cell, visiblePaletteParentIndex));
                  if (foundIndex.isValid())
                        return _userPalette->setData(foundIndex, _visible, PaletteTreeModel::VisibleRole);
                  }
            }
      else if (mimeData.contains(mimeSymbolFormat) && (action == Qt::CopyAction))
            cell = PaletteCell::readElementMimeData(mimeData[mimeSymbolFormat].toByteArray());

      if (!cell)
            return false;

      if (_filterCustom) {
            if (!_custom)
                  return false; // can only move non-custom cells
            cell->custom = _custom;
            }
      cell->visible = _visible;

      QMimeData data;
      data.setData(PaletteCell::mimeDataFormat, cell->mimeData());
      constexpr int column = 0;
      return model()->dropMimeData(&data, action, row, column, parent);
      }

//---------------------------------------------------------
//   UserPaletteController::move
//---------------------------------------------------------

bool UserPaletteController::move(const QModelIndex& sourceParent, int sourceRow, const QModelIndex& destinationParent, int destinationChild)
      {
      if (readOnly())
            return false;
      if (sourceParent == destinationParent && (sourceParent.model() == model() || !sourceParent.isValid())) {
            const QModelIndex srcIndex = convertProxyIndex(model()->index(sourceRow, 0, sourceParent), _userPalette);
            const QModelIndex destIndex = convertProxyIndex(model()->index(destinationChild, 0, destinationParent), _userPalette);
            return _userPalette->moveRow(srcIndex.parent(), srcIndex.row(), destIndex.parent(), destIndex.row());
            }
      return false;
      }

//---------------------------------------------------------
//   UserPaletteController::remove
//---------------------------------------------------------

bool UserPaletteController::remove(const QModelIndex& parent, int row)
      {
      if (readOnly())
            return false;

      const QModelIndex index = model()->index(row, 0, parent);
      const bool custom = model()->data(index, PaletteTreeModel::CustomRole).toBool(); // TODO: check canConvert?
      const bool visible = model()->data(index, PaletteTreeModel::VisibleRole).toBool(); // TODO: check canConvert?
      const bool isCell = bool(model()->data(index, PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>());
      if ((custom || !isCell) && visible)
            return model()->setData(index, false, PaletteTreeModel::VisibleRole);
      else // no need to keep standard insivible cells, just remove them from the model
            return model()->removeRow(row, parent);
      }

//---------------------------------------------------------
//   UserPaletteController::editPaletteProperties
//---------------------------------------------------------

void UserPaletteController::editPaletteProperties(const QModelIndex& index)
      {
      if (readOnly())
            return;

      const QModelIndex srcIndex = convertProxyIndex(index, _userPalette);
      PalettePanel* p = _userPalette->findPalettePanel(srcIndex);
      if (!p)
            return;

      PalettePropertiesDialog* d = new PalettePropertiesDialog(p, mscore);
      PaletteTreeModel* m = _userPalette;
      connect(d, &QDialog::accepted, m, [m, srcIndex]() { m->itemDataChanged(srcIndex); });
      d->setModal(true);
      d->setAttribute(Qt::WA_DeleteOnClose);
      d->open();
      }

//---------------------------------------------------------
//   UserPaletteController::editCellProperties
//---------------------------------------------------------

void UserPaletteController::editCellProperties(const QModelIndex& index)
      {
      if (readOnly())
            return;

      const QModelIndex srcIndex = convertProxyIndex(index, _userPalette);
      PaletteCell* cell = _userPalette->findCell(srcIndex);
      if (!cell)
            return;

      PaletteCellProperties* d = new PaletteCellProperties(cell, mscore);
      PaletteTreeModel* m = _userPalette;
      connect(d, &QDialog::accepted, m, [m, srcIndex]() { m->itemDataChanged(srcIndex); });
      d->setModal(true);
      d->setAttribute(Qt::WA_DeleteOnClose);
      d->open();
      }

//---------------------------------------------------------
//   UserPaletteController::applyPaletteElement
//---------------------------------------------------------

void UserPaletteController::applyPaletteElement(const QModelIndex& index, Qt::KeyboardModifiers modifiers)
      {
      const PaletteCell* cell = model()->data(index, PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>();
      if (cell && cell->element)
            Palette::applyPaletteElement(cell->element.get(), modifiers);
      }

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteModel
//---------------------------------------------------------

QAbstractItemModel* PaletteWorkspace::mainPaletteModel()
      {
      if (!mainPalette) {
//             PaletteCellFilter* filter = new VisibilityCellFilter(/* acceptedValue */ true);
//             mainPalette = new FilterPaletteTreeModel(filter, userPalette, this);
            QSortFilterProxyModel* visFilterModel = new QSortFilterProxyModel(this);
            visFilterModel->setFilterRole(PaletteTreeModel::VisibleRole);
            visFilterModel->setFilterFixedString("true");
            visFilterModel->setSourceModel(userPalette);

            // Wrap it into another proxy model to enable filtering by palette cell name
            QSortFilterProxyModel* textFilterModel = new RecursiveFilterProxyModel(this);
            textFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            textFilterModel->setSourceModel(visFilterModel);
            visFilterModel->setParent(textFilterModel);

            mainPalette = textFilterModel;
            }
      return mainPalette;
      }

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteModel
//---------------------------------------------------------

AbstractPaletteController* PaletteWorkspace::getMainPaletteController()
      {
      if (!mainPaletteController)
            mainPaletteController = new UserPaletteController(mainPaletteModel(), userPalette, this);
//             mainPaletteController = new PaletteController(mainPaletteModel(), this, this);
      return mainPaletteController;
      }

//---------------------------------------------------------
//   PaletteWorkspace::customPaletteIndex
//---------------------------------------------------------

QModelIndex PaletteWorkspace::customElementsPaletteIndex(const QModelIndex& index)
      {
      const QAbstractItemModel* model = customElementsPaletteModel();
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
            customElementsPaletteController->setVisible(false);
            customElementsPaletteController->setCustom(true);
            }
//             customElementsPaletteController = new CustomPaletteController(customElementsPaletteModel(), this, this);
      return customElementsPaletteController;
      }

//---------------------------------------------------------
//   PaletteWorkspace::poolPaletteIndex
//---------------------------------------------------------

QModelIndex PaletteWorkspace::poolPaletteIndex(const QModelIndex& index, Ms::FilterPaletteTreeModel* poolPalette)
      {
      const QModelIndex poolPaletteIndex = convertIndex(index, poolPalette);
      if (poolPaletteIndex.isValid())
            return poolPaletteIndex;
      return findPaletteIndex(poolPalette, PalettePanel::Type::Clef);
      }

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteModel
//---------------------------------------------------------

FilterPaletteTreeModel* PaletteWorkspace::poolPaletteModel(const QModelIndex& index)
      {
      const QModelIndex filterIndex = convertProxyIndex(index, userPalette);
      PaletteCellFilter* filter = userPalette->getFilter(filterIndex); // TODO: or mainPalette?
      if (!filter)
            return nullptr;

      FilterPaletteTreeModel* m = new FilterPaletteTreeModel(filter, masterPalette);
      QQmlEngine::setObjectOwnership(m, QQmlEngine::JavaScriptOwnership);
      return m;
      }

//---------------------------------------------------------
//   PaletteWorkspace::masterPaletteController
//---------------------------------------------------------

AbstractPaletteController* PaletteWorkspace::poolPaletteController(FilterPaletteTreeModel* poolPaletteModel, const QModelIndex& rootIndex)
      {
      UserPaletteController* c = new UserPaletteController(poolPaletteModel, userPalette);
      c->setVisible(false);
      c->setCustom(false);
      c->setReadOnly(true); // TODO: rename to disable just internal move
//       AbstractPaletteController* c = new MasterPaletteController(poolPaletteModel, userPalette, this);
      QQmlEngine::setObjectOwnership(c, QQmlEngine::JavaScriptOwnership);
      return c;
      }

//---------------------------------------------------------
//   PaletteWorkspace::availableExtraPalettePanelsModel
//---------------------------------------------------------

QAbstractItemModel* PaletteWorkspace::availableExtraPalettePanelsModel()
      {
      QStandardItemModel* m = new QStandardItemModel;

      QStandardItem* root = m->invisibleRootItem();

      const int masterRows = masterPalette->rowCount();
      for (int row = 0; row < masterRows; ++row) {
            const QModelIndex idx = masterPalette->index(row, 0);
            // add everything that cannot be found in visible palette
            if (!convertIndex(idx, mainPalette).isValid()) {
                  const QString name = masterPalette->data(idx, Qt::DisplayRole).toString();
                  QStandardItem* item = new QStandardItem(name);
                  root->appendRow(item);
                  }
            }

      const int userRows = userPalette->rowCount();
      for (int row = 0; row < userRows; ++row) {
            const QModelIndex idx = userPalette->index(row, 0);
            // add invisible custom palettes
            const bool visible = userPalette->data(idx, PaletteTreeModel::VisibleRole).toBool();
            const bool custom = userPalette->data(idx, PaletteTreeModel::CustomRole).toBool();
            if (!visible && custom) {
                  const QString name = userPalette->data(idx, Qt::DisplayRole).toString();
                  QStandardItem* item = new QStandardItem(name);
                  root->appendRow(item);
                  }
            }

      QQmlEngine::setObjectOwnership(m, QQmlEngine::JavaScriptOwnership);
      return m;
      }

//---------------------------------------------------------
//   PaletteWorkspace::addPalette
//---------------------------------------------------------

bool PaletteWorkspace::addPalette(QString name)
      {
      const int role = Qt::DisplayRole;
      const QVariant value(name);

      // try to find a palette in invisible user palette's items
      {
      const QModelIndex start = userPalette->index(0, 0);
      const QAbstractItemModel* m = userPalette;
      const QModelIndexList foundIndexList = m->match(start, role, value);
      if (!foundIndexList.empty())
            return userPalette->setData(foundIndexList[0], true, PaletteTreeModel::VisibleRole);
      }

      // if not found, add from a master palette
      // TODO: need some "basic" default palette
      {
      const QModelIndex start = masterPalette->index(0, 0);
      const QAbstractItemModel* m = masterPalette;
      const QModelIndexList foundIndexList = m->match(start, role, value);
      if (!foundIndexList.empty()) {
            const QMimeData* data = masterPalette->mimeData(foundIndexList);
            return userPalette->dropMimeData(data, Qt::CopyAction, 0, 0, QModelIndex());
            }
      }

      return false;
      }

//---------------------------------------------------------
//   PaletteWorkspace::addCustomPalette
//---------------------------------------------------------

bool PaletteWorkspace::addCustomPalette(int idx)
      {
      if (idx < 0)
            idx = 0; // insert to the beginning by default

      return userPalette->insertRow(idx, QModelIndex());
      }

//---------------------------------------------------------
//   PaletteWorkspace::write
//---------------------------------------------------------

void PaletteWorkspace::write(XmlWriter& xml) const
      {
      if (!userPalette)
            return;
      if (const PaletteTree* tree = userPalette->paletteTree())
            tree->write(xml);
      }

//---------------------------------------------------------
//   PaletteWorkspace::read
//---------------------------------------------------------

bool PaletteWorkspace::read(XmlReader& e)
      {
      std::unique_ptr<PaletteTree> tree(new PaletteTree);
      if (!tree->read(e))
            return false;

      if (userPalette)
            userPalette->setPaletteTree(std::move(tree));
      else
            userPalette = new PaletteTreeModel(std::move(tree), /* parent */ this);

      return true;
      }
} // namespace Ms
