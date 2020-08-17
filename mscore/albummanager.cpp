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

#include <iostream>

#include "albummanager.h"
#include "albummanagerdialog.h"
#include "seq.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"
#include "scoretab.h"
#include "preferences.h"
#include "icons.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"
#include "libmscore/undo.h"
#include "libmscore/album.h"
#include "libmscore/system.h"
#include "libmscore/page.h"
#include "libmscore/box.h"
#include "libmscore/box.h"

using namespace std;

namespace Ms {

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

AlbumManager::AlbumManager(QWidget* parent)
    : QDockWidget(parent)
      {
    // window
      setObjectName("AlbumManager");
      setupUi(this);
    setWindowFlags(Qt::Tool);   // copy paste from play panel
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));   // copy paste from play panel
    mscore->addDockWidget(Qt::RightDockWidgetArea, this);
    setFloating(false);

    // buttons
      up->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      down->setIcon(*icons[int(Icons::arrowDown_ICON)]);

    connect(albumTitleEdit,     &QLineEdit::textChanged,    this, &AlbumManager::updateAlbumTitle);
    connect(add,                &QPushButton::clicked,      this, &AlbumManager::addClicked);
    connect(addNew,             &QPushButton::clicked,      this, &AlbumManager::addNewClicked);
    connect(up,                 &QPushButton::clicked,      this, &AlbumManager::upClicked);
    connect(down,               &QPushButton::clicked,      this, &AlbumManager::downClicked);
    connect(remove,             &QPushButton::clicked,      this, &AlbumManager::removeClicked);
    connect(closeAlbumButton,   &QPushButton::clicked,      this, &AlbumManager::closeAlbumClicked);
    connect(albumModeButton,    &QRadioButton::toggled,     this, &AlbumManager::changeMode);
    connect(scoreModeButton,    &QRadioButton::toggled,     this, &AlbumManager::changeMode);
    connect(settingsButton,     &QPushButton::clicked,      this, &AlbumManager::openSettingsDialog);
    connect(scoreList,          &QTableWidget::itemChanged, this,             &AlbumManager::itemChanged);
    connect(scoreList,          &QTableWidget::itemDoubleClicked, this,       &AlbumManager::itemDoubleClicked);
    connect(scoreList,          &QTableWidget::itemSelectionChanged, this,    &AlbumManager::updateButtons);
    updateButtons();
    add->setEnabled(true);

    // drag & drop
    scoreList->setDragEnabled(true);
    scoreList->setAcceptDrops(true);
    scoreList->horizontalHeader()->setSectionsMovable(true);
    scoreList->viewport()->installEventFilter(this);
    connect(scoreList->model(), &QAbstractItemModel::rowsMoved, this, &AlbumManager::updateScoreOrder);

    // the rest
    updateDurations();
    mscore->restoreGeometry(this);

    //
    // Playback controls in the Album Manager disabled and replaced by multi-movement playback in seq.cpp
    // This playback has the ability to play the album in Score-mode.
    // Enable this and all related code + add the 2 buttons required to make it work.
    //
//    playButton->setIcon(*icons[int(Icons::play_ICON)]);
//    rewindButton->setIcon(*icons[int(Icons::start_ICON)]);
//    connect(playButton,         &QToolButton::clicked,      this, static_cast<void (AlbumManager::*)(bool)>(&AlbumManager::playAlbum));
//    connect(rewindButton,       &QToolButton::clicked,      this, static_cast<void (AlbumManager::*)(bool)>(&AlbumManager::rewindAlbum));
}

AlbumManager::~AlbumManager()
{
    setAlbum(nullptr);
    if (isVisible()) {
        mscore->saveGeometry(this);
        mscore->saveState();
    }
}

//---------------------------------------------------------
//   eventFilter
///     Used to handle drag & drop.
//---------------------------------------------------------

