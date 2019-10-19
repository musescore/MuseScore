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

#ifndef __PALETTEWORKSPACE_H__
#define __PALETTEWORKSPACE_H__

#include "palettemodel.h"

namespace Ms {

class AbstractPaletteController;
class PaletteWorkspace;

//---------------------------------------------------------
//   PaletteElementEditor
//---------------------------------------------------------

class PaletteElementEditor : public QObject {
      Q_OBJECT

      AbstractPaletteController* _controller = nullptr;
      QPersistentModelIndex _paletteIndex;
      PalettePanel::Type _type = PalettePanel::Type::Unknown;

      Q_PROPERTY(bool valid READ valid CONSTANT)
      Q_PROPERTY(QString actionName READ actionName CONSTANT) // TODO: make NOTIFY instead of CONSTANT for retranslations

   private slots:
      void onElementAdded(const Element*);

   public:
      PaletteElementEditor(QObject* parent = nullptr) : QObject(parent) {}
      PaletteElementEditor(AbstractPaletteController* controller, QPersistentModelIndex paletteIndex, PalettePanel::Type type, QObject* parent = nullptr)
         : QObject(parent), _controller(controller), _paletteIndex(paletteIndex), _type(type) {}

      bool valid() const;
      QString actionName() const;

      Q_INVOKABLE void open();
      };

//---------------------------------------------------------
//   AbstractPaletteController
//---------------------------------------------------------

class AbstractPaletteController : public QObject {
      Q_OBJECT

      /// Whether dropping new elements to this palette is generally allowed
      Q_PROPERTY(bool canDropElements READ canDropElements CONSTANT)

      virtual bool canDropElements() const { return false; }

   public:
      enum class RemoveAction {
            NoAction,
            Hide,
            DeletePermanently,
            AutoAction
            };

      AbstractPaletteController(QObject* parent = nullptr) : QObject(parent) {}

      Q_INVOKABLE virtual Qt::DropAction dropAction(const QVariantMap& mimeData, Qt::DropAction proposedAction, const QModelIndex& parent, bool internal) const
            {
            Q_UNUSED(mimeData); Q_UNUSED(proposedAction); Q_UNUSED(parent); Q_UNUSED(internal);
            return Qt::IgnoreAction;
            }

      Q_INVOKABLE virtual bool move(const QModelIndex& sourceParent, int sourceRow, const QModelIndex& destinationParent, int destinationChild) = 0;
      Q_INVOKABLE virtual bool insert(const QModelIndex& parent, int row, const QVariantMap& mimeData, Qt::DropAction action) = 0;
      Q_INVOKABLE virtual bool insertNewItem(const QModelIndex& parent, int row) = 0;
      Q_INVOKABLE virtual void remove(const QModelIndex&) = 0;
      Q_INVOKABLE virtual void removeSelection(const QModelIndexList&, const QModelIndex& parent) = 0;

      Q_INVOKABLE virtual bool canEdit(const QModelIndex&) const { return false; }

      Q_INVOKABLE virtual void editPaletteProperties(const QModelIndex& index) { Q_UNUSED(index); }
      Q_INVOKABLE virtual void editCellProperties(const QModelIndex& index) { Q_UNUSED(index); }

      Q_INVOKABLE virtual void applyPaletteElement(const QModelIndex& index, Qt::KeyboardModifiers modifiers) { Q_UNUSED(index); Q_UNUSED(modifiers); }

      Q_INVOKABLE Ms::PaletteElementEditor* elementEditor(const QModelIndex& index);
      };

//---------------------------------------------------------
//   UserPaletteController
//---------------------------------------------------------

class UserPaletteController : public AbstractPaletteController {
      Q_OBJECT

      QAbstractItemModel* _model;
      PaletteTreeModel* _userPalette;

      bool _visible = true;
      bool _custom = false;
      bool _filterCustom = false;

      bool _userEditable = true;

      bool canDropElements() const override { return _userEditable; }

      void showHideOrDeleteDialog(const QString& question, std::function<void(RemoveAction)> resultHandler) const;
      void queryRemove(const QModelIndexList&, int customCount);

      enum RemoveActionConfirmationType {
            NoConfirmation,
            CustomCellHideDeleteConfirmation,
            CustomPaletteHideDeleteConfirmation
            };

      void remove(const QModelIndexList&, AbstractPaletteController::RemoveAction);

   protected:
      QAbstractItemModel* model() { return _model; }
      const QAbstractItemModel* model() const { return _model; }

   public:
      UserPaletteController(QAbstractItemModel* m, PaletteTreeModel* userPalette, QObject* parent = nullptr)
         : AbstractPaletteController(parent), _model(m), _userPalette(userPalette) {}

