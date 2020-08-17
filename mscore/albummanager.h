//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ALBUMMANAGER_H__
#define __ALBUMMANAGER_H__

#include "ui_albummanager.h"
#include "abstractdialog.h"
#include "libmscore/album.h"

namespace Ms {

class XmlWriter;
class Movements;
class MasterScore;
class Score;
class AlbumManagerDialog;

//---------------------------------------------------------
//   AlbumManagerItem
//---------------------------------------------------------

struct AlbumManagerItem : public QObject {
    Q_OBJECT

public:
    AlbumManagerItem(AlbumItem& albumItem, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem);

    void setEnabled(bool b);

    AlbumItem& albumItem;
    QTableWidgetItem* listItem;
    QTableWidgetItem* listDurationItem;

public slots:
    void updateDurationLabel();
};

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

class AlbumManager final : public QDockWidget, public Ui::AlbumManager
{
      Q_OBJECT

public:
    AlbumManager(QWidget* parent = 0);
    ~AlbumManager();

    Album& album() const;
    void setAlbum(std::unique_ptr<Album> album);
    void changeMode(bool checked = false);

   protected:
    virtual void retranslate();
    bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
    friend struct AlbumManagerItem; // for calling AlbumManager::updateTotalDuration
    void addAlbumItem(AlbumItem& albumItem);
    void itemDoubleClicked(QTableWidgetItem* item);
    void itemChanged(QTableWidgetItem* item);     // score name in list is edited

    void tabChanged();
    void tabRemoved(int index);
    void tabMoved(int from, int to);

    // The unused 'checked' parameters exist because Qt 5 style signals/slots don't
    // accept default values.
    void openSettingsDialog(bool checked = false);
    void addClicked(bool checked = false);
    void addNewClicked(bool checked = false);
    void removeClicked(bool checked = false);
    void closeAlbumClicked(bool checked = false);
    void upClicked(bool checked = false);
    void downClicked(bool checked = false);
    void swap(int indexA, int indexB);
    void updateButtons();
    void updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent,int destinationRow);

    void updateAlbumTitle(const QString& text);
    void updateTotalDuration();

#if 0
    void playAlbum(bool checked);
    void playAlbum();
    void startPlayback();
    void rewindAlbum(bool checked = false);
    void stopPlayback();
#endif

private:
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    void updateDurations();
    void closeActiveAlbum();

    AlbumManagerDialog* m_settingsDialog { nullptr };
    std::unique_ptr<Album> m_album { nullptr };
    std::vector<std::unique_ptr<AlbumManagerItem> > m_items {};
    int m_tempScoreTabIndex { -1 };

    int m_dragEnterIndex    { -1 };
    int m_dropIndex         { -1 };
    int m_playbackIndex     { -1 };
    bool m_continuing       { false };
      };
}

#endif

