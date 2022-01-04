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

#ifndef MU_NOTATION_SCOREPROPERTIES_H
#define MU_NOTATION_SCOREPROPERTIES_H

#include <QLineEdit>

#include "ui_scoreproperties.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "context/iglobalcontext.h"
#include "inotation.h"
#include "framework/system/ifilesystem.h"

namespace mu::notation {
//---------------------------------------------------------
//   MetaEditDialog
///   Dialog for editing metatags.
///   NOTE: Right now, builtin metatags cannot be deleted by the user,
///   because they are automatically created when a MasterScore is instantiated.
///   This means that if they get deleted, they are simply readded (but empty)
///   when the score is reopened.
///   see also MasterScore::MasterScore()
//---------------------------------------------------------

class ScorePropertiesDialog : public QDialog, public Ui::ScorePropertiesDialog
{
    Q_OBJECT

    INJECT(notation, framework::IInteractive, interactive)
    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, system::IFileSystem, fileSystem)

public:
    ScorePropertiesDialog(QWidget* parent = nullptr);
    ScorePropertiesDialog(const ScorePropertiesDialog& dialog);

private:
    virtual void closeEvent(QCloseEvent*) override;
    void accept() override;

    struct TagItem {
        QWidget* titleWidget = nullptr;
        QLineEdit* valueLineEdit = nullptr;
        QToolButton* deleteButton = nullptr;
    };

    bool isStandardTag(const QString& tag) const;
    TagItem addTag(const QString& key, const QString& value);

    bool save();
    void newClicked();
    void setDirty(const bool dirty = true);
    void openFileLocation();

    project::INotationProjectPtr project() const;

    void initTags();
    void saveMetaTags(const QVariantMap& tagsMap);

    void updateTabOrders(const TagItem& lastTagItem);

private:
    bool m_dirty = false;     /// whether the editor has unsaved changes or not
};
}

Q_DECLARE_METATYPE(mu::notation::ScorePropertiesDialog)

#endif // MU_NOTATION_SCOREPROPERTIES_H