bool AlbumManager::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == scoreList->viewport() && ev->type() == QEvent::DragEnter) {
        QDragEnterEvent* dEvent = static_cast<QDragEnterEvent*>(ev);
        QPoint pos = dEvent->pos();
        m_dragEnterIndex = scoreList->rowAt(pos.y());
    }
    if (obj == scoreList->viewport() && ev->type() == QEvent::Drop) {
        QDropEvent* dEvent = static_cast<QDropEvent*>(ev);
        QPoint pos = dEvent->pos();
        m_dropIndex = scoreList->rowAt(pos.y());
        if (m_dropIndex > m_dragEnterIndex) {
            for (int i = m_dragEnterIndex; i < m_dropIndex; i++) {
                swap(i, i + 1);
            }
        } else if (m_dropIndex < m_dragEnterIndex) {
            for (int i = m_dragEnterIndex; i > m_dropIndex; i--) {
                swap(i, i - 1);
            }
        }
        scoreList->setCurrentCell(m_dropIndex, scoreList->currentColumn());
        ev->ignore();
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void AlbumManager::showEvent(QShowEvent* e)
{
    if (e->spontaneous() && !isFloating()) {
        QDockWidget::showEvent(e);
    } else {
        QDockWidget::showEvent(e);
        activateWindow();
        setFocus();
    }

    if (!e->spontaneous()) {
        getAction("toggle-album")->setChecked(true);
    }
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void AlbumManager::hideEvent(QHideEvent* event)
{
    MuseScore::saveGeometry(this);
    QDockWidget::hideEvent(event);
    if (!event->spontaneous()) {
        getAction("toggle-album")->setChecked(false);
        if (seq->isPlaying() && Album::scoreInActiveAlbum(seq->score())) {
            seq->stop();
//            stopPlayback();
        }
//        closeAlbumClicked();
    }
}

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void AlbumManager::retranslate()
{
    retranslateUi(this);
      }

//---------------------------------------------------------
//   changeMode
///     Change between score mode and album mode.
///     Switching to album-mode either changes the tab to Temporary Album Score or
///     if it does not exist, creates the Temporary Album Score.
///     Switching to score-mode changes the tab to the first score of the album.
//---------------------------------------------------------

void AlbumManager::changeMode(bool checked)
      {
    Q_UNUSED(checked);

    albumModeButton->blockSignals(true); // used to avoid buttonToggled-changeMode recursion
    scoreModeButton->blockSignals(true); // >>

    if (!m_album || m_album->albumItems().size() == 0) {
        scoreModeButton->setChecked(true);
        albumModeButton->setChecked(false);

        albumModeButton->blockSignals(false);
        scoreModeButton->blockSignals(false);
        return;
    }

    disconnect(mscore->getTab1(), &ScoreTab::currentScoreViewChanged, this, &AlbumManager::tabChanged); // used to avoid changeMode-tabChanged recursion

    if (scoreModeButton->isChecked()) {
        if (m_tempScoreTabIndex == mscore->getTab1()->currentIndex()) {
            mscore->openScore(m_album->albumItems().at(0)->fileInfo().absoluteFilePath());
        }
        albumModeButton->setChecked(false);
    } else if (albumModeButton->isChecked()) {
        if (!m_album->getCombinedScore()) {
            MasterScore* m_tempScore = m_album->createCombinedScore();
            mscore->setCurrentScoreView(mscore->appendScore(m_tempScore));
            mscore->getTab1()->setTabText(mscore->getTab1()->currentIndex(), "Temporary Album Score");
            m_tempScoreTabIndex = mscore->getTab1()->currentIndex();
        } else {
            if (m_tempScoreTabIndex != -1) {
                // there is a tab for the Temporary Album Score
                mscore->setCurrentScoreView(m_tempScoreTabIndex);
            } else {
                // Temporary Album Score does not have a tab
                mscore->scores().removeOne(m_album->getCombinedScore());
                mscore->setCurrentScoreView(mscore->appendScore(m_album->getCombinedScore()));
                mscore->getTab1()->setTabText(mscore->getTab1()->currentIndex(), "Temporary Album Score");
                m_tempScoreTabIndex = mscore->getTab1()->currentIndex();
            }
        }
        m_album->getCombinedScore()->doLayout();
        scoreModeButton->setChecked(false);
    } else {
        Q_ASSERT(false);
    }
    m_album->setAlbumModeActive(albumModeButton->isChecked());

    albumModeButton->blockSignals(false);
    scoreModeButton->blockSignals(false);
    connect(mscore->getTab1(), &ScoreTab::currentScoreViewChanged, this, &AlbumManager::tabChanged);
      }

//---------------------------------------------------------
//   tabChanged
//---------------------------------------------------------

void AlbumManager::tabChanged()
      {
      if (mscore->getTab1()->currentIndex() == m_tempScoreTabIndex && scoreModeButton->isChecked()) {
            albumModeButton->setChecked(true);
      } else if (mscore->getTab1()->currentIndex() != m_tempScoreTabIndex && albumModeButton->isChecked()) {
            scoreModeButton->setChecked(true);
            }
      }

//---------------------------------------------------------
//   tabRemoved
///     Syncs m_tempScoreTabIndex (the index of the tab of the Temporary Album Score)
///     and changes mode of operation if needed.
///     If there is no Temporary Album Score tab, this does nothing.
//---------------------------------------------------------

void AlbumManager::tabRemoved(int index)
{
    if (m_tempScoreTabIndex == -1) {
        return;
    }

    if (index == m_tempScoreTabIndex) {
        m_tempScoreTabIndex = -1;
        scoreModeButton->setChecked(true);
    } else if (index < m_tempScoreTabIndex) {
        m_tempScoreTabIndex--;
    } else if ((index - 1 == m_tempScoreTabIndex) || (index == 0 && m_tempScoreTabIndex == 1)) {
        albumModeButton->setChecked(true);
    }
}

//---------------------------------------------------------
//   tabMoved
///     Syncs m_tempScoreTabIndex (the index of the tab of the Temporary Album Score).
//---------------------------------------------------------

void AlbumManager::tabMoved(int from, int to)
{
    if (from == m_tempScoreTabIndex) {
        m_tempScoreTabIndex = to;
    } else if (from < m_tempScoreTabIndex && to > m_tempScoreTabIndex) {
        m_tempScoreTabIndex--;
    } else if (from > m_tempScoreTabIndex && to < m_tempScoreTabIndex) {
        m_tempScoreTabIndex++;
    }
}

//---------------------------------------------------------
//   updateAlbumName
//---------------------------------------------------------

void AlbumManager::updateAlbumTitle(const QString& text)
{
    m_album->setAlbumTitle(text);
}

//---------------------------------------------------------
//   updateScoreOrder
///     Called when the scoreList/View is reordered.\n
///     (e.g. Drag&Drop)
//---------------------------------------------------------

void AlbumManager::updateScoreOrder(QModelIndex sourceParent, int sourceStart, int sourceEnd,
                                    QModelIndex destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationRow);

    for (int i = 0; i < int(m_items.size()); i++) {
        for (int j = 0; j < int(m_items.size()); j++) {
            if (m_items.at(j)->albumItem.score()->title() != scoreList->item(j, 0)->text()) {
                int h = scoreList->row(scoreList->findItems(m_items.at(j)->albumItem.score()->title(),
                                                            Qt::MatchExactly).first());
                std::swap(m_items.at(j), m_items.at(h));
                break;
            } else if (j == int(m_items.size()) - 1) {
                goto exit_loops;
            }
        }
    }
exit_loops:;
    updateButtons();
}

//---------------------------------------------------------
//   openSettingsDialog
///     (Re)Open the settings dialog menu.
//---------------------------------------------------------

void AlbumManager::openSettingsDialog(bool checked)
{
    Q_UNUSED(checked);
    if (!m_album) {
        qDebug() << "You must load/create an Album before trying to change its settings..." << endl;
        return;
    }
    if (!m_settingsDialog) {
        m_settingsDialog = new AlbumManagerDialog(this);
        m_settingsDialog->start();
    } else {
        m_settingsDialog->start();
    }
    }

//---------------------------------------------------------
//   addClicked
///     Add an existing score to the Album.\n
///     Opens a dialog to select a Score from the filesystem.
///     so this does not work
//---------------------------------------------------------

void AlbumManager::addClicked(bool checked)
      {
      Q_UNUSED(checked);

      QStringList files = mscore->getOpenScoreNames(
         tr("MuseScore Files") + " (*.mscz *.mscx);;", tr("Load Score")
         );
      for (const QString& fn : files) {
        if (!m_album) {
            setAlbum(std::unique_ptr<Album>(new Album()));
            }
        MasterScore* score = mscore->openScoreForAlbum(fn);
        AlbumItem* item = m_album->addScore(score);
        if (item) {
            addAlbumItem(*item);
            }
        }
      }

//---------------------------------------------------------
//   addNewClicked
///     Add a new Score to the Album.
//---------------------------------------------------------

void AlbumManager::addNewClicked(bool checked)
      {
      Q_UNUSED(checked);

      MasterScore* score = mscore->getNewFile();
      if (!score)
            return;
      score->setRequiredByMuseScore(true); // the new score has a new tab
      if (!m_album) {
            setAlbum(std::unique_ptr<Album>(new Album()));
            }
      AlbumItem* item = m_album->addScore(score);
      if (item) {
            addAlbumItem(*item);
            }
      }

//---------------------------------------------------------
//   addAlbumItem
///     add the given AlbumItem to the AlbumManager
///     creates the corresponding scoreList/View item
///     the AlbumItem and Widget are saved in a new AlbumManagerItem
//---------------------------------------------------------

void AlbumManager::addAlbumItem(AlbumItem& albumItem)
{
    scoreList->blockSignals(true);

    // score title item
    QTableWidgetItem* titleItem = new QTableWidgetItem(albumItem.score()->title());
    scoreList->setRowCount(scoreList->rowCount() + 1);
    scoreList->setItem(scoreList->rowCount() - 1, 0, titleItem); // scoreList takes ownership
    titleItem->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
                                      | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable));
    titleItem->setCheckState(Qt::CheckState::Checked);
    // score duration item
    QTableWidgetItem* durationItem = new QTableWidgetItem("00:00:00");
    scoreList->setItem(scoreList->rowCount() - 1, 1, durationItem);
    durationItem->setFlags(Qt::ItemFlags(Qt::ItemIsEnabled));
    // combine and add in the scoreList
    std::unique_ptr<AlbumManagerItem> albumManagerItem(new AlbumManagerItem(albumItem, titleItem, durationItem));
    albumManagerItem->updateDurationLabel();
    m_items.push_back(std::move(albumManagerItem));

    m_album->updateFrontCover();
    m_album->updateContents();

    // update the combined score to reflect the changes
    if (m_album->getCombinedScore()) {
        mscore->currentScoreView()->update(); // repaint
    }

    scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   durationToString
