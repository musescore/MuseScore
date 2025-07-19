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

#ifndef __PALETTEWORKSPACE_H__
#define __PALETTEWORKSPACE_H__

#include <QAbstractItemModel>

#include "view/palettemodel.h"

#include "ipaletteprovider.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "ipaletteconfiguration.h"
#include "context/iglobalcontext.h"

namespace mu::palette {
class AbstractPaletteController;
class PaletteProvider;

// ========================================================
//   PaletteElementEditor
// ========================================================

class PaletteElementEditor : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(muse::IInteractive, interactive)
    INJECT(IPaletteProvider, paletteProvider)

    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(QString actionName READ actionName CONSTANT) // TODO: make NOTIFY instead of CONSTANT for retranslations

public:
    PaletteElementEditor(QObject* parent = nullptr)
        : QObject(parent) {}
    PaletteElementEditor(AbstractPaletteController* controller, QPersistentModelIndex paletteIndex,
                         Palette::Type type, QObject* parent = nullptr)
        : QObject(parent), _controller(controller), _paletteIndex(paletteIndex), _type(type) {}

    bool valid() const;
    QString actionName() const;

    void setPaletteIndex(QPersistentModelIndex paletteIndex);

    Q_INVOKABLE void open();

private:
    void onElementAdded(const engraving::ElementPtr element);

    AbstractPaletteController* _controller = nullptr;
    QPersistentModelIndex _paletteIndex;
    Palette::Type _type = Palette::Type::Unknown;
};

// ========================================================
//   AbstractPaletteController
// ========================================================

class AbstractPaletteController : public QObject
{
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

    AbstractPaletteController(QObject* parent = nullptr)
        : QObject(parent) {}

    Q_INVOKABLE virtual Qt::DropAction dropAction(const QVariantMap& mimeData, Qt::DropAction proposedAction,
                                                  const QModelIndex& parent, bool internal) const
    {
        Q_UNUSED(mimeData);
        Q_UNUSED(proposedAction);
        Q_UNUSED(parent);
        Q_UNUSED(internal);
        return Qt::IgnoreAction;
    }

    Q_INVOKABLE virtual bool move(const QModelIndex& sourceParent, int sourceRow, const QModelIndex& destinationParent,
                                  int destinationChild) = 0;
    Q_INVOKABLE virtual bool insert(const QModelIndex& parent, int row, const QVariantMap& mimeData, Qt::DropAction action) = 0;
    Q_INVOKABLE virtual bool insertNewItem(const QModelIndex& parent, int row, const QString& name) = 0;
    Q_INVOKABLE virtual void remove(const QModelIndex&) = 0;
    Q_INVOKABLE virtual void removeSelection(const QModelIndexList&, const QModelIndex& parent = QModelIndex()) = 0;

    Q_INVOKABLE virtual bool canEdit(const QModelIndex&) const { return false; }

    Q_INVOKABLE virtual void editPaletteProperties(const QModelIndex& index) { Q_UNUSED(index); }
    Q_INVOKABLE virtual void editCellProperties(const QModelIndex& index) { Q_UNUSED(index); }

    Q_INVOKABLE virtual bool applyPaletteElement(const QModelIndex& index, Qt::KeyboardModifiers modifiers)
    {
        Q_UNUSED(index);
        Q_UNUSED(modifiers);
        return false;
    }

    Q_INVOKABLE mu::palette::PaletteElementEditor* elementEditor(const QModelIndex& index);

private:
    QMap<Palette::Type, PaletteElementEditor*> m_paletteElementEditorMap;
};

// ========================================================
//   UserPaletteController
// ========================================================

class UserPaletteController : public AbstractPaletteController, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)
    INJECT(muse::IInteractive, interactive)
    INJECT(IPaletteConfiguration, configuration)

    QAbstractItemModel* _model;
    PaletteTreeModel* _userPalette;

    bool _visible = true;
    bool _custom = false;
    bool _filterCustom = false;

    bool _userEditable = true;

    bool canDropElements() const override { return _userEditable; }

    void showHideOrDeleteDialog(const std::string& question, std::function<void(RemoveAction)> resultHandler) const;
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

    Qt::DropAction dropAction(const QVariantMap& mimeData, Qt::DropAction proposedAction, const QModelIndex& parent,
                              bool internal) const override;

    bool move(const QModelIndex& sourceParent, int sourceRow, const QModelIndex& destinationParent, int destinationChild) override;
    bool insert(const QModelIndex& parent, int row, const QVariantMap& mimeData, Qt::DropAction action) override;
    bool insertNewItem(const QModelIndex& parent, int row, const QString& name) override;
    void remove(const QModelIndex& index) override;
    void removeSelection(const QModelIndexList&, const QModelIndex& parent = QModelIndex()) override;

    void editPaletteProperties(const QModelIndex& index) override;
    void editCellProperties(const QModelIndex& index) override;

    bool userEditable() const { return _userEditable; }
    void setUserEditable(bool val) { _userEditable = val; }

    bool canEdit(const QModelIndex&) const override;

    bool applyPaletteElement(const QModelIndex& index, Qt::KeyboardModifiers modifiers) override;
};

// ========================================================
//   PaletteProvider
// ========================================================

