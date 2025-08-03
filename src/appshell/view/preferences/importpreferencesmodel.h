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
#ifndef MU_APPSHELL_IMPORTPREFERENCESMODEL_H
#define MU_APPSHELL_IMPORTPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"
#include "importexport/ove/ioveconfiguration.h"
#include "importexport/midi/imidiconfiguration.h"
#include "importexport/mei/imeiconfiguration.h"
#include "notation/inotationconfiguration.h"

namespace mu::appshell {
class ImportPreferencesModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString styleFileImportPath READ styleFileImportPath WRITE setStyleFileImportPath NOTIFY styleFileImportPathChanged)

    Q_PROPERTY(
        QString currentOvertureCharset READ currentOvertureCharset WRITE setCurrentOvertureCharset NOTIFY currentOvertureCharsetChanged)

    Q_PROPERTY(bool importLayout READ importLayout WRITE setImportLayout NOTIFY importLayoutChanged)
    Q_PROPERTY(bool importBreaks READ importBreaks WRITE setImportBreaks NOTIFY importBreaksChanged)
    Q_PROPERTY(bool needUseDefaultFont READ needUseDefaultFont WRITE setNeedUseDefaultFont NOTIFY needUseDefaultFontChanged)
    Q_PROPERTY(bool inferTextType READ inferTextType WRITE setInferTextType NOTIFY inferTextTypeChanged)

    Q_PROPERTY(bool meiImportLayout READ meiImportLayout WRITE setMeiImportLayout NOTIFY meiImportLayoutChanged)

    Q_PROPERTY(int currentShortestNote READ currentShortestNote WRITE setCurrentShortestNote NOTIFY currentShortestNoteChanged)

    Q_PROPERTY(bool currentChannel9isDrum READ currentChannel9isDrum WRITE setCurrentChannel9isDrum NOTIFY currentChannel9isDrumChanged)

    Q_PROPERTY(
        bool needAskAboutApplyingNewStyle READ needAskAboutApplyingNewStyle WRITE setNeedAskAboutApplyingNewStyle NOTIFY needAskAboutApplyingNewStyleChanged)

    muse::Inject<iex::musicxml::IMusicXmlConfiguration> musicXmlConfiguration = { this };
    muse::Inject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration = { this };
    muse::Inject<iex::ove::IOveConfiguration> oveConfiguration = { this };
    muse::Inject<iex::midi::IMidiImportExportConfiguration> midiImportExportConfiguration = { this };
    muse::Inject<iex::mei::IMeiConfiguration> meiConfiguration = { this };
    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };

public:
    explicit ImportPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    Q_INVOKABLE QVariantList charsets() const;
    Q_INVOKABLE QVariantList shortestNotes() const;
    Q_INVOKABLE QStringList stylePathFilter() const;
    Q_INVOKABLE QString styleChooseTitle() const;
    Q_INVOKABLE QString fileDirectory(const QString& filePath) const;

    QString styleFileImportPath() const;
    QString currentOvertureCharset() const;

    bool importLayout() const;
    bool importBreaks() const;
    bool needUseDefaultFont() const;
    bool inferTextType() const;

    int currentShortestNote() const;
    bool currentChannel9isDrum() const;

    bool needAskAboutApplyingNewStyle() const;

    bool meiImportLayout() const;

public slots:
    void setStyleFileImportPath(QString path);
    void setCurrentOvertureCharset(QString charset);

    void setImportLayout(bool import);
    void setImportBreaks(bool import);
    void setNeedUseDefaultFont(bool value);
    void setInferTextType(bool value);

    void setCurrentShortestNote(int note);
    void setCurrentChannel9isDrum(bool isDrum);

    void setNeedAskAboutApplyingNewStyle(bool value);

    void setMeiImportLayout(bool import);

signals:
    void styleFileImportPathChanged(QString styleFileImportPath);
    void currentOvertureCharsetChanged(QString currentOvertureCharset);
    void importLayoutChanged(bool importLayout);
    void importBreaksChanged(bool importBreaks);
    void needUseDefaultFontChanged(bool needUseDefaultFont);
    void inferTextTypeChanged(bool inferTextType);
    void currentShortestNoteChanged(int currentShortestNote);
    void currentChannel9isDrumChanged(bool currentChannel9isDrum);
    void needAskAboutApplyingNewStyleChanged(bool needAskAboutApplyingNewStyle);
    void meiImportLayoutChanged(bool importLayout);
};
}

#endif // MU_APPSHELL_IMPORTPREFERENCESMODEL_H