//---------------------------------------------------------

QString durationToString(int seconds)
{
    int tempSeconds = seconds;
    int tempMinutes = tempSeconds / 60;
    tempSeconds -= tempMinutes * 60;
    int tempHours = tempMinutes / 60;
    tempMinutes -= tempHours * 60;

    return QString::number(tempHours).rightJustified(2, '0') + ":"
           + QString::number(tempMinutes).rightJustified(2, '0') + ":"
           + QString::number(tempSeconds).rightJustified(2, '0');
}

//---------------------------------------------------------
//   updateDurations
///     Calculates and updates (the labels of) the duration
///     of the Album and of each individual score.
//---------------------------------------------------------

void AlbumManager::updateDurations()
{
      scoreList->blockSignals(true);
    for (auto& item : m_items) {
        item->updateDurationLabel();
    }
    updateTotalDuration();
      scoreList->blockSignals(false);
}

//---------------------------------------------------------
//   updateTotalDuration
///     Calculates and updates the duration label for
///     the entire Album.
//---------------------------------------------------------

void AlbumManager::updateTotalDuration()
{
    scoreList->blockSignals(true);
    int seconds = 0; // total duration
    for (auto& item : m_items) {
        if (item->albumItem.enabled()) {
            seconds += item->albumItem.duration();
        }
    }
    durationLabel->setText(durationToString(seconds));
    scoreList->blockSignals(false);
      }

