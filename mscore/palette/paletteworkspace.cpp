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

#include "libmscore/keysig.h"
#include "libmscore/timesig.h"

#include "keyedit.h"
#include "musescore.h"
#include "palette.h" // applyPaletteElement
#include "palettedialog.h"
#include "palettecelldialog.h"
#include "timedialog.h"

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
                  return tr("Create Key Signature");
            case Type::TimeSig:
                  return tr("Create Time Signature");
            default:
                  break;
            }
      return QString();
      }

//---------------------------------------------------------
//   PaletteElementEditor::onElementAdded
//---------------------------------------------------------

void PaletteElementEditor::onElementAdded(const Element* el)
      {
      if (!_paletteIndex.isValid()
         || !_paletteIndex.data(PaletteTreeModel::VisibleRole).toBool()) {
            QMessageBox::information(mscore, "", tr("The palette was hidden or changed"));
            return;
            }
      QVariantMap mimeData;
      mimeData[mimeSymbolFormat] = el->mimeData(QPointF());
      _controller->insert(_paletteIndex, -1, mimeData, Qt::CopyAction);
      }

//---------------------------------------------------------
//   PaletteElementEditor::open
//---------------------------------------------------------

void PaletteElementEditor::open()
      {
      if (!_paletteIndex.isValid())
            return;

      QWidget* editor = nullptr;

      using Type = PalettePanel::Type;
      switch (_type) {
            case Type::KeySig: {
                  KeyEditor* keyEditor = new KeyEditor(mscore);
                  keyEditor->showKeyPalette(false);
                  connect(keyEditor, &KeyEditor::keySigAdded, this, &PaletteElementEditor::onElementAdded);
                  editor = keyEditor;
                  }
                  break;
            case Type::TimeSig: {
                  TimeDialog* timeEditor = new TimeDialog(mscore);
                  timeEditor->showTimePalette(false);
                  connect(timeEditor, &TimeDialog::timeSigAdded, this, &PaletteElementEditor::onElementAdded);
                  editor = timeEditor;
                  }
                  break;
            default:
                  break;
            }

      if (!editor)
            return;

      mscore->stackUnder(editor);
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
//   AbstractPaletteController::elementEditor
//---------------------------------------------------------

PaletteElementEditor* AbstractPaletteController::elementEditor(const QModelIndex& paletteIndex)
      {
      PaletteElementEditor* ed = new PaletteElementEditor(this, paletteIndex, paletteIndex.data(PaletteTreeModel::PaletteTypeRole).value<PalettePanel::Type>(), this);
      QQmlEngine::setObjectOwnership(ed, QQmlEngine::JavaScriptOwnership);
      return ed;
      }

//---------------------------------------------------------
//   UserPaletteController::dropAction
//---------------------------------------------------------

Qt::DropAction UserPaletteController::dropAction(const QVariantMap& mimeData, Qt::DropAction proposedAction, const QModelIndex& parent, bool internal) const
      {
      if (internal && !userEditable())
            return Qt::IgnoreAction;
      const bool parentEditingEnabled = model()->data(parent, PaletteTreeModel::EditableRole).toBool();
      if (!parentEditingEnabled)
            return Qt::IgnoreAction;

      if (mimeData.contains(PaletteCell::mimeDataFormat) && proposedAction == Qt::MoveAction) {
            const auto cell = PaletteCell::readMimeData(mimeData[PaletteCell::mimeDataFormat].toByteArray());
            if (!cell)
                  return Qt::IgnoreAction;
            if (_filterCustom && cell->custom != _custom)
                  return Qt::IgnoreAction;
            return Qt::MoveAction;
            }
      if (mimeData.contains(mimeSymbolFormat) && proposedAction == Qt::CopyAction) {
            if (_filterCustom && !_custom)
                  return Qt::IgnoreAction;
            return Qt::CopyAction;
            }
      return Qt::IgnoreAction;
      }

//---------------------------------------------------------
//   UserPaletteController::insert
//---------------------------------------------------------

bool UserPaletteController::insert(const QModelIndex& parent, int row, const QVariantMap& mimeData, Qt::DropAction action)
      {
      if (dropAction(mimeData, action, parent, false) == Qt::IgnoreAction)
            return false;

      if (row < 0)
            row = parent.model()->rowCount(parent);

      PaletteCellPtr cell;

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
                  else if (!userEditable())
                        return false;
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
//   UserPaletteController::insertNewItem
//---------------------------------------------------------

bool UserPaletteController::insertNewItem(const QModelIndex& parent, int row)
      {
      if (canEdit(parent))
            return model()->insertRow(row, parent);
      return false;
      }

//---------------------------------------------------------
//   UserPaletteController::move
//---------------------------------------------------------

bool UserPaletteController::move(const QModelIndex& sourceParent, int sourceRow, const QModelIndex& destinationParent, int destinationChild)
      {
      if (!canEdit(sourceParent) || !canEdit(destinationParent))
            return false;
      if (sourceParent == destinationParent && (sourceParent.model() == model() || !sourceParent.isValid())) {
            const QModelIndex srcIndex = convertProxyIndex(model()->index(sourceRow, 0, sourceParent), _userPalette);
            const QModelIndex destIndex = convertProxyIndex(model()->index(destinationChild, 0, destinationParent), _userPalette);
            return _userPalette->moveRow(srcIndex.parent(), srcIndex.row(), destIndex.parent(), destIndex.row());
            }
      return false;
      }

//---------------------------------------------------------
//   UserPaletteController::showHideOrDeleteDialog
//---------------------------------------------------------

AbstractPaletteController::RemoveAction UserPaletteController::showHideOrDeleteDialog(const QString& question) const
      {
      QMessageBox msg;
      msg.setIcon(QMessageBox::Question);
      msg.setText(question);
      msg.setTextFormat(Qt::PlainText);
      QPushButton* deleteButton = msg.addButton(tr("Delete permanently"), QMessageBox::DestructiveRole);
      QPushButton* hideButton = msg.addButton(tr("Keep a copy"), QMessageBox::AcceptRole);
      msg.addButton(QMessageBox::Cancel);
      msg.setDefaultButton(hideButton);

      msg.exec();
      QAbstractButton* btn = msg.clickedButton();
      if (btn == deleteButton)
            return RemoveAction::DeletePermanently;
      if (btn == hideButton)
            return RemoveAction::Hide;
      return RemoveAction::NoAction;
      }

//---------------------------------------------------------
//   UserPaletteController::remove
//---------------------------------------------------------

AbstractPaletteController::RemoveAction UserPaletteController::queryRemoveAction(const QModelIndex& index) const
      {
      using RemoveAction = AbstractPaletteController::RemoveAction;

      if (!canEdit(index.parent()))
            return RemoveAction::NoAction;

      const bool custom = model()->data(index, PaletteTreeModel::CustomRole).toBool();
      const bool visible = model()->data(index, PaletteTreeModel::VisibleRole).toBool();
      const bool isCell = bool(model()->data(index, PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>());

      if (isCell) {
            if (!custom) {
                  // no need to keep standard insivible cells, just remove them from the model
                  return RemoveAction::DeletePermanently;
                  }

            if (visible)
                  return showHideOrDeleteDialog(tr("Do you want to permanently delete this custom palette cell or keep a copy in the library?"));
            else {
                  const auto answer = QMessageBox::question(
                        nullptr,
                        "",
                        tr("Do you want to permanently delete this custom palette?"),
                        QMessageBox::Yes | QMessageBox::No
                        );

                  if (answer == QMessageBox::Yes)
                        return RemoveAction::DeletePermanently;
                  return RemoveAction::NoAction;
                  }

            return RemoveAction::NoAction;
            }
      else {
            if (visible && custom)
                  return showHideOrDeleteDialog(tr("Do you want to permanently delete this custom palette or keep a copy in the \"More Palettes\" list?"));
            return RemoveAction::Hide;
            }
      }

//---------------------------------------------------------
//   UserPaletteController::remove
//---------------------------------------------------------

bool UserPaletteController::remove(const QModelIndex& index)
      {
      using RemoveAction = AbstractPaletteController::RemoveAction;
      const RemoveAction action = queryRemoveAction(index);
      switch (action) {
            case RemoveAction::NoAction:
                  break;
            case RemoveAction::Hide:
                  return model()->setData(index, false, PaletteTreeModel::VisibleRole);
            case RemoveAction::DeletePermanently:
                  return model()->removeRow(index.row(), index.parent());
            }
      return false;
      }

//---------------------------------------------------------
//   UserPaletteController::editPaletteProperties
//---------------------------------------------------------

void UserPaletteController::editPaletteProperties(const QModelIndex& index)
      {
      if (!canEdit(index))
            return;

      const QModelIndex srcIndex = convertProxyIndex(index, _userPalette);
      PalettePanel* p = _userPalette->findPalettePanel(srcIndex);
      if (!p)
            return;

      PaletteTreeModel* m = _userPalette;
      bool paletteChangedState = m->paletteTreeChanged();
      PalettePropertiesDialog* d = new PalettePropertiesDialog(p, mscore);
      connect(d, &QDialog::rejected, m, [m, srcIndex, paletteChangedState]() {
            m->itemDataChanged(srcIndex);
            paletteChangedState ? m->setTreeChanged() : m->setTreeUnchanged();
      });
      connect(d, &PalettePropertiesDialog::changed, m, [m, srcIndex]() {
            m->itemDataChanged(srcIndex);
      });
      
      d->setModal(true);
      d->setAttribute(Qt::WA_DeleteOnClose);
      d->open();
      }

//---------------------------------------------------------
//   UserPaletteController::editCellProperties
//---------------------------------------------------------

void UserPaletteController::editCellProperties(const QModelIndex& index)
      {
      if (!canEdit(index))
            return;

      const QModelIndex srcIndex = convertProxyIndex(index, _userPalette);
      PaletteCellPtr cell = _userPalette->findCell(srcIndex);
      if (!cell)
            return;

      PaletteCellPropertiesDialog* d = new PaletteCellPropertiesDialog(cell.get(), mscore);
      PaletteTreeModel* m = _userPalette;
      bool paletteChangedState = m->paletteTreeChanged();
      connect(d, &QDialog::rejected, m, [m, srcIndex, paletteChangedState]() {
            m->itemDataChanged(srcIndex);
            paletteChangedState ? m->setTreeChanged() : m->setTreeUnchanged();
      });
      connect(d, &PaletteCellPropertiesDialog::changed, m, [m, srcIndex]() {
            m->itemDataChanged(srcIndex);
      });

      d->setModal(true);
      d->setAttribute(Qt::WA_DeleteOnClose);
      d->open();
      }

//---------------------------------------------------------
//   UserPaletteController::canEdit
//---------------------------------------------------------

bool UserPaletteController::canEdit(const QModelIndex& index) const
      {
      if (!userEditable())
            return false;

      return model()->data(index, PaletteTreeModel::EditableRole).toBool();
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
//   PaletteWorkspace
//---------------------------------------------------------

PaletteWorkspace::PaletteWorkspace(PaletteTreeModel* user, PaletteTreeModel* master, QObject* parent)
   : QObject(parent), userPalette(user), masterPalette(master), defaultPalette(master)
      {}

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
            QSortFilterProxyModel* textFilterModel = new ChildFilterProxyModel(this);
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
                  item->setData(false, CustomRole); // this palette is from master palette, hence not custom
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
      if (!index.isValid())
            return false;

      if (index.model() == userPalette)
            return userPalette->setData(index, true, PaletteTreeModel::VisibleRole);

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
      if (!index.isValid())
            return false;


      if (index.model() == userPalette) {
            const bool custom = index.data(PaletteTreeModel::CustomRole).toBool();
            if (!custom)
                  return false;

            const auto answer = QMessageBox::question(
                  nullptr,
                  "",
                  tr("Do you want to permanently delete this custom palette?"),
                  QMessageBox::Yes | QMessageBox::No
                  );

            if (answer == QMessageBox::Yes)
                  return userPalette->removeRow(index.row(), index.parent());
            return false;
            }

      return false;
      }

//---------------------------------------------------------
//   PaletteWorkspace::resetPalette
//---------------------------------------------------------

bool PaletteWorkspace::resetPalette(const QModelIndex& index)
      {
      if (!index.isValid())
            return false;

      const auto answer = QMessageBox::question(
            nullptr,
            "",
            tr("Do you want to restore this palette to its default state? All changes to this palette will be lost."),
            QMessageBox::Yes | QMessageBox::No
            );

      if (answer != QMessageBox::Yes)
            return false;

      Q_ASSERT(defaultPalette != userPalette);
      const QModelIndex defaultPaletteIndex = convertIndex(index, defaultPalette);

      const QModelIndex userPaletteIndex = convertProxyIndex(index, userPalette);
      const QModelIndex parent = userPaletteIndex.parent();
      const int row = userPaletteIndex.row();
      const int column = userPaletteIndex.column();

      // restore visibility and expanded state of the palette after restoring its state
      const bool wasVisible = index.data(PaletteTreeModel::VisibleRole).toBool();
      const bool wasExpanded = index.data(PaletteTreeModel::PaletteExpandedRole).toBool();

      if (!userPalette->removeRow(row, parent))
            return false;

      if (defaultPaletteIndex.isValid()) {
            QMimeData* data = defaultPalette->mimeData({ defaultPaletteIndex });
            userPalette->dropMimeData(data, Qt::CopyAction, row, column, parent);
            data->deleteLater();
            }
      else
            userPalette->insertRow(row, parent);

      const QModelIndex newIndex = userPalette->index(row, column, parent);
      userPalette->setData(newIndex, wasVisible, PaletteTreeModel::VisibleRole);
      userPalette->setData(newIndex, wasExpanded, PaletteTreeModel::PaletteExpandedRole);

      return true;
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
