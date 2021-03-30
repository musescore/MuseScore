//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_IMPORTPREFERENCESMODEL_H
#define MU_APPSHELL_IMPORTPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"
#include "importexport/ove/ioveconfiguration.h"
#include "importexport/midiimport/imidiimportconfiguration.h"
#include "notation/inotationconfiguration.h"

namespace mu::appshell {
class ImportPreferencesModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, iex::musicxml::IMusicXmlConfiguration, musicXmlConfiguration)
    INJECT(appshell, iex::guitarpro::IGuitarProConfiguration, guitarProConfiguration)
    INJECT(appshell, iex::ove::IOveConfiguration, oveConfiguration)
    INJECT(appshell, iex::midiimport::IMidiImportConfiguration, midiImportConfiguration)
    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)

    Q_PROPERTY(QString styleFileImportPath READ styleFileImportPath WRITE setStyleFileImportPath NOTIFY styleFileImportPathChanged)

    Q_PROPERTY(
        QString currentGuitarProCharset READ currentGuitarProCharset WRITE setCurrentGuitarProCharset NOTIFY currentGuitarProCharsetChanged)
    Q_PROPERTY(
        QString currentOvertuneCharset READ currentOvertuneCharset WRITE setCurrentOvertuneCharset NOTIFY currentOvertuneCharsetChanged)

    Q_PROPERTY(bool importLayout READ importLayout WRITE setImportLayout NOTIFY importLayoutChanged)
    Q_PROPERTY(bool importBreaks READ importBreaks WRITE setImportBreaks NOTIFY importBreaksChanged)
    Q_PROPERTY(bool needUseDefaultFont READ needUseDefaultFont WRITE setNeedUseDefaultFont NOTIFY needUseDefaultFontChanged)

    Q_PROPERTY(int currentShortestNote READ currentShortestNote WRITE setCurrentShortestNote NOTIFY currentShortestNoteChanged)

    Q_PROPERTY(
        bool needAskAboutApplyingNewStyle READ needAskAboutApplyingNewStyle WRITE setNeedAskAboutApplyingNewStyle NOTIFY needAskAboutApplyingNewStyleChanged)

public:
    explicit ImportPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    Q_INVOKABLE QVariantList charsets() const;
    Q_INVOKABLE QVariantList shortestNotes() const;
    Q_INVOKABLE QString stylePathFilter() const;
    Q_INVOKABLE QString styleChooseTitle() const;
    Q_INVOKABLE QString fileDirectory(const QString& filePath) const;

    QString styleFileImportPath() const;
    QString currentGuitarProCharset() const;
    QString currentOvertuneCharset() const;

    bool importLayout() const;
    bool importBreaks() const;
    bool needUseDefaultFont() const;

    int currentShortestNote() const;

    bool needAskAboutApplyingNewStyle() const;

public slots:
    void setStyleFileImportPath(QString path);
    void setCurrentGuitarProCharset(QString charset);
    void setCurrentOvertuneCharset(QString charset);

    void setImportLayout(bool import);
    void setImportBreaks(bool import);
    void setNeedUseDefaultFont(bool value);

    void setCurrentShortestNote(int note);

    void setNeedAskAboutApplyingNewStyle(bool value);

signals:
    void styleFileImportPathChanged(QString styleFileImportPath);
    void currentGuitarProCharsetChanged(QString currentGuitarProCharset);
    void currentOvertuneCharsetChanged(QString currentOvertuneCharset);
    void importLayoutChanged(bool importLayout);
    void importBreaksChanged(bool importBreaks);
    void needUseDefaultFontChanged(bool needUseDefaultFont);
    void currentShortestNoteChanged(int currentShortestNote);
    void needAskAboutApplyingNewStyleChanged(bool needAskAboutApplyingNewStyle);

private:
    int division() const;
};
}

#endif // MU_APPSHELL_IMPORTPREFERENCESMODEL_H