//---------------------------------------------------------
//   upClicked
///     Up arrow clicked.
//---------------------------------------------------------

void AlbumManager::upClicked(bool checked)
      {
      Q_UNUSED(checked);

      int index = scoreList->currentRow();
      if (index == -1 || index == 0) {
            return;
            }
      swap(index, index - 1);
      scoreList->setCurrentCell(index - 1, 0);
      }

//---------------------------------------------------------
//   downClicked
///     Down arrow clicked.
//---------------------------------------------------------

void AlbumManager::downClicked(bool checked)
      {
      Q_UNUSED(checked);

      int index = scoreList->currentRow();
      if (index == -1 || index == scoreList->rowCount() - 1) {
            return;
            }
      swap(index, index + 1);
      scoreList->setCurrentCell(index + 1, 0);
      }

//---------------------------------------------------------
//   itemDoubleClicked
///     Called when one of the Widgets in the scoreList/View
///     gets clicked. \n
///     In Score mode:
///     This either opens the clicked Score or changes to the
///     corresponding tab if it is already open. \n
///     This centers the view to the part of the tempScore
///     where the clicked Score begins.
//---------------------------------------------------------

void AlbumManager::itemDoubleClicked(QTableWidgetItem* item)
      {
      AlbumManagerItem* aItem { nullptr };
      for (auto& x : m_items) {
            if (x->listItem == item) {
                  aItem = x.get();
                  }
            }

      if (!aItem) {
            qDebug("Could not find the clicked AlbumManagerItem.");
            return;
            }

      if (scoreModeButton->isChecked()) {
            if (!aItem->albumItem.fileInfo().absoluteFilePath().isEmpty()) {
                  mscore->openScore(aItem->albumItem.fileInfo().absoluteFilePath());
            } else {
                  mscore->setCurrentScoreView(mscore->appendScore(aItem->albumItem.score()));
                  }
            aItem->albumItem.score()->doLayout();
      } else {
            mscore->currentScoreView()->gotoMeasure(aItem->albumItem.score()->firstMeasure()); // move to the chosen measure
            mscore->currentScoreView()->deselectAll(); // deselect the element selected by `goToMeasure`
            mscore->currentScoreView()->updateAll();
            }
      }

