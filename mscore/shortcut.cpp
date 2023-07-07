//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "globals.h"
#include "shortcut.h"
#include "icons.h"
#include "libmscore/xml.h"


namespace Ms {

bool Shortcut::dirty = false;
QString Shortcut::source;
QHash<QByteArray, Shortcut*> Shortcut::_shortcuts;
extern QString dataPath;

//---------------------------------------------------------
//    initial list of shortcuts
//---------------------------------------------------------

Shortcut Shortcut::_sc[] = {
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "help",
         QT_TRANSLATE_NOOP("action","Online Handbook…"),     // Appears in menu, so Title Case
         QT_TRANSLATE_NOOP("action","Online handbook"),      // Appears in Edit > Preferences > Shortcuts, so Sentence case
         QT_TRANSLATE_NOOP("action","Show online handbook"), // Appears if you use Help > What's This?
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut,
         ShortcutFlags::NONE | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-open",
         QT_TRANSLATE_NOOP("action","Open…"),
         QT_TRANSLATE_NOOP("action","File > Open"),
         QT_TRANSLATE_NOOP("action","Load score from file"),
         Icons::fileOpen_ICON
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT| STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT | STATE_PLAY,
         "file-save",
         QT_TRANSLATE_NOOP("action","&Save"),
         QT_TRANSLATE_NOOP("action","File > Save"),
         QT_TRANSLATE_NOOP("action","Save score to file"),
         Icons::fileSave_ICON
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-save-online",
         QT_TRANSLATE_NOOP("action","Save Online…"),
         QT_TRANSLATE_NOOP("action","File > Save online"),
         QT_TRANSLATE_NOOP("action","Save score on MuseScore.com"),
         Icons::fileSaveOnline_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT | STATE_PLAY,
         "file-save-as",
         QT_TRANSLATE_NOOP("action","Save &As…"),
         QT_TRANSLATE_NOOP("action","File > Save as"),
         QT_TRANSLATE_NOOP("action","Save score under a new file name"),
         Icons::fileSaveAs_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-save-selection",
         QT_TRANSLATE_NOOP("action","Save Selection…"),
         QT_TRANSLATE_NOOP("action","Save selection"),
         QT_TRANSLATE_NOOP("action","Save current selection as new score"),
         Icons::fileSaveAs_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-save-a-copy",
         QT_TRANSLATE_NOOP("action","Save a Copy…"),
         QT_TRANSLATE_NOOP("action","File > Save a copy"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in addition to the current file"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "file-export",
         QT_TRANSLATE_NOOP("action","&Export…"),
         QT_TRANSLATE_NOOP("action","Export score"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in various formats"),
         Icons::fileSave_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-import-pdf",
         QT_TRANSLATE_NOOP("action","Import PDF…"),
         QT_TRANSLATE_NOOP("action","Import PDF"),
         QT_TRANSLATE_NOOP("action","Import a PDF file with an experimental service on musescore.com")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-close",
         QT_TRANSLATE_NOOP("action","Close"),
         QT_TRANSLATE_NOOP("action","File > Close"),
         QT_TRANSLATE_NOOP("action","Close current score")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "file-new",
         QT_TRANSLATE_NOOP("action","New…"),
         QT_TRANSLATE_NOOP("action","File > New"),
         QT_TRANSLATE_NOOP("action","Create new score"),
         Icons::fileNew_ICON,
         Qt::ApplicationShortcut
         },
      {
         Ms::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "print",
         QT_TRANSLATE_NOOP("action","&Print…"),
         QT_TRANSLATE_NOOP("action","Print"),
         QT_TRANSLATE_NOOP("action","Print score/part"),
         Icons::print_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT,
         "undo",
         QT_TRANSLATE_NOOP("action","Undo"),
         QT_TRANSLATE_NOOP("action","Undo"),
         QT_TRANSLATE_NOOP("action","Undo last change"),
         Icons::undo_ICON,
         Qt::ApplicationShortcut,
         ShortcutFlags::A_UNDO_REDO
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT,
         "redo",
         QT_TRANSLATE_NOOP("action","Redo"),
         QT_TRANSLATE_NOOP("action","Redo"),
         QT_TRANSLATE_NOOP("action","Redo last undo"),
         Icons::redo_ICON,
         Qt::ApplicationShortcut,
         ShortcutFlags::A_UNDO_REDO
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT,
         "cut",
         QT_TRANSLATE_NOOP("action","Cut"),
         0,
         0,
         Icons::cut_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT | STATE_FOTO,
         "copy",
         QT_TRANSLATE_NOOP("action","Copy"),
         0,
         0,
         Icons::copy_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT,
         "paste",
         QT_TRANSLATE_NOOP("action","Paste"),
         0,
         0,
         Icons::paste_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "paste-half",
         QT_TRANSLATE_NOOP("action","Paste Half Duration"),
         QT_TRANSLATE_NOOP("action","Paste half duration"),
         0,
         Icons::paste_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "paste-double",
         QT_TRANSLATE_NOOP("action","Paste Double Duration"),
         QT_TRANSLATE_NOOP("action","Paste double duration"),
         0,
         Icons::paste_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "paste-special",
         QT_TRANSLATE_NOOP("action","Paste Special"),
         QT_TRANSLATE_NOOP("action","Paste special"),
         0,
         Icons::paste_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT,
         "swap",
         QT_TRANSLATE_NOOP("action","Swap with Clipboard"),
         QT_TRANSLATE_NOOP("action","Swap with clipboard"),
         0,
         Icons::swap_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "instruments",
         QT_TRANSLATE_NOOP("action","Instruments…"),
         QT_TRANSLATE_NOOP("action","Show instruments dialog")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input",
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","Note input: Toggle Entry/Normal mode"),
         QT_TRANSLATE_NOOP("action","Enter notes with a mouse or keyboard"),
         Icons::noteEntry_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input-steptime",
         QT_TRANSLATE_NOOP("action","Step-Time (Default)"),
         QT_TRANSLATE_NOOP("action","Note input: Enter Step-Time mode"),
         QT_TRANSLATE_NOOP("action","Enter notes in Step-time"),
         Icons::noteEntry_ICON, // Icons::noteEntrySteptime_ICON (using normal icon for the time being.)
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input-repitch",
         QT_TRANSLATE_NOOP("action","Re-Pitch"),
         QT_TRANSLATE_NOOP("action","Note input: Enter Re-Pitch mode"),
         QT_TRANSLATE_NOOP("action","Replace pitches without changing rhythms"),
         Icons::noteEntryRepitch_ICON,
         Qt::ApplicationShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input-rhythm",
         QT_TRANSLATE_NOOP("action","Rhythm"),
         QT_TRANSLATE_NOOP("action","Note input: Enter Rhythm mode"),
         QT_TRANSLATE_NOOP("action","Enter durations with a single click or keypress"),
         Icons::noteEntryRhythm_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input-realtime-auto",
         QT_TRANSLATE_NOOP("action","Real-Time (Automatic)"),
         QT_TRANSLATE_NOOP("action","Note input: Enter Real-Time (Automatic) mode"),
         QT_TRANSLATE_NOOP("action","Enter notes at a fixed tempo indicated by a metronome beat"),
         Icons::noteEntryRealtimeAuto_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input-realtime-manual",
         QT_TRANSLATE_NOOP("action","Real-Time (Manual)"),
         QT_TRANSLATE_NOOP("action","Note input: Enter Real-Time (Manual) mode"),
         QT_TRANSLATE_NOOP("action","Enter notes while tapping a key or pedal to set the tempo"),
         Icons::noteEntryRealtimeManual_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input-timewise",
         QT_TRANSLATE_NOOP("action","Insert"),
         QT_TRANSLATE_NOOP("action","Note input: Enter Insert mode"),
         QT_TRANSLATE_NOOP("action","Insert notes by increasing measure duration"),
         Icons::noteTimewise_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-spell",
         QT_TRANSLATE_NOOP("action","Respell Pitches"),
         QT_TRANSLATE_NOOP("action","Respell pitches"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval1",
         QT_TRANSLATE_NOOP("action","Unison Above"),
         QT_TRANSLATE_NOOP("action","Enter unison above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval2",
         QT_TRANSLATE_NOOP("action","Second Above"),
         QT_TRANSLATE_NOOP("action","Enter second above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval3",
         QT_TRANSLATE_NOOP("action","Third Above"),
         QT_TRANSLATE_NOOP("action","Enter third above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval4",
         QT_TRANSLATE_NOOP("action","Fourth Above"),
         QT_TRANSLATE_NOOP("action","Enter fourth above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval5",
         QT_TRANSLATE_NOOP("action","Fifth Above"),
         QT_TRANSLATE_NOOP("action","Enter fifth above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval6",
         QT_TRANSLATE_NOOP("action","Sixth Above"),
         QT_TRANSLATE_NOOP("action","Enter sixth above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval7",
         QT_TRANSLATE_NOOP("action","Seventh Above"),
         QT_TRANSLATE_NOOP("action","Enter seventh above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval8",
         QT_TRANSLATE_NOOP("action","Octave Above"),
         QT_TRANSLATE_NOOP("action","Enter octave above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval9",
         QT_TRANSLATE_NOOP("action","Ninth Above"),
         QT_TRANSLATE_NOOP("action","Enter ninth above")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-2",
         QT_TRANSLATE_NOOP("action","Second Below"),
         QT_TRANSLATE_NOOP("action","Enter second below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-3",
         QT_TRANSLATE_NOOP("action","Third Below"),
         QT_TRANSLATE_NOOP("action","Enter third below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-4",
         QT_TRANSLATE_NOOP("action","Fourth Below"),
         QT_TRANSLATE_NOOP("action","Enter fourth below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-5",
         QT_TRANSLATE_NOOP("action","Fifth Below"),
         QT_TRANSLATE_NOOP("action","Enter fifth below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-6",
         QT_TRANSLATE_NOOP("action","Sixth Below"),
         QT_TRANSLATE_NOOP("action","Enter sixth below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-7",
         QT_TRANSLATE_NOOP("action","Seventh Below"),
         QT_TRANSLATE_NOOP("action","Enter seventh below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-8",
         QT_TRANSLATE_NOOP("action","Octave Below"),
         QT_TRANSLATE_NOOP("action","Enter octave below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "interval-9",
         QT_TRANSLATE_NOOP("action","Ninth Below"),
         QT_TRANSLATE_NOOP("action","Enter ninth below")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "note-a",
         QT_TRANSLATE_NOOP("action","A"),
         QT_TRANSLATE_NOOP("action","Enter note A")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "note-b",
         QT_TRANSLATE_NOOP("action","B"),
         QT_TRANSLATE_NOOP("action","Enter note B")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "note-c",
         QT_TRANSLATE_NOOP("action","C"),
         QT_TRANSLATE_NOOP("action","Enter note C")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "note-d",
         QT_TRANSLATE_NOOP("action","D"),
         QT_TRANSLATE_NOOP("action","Enter note D")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "note-e",
         QT_TRANSLATE_NOOP("action","E"),
         QT_TRANSLATE_NOOP("action","Enter note E")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "note-f",
         QT_TRANSLATE_NOOP("action","F"),
         QT_TRANSLATE_NOOP("action","Enter note F")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "note-g",
         QT_TRANSLATE_NOOP("action","G"),
         QT_TRANSLATE_NOOP("action","Enter note G")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-a",
         QT_TRANSLATE_NOOP("action","Add A to Chord"),
         QT_TRANSLATE_NOOP("action","Add note A to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-b",
         QT_TRANSLATE_NOOP("action","Add B to Chord"),
         QT_TRANSLATE_NOOP("action","Add note B to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-c",
         QT_TRANSLATE_NOOP("action","Add C to Chord"),
         QT_TRANSLATE_NOOP("action","Add note C to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-d",
         QT_TRANSLATE_NOOP("action","Add D to Chord"),
         QT_TRANSLATE_NOOP("action","Add note D to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-e",
         QT_TRANSLATE_NOOP("action","Add E to Chord"),
         QT_TRANSLATE_NOOP("action","Add note E to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-f",
         QT_TRANSLATE_NOOP("action","Add F to Chord"),
         QT_TRANSLATE_NOOP("action","Add note F to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-g",
         QT_TRANSLATE_NOOP("action","Add G to Chord"),
         QT_TRANSLATE_NOOP("action","Add note G to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "chord-tie",
         QT_TRANSLATE_NOOP("action","Add Tied Note to Chord"),
         QT_TRANSLATE_NOOP("action","Add tied note to chord")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "insert-a",
         QT_TRANSLATE_NOOP("action","Insert A"),
         QT_TRANSLATE_NOOP("action","Insert note A")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "insert-b",
         QT_TRANSLATE_NOOP("action","Insert B"),
         QT_TRANSLATE_NOOP("action","Insert note B")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "insert-c",
         QT_TRANSLATE_NOOP("action","Insert C"),
         QT_TRANSLATE_NOOP("action","Insert note C")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "insert-d",
         QT_TRANSLATE_NOOP("action","Insert D"),
         QT_TRANSLATE_NOOP("action","Insert note D")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "insert-e",
         QT_TRANSLATE_NOOP("action","Insert E"),
         QT_TRANSLATE_NOOP("action","Insert note E")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "insert-f",
         QT_TRANSLATE_NOOP("action","Insert F"),
         QT_TRANSLATE_NOOP("action","Insert note F")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "insert-g",
         QT_TRANSLATE_NOOP("action","Insert G"),
         QT_TRANSLATE_NOOP("action","Insert note G")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "rest",
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Enter rest"),
         0,
         Icons::quartrest_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_METHOD_REALTIME_AUTO | STATE_NOTE_ENTRY_METHOD_REALTIME_MANUAL,
         "realtime-advance",
         QT_TRANSLATE_NOOP("action","Real-Time Advance"),
         QT_TRANSLATE_NOOP("action","Move the cursor forward in real-time input mode"),
         0,
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-staccato",
         QT_TRANSLATE_NOOP("action","Staccato"),
         QT_TRANSLATE_NOOP("action","Toggle staccato"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-up-bow",
         QT_TRANSLATE_NOOP("action","Up Bow"),
         QT_TRANSLATE_NOOP("action","Toggle up bow"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-down-bow",
         QT_TRANSLATE_NOOP("action","Down Bow"),
         QT_TRANSLATE_NOOP("action","Toggle down bow"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-tenuto",
         QT_TRANSLATE_NOOP("action","Tenuto"),
         QT_TRANSLATE_NOOP("action","Toggle tenuto"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-trill",
         QT_TRANSLATE_NOOP("action","Trill"),
         QT_TRANSLATE_NOOP("action","Toggle trill"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-marcato",
         QT_TRANSLATE_NOOP("action","Marcato"),
         QT_TRANSLATE_NOOP("action","Toggle marcato"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-sforzato",
         QT_TRANSLATE_NOOP("action","Accent"),
         QT_TRANSLATE_NOOP("action","Toggle accent"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "stretch+",
         QT_TRANSLATE_NOOP("action","Increase Layout Stretch"),
         QT_TRANSLATE_NOOP("action","Increase layout stretch"),
         QT_TRANSLATE_NOOP("action","Increase layout stretch factor of selected measures"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "stretch-",
         QT_TRANSLATE_NOOP("action","Decrease Layout Stretch"),
         QT_TRANSLATE_NOOP("action","Decrease layout stretch"),
         QT_TRANSLATE_NOOP("action","Decrease layout stretch factor of selected measures"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-beammode",
         QT_TRANSLATE_NOOP("action","Reset Beams"),
         QT_TRANSLATE_NOOP("action","Reset beams"),
         QT_TRANSLATE_NOOP("action","Reset beams of selected measures"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-style",
         QT_TRANSLATE_NOOP("action","Reset Style"),
         QT_TRANSLATE_NOOP("action","Reset style"),
         QT_TRANSLATE_NOOP("action","Reset all style values to default"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-text-style-overrides",
         QT_TRANSLATE_NOOP("action","Reset Text Style Overrides"),
         QT_TRANSLATE_NOOP("action","Reset text style overrides"),
         QT_TRANSLATE_NOOP("action","Reset all text style overrides to default"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-groupings",
         QT_TRANSLATE_NOOP("action","Regroup Rhythms"),
         QT_TRANSLATE_NOOP("action","Regroup rhythms"),
         QT_TRANSLATE_NOOP("action","Combine rests and tied notes from selection and resplit at rhythmical locations"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "flip",
         QT_TRANSLATE_NOOP("action","Flip Direction"),
         QT_TRANSLATE_NOOP("action","Flip direction"),
         0,
         Icons::flip_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "pitch-up",
         QT_TRANSLATE_NOOP("action","Up"),
         QT_TRANSLATE_NOOP("action","Pitch up or move text or articulation up"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-up-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic Up"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch up"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-up-diatonic-alterations",
         QT_TRANSLATE_NOOP("action","Diatonic Up (Keep Degree Alterations)"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch up (Keep degree alterations)"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-up-octave",
         QT_TRANSLATE_NOOP("action","Up Octave"),
         QT_TRANSLATE_NOOP("action","Pitch up octave"),
         QT_TRANSLATE_NOOP("action","Pitch up by an octave or move text or articulation up"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "up-chord",
         QT_TRANSLATE_NOOP("action","Up Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to higher pitched note in chord"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "top-chord",
         QT_TRANSLATE_NOOP("action","Top Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to top note in chord"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "move-up",
         QT_TRANSLATE_NOOP("action","Move Up"),
         QT_TRANSLATE_NOOP("action","Move chord/rest to staff above"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "pitch-down",
         QT_TRANSLATE_NOOP("action","Down"),
         QT_TRANSLATE_NOOP("action","Pitch down or move text or articulation down"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-down-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic Down"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch down"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-down-diatonic-alterations",
         QT_TRANSLATE_NOOP("action","Diatonic Down (Keep Degree Alterations)"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch down (Keep degree alterations)"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-down-octave",
         QT_TRANSLATE_NOOP("action","Down Octave"),
         QT_TRANSLATE_NOOP("action","Pitch down octave"),
         QT_TRANSLATE_NOOP("action","Pitch down by an octave or move text or articulation down"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "down-chord",
         QT_TRANSLATE_NOOP("action","Down Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to lower pitched note in chord"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "next-segment-element",
         QT_TRANSLATE_NOOP("action","Next Segment Element"),
         QT_TRANSLATE_NOOP("action","Accessibility: Next segment element"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "prev-segment-element",
         QT_TRANSLATE_NOOP("action","Previous Segment Element"),
         QT_TRANSLATE_NOOP("action","Accessibility: Previous segment element"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_TEXT_EDIT | STATE_HARMONY_FIGBASS_EDIT,
         "next-element",
         QT_TRANSLATE_NOOP("action","Next Element"),
         QT_TRANSLATE_NOOP("action","Accessibility: Next element"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_TEXT_EDIT | STATE_HARMONY_FIGBASS_EDIT,
         "prev-element",
         QT_TRANSLATE_NOOP("action","Previous Element"),
         QT_TRANSLATE_NOOP("action","Accessibility: Previous element"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "get-location",
         QT_TRANSLATE_NOOP("action","Get Location"),
         QT_TRANSLATE_NOOP("action","Accessibility: Get location"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "palette-search",
         QT_TRANSLATE_NOOP("action","Palette Search"),
         QT_TRANSLATE_NOOP("action","Palette Search"),
         QT_TRANSLATE_NOOP("action","Palette Search"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "apply-current-palette-element",
         QT_TRANSLATE_NOOP("action","Apply Current Palette Element"),
         QT_TRANSLATE_NOOP("action","Apply current palette element"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "first-element",
         QT_TRANSLATE_NOOP("action","First Element"),
         QT_TRANSLATE_NOOP("action","Go to the first element"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "last-element",
         QT_TRANSLATE_NOOP("action","Last Element"),
         QT_TRANSLATE_NOOP("action","Go to the last element"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "bottom-chord",
         QT_TRANSLATE_NOOP("action","Bottom Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to bottom note in chord"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "move-down",
         QT_TRANSLATE_NOOP("action","Move Down"),
         QT_TRANSLATE_NOOP("action","Move chord/rest to staff below"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-chord",
         QT_TRANSLATE_NOOP("action","Previous Chord"),
         QT_TRANSLATE_NOOP("action","Go to previous chord or move text left")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-measure",
         QT_TRANSLATE_NOOP("action","Previous Measure"),
         QT_TRANSLATE_NOOP("action","Go to previous measure or move text left")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-system",
         QT_TRANSLATE_NOOP("action","Previous System"),
         QT_TRANSLATE_NOOP("action","Go to previous system")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-frame",
         QT_TRANSLATE_NOOP("action","Previous Frame"),
         QT_TRANSLATE_NOOP("action","Go to previous frame")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-section",
         QT_TRANSLATE_NOOP("action","Previous Section"),
         QT_TRANSLATE_NOOP("action","Go to previous section")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-track",
         QT_TRANSLATE_NOOP("action","Previous Staff or Voice"),
         QT_TRANSLATE_NOOP("action","Previous staff or voice")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-chord",
         QT_TRANSLATE_NOOP("action","Next Chord"),
         QT_TRANSLATE_NOOP("action","Go to next chord or move text right")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-measure",
         QT_TRANSLATE_NOOP("action","Next Measure"),
         QT_TRANSLATE_NOOP("action","Go to next measure or move text right")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-system",
         QT_TRANSLATE_NOOP("action","Next System"),
         QT_TRANSLATE_NOOP("action","Go to next system")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-frame",
         QT_TRANSLATE_NOOP("action","Next Frame"),
         QT_TRANSLATE_NOOP("action","Go to next frame")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-section",
         QT_TRANSLATE_NOOP("action","Next Section"),
         QT_TRANSLATE_NOOP("action","Go to next section")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "top-staff",
         QT_TRANSLATE_NOOP("action","Top Staff"),
         QT_TRANSLATE_NOOP("action","Go to top staff")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "playback-position",
         QT_TRANSLATE_NOOP("action","Playback Cursor Position"),
         QT_TRANSLATE_NOOP("action","Go to recent playback cursor position")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "empty-trailing-measure",
         QT_TRANSLATE_NOOP("action","First Empty Trailing Measure"),
         QT_TRANSLATE_NOOP("action","Go to first empty trailing measure")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-track",
         QT_TRANSLATE_NOOP("action","Next Staff or Voice"),
         QT_TRANSLATE_NOOP("action","Next staff or voice")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-prev-chord",
         QT_TRANSLATE_NOOP("action","Add Previous Chord to Selection"),
         QT_TRANSLATE_NOOP("action","Add previous chord to selection")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-prev-measure",
         QT_TRANSLATE_NOOP("action","Select to Beginning of Measure"),
         QT_TRANSLATE_NOOP("action","Select to beginning of measure")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-next-chord",
         QT_TRANSLATE_NOOP("action","Add Next Chord to Selection"),
         QT_TRANSLATE_NOOP("action","Add next chord to selection")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-section",
         QT_TRANSLATE_NOOP("action","Select Section"),
         QT_TRANSLATE_NOOP("action","Select section")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "move-right",
         QT_TRANSLATE_NOOP("action","Move Chord/Rest Right"),
         QT_TRANSLATE_NOOP("action","Move chord/rest right")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "move-left",
         QT_TRANSLATE_NOOP("action","Move Chord/Rest left"),
         QT_TRANSLATE_NOOP("action","Move chord/rest left")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-next-measure",
         QT_TRANSLATE_NOOP("action","Select to End of Measure"),
         QT_TRANSLATE_NOOP("action","Select to end of measure")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-begin-line",
         QT_TRANSLATE_NOOP("action","Select to Beginning of Line"),
         QT_TRANSLATE_NOOP("action","Select to beginning of line")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-end-line",
         QT_TRANSLATE_NOOP("action","Select to End of Line"),
         QT_TRANSLATE_NOOP("action","Select to end of line")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-begin-score",
         QT_TRANSLATE_NOOP("action","Select to Beginning of Score"),
         QT_TRANSLATE_NOOP("action","Select to beginning of score")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-end-score",
         QT_TRANSLATE_NOOP("action","Select to End of Score"),
         QT_TRANSLATE_NOOP("action","Select to end of score")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-staff-above",
         QT_TRANSLATE_NOOP("action","Add Staff Above to Selection"),
         QT_TRANSLATE_NOOP("action","Add staff above to selection")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-staff-below",
         QT_TRANSLATE_NOOP("action","Add Staff Below to Selection"),
         QT_TRANSLATE_NOOP("action","Add staff below to selection")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "scr-prev",
         QT_TRANSLATE_NOOP("action","Screen: Previous")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "scr-next",
         QT_TRANSLATE_NOOP("action","Screen: Next")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-prev",
         QT_TRANSLATE_NOOP("action","Page: Previous")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-next",
         QT_TRANSLATE_NOOP("action","Page: Next")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-top",
         QT_TRANSLATE_NOOP("action","Page: Top")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-end",
         QT_TRANSLATE_NOOP("action","Page: End")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-slur",
         QT_TRANSLATE_NOOP("action","Slur"),
         QT_TRANSLATE_NOOP("action","Add slur")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-hairpin",
         QT_TRANSLATE_NOOP("action","Crescendo"),
         QT_TRANSLATE_NOOP("action","Add crescendo"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-hairpin-reverse",
         QT_TRANSLATE_NOOP("action","Decrescendo"),
         QT_TRANSLATE_NOOP("action","Add decrescendo"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-8va",
         QT_TRANSLATE_NOOP("action","Ottava 8va alta"),
         QT_TRANSLATE_NOOP("action","Add ottava 8va alta"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-8vb",
         QT_TRANSLATE_NOOP("action","Ottava 8va bassa"),
         QT_TRANSLATE_NOOP("action","Add ottava 8va bassa"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT | STATE_PLAY | STATE_FOTO,
         "escape",
         QT_TRANSLATE_NOOP("action","Escape")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "delete",
         QT_TRANSLATE_NOOP("action","Delete"),
         QT_TRANSLATE_NOOP("action","Delete"),
         QT_TRANSLATE_NOOP("action","Delete the selected element(s)"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "full-measure-rest",
         QT_TRANSLATE_NOOP("action","Full Measure Rest"),
         QT_TRANSLATE_NOOP("action","Full measure rest"),
         QT_TRANSLATE_NOOP("action","Convert the measure to a full measure rest"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "time-delete",
         QT_TRANSLATE_NOOP("action","Remove Selected Range"),
         QT_TRANSLATE_NOOP("action","Remove selected range"),
         QT_TRANSLATE_NOOP("action","Remove element and duration"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-measure",
         QT_TRANSLATE_NOOP("action","Append One Measure"),
         QT_TRANSLATE_NOOP("action","Append one measure")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-measures",
         QT_TRANSLATE_NOOP("action","Append Measures…"),
         QT_TRANSLATE_NOOP("action","Append measures")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-measure",
         QT_TRANSLATE_NOOP("action","Insert One Measure"),
         QT_TRANSLATE_NOOP("action","Insert one measure"),
         0,
         Icons::measure_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-measures",
         QT_TRANSLATE_NOOP("action","Insert Measures…"),
         QT_TRANSLATE_NOOP("action","Insert measures")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-hbox",
         QT_TRANSLATE_NOOP("action","Insert Horizontal Frame"),
         QT_TRANSLATE_NOOP("action","Insert horizontal frame"),
         0,
         Icons::hframe_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-textframe",
         QT_TRANSLATE_NOOP("action","Insert Text Frame"),
         QT_TRANSLATE_NOOP("action","Insert text frame"),
         0,
         Icons::tframe_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-textframe",
         QT_TRANSLATE_NOOP("action","Append Text Frame"),
         QT_TRANSLATE_NOOP("action","Append text frame")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-fretframe",
         QT_TRANSLATE_NOOP("action","Insert Fretboard Diagram Frame"),
         QT_TRANSLATE_NOOP("action","Insert fretboard diagram frame"),
         0,
         Icons::fframe_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-vbox",
         QT_TRANSLATE_NOOP("action","Insert Vertical Frame"),
         QT_TRANSLATE_NOOP("action","Insert vertical frame"),
         0,
         Icons::vframe_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-hbox",
         QT_TRANSLATE_NOOP("action","Append Horizontal Frame"),
         QT_TRANSLATE_NOOP("action","Append horizontal frame")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-vbox",
         QT_TRANSLATE_NOOP("action","Append Vertical Frame"),
         QT_TRANSLATE_NOOP("action","Append vertical frame")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "duplet",
         QT_TRANSLATE_NOOP("action","Duplet")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "triplet",
         QT_TRANSLATE_NOOP("action","Triplet")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "quadruplet",
         QT_TRANSLATE_NOOP("action","Quadruplet")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "quintuplet",
         QT_TRANSLATE_NOOP("action","Quintuplet")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "sextuplet",
         QT_TRANSLATE_NOOP("action","Sextuplet")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "septuplet",
         QT_TRANSLATE_NOOP("action","Septuplet")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "octuplet",
         QT_TRANSLATE_NOOP("action","Octuplet")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "nonuplet",
         QT_TRANSLATE_NOOP("action","Nonuplet")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tuplet-dialog",
         QT_TRANSLATE_NOOP("action","Other…"),
         QT_TRANSLATE_NOOP("action","Other tuplets")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-longa",
         QT_TRANSLATE_NOOP("action","Longa"),
         QT_TRANSLATE_NOOP("action","Note duration: Longa"),
         QT_TRANSLATE_NOOP("action","Longa"),
         Icons::longaUp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-breve",
         QT_TRANSLATE_NOOP("action","Double Whole Note"),
         QT_TRANSLATE_NOOP("action","Note duration: Double whole"),
         QT_TRANSLATE_NOOP("action","Double whole note"),
         Icons::brevis_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-1",
         QT_TRANSLATE_NOOP("action","Whole Note"),
         QT_TRANSLATE_NOOP("action","Note duration: Whole"),
         QT_TRANSLATE_NOOP("action","Whole note"),
         Icons::note_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-2",
         QT_TRANSLATE_NOOP("action","Half Note"),
         QT_TRANSLATE_NOOP("action","Note duration: Half"),
         QT_TRANSLATE_NOOP("action","Half note"),
         Icons::note2_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-4",
         QT_TRANSLATE_NOOP("action","Quarter Note"),
         QT_TRANSLATE_NOOP("action","Note duration: Quarter"),
         QT_TRANSLATE_NOOP("action","Quarter note"),
         Icons::note4_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-8",
         QT_TRANSLATE_NOOP("action","Eighth Note"),
         QT_TRANSLATE_NOOP("action","Note duration: Eighth"),
         QT_TRANSLATE_NOOP("action","Eighth note"),
         Icons::note8_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-16",
         QT_TRANSLATE_NOOP("action","16th Note"),
         QT_TRANSLATE_NOOP("action","Note duration: 16th"),
         QT_TRANSLATE_NOOP("action","16th note"),
         Icons::note16_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-32",
         QT_TRANSLATE_NOOP("action","32nd Note"),
         QT_TRANSLATE_NOOP("action","Note duration: 32nd"),
         QT_TRANSLATE_NOOP("action","32nd note"),
         Icons::note32_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-64",
         QT_TRANSLATE_NOOP("action","64th Note"),
         QT_TRANSLATE_NOOP("action","Note duration: 64th"),
         QT_TRANSLATE_NOOP("action","64th note"),
         Icons::note64_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-128",
         QT_TRANSLATE_NOOP("action","128th Note"),
         QT_TRANSLATE_NOOP("action","Note duration: 128th"),
         QT_TRANSLATE_NOOP("action","128th note"),
         Icons::note128_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-256",
         QT_TRANSLATE_NOOP("action","256th Note"),
         QT_TRANSLATE_NOOP("action","Note duration: 256th"),
         QT_TRANSLATE_NOOP("action","256th note"),
         Icons::note256_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-512",
         QT_TRANSLATE_NOOP("action","512th Note"),
         QT_TRANSLATE_NOOP("action","Note duration: 512th"),
         QT_TRANSLATE_NOOP("action","512th note"),
         Icons::note512_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-note-1024",
         QT_TRANSLATE_NOOP("action","1024th Note"),
         QT_TRANSLATE_NOOP("action","Note duration: 1024th"),
         QT_TRANSLATE_NOOP("action","1024th note"),
         Icons::note1024_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "pad-note-increase",
         QT_TRANSLATE_NOOP("action","Increase Active Duration"),
         QT_TRANSLATE_NOOP("action","Increase active duration")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "pad-note-decrease",
         QT_TRANSLATE_NOOP("action","Decrease Active Duration"),
         QT_TRANSLATE_NOOP("action","Decrease active duration")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dot",
         QT_TRANSLATE_NOOP("action","Augmentation Dot"),
         QT_TRANSLATE_NOOP("action","Note duration: Augmentation dot"),
         QT_TRANSLATE_NOOP("action","Augmentation dot"),
         Icons::dot_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dotdot",
         QT_TRANSLATE_NOOP("action","Double Augmentation Dot"),
         QT_TRANSLATE_NOOP("action","Note duration: Double augmentation dot"),
         QT_TRANSLATE_NOOP("action","Double augmentation dot"),
         Icons::dotdot_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dot3",
         QT_TRANSLATE_NOOP("action","Triple Augmentation Dot"),
         QT_TRANSLATE_NOOP("action","Note duration: Triple augmentation dot"),
         QT_TRANSLATE_NOOP("action","Triple augmentation dot"),
         Icons::dot3_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dot4",
         QT_TRANSLATE_NOOP("action","Quadruple Augmentation Dot"),
         QT_TRANSLATE_NOOP("action","Note duration: Quadruple augmentation dot"),
         QT_TRANSLATE_NOOP("action","Quadruple augmentation dot"),
         Icons::dot4_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tie",
         QT_TRANSLATE_NOOP("action","Tie"),
         QT_TRANSLATE_NOOP("action","Note duration: Tie"),
         QT_TRANSLATE_NOOP("action","Tie"),
         Icons::tie_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "pad-rest",
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Note input: Rest"),
         QT_TRANSLATE_NOOP("action","Rest"),
         Icons::quartrest_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "sharp2",
         QT_TRANSLATE_NOOP("action","Double ♯"),
         QT_TRANSLATE_NOOP("action","Note input: Double ♯"),
         QT_TRANSLATE_NOOP("action","Double ♯"),
         Icons::sharpsharp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "sharp",
         QT_TRANSLATE_NOOP("action","♯"),
         QT_TRANSLATE_NOOP("action","Note input: ♯"),
         QT_TRANSLATE_NOOP("action","♯"),
         Icons::sharp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "nat",
         QT_TRANSLATE_NOOP("action","♮"),
         QT_TRANSLATE_NOOP("action","Note input: ♮"),
         QT_TRANSLATE_NOOP("action","♮"),
         Icons::natural_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "flat",
         QT_TRANSLATE_NOOP("action","♭"),
         QT_TRANSLATE_NOOP("action","Note input: ♭"),
         QT_TRANSLATE_NOOP("action","♭"),
         Icons::flat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "flat2",
         QT_TRANSLATE_NOOP("action","Double ♭"),
         QT_TRANSLATE_NOOP("action","Note input: Double ♭"),
         QT_TRANSLATE_NOOP("action","Double ♭"),
         Icons::flatflat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "sharp2-post",
         QT_TRANSLATE_NOOP("action","Double ♯ (non-toggle)"),
         QT_TRANSLATE_NOOP("action","Note input (non-toggle): Double ♯"),
         QT_TRANSLATE_NOOP("action","Double ♯ (non-toggle)"),
         Icons::sharpsharp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "sharp-post",
         QT_TRANSLATE_NOOP("action","♯ (non-toggle)"),
         QT_TRANSLATE_NOOP("action","Note input (non-toggle): ♯"),
         QT_TRANSLATE_NOOP("action","♯ (non-toggle)"),
         Icons::sharp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "nat-post",
         QT_TRANSLATE_NOOP("action","♮ (non-toggle)"),
         QT_TRANSLATE_NOOP("action","Note input (non-toggle): ♮"),
         QT_TRANSLATE_NOOP("action","♮ (non-toggle)"),
         Icons::natural_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "flat-post",
         QT_TRANSLATE_NOOP("action","♭ (non-toggle)"),
         QT_TRANSLATE_NOOP("action","Note input (non-toggle): ♭"),
         QT_TRANSLATE_NOOP("action","♭ (non-toggle)"),
         Icons::flat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "flat2-post",
         QT_TRANSLATE_NOOP("action","Double ♭ (non-toggle)"),
         QT_TRANSLATE_NOOP("action","Note input (non-toggle): Double ♭"),
         QT_TRANSLATE_NOOP("action","Double ♭ (non-toggle)"),
         Icons::flatflat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "acciaccatura",
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         QT_TRANSLATE_NOOP("action","Add acciaccatura"),
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         Icons::acciaccatura_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "appoggiatura",
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         QT_TRANSLATE_NOOP("action","Add appoggiatura"),
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         Icons::appoggiatura_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         /* no stroke: 4th*/
         "grace4",
         QT_TRANSLATE_NOOP("action","Grace: Quarter"),
         QT_TRANSLATE_NOOP("action","Add quarter grace note"),
         QT_TRANSLATE_NOOP("action","Grace: quarter"),
         Icons::grace4_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         /* no stroke: 16th*/
         "grace16",
         QT_TRANSLATE_NOOP("action","Grace: 16th"),
         QT_TRANSLATE_NOOP("action","Add 16th grace note"),
         QT_TRANSLATE_NOOP("action","Grace: 16th"),
         Icons::grace16_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         /* no stroke: 32nd*/
         "grace32",
         QT_TRANSLATE_NOOP("action","Grace: 32nd"),
         QT_TRANSLATE_NOOP("action","Add 32nd grace note"),
         QT_TRANSLATE_NOOP("action","Grace: 32nd"),
         Icons::grace32_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         /* no stroke: Eighth*/
         "grace8after",
         QT_TRANSLATE_NOOP("action","Grace: Eighth After"),
         QT_TRANSLATE_NOOP("action","Add Eighth grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: eighth after"),
         Icons::grace8after_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         /* no stroke: 16th*/
         "grace16after",
         QT_TRANSLATE_NOOP("action","Grace: 16th After"),
         QT_TRANSLATE_NOOP("action","Add 16th grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: 16th after"),
         Icons::grace16after_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         /* no stroke: 32nd*/
         "grace32after",
         QT_TRANSLATE_NOOP("action","Grace: 32nd After"),
         QT_TRANSLATE_NOOP("action","Add 32nd grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: 32nd after"),
         Icons::grace32after_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-1",
         QT_TRANSLATE_NOOP("action","1"),
         QT_TRANSLATE_NOOP("action","Voice 1"),
         0,
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-2",
         QT_TRANSLATE_NOOP("action","2"),
         QT_TRANSLATE_NOOP("action","Voice 2"),
         0,
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-3",
         QT_TRANSLATE_NOOP("action","3"),
         QT_TRANSLATE_NOOP("action","Voice 3"),
         0,
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-4",
         QT_TRANSLATE_NOOP("action","4"),
         QT_TRANSLATE_NOOP("action","Voice 4"),
         0,
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "midi-on",
         QT_TRANSLATE_NOOP("action","MIDI Input"),
         QT_TRANSLATE_NOOP("action","Toggle 'MIDI Input'"),
         0,
         Icons::midiin_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam-start",
         QT_TRANSLATE_NOOP("action","Beam Start"),
         QT_TRANSLATE_NOOP("action","Beam start"),
         0,
         Icons::sbeam_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam-mid",
         QT_TRANSLATE_NOOP("action","Beam Middle"),
         QT_TRANSLATE_NOOP("action","Beam middle"),
         0,
         Icons::mbeam_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "no-beam",
         QT_TRANSLATE_NOOP("action","No Beam"),
         QT_TRANSLATE_NOOP("action","No beam"),
         0,
         Icons::nbeam_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam32",
         QT_TRANSLATE_NOOP("action","Beam 16th Sub"),
         QT_TRANSLATE_NOOP("action","Beam 16th sub"),
         0,
         Icons::beam32_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam64",
         QT_TRANSLATE_NOOP("action","Beam 32nd Sub"),
         QT_TRANSLATE_NOOP("action","Beam 32nd sub"),
         0,
         Icons::beam64_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "auto-beam",
         QT_TRANSLATE_NOOP("action","Auto Beam"),
         QT_TRANSLATE_NOOP("action","Auto beam"),
         0,
         Icons::abeam_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "fbeam1",
         QT_TRANSLATE_NOOP("action","Feathered Beam, Slower"),
         QT_TRANSLATE_NOOP("action","Feathered beam, slower"),
         0,
         Icons::fbeam1_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "fbeam2",
         QT_TRANSLATE_NOOP("action","Feathered Beam, Faster"),
         QT_TRANSLATE_NOOP("action","Feathered beam, faster"),
         0,
         Icons::fbeam2_ICON
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-palette",
         QT_TRANSLATE_NOOP("action","Palettes"),
         QT_TRANSLATE_NOOP("action","Toggle 'Palettes'"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-playpanel",
         QT_TRANSLATE_NOOP("action","Play Panel"),
         QT_TRANSLATE_NOOP("action","Toggle 'Play Panel'"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-selection-window",
         QT_TRANSLATE_NOOP("action","Selection Filter"),
         QT_TRANSLATE_NOOP("action","Toggle 'Selection Filter'"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-navigator",
         QT_TRANSLATE_NOOP("action","Navigator"),
         QT_TRANSLATE_NOOP("action","Toggle 'Navigator'"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT  | STATE_PLAY,
         "toggle-timeline",
         QT_TRANSLATE_NOOP("action","Timeline"),
         QT_TRANSLATE_NOOP("action","Toggle 'Timeline'"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "toggle-midiimportpanel",
         QT_TRANSLATE_NOOP("action","MIDI Import Panel"),
         QT_TRANSLATE_NOOP("action","Toggle 'MIDI Import Panel'"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
#ifdef Q_OS_MAC
         //Avoid conflict with M in text
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
#else
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
#endif
         "toggle-mixer",
         QT_TRANSLATE_NOOP("action","Mixer"),
         QT_TRANSLATE_NOOP("action","Toggle 'Mixer'"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-fileoperations",
         QT_TRANSLATE_NOOP("action","File Operations"),
         QT_TRANSLATE_NOOP("action","Toggle 'File Operations' toolbar")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-transport",
         QT_TRANSLATE_NOOP("action","Playback Controls"),
         QT_TRANSLATE_NOOP("action","Toggle 'Playback Controls' toolbar")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-concertpitch",
         QT_TRANSLATE_NOOP("action","Concert Pitch"),
         QT_TRANSLATE_NOOP("action","Toggle 'Concert Pitch' toolbar")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-imagecapture",
         QT_TRANSLATE_NOOP("action","Image Capture"),
         QT_TRANSLATE_NOOP("action","Toggle 'Image Capture' toolbar")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-noteinput",
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","Toggle 'Note Input' toolbar")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-feedback",
         QT_TRANSLATE_NOOP("action","Feedback"),
         QT_TRANSLATE_NOOP("action","Toggle 'Feedback' toolbar"),
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-workspaces-toolbar",
         QT_TRANSLATE_NOOP("action","Workspaces"),
         QT_TRANSLATE_NOOP("action","Toggle 'Workspaces' toolbar"),
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "toggle-statusbar",
         QT_TRANSLATE_NOOP("action","Status Bar"),
         QT_TRANSLATE_NOOP("action","Toggle 'Status Bar'")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "create-new-workspace",
         "+",
         QT_TRANSLATE_NOOP("action","Add new workspace"),
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_PLAY | STATE_FOTO,
         "quit",
         QT_TRANSLATE_NOOP("action","Quit")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT ,
         "mag",
         QT_TRANSLATE_NOOP("action","Zoom Canvas"),
         QT_TRANSLATE_NOOP("action","Zoom canvas")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "lyrics",
         QT_TRANSLATE_NOOP("action","Lyrics"),
         QT_TRANSLATE_NOOP("action","Add lyrics"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
//         ,ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tempo",
         QT_TRANSLATE_NOOP("action","Tempo Marking"),
         QT_TRANSLATE_NOOP("action","Add tempo marking"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "system-text",
         QT_TRANSLATE_NOOP("action","System Text"),
         QT_TRANSLATE_NOOP("action","Add system text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "staff-text",
         QT_TRANSLATE_NOOP("action","Staff Text"),
         QT_TRANSLATE_NOOP("action","Add staff text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "expression-text",
         QT_TRANSLATE_NOOP("action","Expression Text"),
         QT_TRANSLATE_NOOP("action","Expression text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "frame-text",
         QT_TRANSLATE_NOOP("action","Text"),
         QT_TRANSLATE_NOOP("action","Add frame text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "title-text",
         QT_TRANSLATE_NOOP("action","Title"),
         QT_TRANSLATE_NOOP("action","Add title text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "subtitle-text",
         QT_TRANSLATE_NOOP("action","Subtitle"),
         QT_TRANSLATE_NOOP("action","Add subtitle text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "composer-text",
         QT_TRANSLATE_NOOP("action","Composer"),
         QT_TRANSLATE_NOOP("action","Add composer text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "poet-text",
         QT_TRANSLATE_NOOP("action","Lyricist"),
         QT_TRANSLATE_NOOP("action","Add lyricist text")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "part-text",
         QT_TRANSLATE_NOOP("action","Part Name"),
         QT_TRANSLATE_NOOP("action","Add part name")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-text",
         QT_TRANSLATE_NOOP("action","Chord Symbol"),
         QT_TRANSLATE_NOOP("action","Add chord symbol")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "roman-numeral-text",
         QT_TRANSLATE_NOOP("action","Roman Numeral Analysis"),
         QT_TRANSLATE_NOOP("action","Add Roman numeral analysis")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "nashville-number-text",
         QT_TRANSLATE_NOOP("action","Nashville Number"),
         QT_TRANSLATE_NOOP("action","Add Nashville number")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rehearsalmark-text",
         QT_TRANSLATE_NOOP("action","Rehearsal Mark"),
         QT_TRANSLATE_NOOP("action","Add rehearsal mark")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "instrument-change-text",
         QT_TRANSLATE_NOOP("action","Instrument Change"),
         QT_TRANSLATE_NOOP("action","Add instrument change")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "fingering-text",
         QT_TRANSLATE_NOOP("action","Fingering"),
         QT_TRANSLATE_NOOP("action","Add fingering")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "sticking-text",
         QT_TRANSLATE_NOOP("action","Sticking"),
         QT_TRANSLATE_NOOP("action","Add sticking")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "picture",
         QT_TRANSLATE_NOOP("action","Image"),
         QT_TRANSLATE_NOOP("action","Add image")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "play",
         QT_TRANSLATE_NOOP("action","Play"),
         QT_TRANSLATE_NOOP("action","Player: play"),
         QT_TRANSLATE_NOOP("action","Start or stop playback"),
         Icons::play_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-prev-chord",
         QT_TRANSLATE_NOOP("action","Play Previous Chord"),
         QT_TRANSLATE_NOOP("action","Play previous chord"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-prev-measure",
         QT_TRANSLATE_NOOP("action","Play Previous Measure"),
         QT_TRANSLATE_NOOP("action","Play previous measure"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-next-chord",
         QT_TRANSLATE_NOOP("action","Play Next Chord"),
         QT_TRANSLATE_NOOP("action","Play next chord"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-next-measure",
         QT_TRANSLATE_NOOP("action","Play Next Measure"),
         QT_TRANSLATE_NOOP("action","Play next measure"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "seek-begin",
         QT_TRANSLATE_NOOP("action","Seek to Begin"),
         QT_TRANSLATE_NOOP("action","Player: seek to begin"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "rewind",
         QT_TRANSLATE_NOOP("action","Rewind"),
         QT_TRANSLATE_NOOP("action","Player: rewind"),
         QT_TRANSLATE_NOOP("action","Rewind to start position"),
         Icons::start_ICON
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "seek-end",
         QT_TRANSLATE_NOOP("action","Seek to End"),
         QT_TRANSLATE_NOOP("action","Player: seek to end")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "repeat",
         QT_TRANSLATE_NOOP("action","Play Repeats"),
         QT_TRANSLATE_NOOP("action","Toggle 'Play Repeats'"),
         QT_TRANSLATE_NOOP("action","Play repeats"),
         Icons::repeat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE | ShortcutFlags::A_CHECKED
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "pan",
         QT_TRANSLATE_NOOP("action","Pan Score"),
         QT_TRANSLATE_NOOP("action","Toggle 'Pan Score'"),
         QT_TRANSLATE_NOOP("action","Pan score automatically"),
         Icons::pan_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE | ShortcutFlags::A_CHECKED
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "load-style",
         QT_TRANSLATE_NOOP("action","Load Style…"),
         QT_TRANSLATE_NOOP("action","Load style"),
         0,
         Icons::fileOpen_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "save-style",
         QT_TRANSLATE_NOOP("action","Save Style…"),
         QT_TRANSLATE_NOOP("action","Save style"),
         0,
         Icons::fileSave_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "select-all",
         QT_TRANSLATE_NOOP("action","Select All"),
         QT_TRANSLATE_NOOP("action","Select all"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "transpose",
         QT_TRANSLATE_NOOP("action","&Transpose…"),
         QT_TRANSLATE_NOOP("action","Transpose"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "clef-violin",
         QT_TRANSLATE_NOOP("action","Treble Clef"),
         QT_TRANSLATE_NOOP("action","Add treble clef"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "clef-bass",
         QT_TRANSLATE_NOOP("action","Bass Clef"),
         QT_TRANSLATE_NOOP("action","Add bass clef"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x12",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-2"),
         QT_TRANSLATE_NOOP("action","Exchange voice 1-2"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x13",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-3"),
         QT_TRANSLATE_NOOP("action","Exchange voice 1-3"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x14",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-4"),
         QT_TRANSLATE_NOOP("action","Exchange voice 1-4"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x23",
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-3"),
         QT_TRANSLATE_NOOP("action","Exchange voice 2-3"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x24",
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-4"),
         QT_TRANSLATE_NOOP("action","Exchange voice 2-4"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x34",
         QT_TRANSLATE_NOOP("action","Exchange Voice 3-4"),
         QT_TRANSLATE_NOOP("action","Exchange voice 3-4"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "concert-pitch",
         QT_TRANSLATE_NOOP("action","Concert Pitch"),
         QT_TRANSLATE_NOOP("action","Toggle 'Concert Pitch'"),
         QT_TRANSLATE_NOOP("action","Switch between concert/sounding pitch and transposing/written pitch"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "repeat-cmd",
         QT_TRANSLATE_NOOP("action","Repeat Last Command"),
         QT_TRANSLATE_NOOP("action","Repeat last command"),
         0,
         Icons::fileOpen_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-info",
         QT_TRANSLATE_NOOP("action","Score Properties…"),
         QT_TRANSLATE_NOOP("action","Edit score properties"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "system-break",
         QT_TRANSLATE_NOOP("action","Toggle System Break"),
         QT_TRANSLATE_NOOP("action","Toggle 'System Break'"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-break",
         QT_TRANSLATE_NOOP("action","Toggle Page Break"),
         QT_TRANSLATE_NOOP("action","Toggle 'Page Break'"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "section-break",
         QT_TRANSLATE_NOOP("action","Toggle Section Break"),
         QT_TRANSLATE_NOOP("action","Toggle 'Section Break'"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "edit-element",
         QT_TRANSLATE_NOOP("action","Edit Element"),
         QT_TRANSLATE_NOOP("action","Edit element")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_EDIT,
         "reset",
         QT_TRANSLATE_NOOP("action","Reset Shapes and Positions"),
         QT_TRANSLATE_NOOP("action","Reset shapes and positions"),
         QT_TRANSLATE_NOOP("action","Reset shapes and positions of selected elements to their defaults")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "debugger",
         QT_TRANSLATE_NOOP("action","Debugger")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-stretch",
         QT_TRANSLATE_NOOP("action","Reset Layout Stretch"),
         QT_TRANSLATE_NOOP("action","Reset layout stretch"),
         QT_TRANSLATE_NOOP("action","Reset layout stretch factor of selected measures or entire score"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-invisible",
         QT_TRANSLATE_NOOP("action","Show Invisible"),
         QT_TRANSLATE_NOOP("action","Show invisible"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-unprintable",
         QT_TRANSLATE_NOOP("action","Show Unprintable"),
         QT_TRANSLATE_NOOP("action","Show unprintable"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-frames",
         QT_TRANSLATE_NOOP("action","Show Frames"),
         QT_TRANSLATE_NOOP("action","Show frames"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-pageborders",
         QT_TRANSLATE_NOOP("action","Show Page Margins"),
         QT_TRANSLATE_NOOP("action","Show page margins"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "mark-irregular",
         QT_TRANSLATE_NOOP("action","Mark Irregular Measures"),
         QT_TRANSLATE_NOOP("action","Mark irregular measures"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_TEXT_EDIT | STATE_LYRICS_EDIT | STATE_HARMONY_FIGBASS_EDIT,
         "show-keys",
         QT_TRANSLATE_NOOP("action","Insert Special Characters…"),
         QT_TRANSLATE_NOOP("action","Insert special characters"),
         0,
         Icons::keys_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-1",
         QT_TRANSLATE_NOOP("action","Whole Rest"),
         QT_TRANSLATE_NOOP("action","Note input: Whole rest")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-2",
         QT_TRANSLATE_NOOP("action","Half Rest"),
         QT_TRANSLATE_NOOP("action","Note input: Half rest")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-4",
         QT_TRANSLATE_NOOP("action","Quarter Rest"),
         QT_TRANSLATE_NOOP("action","Note input: Quarter rest")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-8",
         QT_TRANSLATE_NOOP("action","Eighth Rest"),
         QT_TRANSLATE_NOOP("action","Note input: Eighth rest")
         },
      {                     // mapped to undo in note entry mode
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "backspace",
         QT_TRANSLATE_NOOP("action","Backspace"),
//         0,
//         0,
//         Icons::Invalid_ICON,
//         Qt::WindowShortcut,
//         ShortcutFlags::A_CMD
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "find",
         QT_TRANSLATE_NOOP("action","Find / Go To"),
         QT_TRANSLATE_NOOP("action","Find / Go to")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "zoomin",
         QT_TRANSLATE_NOOP("action","Zoom In"),
         QT_TRANSLATE_NOOP("action","Zoom in")
         },
      {
         MsWidget::MAIN_WINDOW,
         // conflicts with Ctrl+- in edit mode to enter lyrics hyphen
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "zoomout",
         QT_TRANSLATE_NOOP("action","Zoom Out"),
         QT_TRANSLATE_NOOP("action","Zoom out")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "zoom100",
         QT_TRANSLATE_NOOP("action","Zoom to 100%"),
         QT_TRANSLATE_NOOP("action","Zoom to 100%")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "zoom-page-width",
         QT_TRANSLATE_NOOP("action","Zoom to Page Width or Previous Magnification Level"),
         QT_TRANSLATE_NOOP("action","Zoom to page-width / previous magnification level")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "mirror-note",
         QT_TRANSLATE_NOOP("action","Mirror Notehead"),
         QT_TRANSLATE_NOOP("action","Mirror notehead"),
         0,
         Icons::flip_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-style",
         QT_TRANSLATE_NOOP("action","Style…"),
         QT_TRANSLATE_NOOP("action","Edit style"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-harmony",
         QT_TRANSLATE_NOOP("action","Chord Symbols…"),
         QT_TRANSLATE_NOOP("action","Edit chord symbols style")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-similar",
         QT_TRANSLATE_NOOP("action","All Similar Elements"),
         QT_TRANSLATE_NOOP("action","Select all similar elements")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-similar-staff",
         QT_TRANSLATE_NOOP("action","All Similar Elements in Same Staff"),
         QT_TRANSLATE_NOOP("action","Select all similar elements in same staff")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-similar-range",
         QT_TRANSLATE_NOOP("action","All Similar Elements in Range Selection"),
         QT_TRANSLATE_NOOP("action","Select all similar elements in the range selection")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-dialog",
         QT_TRANSLATE_NOOP("action","All Similar Elements with More Options"),
         QT_TRANSLATE_NOOP("action","Select all similar elements with more options")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "synth-control",
         QT_TRANSLATE_NOOP("action","Synthesizer"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         "double-duration",
         QT_TRANSLATE_NOOP("action","Double Duration"),
         QT_TRANSLATE_NOOP("action","Double duration"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         "half-duration",
         QT_TRANSLATE_NOOP("action","Half Duration"),
         QT_TRANSLATE_NOOP("action","Half duration"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         "inc-duration-dotted",
         QT_TRANSLATE_NOOP("action","Increase Duration Dotted"),
         QT_TRANSLATE_NOOP("action","Increase duration dotted"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         "dec-duration-dotted",
         QT_TRANSLATE_NOOP("action","Decrease Duration Dotted"),
         QT_TRANSLATE_NOOP("action","Decrease duration dotted"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "repeat-sel",
         QT_TRANSLATE_NOOP("action","Repeat Selection"),
         QT_TRANSLATE_NOOP("action","Repeat selection"),
         0,
         Icons::fileOpen_ICON
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "follow",
         QT_TRANSLATE_NOOP("action","Pan Piano Roll"),
         QT_TRANSLATE_NOOP("action","Toggle pan piano roll"),
         QT_TRANSLATE_NOOP("action","Pan roll during playback"),
         Icons::pan_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE | ShortcutFlags::A_CHECKED
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "split-h",
         QT_TRANSLATE_NOOP("action","Documents Side by Side"),
         QT_TRANSLATE_NOOP("action","Display documents side by side")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "split-v",
         QT_TRANSLATE_NOOP("action","Documents Stacked"),
         QT_TRANSLATE_NOOP("action","Display documents stacked")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "parts",
         QT_TRANSLATE_NOOP("action","Par&ts…"),
         QT_TRANSLATE_NOOP("action","Manage parts")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "enh-both",
         QT_TRANSLATE_NOOP("action","Change Enharmonic Spelling (Both Modes)"),
         QT_TRANSLATE_NOOP("action","Change enharmonic spelling (both modes)"),
         QT_TRANSLATE_NOOP("action","Change enharmonic note (alters the spelling in concert pitch and transposed mode)")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY_STAFF_PITCHED | STATE_NOTE_ENTRY_STAFF_DRUM,
         "enh-current",
         QT_TRANSLATE_NOOP("action","Change Enharmonic Spelling (Current Mode)"),
         QT_TRANSLATE_NOOP("action","Change enharmonic spelling (current mode)"),
         QT_TRANSLATE_NOOP("action","Change enharmonic note (alters the spelling in the current mode only)")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "revision",
         QT_TRANSLATE_NOOP("action","Create New Revision"),
         QT_TRANSLATE_NOOP("action","Create new revision")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_FOTO,
         "fotomode",
         QT_TRANSLATE_NOOP("action","Image Capture"),
         QT_TRANSLATE_NOOP("action","Toggle 'Image Capture'"),
         0,
         Icons::fotomode_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
#ifdef OMR
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "show-omr",
         QT_TRANSLATE_NOOP("action","Show PDF Image"),
         QT_TRANSLATE_NOOP("action","Show PDF image")
         },
#endif
      {
         MsWidget::MAIN_WINDOW,
         ~STATE_TEXT_EDIT,
         "fullscreen",
         QT_TRANSLATE_NOOP("action","Full Screen"),
         QT_TRANSLATE_NOOP("action","Full screen")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "hraster",
         QT_TRANSLATE_NOOP("action","Enable Snap to Horizontal Grid"),
         QT_TRANSLATE_NOOP("action","Enable snap to horizontal grid"),
         0,
         Icons::hraster_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "vraster",
         QT_TRANSLATE_NOOP("action","Enable Snap to Vertical Grid"),
         QT_TRANSLATE_NOOP("action","Enable snap to vertical grid"),
         0,
         Icons::vraster_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "config-raster",
         QT_TRANSLATE_NOOP("action","Configure Grid"),
         QT_TRANSLATE_NOOP("action","Configure grid")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_FOTO | STATE_EDIT,
         "toggle-piano",
         QT_TRANSLATE_NOOP("action","Piano Keyboard"),
         QT_TRANSLATE_NOOP("action","Piano keyboard")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_FOTO | STATE_EDIT,
         "toggle-scorecmp-tool",
         QT_TRANSLATE_NOOP("action","Score Comparison Tool"),
         QT_TRANSLATE_NOOP("action","Score comparison tool")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT  | STATE_PLAY | STATE_FOTO,
         "media",
         QT_TRANSLATE_NOOP("action","Additional Media…"),
         QT_TRANSLATE_NOOP("action","Show media dialog")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "split-measure",
         QT_TRANSLATE_NOOP("action","Split Measure Before Selected Note/Rest"),
         QT_TRANSLATE_NOOP("action","Split measure before selected note/rest")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "join-measures",
         QT_TRANSLATE_NOOP("action","Join Selected Measures"),
         QT_TRANSLATE_NOOP("action","Join selected measures")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY | STATE_FOTO,
         "page-settings",
         QT_TRANSLATE_NOOP("action","Page Settings…"),
         QT_TRANSLATE_NOOP("action","Page settings")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL,
         "album",
         QT_TRANSLATE_NOOP("action","Album…"),
         QT_TRANSLATE_NOOP("action","Album"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "layer",
         QT_TRANSLATE_NOOP("action","Layers…"),
         QT_TRANSLATE_NOOP("action","Layers"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-score",
         QT_TRANSLATE_NOOP("action","Next Score"),
         QT_TRANSLATE_NOOP("action","Next score"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "previous-score",
         QT_TRANSLATE_NOOP("action","Previous Score"),
         QT_TRANSLATE_NOOP("action","Previous score"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         // conflicts with Ctrl+Shift-P when editing dynamics
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_FOTO,
         "plugin-creator",
         QT_TRANSLATE_NOOP("action", "Plugin Creator…"),
         QT_TRANSLATE_NOOP("action", "Plugin creator"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "plugin-manager",
         QT_TRANSLATE_NOOP("action", "Plugin Manager…"),
         QT_TRANSLATE_NOOP("action", "Plugin manager"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_FOTO | STATE_TEXT_EDIT,
         "inspector",
         QT_TRANSLATE_NOOP("action","Inspector"),
         QT_TRANSLATE_NOOP("action","Show inspector")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "resource-manager",
         QT_TRANSLATE_NOOP("action", "&Resource Manager…"),
         QT_TRANSLATE_NOOP("action", "Resource manager"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
#ifdef OMR
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         "omr",
         QT_TRANSLATE_NOOP("action","PDF Transcribing Assistant"),
         QT_TRANSLATE_NOOP("action","Show PDF transcribing assistant")
         },
#endif
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "loop",
         QT_TRANSLATE_NOOP("action","Loop Playback"),
         QT_TRANSLATE_NOOP("action","Toggle 'Loop Playback'"),
         0,
         Icons::loop_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "loop-in",
         QT_TRANSLATE_NOOP("action","Loop In"),
         QT_TRANSLATE_NOOP("action","Set loop in position"),
         0,
         Icons::loopIn_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "loop-out",
         QT_TRANSLATE_NOOP("action","Loop Out"),
         QT_TRANSLATE_NOOP("action","Set loop out position"),
         0,
         Icons::loopOut_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         "metronome",
         QT_TRANSLATE_NOOP("action","Metronome"),
         QT_TRANSLATE_NOOP("action","Toggle metronome playback"),
         QT_TRANSLATE_NOOP("action","Play metronome during playback"),
         Icons::metronome_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         "countin",
         QT_TRANSLATE_NOOP("action","Count-In"),
         QT_TRANSLATE_NOOP("action","Toggle 'Count-In' playback"),
         QT_TRANSLATE_NOOP("action","Play count-in at playback start"),
         Icons::countin_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         "playback-speed-increase",
         QT_TRANSLATE_NOOP("action","Increase Playback Speed"),
         QT_TRANSLATE_NOOP("action","Increase playback speed"),
         QT_TRANSLATE_NOOP("action","Increase the playback speed")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         "playback-speed-decrease",
         QT_TRANSLATE_NOOP("action","Decrease Playback Speed"),
         QT_TRANSLATE_NOOP("action","Decrease playback speed"),
         QT_TRANSLATE_NOOP("action","Decrease the playback speed")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT,
         "playback-speed-reset",
         QT_TRANSLATE_NOOP("action","Reset Playback Speed"),
         QT_TRANSLATE_NOOP("action","Reset playback speed"),
         QT_TRANSLATE_NOOP("action","Reset the playback speed to 100%")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "figured-bass",
         QT_TRANSLATE_NOOP("action","Figured Bass"),
         QT_TRANSLATE_NOOP("action","Add figured bass"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "transpose-up",
         QT_TRANSLATE_NOOP("action","Transpose Up"),
         QT_TRANSLATE_NOOP("action","Transpose up")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "transpose-down",
         QT_TRANSLATE_NOOP("action","Transpose Down"),
         QT_TRANSLATE_NOOP("action","Transpose down")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "masterpalette",
         QT_TRANSLATE_NOOP("action","Master Palette…"),
         QT_TRANSLATE_NOOP("action","Show master palette"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "key-signatures",
         QT_TRANSLATE_NOOP("action","Key Signatures…"),
         QT_TRANSLATE_NOOP("action","Show key signature palette"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "time-signatures",
         QT_TRANSLATE_NOOP("action","Time Signatures…"),
         QT_TRANSLATE_NOOP("action","Show time signature palette"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "symbols",
         QT_TRANSLATE_NOOP("action","Symbols…"),
         QT_TRANSLATE_NOOP("action","Show symbol palette"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "viewmode",
         QT_TRANSLATE_NOOP("action","Toggle View Mode"),
         QT_TRANSLATE_NOOP("action","Toggle 'View Mode'"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_LYRICS_EDIT,
         "next-lyric",
         QT_TRANSLATE_NOOP("action","Next Syllable"),
         QT_TRANSLATE_NOOP("action","Next syllable")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_LYRICS_EDIT,
         "prev-lyric",
         QT_TRANSLATE_NOOP("action","Previous Syllable"),
         QT_TRANSLATE_NOOP("action","Previous syllable")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "toggle-visible",
         QT_TRANSLATE_NOOP("action","Toggle Visibility"),
         QT_TRANSLATE_NOOP("action","Toggle 'Visibility'")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "set-visible",
         QT_TRANSLATE_NOOP("action","Set Visible"),
         QT_TRANSLATE_NOOP("action","Set visible")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "unset-visible",
         QT_TRANSLATE_NOOP("action","Set Invisible"),
         QT_TRANSLATE_NOOP("action","Set invisible")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-noteline",
         QT_TRANSLATE_NOOP("action","Note Anchored Line"),
         QT_TRANSLATE_NOOP("action","Note anchored line")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_LOCK,
         "lock",
         QT_TRANSLATE_NOOP("action","Lock Score"),
         QT_TRANSLATE_NOOP("action","Lock score")
         },

      // TAB-specific actions
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,                     // use a STATE value which is never used: shortcut is never active
         "note-longa-TAB",
         QT_TRANSLATE_NOOP("action","Longa (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: Longa (TAB)"),
         QT_TRANSLATE_NOOP("action","Longa note"),
         Icons::longaUp_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "note-breve-TAB",
         QT_TRANSLATE_NOOP("action","Double Whole Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: Double whole (TAB)"),
         QT_TRANSLATE_NOOP("action","Double whole note"),
         Icons::brevis_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-1-TAB",
         QT_TRANSLATE_NOOP("action","Whole Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: Whole (TAB)"),
         QT_TRANSLATE_NOOP("action","Whole note"),
         Icons::note_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-2-TAB",
         QT_TRANSLATE_NOOP("action","Half Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: Half (TAB)"),
         QT_TRANSLATE_NOOP("action","Half note"),
         Icons::note2_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-4-TAB",
         QT_TRANSLATE_NOOP("action","Quarter Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: Quarter (TAB)"),
         QT_TRANSLATE_NOOP("action","Quarter note"),
         Icons::note4_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-8-TAB",
         QT_TRANSLATE_NOOP("action","Eighth Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: Eighth (TAB)"),
         QT_TRANSLATE_NOOP("action","Eighth note"),
         Icons::note8_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-16-TAB",
         QT_TRANSLATE_NOOP("action","16th Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 16th (TAB)"),
         QT_TRANSLATE_NOOP("action","16th note"),
         Icons::note16_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-32-TAB",
         QT_TRANSLATE_NOOP("action","32nd Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 32nd (TAB)"),
         QT_TRANSLATE_NOOP("action","32nd note"),
         Icons::note32_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-64-TAB",
         QT_TRANSLATE_NOOP("action","64th Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 64th (TAB)"),
         QT_TRANSLATE_NOOP("action","64th note"),
         Icons::note64_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-128-TAB",
         QT_TRANSLATE_NOOP("action","128th Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 128th (TAB)"),
         QT_TRANSLATE_NOOP("action","128th note"),
         Icons::note128_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-256-TAB",
         QT_TRANSLATE_NOOP("action","256th Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 256th (TAB)"),
         QT_TRANSLATE_NOOP("action","256th note"),
         Icons::note256_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-512-TAB",
         QT_TRANSLATE_NOOP("action","512th Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 512th (TAB)"),
         QT_TRANSLATE_NOOP("action","512th note"),
         Icons::note512_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-1024-TAB",
         QT_TRANSLATE_NOOP("action","1024th Note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 1024th (TAB)"),
         QT_TRANSLATE_NOOP("action","1024th note"),
         Icons::note1024_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "pad-note-increase-TAB",
         QT_TRANSLATE_NOOP("action","Increase Active Duration (TAB)"),
         QT_TRANSLATE_NOOP("action","Increase active duration (TAB)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "pad-note-decrease-TAB",
         QT_TRANSLATE_NOOP("action","Decrease Active Duration (TAB)"),
         QT_TRANSLATE_NOOP("action","Decrease active duration (TAB)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "rest-TAB",
         QT_TRANSLATE_NOOP("action","Rest (TAB)"),
         QT_TRANSLATE_NOOP("action","Enter rest (TAB)"),
         0,
         Icons::quartrest_ICON
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "pad-rest-TAB",
         QT_TRANSLATE_NOOP("action","Rest (TAB)"),
         QT_TRANSLATE_NOOP("action","Note input: Rest (TAB)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "string-above",
         QT_TRANSLATE_NOOP("action","String Above (TAB)"),
         QT_TRANSLATE_NOOP("action","Select string above (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "string-below",
         QT_TRANSLATE_NOOP("action","String Below (TAB)"),
         QT_TRANSLATE_NOOP("action","Select string below (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-0",
         QT_TRANSLATE_NOOP("action","Fret 0 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 0 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-1",
         QT_TRANSLATE_NOOP("action","Fret 1 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 1 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-2",
         QT_TRANSLATE_NOOP("action","Fret 2 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 2 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-3",
         QT_TRANSLATE_NOOP("action","Fret 3 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 3 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-4",
         QT_TRANSLATE_NOOP("action","Fret 4 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 4 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-5",
         QT_TRANSLATE_NOOP("action","Fret 5 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 5 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-6",
         QT_TRANSLATE_NOOP("action","Fret 6 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 6 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-7",
         QT_TRANSLATE_NOOP("action","Fret 7 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 7 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-8",
         QT_TRANSLATE_NOOP("action","Fret 8 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 8 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-9",
         QT_TRANSLATE_NOOP("action","Fret 9 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 9 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-10",
         QT_TRANSLATE_NOOP("action","Fret 10 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 10 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-11",
         QT_TRANSLATE_NOOP("action","Fret 11 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 11 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-12",
         QT_TRANSLATE_NOOP("action","Fret 12 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 12 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-13",
         QT_TRANSLATE_NOOP("action","Fret 13 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 13 on current string (TAB only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_STAFF_TAB,
         "fret-14",
         QT_TRANSLATE_NOOP("action","Fret 14 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 14 on current string (TAB only)")
         },

      // HARMONY / FIGURED BASS specific actions

      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-longa",
         QT_TRANSLATE_NOOP("action","Longa Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a longa (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-breve",
         QT_TRANSLATE_NOOP("action","Breve Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a double whole note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-1",
         QT_TRANSLATE_NOOP("action","Whole Note Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a whole note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-2",
         QT_TRANSLATE_NOOP("action","Half Note Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a half note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-4",
         QT_TRANSLATE_NOOP("action","Quarter Note Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a quarter note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-8",
         QT_TRANSLATE_NOOP("action","Eighth Note Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of an eighth note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-16",
         QT_TRANSLATE_NOOP("action","16th Note Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a 16th note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-32",
         QT_TRANSLATE_NOOP("action","32nd Note Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a 32nd note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-64",
         QT_TRANSLATE_NOOP("action","64th Note Advance (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Advance of a 64th note (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "prev-measure-TEXT",
         QT_TRANSLATE_NOOP("action","Previous Measure (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Previous measure (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "next-measure-TEXT",
         QT_TRANSLATE_NOOP("action","Next Measure (F.B./Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Next measure (Figured bass/Chord symbol only)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "prev-beat-TEXT",
         QT_TRANSLATE_NOOP("action","Previous Beat (Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Previous beat (Chord symbol)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "next-beat-TEXT",
         QT_TRANSLATE_NOOP("action","Next Beat (Chord Symbol)"),
         QT_TRANSLATE_NOOP("action","Next beat (Chord symbol)")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-brackets",
         QT_TRANSLATE_NOOP("action","Add Brackets to Accidental"),
         QT_TRANSLATE_NOOP("action","Add brackets to accidental"),
         0,
         Icons::brackets_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-parentheses",
         QT_TRANSLATE_NOOP("action","Add Parentheses to Element"),
         QT_TRANSLATE_NOOP("action","Add parentheses to element"),
         0,
         Icons::parentheses_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-braces",
         QT_TRANSLATE_NOOP("action","Add Braces to Element"),
         QT_TRANSLATE_NOOP("action","Add Braces to element"),
         0,
         Icons::braces_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "toggle-mmrest",
         QT_TRANSLATE_NOOP("action","Toggle 'Create Multimeasure Rest'"),
         QT_TRANSLATE_NOOP("action","Toggle 'Create Multimeasure Rest'"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "toggle-hide-empty",
         QT_TRANSLATE_NOOP("action","Toggle 'Hide Empty Staves'"),
         QT_TRANSLATE_NOOP("action","Toggle 'Hide Empty Staves'"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-b",
         QT_TRANSLATE_NOOP("action","Bold Face"),
         QT_TRANSLATE_NOOP("action","Bold face")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-i",
         QT_TRANSLATE_NOOP("action","Italic")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-u",
         QT_TRANSLATE_NOOP("action","Underline")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-s",
         QT_TRANSLATE_NOOP("action","Strike-through")
         },
      {
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-word-left",
         QT_TRANSLATE_NOOP("action","Move Word Left"),
         QT_TRANSLATE_NOOP("action","Move word left"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-word-right",
         QT_TRANSLATE_NOOP("action","Move Word Right"),
         QT_TRANSLATE_NOOP("action","Move word right"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "explode",
         QT_TRANSLATE_NOOP("action","Explode"),
         QT_TRANSLATE_NOOP("action","Explode"),
         QT_TRANSLATE_NOOP("action","Explode contents of top selected staff into staves below"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "realize-chord-symbols",
         QT_TRANSLATE_NOOP("action","Realize Chord Symbols"),
         QT_TRANSLATE_NOOP("action","Realize chord symbols"),
         QT_TRANSLATE_NOOP("action","Convert chord symbols into notes"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "implode",
         QT_TRANSLATE_NOOP("action","Implode"),
         QT_TRANSLATE_NOOP("action","Implode"),
         QT_TRANSLATE_NOOP("action","Implode contents of selected staves into top selected staff"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "slash-fill",
         QT_TRANSLATE_NOOP("action","Fill With Slashes"),
         QT_TRANSLATE_NOOP("action","Fill with slashes"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "slash-rhythm",
         QT_TRANSLATE_NOOP("action","Toggle Rhythmic Slash Notation"),
         QT_TRANSLATE_NOOP("action","Toggle 'Rhythmic Slash Notation'"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "add-remove-breaks",
         QT_TRANSLATE_NOOP("action","Add/Remove System Breaks…"),
         QT_TRANSLATE_NOOP("action","Add/remove system breaks"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "resequence-rehearsal-marks",
         QT_TRANSLATE_NOOP("action","Resequence Rehearsal Marks"),
         QT_TRANSLATE_NOOP("action","Resequence rehearsal marks"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "copy-lyrics-to-clipboard",
         QT_TRANSLATE_NOOP("action","Copy Lyrics to Clipboard"),
         QT_TRANSLATE_NOOP("action","Copy lyrics to clipboard"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_PLAY,
         "startcenter",
         QT_TRANSLATE_NOOP("action","Start Center…"),
         QT_TRANSLATE_NOOP("action","Start center"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "edit-toolbars",
         QT_TRANSLATE_NOOP("action","Customize Toolbars…"),
         QT_TRANSLATE_NOOP("action","Customize toolbars"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "del-empty-measures",
         QT_TRANSLATE_NOOP("action","Remove Empty Trailing Measures"),
         QT_TRANSLATE_NOOP("action","Remove empty trailing measures"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "unroll-repeats",
         QT_TRANSLATE_NOOP("action","Unroll Repeats"),
         QT_TRANSLATE_NOOP("action","Unroll Repeats"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "show-tours",
         QT_TRANSLATE_NOOP("action", "Show Tours"),
         QT_TRANSLATE_NOOP("action", "Show tours"),
         QT_TRANSLATE_NOOP("action", "Toggle display of tours"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "reset-tours",
         QT_TRANSLATE_NOOP("action", "Reset Tours"),
         QT_TRANSLATE_NOOP("action", "Reset tours"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "toggle-autoplace",
         QT_TRANSLATE_NOOP("action","Toggle Automatic Placement"),
         QT_TRANSLATE_NOOP("action","Toggle 'Automatic Placement' for selected elements")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "autoplace-enabled",
         QT_TRANSLATE_NOOP("action","Toggle Automatic Placement Globally"),
         QT_TRANSLATE_NOOP("action","Toggle 'Automatic Placement' globally"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "report-bug",
         QT_TRANSLATE_NOOP("action", "Report a Bug"),
         QT_TRANSLATE_NOOP("action", "Report a bug"),
         0,
         Icons::bug_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "apply-input-state",
         QT_TRANSLATE_NOOP("action","Apply Input State"),
         QT_TRANSLATE_NOOP("action","Apply input state")
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "leave-feedback",
         QT_TRANSLATE_NOOP("action", "Feedback"),
         QT_TRANSLATE_NOOP("action", "Leave feedback"),
         0,
         Icons::mail_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::PIANO_ROLL_EDITOR,
         STATE_ALL,
         "zoom-in-horiz-pre",
         QT_TRANSLATE_NOOP("action", "Zoom In Horizontally"),
         QT_TRANSLATE_NOOP("action", "Zoom in horizontally - piano roll editor"),
         0,
         Icons::Invalid_ICON
         },
      {
         MsWidget::PIANO_ROLL_EDITOR,
         STATE_ALL,
         "zoom-out-horiz-pre",
         QT_TRANSLATE_NOOP("action", "Zoom Out Horizontally"),
         QT_TRANSLATE_NOOP("action", "Zoom out horizontally - piano roll editor"),
         0,
         Icons::Invalid_ICON
         },
      {
         MsWidget::PIANO_ROLL_EDITOR,
         STATE_ALL,
         "zoom-in-vert-pre",
         QT_TRANSLATE_NOOP("action", "Zoom In Vertically"),
         QT_TRANSLATE_NOOP("action", "Zoom in vertically - piano roll editor"),
         0,
         Icons::Invalid_ICON
         },
      {
         MsWidget::PIANO_ROLL_EDITOR,
         STATE_ALL,
         "zoom-out-vert-pre",
         QT_TRANSLATE_NOOP("action", "Zoom Out Vertically"),
         QT_TRANSLATE_NOOP("action", "Zoom out vertically - piano roll editor"),
         0,
         Icons::Invalid_ICON
         },

#ifdef MSCORE_UNSTABLE
      {
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "toggle-script-recorder",
         "Script Recorder",
         "Script recorder",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
#endif
#ifndef NDEBUG
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "no-horizontal-stretch",
         "No Horizontal Stretch",
         "No horizontal stretch",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "no-vertical-stretch",
         "No Vertical Stretch",
         "No vertical stretch",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "show-segment-shapes",
         "Show Segment Shapes",
         "Show segment shapes",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "show-skylines",
         "Show Skylines",
         "Show Skylines",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "show-bounding-rect",
         "Show Bounding Rectangles",
         "Show bounding rectangles for selected elements",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "show-system-bounding-rect",
         "Show System Bounding Rectangles",
         "Show bounding rectangles for systems",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "show-corrupted-measures",
         "Show Corrupted Measures",
         "Show corrupted measures",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL & ~STATE_TEXT_EDIT,
         "relayout",
         "Re-Layout",
         "Re-layout",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
      {
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         "qml-reload-source",
         "Reload QML code",
         "Reload QML code",
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         },
#endif
      };


//---------------------------------------------------------
//   Shortcut
//---------------------------------------------------------

Shortcut::Shortcut(MsWidget assignedWidget, int s, const char* name,
   const char* txt, const char* d, const char* h, Icons i, Qt::ShortcutContext cont, ShortcutFlags f)
      {
      _assignedWidget = assignedWidget;
      _state       = s;
      _flags       = f;
      _key         = QByteArray(name);
      _context     = cont;
      if (txt)
            _text = QByteArray(txt);
      _descr       = d ? QByteArray(d) : _text;
      _help        = h ? QByteArray(h) : _descr;
      _icon        = i;
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Shortcut::clear()
      {
      _standardKey = QKeySequence::UnknownKey;
      _keys.clear();
      if (_action)
            _action->setShortcuts(_keys);
      }

//---------------------------------------------------------
//   setKeys
//---------------------------------------------------------

void Shortcut::setKeys(const QList<QKeySequence>& ks)
      {
      _standardKey = QKeySequence::UnknownKey;
      _keys = ks;
      if (_action)
            _action->setShortcuts(_keys);
      }

//---------------------------------------------------------
//   setStandardKey
//---------------------------------------------------------

void Shortcut::setStandardKey(QKeySequence::StandardKey k)
      {
      if (QKeySequence::keyBindings(k).empty()) // make sure key binding is set for OS
            return;

      _standardKey = k;
      if (_action && k != QKeySequence::UnknownKey)
            _action->setShortcuts(_standardKey);
      }

//---------------------------------------------------------
//   setKeys
//    Copies keys from the other Shortcut object
//---------------------------------------------------------

void Shortcut::setKeys(const Shortcut& sc)
      {
      setKeys(sc._keys);
      setStandardKey(sc._standardKey);
      }

//---------------------------------------------------------
//   descr
//---------------------------------------------------------

QString Shortcut::descr() const
      {
      return qApp->translate("action", _descr.data());
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Shortcut::text() const
      {
      return qApp->translate("action", _text.data());
      }

//---------------------------------------------------------
//   help
//---------------------------------------------------------

QString Shortcut::help() const
      {
      return qApp->translate("action", _help.data());
      }

//---------------------------------------------------------
//   getShortcut
//---------------------------------------------------------

Shortcut* Shortcut::getShortcut(const char* id)
      {
      Shortcut* s = _shortcuts.value(QByteArray(id));
      if (s == 0) {
            qDebug("Internal error: shortcut <%s> not found", id);
            return 0;
            }
      return s;
      }

//---------------------------------------------------------
//   getAction
//    returns action for shortcut
//---------------------------------------------------------

QAction* getAction(const char* id)
      {
      Shortcut* s = Shortcut::getShortcut(id);
      return s ? s->action() : 0;
      }

//---------------------------------------------------------
//   aAction
//---------------------------------------------------------

QAction* Shortcut::action() const
      {
      if (_action)
            return _action;

      if (_state == STATE_NEVER)
            return 0;

      _action = new QAction(0);
      _action->setData(_key);
      _action->setIconVisibleInMenu (false);
      if (isCheckable()) {
            _action->setCheckable(isCheckable());
            _action->setChecked(isChecked());
            }

      if (_keys.isEmpty())
            _action->setShortcuts(_standardKey);
      else
            _action->setShortcuts(_keys);

      _action->setShortcutContext(_context);
      translateAction(_action);

      if (_icon != Icons::Invalid_ICON)
            _action->setIcon(*icons[int(_icon)]);

      return _action;
      }

//---------------------------------------------------------
//   translateAction
//---------------------------------------------------------

void Shortcut::translateAction(QAction* action) const
      {
      action->setText(text());
      if (!_help.isEmpty()) {
            action->setToolTip(help());
            action->setWhatsThis(help());
            }
      else {
            action->setToolTip(descr());
            action->setWhatsThis(descr());
            }
      action->setStatusTip(QString("action:%1").arg(_key.data()));
      QList<QKeySequence> kl = action->shortcuts();
      if (!kl.isEmpty()) {
            QString s(action->toolTip());
            s += " (";
            for (int i = 0; i < kl.size(); ++i) {
                  if (i)
                        s += ",";
                  s += Shortcut::keySeqToString(kl[i], QKeySequence::NativeText);
                  }
            s += ")";
            action->setToolTip(s);
            }
      }

//---------------------------------------------------------
//   addShortcut
//---------------------------------------------------------

void Shortcut::addShortcut(const QKeySequence& ks)
      {
      _keys.append(ks);
      if (_action)
            _action->setShortcuts(_keys);
      dirty = true;
      }

//---------------------------------------------------------
//   keysToString
//---------------------------------------------------------

QString Shortcut::keysToString() const
      {
      QString s;
      for (int i = 0; i < _keys.size(); ++i) {
            if (i)
                  s += "; ";
            s += Shortcut::keySeqToString(_keys[i], QKeySequence::NativeText);
            }
      if (s.isEmpty() && _standardKey != QKeySequence::UnknownKey) {
            QList<QKeySequence> keySeqList = QKeySequence::keyBindings(_standardKey);
            for (int i = 0; i < keySeqList.size(); i++) {
                  if (i)
                        s += "; ";
                  s += Shortcut::keySeqToString(keySeqList[i], QKeySequence::NativeText);
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   getMenuShortcutString
//---------------------------------------------------------

QString Shortcut::getMenuShortcutString(const QMenu *menu)
      {
      if (menu->title().isEmpty())
            return "";
      int shortcutKeyPosition = menu->title().indexOf('&');
      if (shortcutKeyPosition < 0)
            return "";
      return QString("Alt+") + menu->title().at(shortcutKeyPosition + 1);
      }

//---------------------------------------------------------
//   compareKeys
//    return true if keys are equal
//---------------------------------------------------------

bool Shortcut::compareKeys(const Shortcut& sc) const
      {
      if (sc._keys.size() != _keys.size())
            return false;
      for (int i = 0; i < _keys.size(); ++i) {
            if (sc._keys[i] != _keys[i])
                  return false;
            }
      return _standardKey == sc._standardKey;
      }

//---------------------------------------------------------
//   initShortcuts
//---------------------------------------------------------

void Shortcut::init()
      {
      //
      // initialize shortcut hash table
      //
      _shortcuts.clear();
      for (Shortcut& i : _sc)
            _shortcuts.insert(i._key, &i);
      if (!MScore::noGui)
            load();
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void Shortcut::retranslate()
      {
      for (const Shortcut& i : _sc) {
            if (i._action) {
                  i.translateAction(i._action);
                  }
            }
      }

void Shortcut::refreshIcons()
      {
      for (Shortcut* s : _shortcuts) {
            QAction* a = s->action();
            if (a && s->icon() != Icons::Invalid_ICON) {
                  a->setIcon(*icons[int(s->icon())]);
                  }
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Shortcut::save()
      {
      QFile f(dataPath + "/shortcuts.xml");
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot save shortcuts");
            return;
            }
      XmlWriter xml(0, &f);
      xml.header();
      xml.stag("Shortcuts");
      for (auto i : _sc)
            i.write(xml);
      xml.etag();
      f.close();
      }

void Shortcut::saveToNewFile(QString fileLocation)
      {
      QFile f(fileLocation);
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot save shortcuts");
            return;
            }
      XmlWriter xml(0, &f);
      xml.header();
      xml.stag("Shortcuts");
      for (auto i : _sc)
            i.write(xml);
      xml.etag();
      f.close();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Shortcut::write(XmlWriter& xml) const
      {
      xml.stag("SC");
      xml.tag("key", _key.data());
      if (_standardKey != QKeySequence::UnknownKey)
            xml.tag("std", QString("%1").arg(_standardKey));
      for (QKeySequence ks : _keys)
            xml.tag("seq", Shortcut::keySeqToString(ks, QKeySequence::PortableText, true));
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Shortcut::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "key")
                  _key = e.readElementText().toLocal8Bit();
            else if (tag == "std") {
                  int i = e.readInt();
                  if (!QKeySequence::keyBindings((QKeySequence::StandardKey(i))).empty()) // make sure key binding is set for OS
                        _standardKey = QKeySequence::StandardKey(i);
                  }
            else if (tag == "seq") {
                  QKeySequence seq  = Shortcut::keySeqFromString(e.readElementText(), QKeySequence::PortableText);
#ifndef NDEBUG
                  for (const Shortcut& sc : _sc) {
                        for (const QKeySequence& s : sc._keys) {
                              if (s == seq)
                                    qDebug("Ambiguous shortcut for action <%s>", _key.data());
                              }
                        }
#endif
                  _keys.append(seq);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Shortcut::load()
      {
      QFile f(dataPath + "/shortcuts.xml");
      if (!f.exists())
            f.setFileName(defaultFileName);
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Cannot open shortcuts <%s>", qPrintable(f.fileName()));
            return;
            }
      if (MScore::debugMode)
            qDebug("read shortcuts from <%s>", qPrintable(f.fileName()));

      XmlReader e(&f);

      while (e.readNextStartElement()) {
            if (e.name() == "Shortcuts") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "SC") {
                              Shortcut* sc = 0;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "key") {
                                          QString val(e.readElementText());
                                          sc = getShortcut(qPrintable(val));
                                          if (!sc)
                                                qDebug("cannot find shortcut <%s>", qPrintable(val));
                                          else
                                                sc->clear();
                                          }
                                    else if (tag == "std") {
                                          int i = e.readInt();
                                          if(sc && !QKeySequence::keyBindings((QKeySequence::StandardKey(i))).empty()) // make sure key binding is set for OS
                                                sc->_standardKey = QKeySequence::StandardKey(i);
                                          }
                                    else if (tag == "seq") {
                                          QString s = e.readElementText();
                                          if (sc)
                                                sc->_keys.append(Shortcut::keySeqFromString(s, QKeySequence::PortableText));
                                          }
                                    else
                                          e.unknown();
                                    }
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      source = f.fileName();
      dirty = false;
      }

//---------------------------------------------------------
//   Shortcut1
//---------------------------------------------------------

struct Shortcut1 {
      QByteArray key;
      QList<QKeySequence> keys;
      QKeySequence::StandardKey standardKey { QKeySequence::UnknownKey };
      };


//---------------------------------------------------------
//   read
//---------------------------------------------------------

static QList<Shortcut1> loadShortcuts(QString fileLocation)
      {
      QList<Shortcut1> list;
      QFile f(fileLocation);
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Cannot open shortcuts");
            QMessageBox::critical(0, QObject::tr("Load Shortcuts"), QObject::tr("Can't load shortcuts file: %1").arg(strerror(errno)));
            return list;
            }
      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "Shortcuts") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "SC") {
                              Shortcut1 sc;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "key")
                                          sc.key = e.readElementText().toLocal8Bit();
                                    else if (tag == "std") {
                                          int i = e.readInt();
                                          if (!QKeySequence::keyBindings(QKeySequence::StandardKey(i)).empty()) // make sure key binding is set for OS
                                                sc.standardKey = QKeySequence::StandardKey(i);
                                          }
                                    else if (tag == "seq")
                                          sc.keys.append(Shortcut::keySeqFromString(e.readElementText(), QKeySequence::PortableText));
                                    else
                                          e.unknown();
                                    }
                              list.append(sc);
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return list;
      }

void Shortcut::loadFromNewFile(QString fileLocation)
      {
      QList<Shortcut1> list = loadShortcuts(fileLocation);
      for (const Shortcut1& sc : list) {
            Shortcut* s = getShortcut(sc.key);
            if (s) {
                  s->setKeys(sc.keys);
                  s->setStandardKey(sc.standardKey);
                  }
            }
      source = fileLocation;
      dirty = true;
      }

//---------------------------------------------------------
//    getActionGroupForWidget
//---------------------------------------------------------

QActionGroup* Shortcut::getActionGroupForWidget(MsWidget w)
      {
      QActionGroup* ag = new QActionGroup(NULL);
      ag->setExclusive(false);
      ag->setEnabled(true);
      for (Shortcut* s : Shortcut::shortcuts()) {
            if (s->assignedWidget() == w) {
                  QAction* a = s->action();
                  if (a)
                        ag->addAction(s->action());
                  }
            }
      return ag;
      }

QActionGroup* Shortcut::getActionGroupForWidget(MsWidget w, Qt::ShortcutContext newShortcutContext)
      {
      QActionGroup* ag = Shortcut::getActionGroupForWidget(w);
      for (QAction* a : ag->actions())
            a->setShortcutContext(newShortcutContext);
      return ag;
      }

//---------------------------------------------------------
//   resetToBuildin
//    reset all shortcuts to builtin values
//---------------------------------------------------------

void Shortcut::resetToDefault()
      {
      QList<Shortcut1> sl = loadShortcuts(defaultFileName);
      for (const Shortcut1& sc : sl) {
            Shortcut* s = getShortcut(sc.key);
            if (s) {
                  s->setKeys(sc.keys);
                  s->setStandardKey(sc.standardKey);
                  }
            }
      source = defaultFileName;
      dirty = true;
      }

//---------------------------------------------------------
//   getShortcutByKeySequence
//---------------------------------------------------------

Shortcut* Shortcut::getShortcutByKeySequence(const QKeySequence& keySequence, const ScoreState state)
      {
      for (Shortcut* shortcut : _shortcuts.values()) {
            if (!(shortcut->state() & state))
                  continue;

            QAction* action = shortcut->action();

            if (!action)
                  continue;

            for (const QKeySequence& _keySequence : action->shortcuts()) {
                  if (_keySequence == keySequence)
                        return shortcut;
                  }
            }

      return nullptr;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Shortcut::reset()
      {
      _standardKey = QKeySequence::UnknownKey;
      _keys.clear();
      QList<Shortcut1> sl = loadShortcuts(defaultFileName);
      for (const Shortcut1& sc : sl) {
            if (sc.key == _key) {
                  setKeys(sc.keys);
                  setStandardKey(sc.standardKey);
                  break;
                  }
            }
      dirty = true;
      }

//---------------------------------------------------------
//   keySeqToString / keySeqFromString
//---------------------------------------------------------

static const QString numPadPrefix("NumPad+");
static const int NUMPADPREFIX_SIZE = numPadPrefix.size();

QString Shortcut::keySeqToString(const QKeySequence& keySeq, QKeySequence::SequenceFormat fmt, bool escapeKeyStr /* = false */)
      {
      QString s;
      for (int i = 0; i < KEYSEQ_SIZE; ++i) {
            int code;
            if ( (code = keySeq[i]) == 0)
                  break;
            if (i)
                  s += ",";
            if (code & Qt::KeypadModifier) {
                  s += numPadPrefix;
                  code &= ~Qt::KeypadModifier;
                  }
            QString kStr = QKeySequence(code).toString(fmt);
            if (escapeKeyStr) {
                  kStr.replace("\\", "\\\\");
                  kStr.replace(",", "\\,");
                  }
            s += kStr;
            }
      return s;
      }

//---------------------------------------------------------
//   keySeqFromString
//---------------------------------------------------------

QKeySequence Shortcut::keySeqFromString(const QString& str, QKeySequence::SequenceFormat fmt)
      {
      int code[KEYSEQ_SIZE], i;
      for (i = 0; i < KEYSEQ_SIZE; ++i)
            code[i] = 0;

      QStringList strList = str.split(QRegularExpression("(?<!\\\\),|(?<=\\\\\\\\),"), QString::SkipEmptyParts);
      //split based on commas that are not preceded by a single slash; two is okay
      //original regex: (?<!\\),|(?<=\\\\),

      i = 0;
      for (const QString& s : strList) {
            QString keyStr = s.trimmed();
            if (keyStr.contains("\\"))
                  keyStr.remove(keyStr.length() - 2, 1); //remove escaped characters which will always be second to last
            if (keyStr.startsWith(numPadPrefix, Qt::CaseInsensitive) ) {
                  code[i] += Qt::KeypadModifier;
                  keyStr.remove(0, NUMPADPREFIX_SIZE);
                  }
            QKeySequence seq = QKeySequence::fromString(keyStr, fmt);
            code[i] += seq[0];
            if (++i >= KEYSEQ_SIZE)
                  break;
            }
      QKeySequence keySeq(code[0], code[1], code[2], code[3]);
      return keySeq;
      }
}