      bool visible() const { return _visible; }
      void setVisible(bool val) { _visible = val; }
      bool custom() const { return _custom; }
      void setCustom(bool val) { _custom = val; _filterCustom = true; }

      Qt::DropAction dropAction(const QVariantMap& mimeData, Qt::DropAction proposedAction, const QModelIndex& parent, bool internal) const override;

      bool move(const QModelIndex& sourceParent, int sourceRow, const QModelIndex& destinationParent, int destinationChild) override;
      bool insert(const QModelIndex& parent, int row, const QVariantMap& mimeData, Qt::DropAction action) override;
      bool insertNewItem(const QModelIndex& parent, int row) override;
      void remove(const QModelIndex& index) override;
      void removeSelection(const QModelIndexList&, const QModelIndex& parent) override;

      void editPaletteProperties(const QModelIndex& index) override;
      void editCellProperties(const QModelIndex& index) override;

      bool userEditable() const { return _userEditable; }
      void setUserEditable(bool val) { _userEditable = val; }

      bool canEdit(const QModelIndex&) const override;

      void applyPaletteElement(const QModelIndex& index, Qt::KeyboardModifiers modifiers) override;
      };

//---------------------------------------------------------
//   PaletteWorkspace
//---------------------------------------------------------

class PaletteWorkspace : public QObject {
      Q_OBJECT

      PaletteTreeModel* userPalette;
      PaletteTreeModel* masterPalette;
      PaletteTreeModel* defaultPalette; // palette used by "Reset palette" action

      QAbstractItemModel* mainPalette = nullptr;            ///< visible userPalette entries
//       PaletteTreeModel* poolPalette;               ///< masterPalette entries not yet added to mainPalette
      FilterPaletteTreeModel* customPoolPalette = nullptr;  ///< invisible userPalette entries that do not belong to masterPalette

      UserPaletteController* mainPaletteController = nullptr;
//       PaletteController* masterPaletteController;
      UserPaletteController* customElementsPaletteController = nullptr;

      Q_PROPERTY(QAbstractItemModel* mainPaletteModel READ mainPaletteModel CONSTANT)
      Q_PROPERTY(Ms::AbstractPaletteController* mainPaletteController READ getMainPaletteController CONSTANT)

      Q_PROPERTY(Ms::FilterPaletteTreeModel* customElementsPaletteModel READ customElementsPaletteModel CONSTANT)
      Q_PROPERTY(Ms::AbstractPaletteController* customElementsPaletteController READ getCustomElementsPaletteController CONSTANT)

      QAbstractItemModel* mainPaletteModel();
      AbstractPaletteController* getMainPaletteController();

      FilterPaletteTreeModel* customElementsPaletteModel();
      AbstractPaletteController* getCustomElementsPaletteController();

      enum PalettesModelRoles {
            CustomRole = Qt::UserRole + 1,
            PaletteIndexRole
            };

   signals:
      void userPaletteChanged();

   public:
      explicit PaletteWorkspace(PaletteTreeModel* user, PaletteTreeModel* master = nullptr, QObject* parent = nullptr);

      Q_INVOKABLE QModelIndex poolPaletteIndex(const QModelIndex& index, Ms::FilterPaletteTreeModel* poolPalette);
      Q_INVOKABLE QModelIndex customElementsPaletteIndex(const QModelIndex& index);

      Q_INVOKABLE Ms::FilterPaletteTreeModel* poolPaletteModel(const QModelIndex& index);
      Q_INVOKABLE Ms::AbstractPaletteController* poolPaletteController(Ms::FilterPaletteTreeModel*, const QModelIndex& rootIndex);

      PaletteTreeModel* userPaletteModel() { return userPalette; }

      Q_INVOKABLE QAbstractItemModel* availableExtraPalettesModel();
      Q_INVOKABLE bool addPalette(const QPersistentModelIndex&);
      Q_INVOKABLE bool removeCustomPalette(const QPersistentModelIndex&);

      Q_INVOKABLE bool resetPalette(const QModelIndex&);

      Q_INVOKABLE bool savePalette(const QModelIndex&);
      Q_INVOKABLE bool loadPalette(const QModelIndex&);

      bool paletteChanged() const { return userPalette->paletteTreeChanged(); }

      void setUserPaletteTree(std::unique_ptr<PaletteTree> tree);
      void setDefaultPaletteTree(std::unique_ptr<PaletteTree> tree);
      void write(XmlWriter&) const;
      bool read(XmlReader&);

      void updateCellsState(const Selection& sel, bool deactivateAll) { userPalette->updateCellsState(sel, deactivateAll); }
      void retranslate() { userPalette->retranslate(); masterPalette->retranslate(); defaultPalette->retranslate(); }
      };

} // namespace Ms

#endif