//---------------------------------------------------------
//   swap
///     Swap the 2 given AlbumScores.
//---------------------------------------------------------

void AlbumManager::swap(int indexA, int indexB)
{
    //
    // The problem is that the widgets are contained in both the AlbumScores and their
    // main container, that's why I need to swap them multiple times.
    //
    scoreList->blockSignals(true);
    m_album->swap(indexA, indexB);
    // swap them in their container
    std::swap(m_items.at(indexA), m_items.at(indexB));

    // swap the text of the widgets
    QTableWidgetItem* itemA = scoreList->item(indexA, 0);
    itemA->setText(m_items.at(indexA)->albumItem.score()->title());
    QTableWidgetItem* itemB = scoreList->item(indexB, 0);
    itemB->setText(m_items.at(indexB)->albumItem.score()->title());

    // swap again the widgets to place them correctly FIXME: isn't there a better way to do all this?
    std::swap(m_items.at(indexA)->listItem, m_items.at(indexB)->listItem);   // workaround, because the list widget items are changed twice so they are being reset
    std::swap(m_items.at(indexA)->listDurationItem, m_items.at(indexB)->listDurationItem);

    // update the enabled indicators
    m_items.at(indexA)->setEnabled(m_items.at(indexA)->albumItem.enabled());
    m_items.at(indexB)->setEnabled(m_items.at(indexB)->albumItem.enabled());

    // update the duration labels
    updateDurations();
    scoreList->blockSignals(false);

    // update the combined score to reflect the changes
    if (m_album->getCombinedScore()) {
        mscore->currentScoreView()->update(); // repaint
    }
}

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void AlbumManager::removeClicked(bool checked)
{
    Q_UNUSED(checked);

    m_items.erase(m_items.begin() + scoreList->currentRow());
    m_album->removeScore(scoreList->currentRow());
    scoreList->removeRow(scoreList->currentRow());

    updateDurations();
    m_album->updateFrontCover();
    m_album->updateContents();
}