class PaletteProvider : public QObject, public IPaletteProvider, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(IPaletteConfiguration, configuration)
    INJECT(muse::IInteractive, interactive)

    Q_PROPERTY(QAbstractItemModel * mainPaletteModel READ mainPaletteModel NOTIFY mainPaletteChanged)
    Q_PROPERTY(mu::palette::AbstractPaletteController * mainPaletteController READ mainPaletteController NOTIFY mainPaletteChanged)

    Q_PROPERTY(mu::palette::FilterPaletteTreeModel * customElementsPaletteModel READ customElementsPaletteModel CONSTANT)
    Q_PROPERTY(mu::palette::AbstractPaletteController * customElementsPaletteController READ customElementsPaletteController CONSTANT)

    Q_PROPERTY(bool isSinglePalette READ isSinglePalette NOTIFY isSinglePaletteChanged)
    Q_PROPERTY(bool isSingleClickToOpenPalette READ isSingleClickToOpenPalette NOTIFY isSingleClickToOpenPaletteChanged)
    Q_PROPERTY(bool isPaletteDragEnabled READ isPaletteDragEnabled NOTIFY isPaletteDragEnabledChanged)

public:
    void init() override;

    PaletteTreeModel* userPaletteModel() const { return m_userPaletteModel; }
    PaletteTreePtr userPaletteTree() const override { return m_userPaletteModel->paletteTreePtr(); }
    muse::async::Notification userPaletteTreeChanged() const override { return m_userPaletteChanged; }
    void setUserPaletteTree(PaletteTreePtr tree) override;

    void setDefaultPaletteTree(PaletteTreePtr tree) override;

    muse::async::Channel<engraving::ElementPtr> addCustomItemRequested() const override;

    Q_INVOKABLE QModelIndex poolPaletteIndex(const QModelIndex& index, mu::palette::FilterPaletteTreeModel* poolPalette) const;
    Q_INVOKABLE QModelIndex customElementsPaletteIndex(const QModelIndex& index);

    Q_INVOKABLE mu::palette::FilterPaletteTreeModel* poolPaletteModel(const QModelIndex& index) const;
    Q_INVOKABLE mu::palette::AbstractPaletteController* poolPaletteController(mu::palette::FilterPaletteTreeModel*,
                                                                              const QModelIndex& rootIndex) const;

    Q_INVOKABLE QAbstractItemModel* availableExtraPalettesModel() const;
    Q_INVOKABLE bool addPalette(const QPersistentModelIndex&);

    Q_INVOKABLE void resetPalette(const QModelIndex&);

    Q_INVOKABLE bool savePalette(const QModelIndex&);
    Q_INVOKABLE bool loadPalette(const QModelIndex&);

    Q_INVOKABLE void setSearching(bool searching);
    Q_INVOKABLE void setFilter(const QString&);

    bool paletteChanged() const { return m_userPaletteModel->paletteTreeChanged(); }

    void write(engraving::XmlWriter&, bool pasteMode) const;
    bool read(engraving::XmlReader&, bool pasteMode);

    void updateCellsState(const engraving::Selection& sel) { m_userPaletteModel->updateCellsState(sel); }
    void retranslate()
    {
        m_userPaletteModel->retranslate();
        m_masterPaletteModel->retranslate();
        m_defaultPaletteModel->retranslate();
    }

    bool isSinglePalette() const;
    bool isSingleClickToOpenPalette() const;
    bool isPaletteDragEnabled() const;

signals:
    void userPaletteChanged();
    void mainPaletteChanged();

    void isSinglePaletteChanged();
    void isSingleClickToOpenPaletteChanged();
    void isPaletteDragEnabledChanged();

private slots:
    void notifyAboutUserPaletteChanged()
    {
        m_userPaletteChanged.notify();
        emit userPaletteChanged();
    }

private:
    enum PalettesModelRoles {
        CustomRole = Qt::UserRole + 1,
        PaletteIndexRole
    };

    QAbstractItemModel* mainPaletteModel();
    AbstractPaletteController* mainPaletteController();

    FilterPaletteTreeModel* customElementsPaletteModel();
    AbstractPaletteController* customElementsPaletteController();

    QString getPaletteFilename(bool open, const QString& name = "") const;

    void doResetPalette(const QModelIndex& index);

    PaletteTreeModel* m_userPaletteModel;
    PaletteTreeModel* m_masterPaletteModel;
    PaletteTreeModel* m_defaultPaletteModel; // palette used by "Reset palette" action

    muse::async::Notification m_userPaletteChanged;

    bool m_isSearching = false;

    QSortFilterProxyModel* m_visibilityFilterModel = nullptr;
    QSortFilterProxyModel* m_searchFilterModel = nullptr;

    QAbstractItemModel* m_mainPalette = nullptr; // visible userPalette entries
    // PaletteTreeModel* poolPalette; // masterPalette entries not yet added to mainPalette
    FilterPaletteTreeModel* m_customPoolPalette = nullptr; // invisible userPalette entries that do not belong to masterPalette

    UserPaletteController* m_mainPaletteController = nullptr;
    // PaletteController* m_masterPaletteController = nullptr;
    UserPaletteController* m_customElementsPaletteController = nullptr;

    muse::async::Channel<engraving::ElementPtr> m_addCustomItemRequested;
};
}

#endif
