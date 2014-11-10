//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: actions.cpp 5657 2012-05-21 15:46:06Z lasconic $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "config.h"
#include "musescore.h"
#include "libmscore/score.h"      // states
#include "icons.h"
#include "shortcut.h"
#include "globals.h"
namespace Ms {

//---------------------------------------------------------
//    initial list of shortcuts
//---------------------------------------------------------

Shortcut Shortcut::sc[] = {
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_ALL,
         0,
         "local-help",
         QT_TRANSLATE_NOOP("action","Local Handbook..."),  // Appears in menu
         QT_TRANSLATE_NOOP("action","Local handbook"),     // Appears in Edit > Preferences > Shortcuts
         QT_TRANSLATE_NOOP("action","Show local handbook") // Appears if you use Help > What's This?
         ),

      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT
            | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "file-open",
         QT_TRANSLATE_NOOP("action","Open..."),
         QT_TRANSLATE_NOOP("action","File open"),
         QT_TRANSLATE_NOOP("action","Load score from file"),
         Icons::fileOpen_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "file-save",
         QT_TRANSLATE_NOOP("action","Save"),
         QT_TRANSLATE_NOOP("action","File save"),
         QT_TRANSLATE_NOOP("action","Save score to file"),
         Icons::fileSave_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "file-save-as",
         QT_TRANSLATE_NOOP("action","Save As..."),
         QT_TRANSLATE_NOOP("action","File save as"),
         QT_TRANSLATE_NOOP("action","Save score under a new file name"),
         Icons::fileSaveAs_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "file-save-selection",
         QT_TRANSLATE_NOOP("action","Save Selection..."),
         QT_TRANSLATE_NOOP("action","Save Selection"),
         QT_TRANSLATE_NOOP("action","Save current selection as new score"),
         Icons::fileSaveAs_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "file-save-a-copy",
         QT_TRANSLATE_NOOP("action","Save a Copy..."),
         QT_TRANSLATE_NOOP("action","File save a copy"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in addition to the current file")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         ShortcutFlags::A_SCORE,
         "file-export",
         QT_TRANSLATE_NOOP("action","Export..."),
         QT_TRANSLATE_NOOP("action","Export score"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in various formats"),
         Icons::fileSave_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "file-part-export",
         QT_TRANSLATE_NOOP("action","Export Parts..."),
         QT_TRANSLATE_NOOP("action","Export Parts"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score's parts in various formats"),
         Icons::fileSave_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "file-close",
         QT_TRANSLATE_NOOP("action","Close"),
         QT_TRANSLATE_NOOP("action","File close"),
         QT_TRANSLATE_NOOP("action","Close current score")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "file-new",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","New..."),
         QT_TRANSLATE_NOOP("action","File new"),
         QT_TRANSLATE_NOOP("action","Create new score"),
         Icons::fileNew_ICON
         ),
      Shortcut(
         Ms::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_LYRICS_EDIT | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "print",
         QT_TRANSLATE_NOOP("action","Print..."),
         QT_TRANSLATE_NOOP("action","Print"),
         QT_TRANSLATE_NOOP("action","Print score"),
         Icons::print_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "undo",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Undo"),
         QT_TRANSLATE_NOOP("action","Undo"),
         QT_TRANSLATE_NOOP("action","Undo last change"),
         Icons::undo_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         0,
         "redo",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Redo"),
         QT_TRANSLATE_NOOP("action","Redo"),
         QT_TRANSLATE_NOOP("action","Redo last undo"),
         Icons::redo_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT,
         0,
         "cut",
         QT_TRANSLATE_NOOP("action","Cut"),
         Icons::cut_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT |STATE_LYRICS_EDIT | STATE_FOTO,
         0,
         "copy",
         QT_TRANSLATE_NOOP("action","Copy"),
         Icons::copy_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT |STATE_LYRICS_EDIT,
         0,
         "paste",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Paste"),
         Icons::paste_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "instruments",
         QT_TRANSLATE_NOOP("action","Instruments..."),
         QT_TRANSLATE_NOOP("action","Show instruments dialog")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "note-input",
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","Note input mode"),
         Icons::noteEntry_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "pitch-spell",
         QT_TRANSLATE_NOOP("action","Respell Pitches")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval1",
         QT_TRANSLATE_NOOP("action","Unison Above"),
         QT_TRANSLATE_NOOP("action","Enter unison above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval2",
         QT_TRANSLATE_NOOP("action","Second Above"),
         QT_TRANSLATE_NOOP("action","Enter second above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval3",
         QT_TRANSLATE_NOOP("action","Third Above"),
         QT_TRANSLATE_NOOP("action","Enter third above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval4",
         QT_TRANSLATE_NOOP("action","Fourth Above"),
         QT_TRANSLATE_NOOP("action","Enter fourth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval5",
         QT_TRANSLATE_NOOP("action","Fifth Above"),
         QT_TRANSLATE_NOOP("action","Enter fifth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval6",
         QT_TRANSLATE_NOOP("action","Sixth Above"),
         QT_TRANSLATE_NOOP("action","Enter sixth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval7",
         QT_TRANSLATE_NOOP("action","Seventh Above"),
         QT_TRANSLATE_NOOP("action","Enter seventh above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval8",
         QT_TRANSLATE_NOOP("action","Octave Above"),
         QT_TRANSLATE_NOOP("action","Enter octave above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval9",
         QT_TRANSLATE_NOOP("action","Ninth Above"),
         QT_TRANSLATE_NOOP("action","Enter ninth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-2",
         QT_TRANSLATE_NOOP("action","Second Below"),
         QT_TRANSLATE_NOOP("action","Enter second below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-3",
         QT_TRANSLATE_NOOP("action","Third Below"),
         QT_TRANSLATE_NOOP("action","Enter third below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-4",
         QT_TRANSLATE_NOOP("action","Fourth Below"),
         QT_TRANSLATE_NOOP("action","Enter fourth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-5",
         QT_TRANSLATE_NOOP("action","Fifth Below"),
         QT_TRANSLATE_NOOP("action","Enter fifth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-6",
         QT_TRANSLATE_NOOP("action","Sixth Below"),
         QT_TRANSLATE_NOOP("action","Enter sixth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-7",
         QT_TRANSLATE_NOOP("action","Seventh Below"),
         QT_TRANSLATE_NOOP("action","Enter seventh below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-8",
         QT_TRANSLATE_NOOP("action","Octave Below"),
         QT_TRANSLATE_NOOP("action","Enter octave below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "interval-9",
         QT_TRANSLATE_NOOP("action","Ninth Below"),
         QT_TRANSLATE_NOOP("action","Enter ninth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "note-a",
         QT_TRANSLATE_NOOP("action","A"),
         QT_TRANSLATE_NOOP("action","Enter note A")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "note-b",
         QT_TRANSLATE_NOOP("action","B"),
         QT_TRANSLATE_NOOP("action","Enter note B")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "note-c",
         QT_TRANSLATE_NOOP("action","C"),
         QT_TRANSLATE_NOOP("action","Enter note C")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "note-d",
         QT_TRANSLATE_NOOP("action","D"),
         QT_TRANSLATE_NOOP("action","Enter note D")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "note-e",
         QT_TRANSLATE_NOOP("action","E"),
         QT_TRANSLATE_NOOP("action","Enter note E")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "note-f",
         QT_TRANSLATE_NOOP("action","F"),
         QT_TRANSLATE_NOOP("action","Enter note F")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "note-g",
         QT_TRANSLATE_NOOP("action","G"),
         QT_TRANSLATE_NOOP("action","Enter note G")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "chord-a",
         QT_TRANSLATE_NOOP("action","Add A"),
         QT_TRANSLATE_NOOP("action","Add note A to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "chord-b",
         QT_TRANSLATE_NOOP("action","Add B"),
         QT_TRANSLATE_NOOP("action","Add note B to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "chord-c",
         QT_TRANSLATE_NOOP("action","Add C"),
         QT_TRANSLATE_NOOP("action","Add note C to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "chord-d",
         QT_TRANSLATE_NOOP("action","Add D"),
         QT_TRANSLATE_NOOP("action","Add note D to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "chord-e",
         QT_TRANSLATE_NOOP("action","Add E"),
         QT_TRANSLATE_NOOP("action","Add note E to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "chord-f",
         QT_TRANSLATE_NOOP("action","Add F"),
         QT_TRANSLATE_NOOP("action","Add note F to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "chord-g",
         QT_TRANSLATE_NOOP("action","Add G"),
         QT_TRANSLATE_NOOP("action","Add note G to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "insert-a",
         QT_TRANSLATE_NOOP("action","Insert A"),
         QT_TRANSLATE_NOOP("action","Insert note A")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "insert-b",
         QT_TRANSLATE_NOOP("action","Insert B"),
         QT_TRANSLATE_NOOP("action","Insert note B")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "insert-c",
         QT_TRANSLATE_NOOP("action","Insert C"),
         QT_TRANSLATE_NOOP("action","Insert note C")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "insert-d",
         QT_TRANSLATE_NOOP("action","Insert D"),
         QT_TRANSLATE_NOOP("action","Insert note D")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "insert-e",
         QT_TRANSLATE_NOOP("action","Insert E"),
         QT_TRANSLATE_NOOP("action","Insert note E")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "insert-f",
         QT_TRANSLATE_NOOP("action","Insert F"),
         QT_TRANSLATE_NOOP("action","Insert note F")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "insert-g",
         QT_TRANSLATE_NOOP("action","Insert G"),
         QT_TRANSLATE_NOOP("action","Insert note G")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "rest",
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Enter rest"),
         Icons::quartrest_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-staccato",
         QT_TRANSLATE_NOOP("action","Staccato"),
         QT_TRANSLATE_NOOP("action","Add staccato")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-tenuto",
         QT_TRANSLATE_NOOP("action","Tenuto"),
         QT_TRANSLATE_NOOP("action","Add tenuto")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-trill",
         QT_TRANSLATE_NOOP("action","Trill"),
         QT_TRANSLATE_NOOP("action","Add trill")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-marcato",
         QT_TRANSLATE_NOOP("action","Marcato"),
         QT_TRANSLATE_NOOP("action","Add marcato")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "stretch+",
         QT_TRANSLATE_NOOP("action","Add More Stretch"),
         QT_TRANSLATE_NOOP("action","Add more stretch"),
         QT_TRANSLATE_NOOP("action","Add more stretch to selected measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "stretch-",
         QT_TRANSLATE_NOOP("action","Add Less Stretch"),
         QT_TRANSLATE_NOOP("action","Add less stretch"),
         QT_TRANSLATE_NOOP("action","Add less stretch to selected measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "reset-beammode",
         QT_TRANSLATE_NOOP("action","Reset Beam Mode"),
         QT_TRANSLATE_NOOP("action","Reset beam mode"),
         QT_TRANSLATE_NOOP("action","Reset beam mode of selected measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "flip",
         QT_TRANSLATE_NOOP("action","Flip direction"),
         Icons::flip_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
      STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "pitch-up",
         QT_TRANSLATE_NOOP("action","Up"),
         QT_TRANSLATE_NOOP("action","Pitch up or move text or articulation up")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "pitch-up-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic up"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch up")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "pitch-up-octave",
         QT_TRANSLATE_NOOP("action","Up Octave"),
         QT_TRANSLATE_NOOP("action","Pitch up octave"),
         QT_TRANSLATE_NOOP("action","Pitch up by an octave or move text or articulation up")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "up-chord",
         QT_TRANSLATE_NOOP("action","Up Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to higher pitched note in chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "top-chord",
         QT_TRANSLATE_NOOP("action","Top Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to top note in chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "move-up",
         QT_TRANSLATE_NOOP("action","Move up")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
      STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "pitch-down",
         QT_TRANSLATE_NOOP("action","Down"),
         QT_TRANSLATE_NOOP("action","Pitch down or move text or articulation down")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "pitch-down-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic down"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch down")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "pitch-down-octave",
         QT_TRANSLATE_NOOP("action","Down octave"),
         QT_TRANSLATE_NOOP("action","Pitch down octave"),
         QT_TRANSLATE_NOOP("action","Pitch down by an octave or move text or articulation down")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "down-chord",
         QT_TRANSLATE_NOOP("action","Down Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to lower pitched note in chord"),
         QT_TRANSLATE_NOOP("action","Go to lower pitched note in chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "next-element",
         QT_TRANSLATE_NOOP("action","Next element"),
         QT_TRANSLATE_NOOP("action","Accessibility: next element"),
         QT_TRANSLATE_NOOP("action","Accessibility: next element")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "prev-element",
         QT_TRANSLATE_NOOP("action","Previous element"),
         QT_TRANSLATE_NOOP("action","Accessibility: previous element"),
         QT_TRANSLATE_NOOP("action","Accessibility: previous element")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "first-element",
         QT_TRANSLATE_NOOP("action","First element"),
         QT_TRANSLATE_NOOP("action","Go to the first element"),
         QT_TRANSLATE_NOOP("action","Go to the first element")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "last-element",
         QT_TRANSLATE_NOOP("action","Last element"),
         QT_TRANSLATE_NOOP("action","Go to the last element"),
         QT_TRANSLATE_NOOP("action","Go to the last element")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "bottom-chord",
         QT_TRANSLATE_NOOP("action","Bottom Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to bottom note in chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "move-down",
         QT_TRANSLATE_NOOP("action","Move down"),
         QT_TRANSLATE_NOOP("action","Move down")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "prev-chord",
         QT_TRANSLATE_NOOP("action","Previous chord"),
         QT_TRANSLATE_NOOP("action","Go to previous chord or move text left")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "prev-measure",
         QT_TRANSLATE_NOOP("action","Previous measure"),
         QT_TRANSLATE_NOOP("action","Go to previous measure or move text left")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "prev-track",
         QT_TRANSLATE_NOOP("action","Previous staff or voice")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "next-chord",
         QT_TRANSLATE_NOOP("action","Next chord"),
         QT_TRANSLATE_NOOP("action","Go to next chord or move text right")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "next-measure",
         QT_TRANSLATE_NOOP("action","Next measure"),
         QT_TRANSLATE_NOOP("action","Go to next measure or move text right")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "next-track",
         QT_TRANSLATE_NOOP("action","Next staff or voice")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-prev-chord",
         QT_TRANSLATE_NOOP("action","Add previous chord to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-prev-measure",
         QT_TRANSLATE_NOOP("action","Select to beginning of measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-next-chord",
         QT_TRANSLATE_NOOP("action","Add next chord to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-section",
         QT_TRANSLATE_NOOP("action","Select Section")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         0,
         "move-right",
         QT_TRANSLATE_NOOP("action","Move chord/rest right")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         0,
         "move-left",
         QT_TRANSLATE_NOOP("action","Move chord/rest left")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-next-measure",
         QT_TRANSLATE_NOOP("action","Select to end of measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-begin-line",
         QT_TRANSLATE_NOOP("action","Select to beginning of line")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-end-line",
         QT_TRANSLATE_NOOP("action","Select to end of line")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-begin-score",
         QT_TRANSLATE_NOOP("action","Select to beginning of score")
      ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "select-end-score",
         QT_TRANSLATE_NOOP("action","Select to end of score")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-staff-above",
         QT_TRANSLATE_NOOP("action","Add staff above to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-staff-below",
         QT_TRANSLATE_NOOP("action","Add staff below to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-prev",
         QT_TRANSLATE_NOOP("action","Page: previous")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-next",
         QT_TRANSLATE_NOOP("action","Page: next")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-top",
         QT_TRANSLATE_NOOP("action","Page: top")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "page-end",
         QT_TRANSLATE_NOOP("action","Page: end")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "add-slur",
         QT_TRANSLATE_NOOP("action","Slur"),
         QT_TRANSLATE_NOOP("action","Add slur")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-hairpin",
         QT_TRANSLATE_NOOP("action","Crescendo"),
         QT_TRANSLATE_NOOP("action","Add crescendo")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-hairpin-reverse",
         QT_TRANSLATE_NOOP("action","Decrescendo"),
         QT_TRANSLATE_NOOP("action","Add decrescendo")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-8va",
         QT_TRANSLATE_NOOP("action","Ottava 8va"),
         QT_TRANSLATE_NOOP("action","Add ottava 8va")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-8vb",
         QT_TRANSLATE_NOOP("action","Ottava 8vb"),
         QT_TRANSLATE_NOOP("action","Add ottava 8vb")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "escape",
         QT_TRANSLATE_NOOP("action","Escape")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "delete",
         QT_TRANSLATE_NOOP("action","Delete"),
         QT_TRANSLATE_NOOP("action","Delete"),
         QT_TRANSLATE_NOOP("action","Delete contents of the selected measures")
    ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "full-measure-rest",
         QT_TRANSLATE_NOOP("action","Full Measure Rest"),
         QT_TRANSLATE_NOOP("action","Full Measure Rest"),
         QT_TRANSLATE_NOOP("action","Converts the measure to a full measure rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "time-delete",
         QT_TRANSLATE_NOOP("action","Timewise delete"),
         QT_TRANSLATE_NOOP("action","Timewise Delete"),
         QT_TRANSLATE_NOOP("action","Delete element and duration")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "delete-measures",
         QT_TRANSLATE_NOOP("action","Delete Selected Measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-measure",
         QT_TRANSLATE_NOOP("action","Append One Measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-measures",
         QT_TRANSLATE_NOOP("action","Append Measures..."),
         QT_TRANSLATE_NOOP("action","Append measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-measure",
         QT_TRANSLATE_NOOP("action","Insert One Measure"),
         QT_TRANSLATE_NOOP("action","Insert One Measure"),
         QT_TRANSLATE_NOOP("action","Insert one measure"),
         Icons::measure_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-measures",
         QT_TRANSLATE_NOOP("action","Insert Measures..."),
         QT_TRANSLATE_NOOP("action","Insert measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-hbox",
         QT_TRANSLATE_NOOP("action","Insert Horizontal Frame"),
         QT_TRANSLATE_NOOP("action","Insert Horizontal Frame"),
         QT_TRANSLATE_NOOP("action","Insert horizontal frame"),
         Icons::hframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-textframe",
         QT_TRANSLATE_NOOP("action","Insert Text Frame"),
         QT_TRANSLATE_NOOP("action","Insert Text Frame"),
         QT_TRANSLATE_NOOP("action","Insert text frame"),
         Icons::tframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-textframe",
         QT_TRANSLATE_NOOP("action","Append Text Frame")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-fretframe",
         QT_TRANSLATE_NOOP("action","Insert Fretboard Diagram Frame"),
         Icons::fframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "insert-vbox",
         QT_TRANSLATE_NOOP("action","Insert Vertical Frame"),
         QT_TRANSLATE_NOOP("action","Insert Vertical Frame"),
         QT_TRANSLATE_NOOP("action","Insert vertical frame"),
         Icons::vframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-hbox",
         QT_TRANSLATE_NOOP("action","Append Horizontal Frame")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "append-vbox",
         QT_TRANSLATE_NOOP("action","Append Vertical Frame")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "duplet",
         QT_TRANSLATE_NOOP("action","Duplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "triplet",
         QT_TRANSLATE_NOOP("action","Triplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "quadruplet",
         QT_TRANSLATE_NOOP("action","Quadruplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "quintuplet",
         QT_TRANSLATE_NOOP("action","Quintuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "sextuplet",
         QT_TRANSLATE_NOOP("action","Sextuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "septuplet",
         QT_TRANSLATE_NOOP("action","Septuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "octuplet",
         QT_TRANSLATE_NOOP("action","Octuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "nonuplet",
         QT_TRANSLATE_NOOP("action","Nonuplet")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "tuplet-dialog",
         QT_TRANSLATE_NOOP("action","Other..."),
         QT_TRANSLATE_NOOP("action","Other tuplets")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "note-longa",
         QT_TRANSLATE_NOOP("action","Longa"),
         QT_TRANSLATE_NOOP("action","Note duration: longa"),
         QT_TRANSLATE_NOOP("action","Longa"),
         Icons::longaUp_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "note-breve",
         QT_TRANSLATE_NOOP("action","Double whole note"),
         QT_TRANSLATE_NOOP("action","Note duration: double whole"),
         QT_TRANSLATE_NOOP("action","Double whole note"),
         Icons::brevis_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-1",
         QT_TRANSLATE_NOOP("action","Whole note"),
         QT_TRANSLATE_NOOP("action","Note duration: whole"),
         QT_TRANSLATE_NOOP("action","Whole note"),
         Icons::note_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-2",
         QT_TRANSLATE_NOOP("action","Half note"),
         QT_TRANSLATE_NOOP("action","Note duration: half"),
         QT_TRANSLATE_NOOP("action","Half note"),
         Icons::note2_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-4",
         QT_TRANSLATE_NOOP("action","Quarter note"),
         QT_TRANSLATE_NOOP("action","Note duration: quarter"),
         QT_TRANSLATE_NOOP("action","Quarter note"),
         Icons::note4_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-8",
         QT_TRANSLATE_NOOP("action","Eighth note"),
         QT_TRANSLATE_NOOP("action","Note duration: eighth"),
         QT_TRANSLATE_NOOP("action","Eighth note"),
         Icons::note8_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-16",
         QT_TRANSLATE_NOOP("action","16th note"),
         QT_TRANSLATE_NOOP("action","Note duration: 16th"),
         QT_TRANSLATE_NOOP("action","16th note"),
         Icons::note16_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-32",
         QT_TRANSLATE_NOOP("action","32nd note"),
         QT_TRANSLATE_NOOP("action","Note duration: 32nd"),
         QT_TRANSLATE_NOOP("action","32nd note"),
         Icons::note32_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-64",
         QT_TRANSLATE_NOOP("action","64th note"),
         QT_TRANSLATE_NOOP("action","Note duration: 64th"),
         QT_TRANSLATE_NOOP("action","64th note"),
         Icons::note64_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         ShortcutFlags::A_CMD,
         "pad-note-128",
         QT_TRANSLATE_NOOP("action","128th note"),
         QT_TRANSLATE_NOOP("action","Note duration: 128th"),
         QT_TRANSLATE_NOOP("action","128th note"),
         Icons::note128_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         0,
         "pad-note-increase",
         QT_TRANSLATE_NOOP("action","Increase active duration"),
         QT_TRANSLATE_NOOP("action","Increase active duration")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         0,
         "pad-note-decrease",
         QT_TRANSLATE_NOOP("action","Decrease active duration"),
         QT_TRANSLATE_NOOP("action","Decrease active duration")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "pad-dot",
         QT_TRANSLATE_NOOP("action","Augmentation dot"),
         QT_TRANSLATE_NOOP("action","Note duration: augmentation dot"),
         QT_TRANSLATE_NOOP("action","Augmentation dot"),
         Icons::dot_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "pad-dotdot",
         QT_TRANSLATE_NOOP("action","Double augmentation dot"),
         QT_TRANSLATE_NOOP("action","Note duration: double augmentation dot"),
         QT_TRANSLATE_NOOP("action","Double augmentation dot"),
         Icons::dotdot_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "tie",
         QT_TRANSLATE_NOOP("action","Tie"),
         QT_TRANSLATE_NOOP("action","Note duration: tie"),
         QT_TRANSLATE_NOOP("action","Tie"),
         Icons::tie_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "pad-rest",
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Note entry: rest"),
         QT_TRANSLATE_NOOP("action","Rest"),
         Icons::quartrest_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "sharp2",
         QT_TRANSLATE_NOOP("action","Double sharp"),
         QT_TRANSLATE_NOOP("action","Note entry: double sharp"),
         QT_TRANSLATE_NOOP("action","Double sharp"),
         Icons::sharpsharp_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "sharp",
         QT_TRANSLATE_NOOP("action","Sharp"),
         QT_TRANSLATE_NOOP("action","Note entry: sharp"),
         QT_TRANSLATE_NOOP("action","Sharp"),
         Icons::sharp_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "nat",
         QT_TRANSLATE_NOOP("action","Natural"),
         QT_TRANSLATE_NOOP("action","Note entry: natural"),
         QT_TRANSLATE_NOOP("action","Natural"),
         Icons::natural_ICON
         ),
       Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "flat",
         QT_TRANSLATE_NOOP("action","Flat"),
         QT_TRANSLATE_NOOP("action","Note entry: flat"),
         QT_TRANSLATE_NOOP("action","Flat"),
         Icons::flat_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         ShortcutFlags::A_CMD,
         "flat2",
         QT_TRANSLATE_NOOP("action","Double flat"),
         QT_TRANSLATE_NOOP("action","Note entry: double flat"),
         QT_TRANSLATE_NOOP("action","Double flat"),
         Icons::flatflat_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "acciaccatura",
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         QT_TRANSLATE_NOOP("action","Add acciaccatura"),
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         Icons::acciaccatura_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "appoggiatura",
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         QT_TRANSLATE_NOOP("action","Add appoggiatura"),
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         Icons::appoggiatura_ICON
         ),
      Shortcut(
        MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
       /* no stroke: 4th*/
        "grace4",
        QT_TRANSLATE_NOOP("action","Grace: quarter"),
        QT_TRANSLATE_NOOP("action","Add quarter grace note"),
        QT_TRANSLATE_NOOP("action","Grace: quarter"),
        Icons::grace4_ICON
        ),
     Shortcut(
        MsWidget::SCORE_TAB,
        STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
      /* no stroke: 16th*/
        "grace16",
        QT_TRANSLATE_NOOP("action","Grace: 16th"),
        QT_TRANSLATE_NOOP("action","Add 16th grace note"),
        QT_TRANSLATE_NOOP("action","Grace: 16th"),
        Icons::grace16_ICON
        ),
     Shortcut(
        MsWidget::SCORE_TAB,
        STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
      /* no stroke: 32nd*/
        "grace32",
        QT_TRANSLATE_NOOP("action","Grace: 32nd"),
        QT_TRANSLATE_NOOP("action","Add 32nd grace note"),
        QT_TRANSLATE_NOOP("action","Grace: 32nd"),
        Icons::grace32_ICON
        ),
        Shortcut(
          MsWidget::SCORE_TAB,
          STATE_NORMAL | STATE_NOTE_ENTRY,
          0,
        /* no stroke: Eighth*/
         "grace8after",
         QT_TRANSLATE_NOOP("action","Grace: eighth after"),
         QT_TRANSLATE_NOOP("action","Add Eighth grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: eighth after"),
         Icons::grace8after_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
          0,
       /* no stroke: 16th*/
         "grace16after",
         QT_TRANSLATE_NOOP("action","Grace: 16th after"),
         QT_TRANSLATE_NOOP("action","Add 16th grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: 16th after"),
         Icons::grace16after_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
          0,
       /* no stroke: 32nd*/
         "grace32after",
         QT_TRANSLATE_NOOP("action","Grace: 32nd after"),
         QT_TRANSLATE_NOOP("action","Add 32nd grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: 32nd after"),
         Icons::grace32after_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-1",
         QT_TRANSLATE_NOOP("action","1"),
         QT_TRANSLATE_NOOP("action","Voice 1"),
         QT_TRANSLATE_NOOP("action","Voice 1")
//         Icons::voice1_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-2",
         QT_TRANSLATE_NOOP("action","2"),
         QT_TRANSLATE_NOOP("action","Voice 2"),
         QT_TRANSLATE_NOOP("action","Voice 2")
//         Icons::voice2_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-3",
         QT_TRANSLATE_NOOP("action","3"),
         QT_TRANSLATE_NOOP("action","Voice 3"),
         QT_TRANSLATE_NOOP("action","Voice 3")
//         Icons::voice3_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "voice-4",
         QT_TRANSLATE_NOOP("action","4"),
         QT_TRANSLATE_NOOP("action","Voice 4"),
         QT_TRANSLATE_NOOP("action","Voice 4")
//         Icons::voice4_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "midi-on",
         QT_TRANSLATE_NOOP("action","MIDI input"),
         QT_TRANSLATE_NOOP("action","Enable MIDI input"),
         QT_TRANSLATE_NOOP("action","Enable MIDI input"),
         Icons::midiin_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "beam-start",
         QT_TRANSLATE_NOOP("action","Beam start"),
         Icons::sbeam_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "beam-mid",
         QT_TRANSLATE_NOOP("action","Beam middle"),
         Icons::mbeam_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "no-beam",
         QT_TRANSLATE_NOOP("action","No beam"),
         Icons::nbeam_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "beam32",
         QT_TRANSLATE_NOOP("action","Beam 16th sub"),
         Icons::beam32_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "beam64",
         QT_TRANSLATE_NOOP("action","Beam 32nd sub"),
         Icons::beam64_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "auto-beam",
         QT_TRANSLATE_NOOP("action","Auto beam"),
         Icons::abeam_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "fbeam1",
         QT_TRANSLATE_NOOP("action","Feathered beam, slower"),
         Icons::fbeam1_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "fbeam2",
         QT_TRANSLATE_NOOP("action","Feathered beam, faster"),
         Icons::fbeam2_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "toggle-palette",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "toggle-playpanel",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Play Panel")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "toggle-selection-window",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Selection Filter")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "toggle-navigator",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Navigator")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "toggle-midiimportpanel",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","MIDI Import Panel")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
#ifdef Q_OS_MAC
         //Avoid conflict with M in text
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
#else
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
#endif
         0,
         "toggle-mixer",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Mixer")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "toggle-transport",
         QT_TRANSLATE_NOOP("action","Transport"),
         QT_TRANSLATE_NOOP("action","Transport toolbar")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "toggle-noteinput",
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","Note input toolbar")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "toggle-statusbar",
         QT_TRANSLATE_NOOP("action","Status Bar")
         ),

      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "quit",
         QT_TRANSLATE_NOOP("action","Quit")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "mag",
         QT_TRANSLATE_NOOP("action","Zoom canvas")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "lyrics",
         QT_TRANSLATE_NOOP("action","Lyrics"),
         QT_TRANSLATE_NOOP("action","Add lyrics")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "tempo",
         QT_TRANSLATE_NOOP("action","Tempo Marking..."),
         QT_TRANSLATE_NOOP("action","Add tempo marking")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "system-text",
         QT_TRANSLATE_NOOP("action","System Text"),
         QT_TRANSLATE_NOOP("action","Add system text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "staff-text",
         QT_TRANSLATE_NOOP("action","Staff Text"),
         QT_TRANSLATE_NOOP("action","Add staff text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "frame-text",
         QT_TRANSLATE_NOOP("action","Text"),
         QT_TRANSLATE_NOOP("action","Add frame text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "title-text",
         QT_TRANSLATE_NOOP("action","Title"),
         QT_TRANSLATE_NOOP("action","Add title text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "subtitle-text",
         QT_TRANSLATE_NOOP("action","Subtitle"),
         QT_TRANSLATE_NOOP("action","Add subtitle text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "composer-text",
         QT_TRANSLATE_NOOP("action","Composer"),
         QT_TRANSLATE_NOOP("action","Add composer text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "poet-text",
         QT_TRANSLATE_NOOP("action","Lyricist"),
         QT_TRANSLATE_NOOP("action","Add lyricist text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "part-text",
         QT_TRANSLATE_NOOP("action","Part Name"),
         QT_TRANSLATE_NOOP("action","Add part name")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "chord-text",
         QT_TRANSLATE_NOOP("action","Chord Symbol"),
         QT_TRANSLATE_NOOP("action","Add chord symbol")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rehearsalmark-text",
         QT_TRANSLATE_NOOP("action","Rehearsal Mark"),
         QT_TRANSLATE_NOOP("action","Add rehearsal mark")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "picture",
         QT_TRANSLATE_NOOP("action","Picture"),
         QT_TRANSLATE_NOOP("action","Add picture")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "play",
         QT_TRANSLATE_NOOP("action","Play"),
         QT_TRANSLATE_NOOP("action","Player play"),
         QT_TRANSLATE_NOOP("action","Start or stop playback"),
         Icons::play_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         0,
         "play-prev-chord",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Play Previous Chord")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         0,
         "play-prev-measure",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Play Previous Measure")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         0,
         "play-next-chord",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Play Next Chord")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         0,
         "play-next-measure",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Play Next Measure")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         0,
         "seek-begin",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Player Seek to Begin")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "rewind",
         QT_TRANSLATE_NOOP("action","Rewind"),
         QT_TRANSLATE_NOOP("action","Player rewind"),
         QT_TRANSLATE_NOOP("action","Rewind to start position"),
         Icons::start_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         0,
         "seek-end",
         QT_TRANSLATE_NOOP("action","Player Seek to End")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         ShortcutFlags::A_SCORE,
         "repeat",
         QT_TRANSLATE_NOOP("action","Play repeats"),
         QT_TRANSLATE_NOOP("action","Toggle repeats playback"),
         QT_TRANSLATE_NOOP("action","Play repeats"),
         Icons::repeat_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "pan",
         QT_TRANSLATE_NOOP("action","Pan"),
         QT_TRANSLATE_NOOP("action","Toggle pan score"),
         QT_TRANSLATE_NOOP("action","Pan score during playback"),
         Icons::pan_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "load-style",
         QT_TRANSLATE_NOOP("action","Load Style..."),
         QT_TRANSLATE_NOOP("action","Load style"),
         Icons::fileOpen_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         ShortcutFlags::A_SCORE,
         "save-style",
         QT_TRANSLATE_NOOP("action","Save Style..."),
         QT_TRANSLATE_NOOP("action","Save style"),
         Icons::fileSave_ICON
         ),
/*      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         A_SCORE,
         "save-default-style",
         QT_TRANSLATE_NOOP("action","Save Style as Default..."),
         Icons::fileSave_ICON
         ),
*/
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         ShortcutFlags::A_CMD,
         "select-all",
         QT_TRANSLATE_NOOP("action","Select All")
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "transpose",
         QT_TRANSLATE_NOOP("action","&Transpose..."),
         QT_TRANSLATE_NOOP("action","Transpose")
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "clef-violin",
         QT_TRANSLATE_NOOP("action","Treble Clef"),
         QT_TRANSLATE_NOOP("action","Add treble clef")
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "clef-bass",
         QT_TRANSLATE_NOOP("action","Bass Clef"),
         QT_TRANSLATE_NOOP("action","Add bass clef")
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "voice-x12",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-2")
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "voice-x13",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-3")
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "voice-x14",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-4")
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "voice-x23",
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-3")
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "voice-x24",
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-4")
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "voice-x34",
         QT_TRANSLATE_NOOP("action","Exchange Voice 3-4")
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "concert-pitch",
         QT_TRANSLATE_NOOP("action","Concert Pitch"),
         QT_TRANSLATE_NOOP("action","Display in concert pitch")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "repeat-cmd",
         QT_TRANSLATE_NOOP("action","Repeat last command"),
         Icons::fileOpen_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "edit-info",
         QT_TRANSLATE_NOOP("action","Info..."),
         QT_TRANSLATE_NOOP("action","Edit score info")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "system-break",
         QT_TRANSLATE_NOOP("action","Toggle System Break")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "page-break",
         QT_TRANSLATE_NOOP("action","Toggle Page Break")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "section-break",
         QT_TRANSLATE_NOOP("action","Toggle Section Break")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "edit-element",
         QT_TRANSLATE_NOOP("action","Edit Element")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_EDIT,
         0,
         "reset",
         QT_TRANSLATE_NOOP("action","Reset"),
         QT_TRANSLATE_NOOP("action","Reset user settings")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "debugger",
         QT_TRANSLATE_NOOP("action","Debugger")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "reset-stretch",
         QT_TRANSLATE_NOOP("action","Reset Stretch"),
         QT_TRANSLATE_NOOP("action","Reset measure stretch")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_SCORE,
         "show-invisible",
         QT_TRANSLATE_NOOP("action","Show Invisible")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_SCORE,
         "show-unprintable",
         QT_TRANSLATE_NOOP("action","Show Unprintable")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_SCORE,
         "show-frames",
         QT_TRANSLATE_NOOP("action","Show Frames")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_SCORE,
         "show-pageborders",
         QT_TRANSLATE_NOOP("action","Show Page Margins")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_TEXT_EDIT | STATE_LYRICS_EDIT | STATE_HARMONY_FIGBASS_EDIT,
         0,
         "show-keys",
         QT_TRANSLATE_NOOP("action","Insert Special Characters..."),
         QT_TRANSLATE_NOOP("action","Insert Special Characters"),
         Icons::keys_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-1",
         QT_TRANSLATE_NOOP("action","Whole rest"),
         QT_TRANSLATE_NOOP("action","Note entry: whole rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-2",
         QT_TRANSLATE_NOOP("action","Half rest"),
         QT_TRANSLATE_NOOP("action","Note entry: half rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-4",
         QT_TRANSLATE_NOOP("action","Quarter rest"),
         QT_TRANSLATE_NOOP("action","Note entry: quarter rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "rest-8",
         QT_TRANSLATE_NOOP("action","Eighth rest"),
         QT_TRANSLATE_NOOP("action","Note entry: eighth rest")
         ),
      Shortcut(                     // mapped to undo in note entry mode
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "backspace",
         QT_TRANSLATE_NOOP("action","Backspace")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "find",
         QT_TRANSLATE_NOOP("action","Find")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         0,
         "zoomin",
         QT_TRANSLATE_NOOP("action","Zoom In")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         // conflicts with Ctrl+- in edit mode to enter lyrics hyphen
         // STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,

         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "zoomout",
         QT_TRANSLATE_NOOP("action","Zoom Out")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "mirror-note",
         QT_TRANSLATE_NOOP("action","Mirror note head"),
         Icons::flip_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "edit-style",
         QT_TRANSLATE_NOOP("action","General..."),
         QT_TRANSLATE_NOOP("action","Edit general style")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "edit-text-style",
         QT_TRANSLATE_NOOP("action","Text..."),
         QT_TRANSLATE_NOOP("action","Edit text style")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "edit-harmony",
         QT_TRANSLATE_NOOP("action","Chord Symbols..."),
         QT_TRANSLATE_NOOP("action","Edit chord symbols style")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-similar",
         QT_TRANSLATE_NOOP("action","All Similar Elements"),
         QT_TRANSLATE_NOOP("action","Select all similar elements")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-similar-staff",
         QT_TRANSLATE_NOOP("action","All Similar Elements in Same Staff"),
         QT_TRANSLATE_NOOP("action","Select all similar elements in same staff")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "select-similar-range",
         QT_TRANSLATE_NOOP("action","All Similar Elements in Range Selection"),
         QT_TRANSLATE_NOOP("action","Select all similar elements in the range selection")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "synth-control",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Synthesizer")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         ShortcutFlags::A_CMD,
         "double-duration",
         QT_TRANSLATE_NOOP("action","Double duration")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         ShortcutFlags::A_CMD,
         "half-duration",
         QT_TRANSLATE_NOOP("action","Half duration")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "repeat-sel",
         QT_TRANSLATE_NOOP("action","Repeat selection"),
         Icons::fileOpen_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "follow",
         QT_TRANSLATE_NOOP("action","Pan piano roll"),
         QT_TRANSLATE_NOOP("action","Toggle pan piano roll"),
         QT_TRANSLATE_NOOP("action","Pan roll during playback"),
         Icons::pan_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "split-h",
         QT_TRANSLATE_NOOP("action","Documents Side by Side"),
         QT_TRANSLATE_NOOP("action","Display documents side by side")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "split-v",
         QT_TRANSLATE_NOOP("action","Documents Stacked"),
         QT_TRANSLATE_NOOP("action","Display documents stacked")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "parts",
         QT_TRANSLATE_NOOP("action","Parts..."),
         QT_TRANSLATE_NOOP("action","Manage parts")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "enh-up",
         QT_TRANSLATE_NOOP("action","Enharmonic up")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         0,
         "enh-down",
         QT_TRANSLATE_NOOP("action","Enharmonic down")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         0,
         "revision",
         QT_TRANSLATE_NOOP("action","Create new revision")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_FOTO,
         0,
         "fotomode",
         QT_TRANSLATE_NOOP("action","Toggle screenshot mode"),
         Icons::fotomode_ICON
         ),
#ifdef OMR
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "show-omr",
         QT_TRANSLATE_NOOP("action","Show OMR image")
         ),
#endif
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL & (~STATE_TEXT_EDIT),
         0,
         "fullscreen",
         QT_TRANSLATE_NOOP("action","Full Screen")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "hraster",
         QT_TRANSLATE_NOOP("action","Enable snap to horizontal grid"),
         Icons::hraster_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "vraster",
         QT_TRANSLATE_NOOP("action","Enable snap to vertical grid"),
         Icons::vraster_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "config-raster",
         QT_TRANSLATE_NOOP("action","Configure grid")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "repitch",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Re-Pitch Mode"),
         QT_TRANSLATE_NOOP("action","Replace pitches without changing rhythms"),
         Icons::repitch_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "toogle-piano",
         QT_TRANSLATE_NOOP("action","Piano Keyboard")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "media",
         QT_TRANSLATE_NOOP("action","Additional Media..."),
         QT_TRANSLATE_NOOP("action","Show media dialog")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "split-measure",
         QT_TRANSLATE_NOOP("action","Split Measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "join-measure",
         QT_TRANSLATE_NOOP("action","Join Measures")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "page-settings",
         QT_TRANSLATE_NOOP("action","Page Settings..."),
         QT_TRANSLATE_NOOP("action","Page Settings")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL,
         0,
         "album",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Album..."),
         QT_TRANSLATE_NOOP("action","Album")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         0,
         "layer",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Layers..."),
         QT_TRANSLATE_NOOP("action","Layers")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "next-score",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Next Score")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "previous-score",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Previous Score")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_INIT | STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         0,
         "musescore-connect",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action", "MuseScore Connect"),
         Icons::community_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "plugin-creator",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action", "Plugin Creator")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "plugin-manager",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action", "Plugin Manager")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_FOTO,
         0,
         "inspector",
         QT_TRANSLATE_NOOP("action","Inspector"),
         QT_TRANSLATE_NOOP("action","Show inspector")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "resource-manager",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action", "Resource Manager")
         ),
#ifdef OMR
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "omr",
         QT_TRANSLATE_NOOP("action","OMR Panel"),
         QT_TRANSLATE_NOOP("action","Show OMR Panel")
         ),
#endif
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "loop",
         QT_TRANSLATE_NOOP("action","Loop"),
         QT_TRANSLATE_NOOP("action","Toggle loop playback"),
         QT_TRANSLATE_NOOP("action","Loop playback"),
         Icons::loop_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "loop-in",
         QT_TRANSLATE_NOOP("action","Loop in"),
         QT_TRANSLATE_NOOP("action","Set loop In position"),
         QT_TRANSLATE_NOOP("action","Set loop In position"),
         Icons::loopIn_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         0,
         "loop-out",
         QT_TRANSLATE_NOOP("action","Loop out"),
         QT_TRANSLATE_NOOP("action","Set loop Out position"),
         QT_TRANSLATE_NOOP("action","Set loop Out position"),
         Icons::loopOut_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "metronome",
         QT_TRANSLATE_NOOP("action","Metronome"),
         QT_TRANSLATE_NOOP("action","Toggle metronome playback"),
         QT_TRANSLATE_NOOP("action","Play metronome during playback"),
         Icons::metronome_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT,
         0,
         "countin",
         QT_TRANSLATE_NOOP("action","Count-in"),
         QT_TRANSLATE_NOOP("action","Toggle count-in playback"),
         QT_TRANSLATE_NOOP("action","Play count-in at playback start"),
         Icons::countin_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "figured-bass",
         QT_TRANSLATE_NOOP("action","Figured Bass"),
         QT_TRANSLATE_NOOP("action","Add figured bass")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "transpose-up",
         QT_TRANSLATE_NOOP("action","Transpose Up")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         0,
         "transpose-down",
         QT_TRANSLATE_NOOP("action","Transpose Down")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "masterpalette",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Master Palette..."),
         QT_TRANSLATE_NOOP("action","Show master palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "key-signatures",
         QT_TRANSLATE_NOOP("action","Key Signatures..."),
         QT_TRANSLATE_NOOP("action","Show key signature palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "time-signatures",
         QT_TRANSLATE_NOOP("action","Time Signatures..."),
         QT_TRANSLATE_NOOP("action","Show time signature palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "symbols",
         QT_TRANSLATE_NOOP("action","Symbols..."),
         QT_TRANSLATE_NOOP("action","Show symbol palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         0,
         "viewmode",
         QT_TRANSLATE_NOOP("action","Toggle View Mode")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_LYRICS_EDIT,
         0,
         "next-lyric",
         QT_TRANSLATE_NOOP("action","Next syllable")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_LYRICS_EDIT,
         0,
         "prev-lyric",
         QT_TRANSLATE_NOOP("action","Previous syllable")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "toggle-visible",
         QT_TRANSLATE_NOOP("action","Toggle visibility")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "set-visible",
         QT_TRANSLATE_NOOP("action","Set visible")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "unset-visible",
         QT_TRANSLATE_NOOP("action","Set invisible")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         0,
         "add-noteline",
         QT_TRANSLATE_NOOP("action","Note anchored Textline")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_LOCK,
         0,
         "lock",
         QT_TRANSLATE_NOOP("action","Lock Score")
         ),

      // TAB-specific actions

      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,                     // use a STATE value which is never used: shortcut is never active
         ShortcutFlags::A_CMD,
         "note-longa-TAB",
         QT_TRANSLATE_NOOP("action","Longa (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: longa (TAB)"),
         Icons::longaUp_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "note-breve-TAB",
         QT_TRANSLATE_NOOP("action","Double whole note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: double whole (TAB)"),
         QT_TRANSLATE_NOOP("action","Double whole note"),
         Icons::brevis_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-1-TAB",
         QT_TRANSLATE_NOOP("action","Whole note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: whole (TAB)"),
         QT_TRANSLATE_NOOP("action","Whole note"),
         Icons::note_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-2-TAB",
         QT_TRANSLATE_NOOP("action","Half note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: half (TAB)"),
         QT_TRANSLATE_NOOP("action","Half note"),
         Icons::note2_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-4-TAB",
         QT_TRANSLATE_NOOP("action","Quarter note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: quarter (TAB)"),
         QT_TRANSLATE_NOOP("action","Quarter note"),
         Icons::note4_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-8-TAB",
         QT_TRANSLATE_NOOP("action","Eighth note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: eighth (TAB)"),
         QT_TRANSLATE_NOOP("action","Eighth note"),
         Icons::note8_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-16-TAB",
         QT_TRANSLATE_NOOP("action","16th note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 16th (TAB)"),
         QT_TRANSLATE_NOOP("action","16th note"),
         Icons::note16_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-32-TAB",
         QT_TRANSLATE_NOOP("action","32nd note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 32nd (TAB)"),
         QT_TRANSLATE_NOOP("action","32nd note"),
         Icons::note32_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-64-TAB",
         QT_TRANSLATE_NOOP("action","64th note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 64th (TAB)"),
         QT_TRANSLATE_NOOP("action","64th note"),
         Icons::note64_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         ShortcutFlags::A_CMD,
         "pad-note-128-TAB",
         QT_TRANSLATE_NOOP("action","128th note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 128th (TAB)"),
         QT_TRANSLATE_NOOP("action","128th note"),
         Icons::note128_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "pad-note-increase-TAB",
         QT_TRANSLATE_NOOP("action","Increase active duration (TAB)"),
         QT_TRANSLATE_NOOP("action","Increase active duration (TAB)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "pad-note-decrease-TAB",
         QT_TRANSLATE_NOOP("action","Decrease active duration (TAB)"),
         QT_TRANSLATE_NOOP("action","Decrease active duration (TAB)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "rest-TAB",
         QT_TRANSLATE_NOOP("action","Rest (TAB)"),
         QT_TRANSLATE_NOOP("action","Enter rest (TAB)"),
         Icons::quartrest_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "pad-rest-TAB",
         QT_TRANSLATE_NOOP("action","Rest (TAB)"),
         QT_TRANSLATE_NOOP("action","Note entry: rest (TAB)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "string-above",
         QT_TRANSLATE_NOOP("action","String above (TAB)"),
         QT_TRANSLATE_NOOP("action","Select string above (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "string-below",
         QT_TRANSLATE_NOOP("action","String below (TAB)"),
         QT_TRANSLATE_NOOP("action","Select string below (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-0",
         QT_TRANSLATE_NOOP("action","Fret 0 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 0 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-1",
         QT_TRANSLATE_NOOP("action","Fret 1 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 1 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-2",
         QT_TRANSLATE_NOOP("action","Fret 2 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 2 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-3",
         QT_TRANSLATE_NOOP("action","Fret 3 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 3 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-4",
         QT_TRANSLATE_NOOP("action","Fret 4 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 4 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-5",
         QT_TRANSLATE_NOOP("action","Fret 5 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 5 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-6",
         QT_TRANSLATE_NOOP("action","Fret 6 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 6 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-7",
         QT_TRANSLATE_NOOP("action","Fret 7 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 7 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-8",
         QT_TRANSLATE_NOOP("action","Fret 8 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 8 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-9",
         QT_TRANSLATE_NOOP("action","Fret 9 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 9 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-10",
         QT_TRANSLATE_NOOP("action","Fret 10 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 10 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-11",
         QT_TRANSLATE_NOOP("action","Fret 11 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 11 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-12",
         QT_TRANSLATE_NOOP("action","Fret 12 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 12 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-13",
         QT_TRANSLATE_NOOP("action","Fret 13 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 13 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         0,
         "fret-14",
         QT_TRANSLATE_NOOP("action","Fret 14 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 14 of current string (TAB only)")
         ),

      // HARMONY / FIGURED BASS specific actions

      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-longa",
         QT_TRANSLATE_NOOP("action","Longa advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a longa (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-breve",
         QT_TRANSLATE_NOOP("action","Breve advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a double whole note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-1",
         QT_TRANSLATE_NOOP("action","Whole note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a whole note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-2",
         QT_TRANSLATE_NOOP("action","Half note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a half note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-4",
         QT_TRANSLATE_NOOP("action","Quarter note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a quarter note (F.B./Harm. only)")
        ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-8",
         QT_TRANSLATE_NOOP("action","Eighth note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of an eighth note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-16",
         QT_TRANSLATE_NOOP("action","16th note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a 16th note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-32",
         QT_TRANSLATE_NOOP("action","32nd note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a 32nd note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "advance-64",
         QT_TRANSLATE_NOOP("action","64th note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a 64th note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "prev-measure-TEXT",
         QT_TRANSLATE_NOOP("action","Previous measure (F.B./Harm.)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "next-measure-TEXT",
         QT_TRANSLATE_NOOP("action","Next measure (F.B./Harm.)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "prev-beat-TEXT",
         QT_TRANSLATE_NOOP("action","Previous beat (Chord symbol)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         0,
         "next-beat-TEXT",
         QT_TRANSLATE_NOOP("action","Next beat (Chord symbol)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         ShortcutFlags::A_CMD,
         "add-brackets",
         QT_TRANSLATE_NOOP("action","Add brackets to element"),
         Icons::brackets_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         ShortcutFlags::A_CMD,
         "toggle-mmrest",
         QT_TRANSLATE_NOOP("action","Toggle create multimeasure rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         0,
         "text-b",
         QT_TRANSLATE_NOOP("action","bold face")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         0,
         "text-i",
         QT_TRANSLATE_NOOP("action","italic")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         0,
         "text-u",
         QT_TRANSLATE_NOOP("action","underline")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         0,
         "startcenter",
         Qt::ApplicationShortcut,
         QT_TRANSLATE_NOOP("action","Toggle Startcenter")
         ),
      // xml==0  marks end of list
      Shortcut(MsWidget::MAIN_WINDOW,0, 0, 0, 0)
      };
}