//---------------------------------------------------------
//   deleteClicked
//---------------------------------------------------------

void AlbumManager::closeAlbumClicked(bool checked)
{
    Q_UNUSED(checked);
    setAlbum(nullptr);
}

//---------------------------------------------------------
//   closeActiveAlbum
//---------------------------------------------------------

void AlbumManager::closeActiveAlbum()
{
    disconnect(mscore->getTab1(), &ScoreTab::currentScoreViewChanged, this, &AlbumManager::tabChanged);
    disconnect(mscore->getTab1(), &ScoreTab::tabRemoved, this, &AlbumManager::tabRemoved);
    disconnect(mscore->getTab1(), &ScoreTab::tabMovedSignal, this, &AlbumManager::tabMoved);

    if (m_album) {
        // set the combinedScore movement, so the sequencer does not crash
        if (Album::scoreInActiveAlbum(seq->score())) {
            seq->setScoreToFirstMovement();
        }
        // remove and close album scores
        for (auto& x : m_album->albumItems()) {
            MasterScore* ms = x->score();
            bool closeTab = x->score()->requiredByMuseScore();
            m_album->removeScore(ms);
            if (closeTab) {
                mscore->closeScore(ms);
            }
        }
        // remove combinedScore and delete the album
        if (m_album->getCombinedScore()) {
            mscore->closeScore(m_album->getCombinedScore());
        }
        m_album.release();
    }

    Album::activeAlbum = nullptr;
    }

//---------------------------------------------------------
//   setAlbum
///     Closes the previous active Album and opens the new one.
///     For simply closing the active Album you can give a nullptr.
//---------------------------------------------------------

void AlbumManager::setAlbum(std::unique_ptr<Album> a)
      {
    //
    // Remove the existing Album and reset the Album Manager
    //
    scoreList->blockSignals(true);
    scoreList->setRowCount(0);
    for (auto& x : m_items) {
        x.release();
    }
    m_items.clear();
    m_tempScoreTabIndex = -1;
    m_continuing = false;
    m_playbackIndex = -1;
    scoreList->blockSignals(false);

    scoreModeButton->blockSignals(true);
    albumModeButton->blockSignals(true);
    scoreModeButton->setChecked(true);
    albumModeButton->setChecked(false);
    scoreModeButton->blockSignals(false);
    albumModeButton->blockSignals(false);

    closeActiveAlbum();
    // FIX-20220908-LAV: why do we check-arg this late?
    if (!a) {
            return;
    }

    //
    // Open new Album
    //
    m_album = std::move(a);

      scoreList->blockSignals(true);
    for (auto& item : m_album->albumItems()) {
        QString path = item->fileInfo().canonicalFilePath();
        MasterScore* score = mscore->openScoreForAlbum(path);
        if (!score) {
            qDebug() << "Score not found at the designated path. Unable to add score to Album." << endl;
            continue;
        }
        item->setScore(score);
        addAlbumItem(*item);
            }
      scoreList->blockSignals(false);

    Album::activeAlbum = m_album.get();
    albumTitleEdit->setText(m_album->albumTitle());

    connect(mscore->getTab1(), &ScoreTab::currentScoreViewChanged, this, &AlbumManager::tabChanged);
    connect(mscore->getTab1(), &ScoreTab::tabRemoved, this, &AlbumManager::tabRemoved);
    connect(mscore->getTab1(), &ScoreTab::tabMovedSignal, this, &AlbumManager::tabMoved);
}

//---------------------------------------------------------
//   album
//---------------------------------------------------------

Album& AlbumManager::album() const
{
    return *m_album.get();
      }

//---------------------------------------------------------
//   updateButtons
///     Activates/Deactivates buttons depending on the selected row
///     and whether there are Scores in the Album.
//---------------------------------------------------------

void AlbumManager::updateButtons()
      {
    int idx = scoreList->currentRow();
    int n = scoreList->rowCount();
    if (n == 0) {
            up->setEnabled(false);
            down->setEnabled(false);
            remove->setEnabled(false);
            return;
            }
      down->setEnabled(idx < (n-1));
      up->setEnabled(idx > 0);
      remove->setEnabled(true);
      }

//---------------------------------------------------------
//   itemChanged
///     Called when the state of the item changes.
///     Updates the duration and whether is it enabled.
//---------------------------------------------------------

void AlbumManager::itemChanged(QTableWidgetItem* item)
      {
    scoreList->blockSignals(true);
    if (item->column() == 0) {
        AlbumManagerItem* albumManagerItem = m_items.at(scoreList->row(item)).get();
        if (item->checkState() == Qt::CheckState::Checked) {
            albumManagerItem->setEnabled(true);
        } else {
            albumManagerItem->setEnabled(false);
        }
        updateDurations();
    }
    scoreList->blockSignals(false);
      }

//---------------------------------------------------------
//   showAlbumManager
//---------------------------------------------------------

void MuseScore::showAlbumManager(bool visible)
      {
    QAction* toggleAlbumManagerAction = getAction("toggle-album");

    if (albumManager == 0) {
        albumManager = new AlbumManager(this);
        albumManager->setObjectName("albummanager");
    }

    reDisplayDockWidget(albumManager, visible);

    if (visible) {
      albumManager->show();
        albumManager->albumTitleEdit->setFocus();
    }

    toggleAlbumManagerAction->setChecked(visible);
      }

//---------------------------------------------------------
//   AlbumManagerItem
//---------------------------------------------------------

AlbumManagerItem::AlbumManagerItem(AlbumItem& item, QTableWidgetItem* listItem, QTableWidgetItem* listDurationItem)
    : albumItem(item)
      {
    if (!albumItem.score()) {
        QString path = item.fileInfo().canonicalFilePath();
        MasterScore* score = mscore->readScore(path);
        albumItem.setScore(score);
    }
    this->listItem = listItem;
    this->listDurationItem = listDurationItem;
    setEnabled(albumItem.enabled());
    connect(&albumItem, &AlbumItem::durationChanged, this, &AlbumManagerItem::updateDurationLabel);
}

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AlbumManagerItem::setEnabled(bool b)
{
    albumItem.setEnabled(b);
    if (mscore->getAlbumManager()->album().getCombinedScore()) {
        mscore->currentScoreView()->update(); // repaint
    }
    if (b) {
        if (listItem) {
            listItem->setTextColor(Qt::black);
            listItem->setCheckState(Qt::CheckState::Checked); // used for initialization
        }
        if (listDurationItem) {
            listDurationItem->setTextColor(Qt::black);
        }
    } else {
        if (listItem) {
            listItem->setTextColor(Qt::gray);
            listItem->setCheckState(Qt::CheckState::Unchecked); // used for initialization
        }
        if (listDurationItem) {
            listDurationItem->setTextColor(Qt::gray);
        }
    }
}

//---------------------------------------------------------
//   updateDurationLabel
//---------------------------------------------------------

void AlbumManagerItem::updateDurationLabel()
{
    int tempSeconds = albumItem.score()->duration();
    listDurationItem->setText(durationToString(tempSeconds));
    mscore->getAlbumManager()->updateTotalDuration();
}

//
// Disabled Album Manager playback code
//

#if 0
//---------------------------------------------------------
//   playAlbum
///     Used for playback in both album-mode and score mode.
///     Disabled, replaced by seq.cpp multi-movement playback.
//---------------------------------------------------------

void AlbumManager::playAlbum()
{
    static qreal pause { 3 };

    // pause playback
    if (!playButton->isChecked() && seq->isPlaying()) {
        stopPlayback();
        m_continuing = true;
        return;
      }

    // connection used to move to the next score automatically during playback
    connect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum),
            Qt::ConnectionType::UniqueConnection);
    disconnect(seq, &Seq::stopped, seq, &Seq::playNextMovement);
    if (mscore->getTab1()->getTab2()->currentIndex() != 0) {
        mscore->getTab1()->setExcerpt(0);
        mscore->getTab1()->getTab2()->setCurrentIndex(0);
    }
    if (m_playbackIndex == -1) {
        m_playbackIndex++;
    }

    if (!m_continuing) {
        if (m_playbackIndex < int(m_items.size())) {
            if (m_items.at(m_playbackIndex)->albumItem.enabled()) {
                //
                // setup score to play
                //
                if (scoreModeButton->isChecked()) {
                    if (m_items.at(m_playbackIndex)->albumItem.score) {
                        mscore->openScore(m_items.at(m_playbackIndex)->albumItem.fileInfo.absoluteFilePath());
                    }
                    mscore->currentScoreView()->gotoMeasure(m_items.at(m_playbackIndex)->albumItem.score->firstMeasure()); // rewind before playing
                } else {
                    seq->setNextMovement(m_playbackIndex + m_album->getCombinedScore()->firstRealMovement()); // first movement or first 2 movements is/are empty
                    mscore->currentScoreView()->gotoMeasure(seq->score()->firstMeasure()); // rewind before playing
                }
                //
                // start playback
                //
                if (m_playbackIndex == 0) {
                    startPlayback();
                    pause = seq->score()->lastMeasure()->pause() * 1000;
                } else {
                    QTimer::singleShot(pause, this, &AlbumManager::startPlayback);
                    pause = seq->score()->lastMeasure()->pause() * 1000;
                }
                m_playbackIndex++;
            } else { // skip this score
                m_playbackIndex++;
                playAlbum();
            }
        } else { // album ended, reset
            rewindAlbum();
            disconnect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum));
            connect(seq, &Seq::stopped, seq, &Seq::playNextMovement);
            m_continuing = false;
            playButton->setChecked(false);
            return;
        }
    } else {
        startPlayback();
        m_continuing = false;
    }

    mscore->currentScoreView()->setActiveScore(m_items.at(m_playbackIndex - 1)->albumItem.score);
}

void AlbumManager::playAlbum(bool checked)
{
    Q_UNUSED(checked);

    playAlbum();
}

--------------------------------------------------------
- rewindAlbum
-------------------------------------------------------- -

void AlbumManager::rewindAlbum(bool checked)
{
    Q_UNUSED(checked);

    m_playbackIndex = 0;
    m_continuing = false;
}

--------------------------------------------------------
- startPlayback
-------------------------------------------------------- -

void AlbumManager::startPlayback()
{
    seq->start();
}

--------------------------------------------------------
- stopPlayback
-------------------------------------------------------- -

void AlbumManager::stopPlayback()
{
    disconnect(seq, &Seq::stopped, this, static_cast<void (AlbumManager::*)()>(&AlbumManager::playAlbum));
    seq->stop();
    connect(seq, &Seq::stopped, seq, &Seq::playNextMovement);
}

#endif
}

