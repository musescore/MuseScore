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
         "local-help",
         QT_TRANSLATE_NOOP("action","Local Handbook..."),  // Appears in menu
         QT_TRANSLATE_NOOP("action","Local handbook"),     // Appears in Edit > Preferences > Shortcuts
         QT_TRANSLATE_NOOP("action","Show local handbook"), // Appears if you use Help > What's This?
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::NONE
         ),

      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT
            | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-open",
         QT_TRANSLATE_NOOP("action","Open..."),
         QT_TRANSLATE_NOOP("action","File open"),
         QT_TRANSLATE_NOOP("action","Load score from file"),
         Icons::fileOpen_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-save",
         QT_TRANSLATE_NOOP("action","Save"),
         QT_TRANSLATE_NOOP("action","File save"),
         QT_TRANSLATE_NOOP("action","Save score to file"),
         Icons::fileSave_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-save-as",
         QT_TRANSLATE_NOOP("action","Save As..."),
         QT_TRANSLATE_NOOP("action","File save as"),
         QT_TRANSLATE_NOOP("action","Save score under a new file name"),
         Icons::fileSaveAs_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-save-selection",
         QT_TRANSLATE_NOOP("action","Save Selection..."),
         QT_TRANSLATE_NOOP("action","Save Selection"),
         QT_TRANSLATE_NOOP("action","Save current selection as new score"),
         Icons::fileSaveAs_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-save-a-copy",
         QT_TRANSLATE_NOOP("action","Save a Copy..."),
         QT_TRANSLATE_NOOP("action","File save a copy"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in addition to the current file"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "file-export",
         QT_TRANSLATE_NOOP("action","Export..."),
         QT_TRANSLATE_NOOP("action","Export score"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score in various formats"),
         Icons::fileSave_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-part-export",
         QT_TRANSLATE_NOOP("action","Export Parts..."),
         QT_TRANSLATE_NOOP("action","Export Parts"),
         QT_TRANSLATE_NOOP("action","Save a copy of the score's parts in various formats"),
         Icons::fileSave_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-close",
         QT_TRANSLATE_NOOP("action","Close"),
         QT_TRANSLATE_NOOP("action","File close"),
         QT_TRANSLATE_NOOP("action","Close current score")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "file-new",
         QT_TRANSLATE_NOOP("action","New..."),
         QT_TRANSLATE_NOOP("action","File new"),
         QT_TRANSLATE_NOOP("action","Create new score"),
         Icons::fileNew_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         Ms::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_LYRICS_EDIT | STATE_PLAY,
         "print",
         QT_TRANSLATE_NOOP("action","Print..."),
         QT_TRANSLATE_NOOP("action","Print"),
         QT_TRANSLATE_NOOP("action","Print score"),
         Icons::print_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "undo",
         QT_TRANSLATE_NOOP("action","Undo"),
         QT_TRANSLATE_NOOP("action","Undo"),
         QT_TRANSLATE_NOOP("action","Undo last change"),
         Icons::undo_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT,
         "redo",
         QT_TRANSLATE_NOOP("action","Redo"),
         QT_TRANSLATE_NOOP("action","Redo"),
         QT_TRANSLATE_NOOP("action","Redo last undo"),
         Icons::redo_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT,
         "cut",
         QT_TRANSLATE_NOOP("action","Cut"),
         0,
         0,
         Icons::cut_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT |STATE_LYRICS_EDIT | STATE_FOTO,
         "copy",
         QT_TRANSLATE_NOOP("action","Copy"),
         0,
         0,
         Icons::copy_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT |STATE_LYRICS_EDIT,
         "paste",
         QT_TRANSLATE_NOOP("action","Paste"),
         0,
         0,
         Icons::paste_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "instruments",
         QT_TRANSLATE_NOOP("action","Instruments..."),
         QT_TRANSLATE_NOOP("action","Show instruments dialog")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "note-input",
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","Note input mode"),
         0,
         Icons::noteEntry_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-spell",
         QT_TRANSLATE_NOOP("action","Respell Pitches"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval1",
         QT_TRANSLATE_NOOP("action","Unison Above"),
         QT_TRANSLATE_NOOP("action","Enter unison above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval2",
         QT_TRANSLATE_NOOP("action","Second Above"),
         QT_TRANSLATE_NOOP("action","Enter second above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval3",
         QT_TRANSLATE_NOOP("action","Third Above"),
         QT_TRANSLATE_NOOP("action","Enter third above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval4",
         QT_TRANSLATE_NOOP("action","Fourth Above"),
         QT_TRANSLATE_NOOP("action","Enter fourth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval5",
         QT_TRANSLATE_NOOP("action","Fifth Above"),
         QT_TRANSLATE_NOOP("action","Enter fifth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval6",
         QT_TRANSLATE_NOOP("action","Sixth Above"),
         QT_TRANSLATE_NOOP("action","Enter sixth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval7",
         QT_TRANSLATE_NOOP("action","Seventh Above"),
         QT_TRANSLATE_NOOP("action","Enter seventh above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval8",
         QT_TRANSLATE_NOOP("action","Octave Above"),
         QT_TRANSLATE_NOOP("action","Enter octave above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval9",
         QT_TRANSLATE_NOOP("action","Ninth Above"),
         QT_TRANSLATE_NOOP("action","Enter ninth above")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-2",
         QT_TRANSLATE_NOOP("action","Second Below"),
         QT_TRANSLATE_NOOP("action","Enter second below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-3",
         QT_TRANSLATE_NOOP("action","Third Below"),
         QT_TRANSLATE_NOOP("action","Enter third below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-4",
         QT_TRANSLATE_NOOP("action","Fourth Below"),
         QT_TRANSLATE_NOOP("action","Enter fourth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-5",
         QT_TRANSLATE_NOOP("action","Fifth Below"),
         QT_TRANSLATE_NOOP("action","Enter fifth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-6",
         QT_TRANSLATE_NOOP("action","Sixth Below"),
         QT_TRANSLATE_NOOP("action","Enter sixth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-7",
         QT_TRANSLATE_NOOP("action","Seventh Below"),
         QT_TRANSLATE_NOOP("action","Enter seventh below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-8",
         QT_TRANSLATE_NOOP("action","Octave Below"),
         QT_TRANSLATE_NOOP("action","Enter octave below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "interval-9",
         QT_TRANSLATE_NOOP("action","Ninth Below"),
         QT_TRANSLATE_NOOP("action","Enter ninth below")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "note-a",
         QT_TRANSLATE_NOOP("action","A"),
         QT_TRANSLATE_NOOP("action","Enter note A")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "note-b",
         QT_TRANSLATE_NOOP("action","B"),
         QT_TRANSLATE_NOOP("action","Enter note B")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "note-c",
         QT_TRANSLATE_NOOP("action","C"),
         QT_TRANSLATE_NOOP("action","Enter note C")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "note-d",
         QT_TRANSLATE_NOOP("action","D"),
         QT_TRANSLATE_NOOP("action","Enter note D")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "note-e",
         QT_TRANSLATE_NOOP("action","E"),
         QT_TRANSLATE_NOOP("action","Enter note E")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "note-f",
         QT_TRANSLATE_NOOP("action","F"),
         QT_TRANSLATE_NOOP("action","Enter note F")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "note-g",
         QT_TRANSLATE_NOOP("action","G"),
         QT_TRANSLATE_NOOP("action","Enter note G")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "chord-a",
         QT_TRANSLATE_NOOP("action","Add A"),
         QT_TRANSLATE_NOOP("action","Add note A to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "chord-b",
         QT_TRANSLATE_NOOP("action","Add B"),
         QT_TRANSLATE_NOOP("action","Add note B to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "chord-c",
         QT_TRANSLATE_NOOP("action","Add C"),
         QT_TRANSLATE_NOOP("action","Add note C to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "chord-d",
         QT_TRANSLATE_NOOP("action","Add D"),
         QT_TRANSLATE_NOOP("action","Add note D to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "chord-e",
         QT_TRANSLATE_NOOP("action","Add E"),
         QT_TRANSLATE_NOOP("action","Add note E to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "chord-f",
         QT_TRANSLATE_NOOP("action","Add F"),
         QT_TRANSLATE_NOOP("action","Add note F to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "chord-g",
         QT_TRANSLATE_NOOP("action","Add G"),
         QT_TRANSLATE_NOOP("action","Add note G to chord")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "insert-a",
         QT_TRANSLATE_NOOP("action","Insert A"),
         QT_TRANSLATE_NOOP("action","Insert note A")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "insert-b",
         QT_TRANSLATE_NOOP("action","Insert B"),
         QT_TRANSLATE_NOOP("action","Insert note B")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "insert-c",
         QT_TRANSLATE_NOOP("action","Insert C"),
         QT_TRANSLATE_NOOP("action","Insert note C")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "insert-d",
         QT_TRANSLATE_NOOP("action","Insert D"),
         QT_TRANSLATE_NOOP("action","Insert note D")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "insert-e",
         QT_TRANSLATE_NOOP("action","Insert E"),
         QT_TRANSLATE_NOOP("action","Insert note E")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "insert-f",
         QT_TRANSLATE_NOOP("action","Insert F"),
         QT_TRANSLATE_NOOP("action","Insert note F")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "insert-g",
         QT_TRANSLATE_NOOP("action","Insert G"),
         QT_TRANSLATE_NOOP("action","Insert note G")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "rest",
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Enter rest"),
         0,
         Icons::quartrest_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-staccato",
         QT_TRANSLATE_NOOP("action","Staccato"),
         QT_TRANSLATE_NOOP("action","Add staccato"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-tenuto",
         QT_TRANSLATE_NOOP("action","Tenuto"),
         QT_TRANSLATE_NOOP("action","Add tenuto"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-trill",
         QT_TRANSLATE_NOOP("action","Trill"),
         QT_TRANSLATE_NOOP("action","Add trill"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-marcato",
         QT_TRANSLATE_NOOP("action","Marcato"),
         QT_TRANSLATE_NOOP("action","Add marcato"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "stretch+",
         QT_TRANSLATE_NOOP("action","Add More Stretch"),
         QT_TRANSLATE_NOOP("action","Add more stretch"),
         QT_TRANSLATE_NOOP("action","Add more stretch to selected measures"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "stretch-",
         QT_TRANSLATE_NOOP("action","Add Less Stretch"),
         QT_TRANSLATE_NOOP("action","Add less stretch"),
         QT_TRANSLATE_NOOP("action","Add less stretch to selected measures"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-beammode",
         QT_TRANSLATE_NOOP("action","Reset Beam Mode"),
         QT_TRANSLATE_NOOP("action","Reset beam mode"),
         QT_TRANSLATE_NOOP("action","Reset beam mode of selected measures"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "flip",
         QT_TRANSLATE_NOOP("action","Flip direction"),
         0,
         0,
         Icons::flip_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "pitch-up",
         QT_TRANSLATE_NOOP("action","Up"),
         QT_TRANSLATE_NOOP("action","Pitch up or move text or articulation up"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-up-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic up"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch up"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-up-octave",
         QT_TRANSLATE_NOOP("action","Up Octave"),
         QT_TRANSLATE_NOOP("action","Pitch up octave"),
         QT_TRANSLATE_NOOP("action","Pitch up by an octave or move text or articulation up"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "up-chord",
         QT_TRANSLATE_NOOP("action","Up Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to higher pitched note in chord"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "top-chord",
         QT_TRANSLATE_NOOP("action","Top Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to top note in chord"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "move-up",
         QT_TRANSLATE_NOOP("action","Move up"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "pitch-down",
         QT_TRANSLATE_NOOP("action","Down"),
         QT_TRANSLATE_NOOP("action","Pitch down or move text or articulation down"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-down-diatonic",
         QT_TRANSLATE_NOOP("action","Diatonic down"),
         QT_TRANSLATE_NOOP("action","Diatonic pitch down"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pitch-down-octave",
         QT_TRANSLATE_NOOP("action","Down octave"),
         QT_TRANSLATE_NOOP("action","Pitch down octave"),
         QT_TRANSLATE_NOOP("action","Pitch down by an octave or move text or articulation down"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "down-chord",
         QT_TRANSLATE_NOOP("action","Down Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to lower pitched note in chord"),
         QT_TRANSLATE_NOOP("action","Go to lower pitched note in chord"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "next-element",
         QT_TRANSLATE_NOOP("action","Next element"),
         QT_TRANSLATE_NOOP("action","Accessibility: next element"),
         QT_TRANSLATE_NOOP("action","Accessibility: next element"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "prev-element",
         QT_TRANSLATE_NOOP("action","Previous element"),
         QT_TRANSLATE_NOOP("action","Accessibility: previous element"),
         QT_TRANSLATE_NOOP("action","Accessibility: previous element"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "first-element",
         QT_TRANSLATE_NOOP("action","First element"),
         QT_TRANSLATE_NOOP("action","Go to the first element"),
         QT_TRANSLATE_NOOP("action","Go to the first element"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "last-element",
         QT_TRANSLATE_NOOP("action","Last element"),
         QT_TRANSLATE_NOOP("action","Go to the last element"),
         QT_TRANSLATE_NOOP("action","Go to the last element"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "bottom-chord",
         QT_TRANSLATE_NOOP("action","Bottom Note in Chord"),
         QT_TRANSLATE_NOOP("action","Go to bottom note in chord"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "move-down",
         QT_TRANSLATE_NOOP("action","Move down"),
         QT_TRANSLATE_NOOP("action","Move down"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-chord",
         QT_TRANSLATE_NOOP("action","Previous chord"),
         QT_TRANSLATE_NOOP("action","Go to previous chord or move text left")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-measure",
         QT_TRANSLATE_NOOP("action","Previous measure"),
         QT_TRANSLATE_NOOP("action","Go to previous measure or move text left")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "prev-track",
         QT_TRANSLATE_NOOP("action","Previous staff or voice")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-chord",
         QT_TRANSLATE_NOOP("action","Next chord"),
         QT_TRANSLATE_NOOP("action","Go to next chord or move text right")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-measure",
         QT_TRANSLATE_NOOP("action","Next measure"),
         QT_TRANSLATE_NOOP("action","Go to next measure or move text right")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-track",
         QT_TRANSLATE_NOOP("action","Next staff or voice")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-prev-chord",
         QT_TRANSLATE_NOOP("action","Add previous chord to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-prev-measure",
         QT_TRANSLATE_NOOP("action","Select to beginning of measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-next-chord",
         QT_TRANSLATE_NOOP("action","Add next chord to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-section",
         QT_TRANSLATE_NOOP("action","Select Section")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "move-right",
         QT_TRANSLATE_NOOP("action","Move chord/rest right")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "move-left",
         QT_TRANSLATE_NOOP("action","Move chord/rest left")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-next-measure",
         QT_TRANSLATE_NOOP("action","Select to end of measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-begin-line",
         QT_TRANSLATE_NOOP("action","Select to beginning of line")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-end-line",
         QT_TRANSLATE_NOOP("action","Select to end of line")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-begin-score",
         QT_TRANSLATE_NOOP("action","Select to beginning of score")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "select-end-score",
         QT_TRANSLATE_NOOP("action","Select to end of score")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-staff-above",
         QT_TRANSLATE_NOOP("action","Add staff above to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-staff-below",
         QT_TRANSLATE_NOOP("action","Add staff below to selection")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-prev",
         QT_TRANSLATE_NOOP("action","Page: previous")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-next",
         QT_TRANSLATE_NOOP("action","Page: next")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-top",
         QT_TRANSLATE_NOOP("action","Page: top")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-end",
         QT_TRANSLATE_NOOP("action","Page: end")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-slur",
         QT_TRANSLATE_NOOP("action","Slur"),
         QT_TRANSLATE_NOOP("action","Add slur")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-hairpin",
         QT_TRANSLATE_NOOP("action","Crescendo"),
         QT_TRANSLATE_NOOP("action","Add crescendo"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-hairpin-reverse",
         QT_TRANSLATE_NOOP("action","Decrescendo"),
         QT_TRANSLATE_NOOP("action","Add decrescendo"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-8va",
         QT_TRANSLATE_NOOP("action","Ottava 8va"),
         QT_TRANSLATE_NOOP("action","Add ottava 8va"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-8vb",
         QT_TRANSLATE_NOOP("action","Ottava 8vb"),
         QT_TRANSLATE_NOOP("action","Add ottava 8vb"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_TEXT_EDIT | STATE_LYRICS_EDIT
            | STATE_HARMONY_FIGBASS_EDIT | STATE_PLAY | STATE_FOTO,
         "escape",
         QT_TRANSLATE_NOOP("action","Escape")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "delete",
         QT_TRANSLATE_NOOP("action","Delete"),
         QT_TRANSLATE_NOOP("action","Delete"),
         QT_TRANSLATE_NOOP("action","Delete contents of the selected measures"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
    ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "full-measure-rest",
         QT_TRANSLATE_NOOP("action","Full Measure Rest"),
         QT_TRANSLATE_NOOP("action","Full Measure Rest"),
         QT_TRANSLATE_NOOP("action","Converts the measure to a full measure rest"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "time-delete",
         QT_TRANSLATE_NOOP("action","Timewise delete"),
         QT_TRANSLATE_NOOP("action","Timewise Delete"),
         QT_TRANSLATE_NOOP("action","Delete element and duration"),
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "delete-measures",
         QT_TRANSLATE_NOOP("action","Delete Selected Measures"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-measure",
         QT_TRANSLATE_NOOP("action","Append One Measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-measures",
         QT_TRANSLATE_NOOP("action","Append Measures..."),
         QT_TRANSLATE_NOOP("action","Append measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-measure",
         QT_TRANSLATE_NOOP("action","Insert One Measure"),
         QT_TRANSLATE_NOOP("action","Insert One Measure"),
         QT_TRANSLATE_NOOP("action","Insert one measure"),
         Icons::measure_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-measures",
         QT_TRANSLATE_NOOP("action","Insert Measures..."),
         QT_TRANSLATE_NOOP("action","Insert measures")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-hbox",
         QT_TRANSLATE_NOOP("action","Insert Horizontal Frame"),
         QT_TRANSLATE_NOOP("action","Insert Horizontal Frame"),
         QT_TRANSLATE_NOOP("action","Insert horizontal frame"),
         Icons::hframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-textframe",
         QT_TRANSLATE_NOOP("action","Insert Text Frame"),
         QT_TRANSLATE_NOOP("action","Insert Text Frame"),
         QT_TRANSLATE_NOOP("action","Insert text frame"),
         Icons::tframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-textframe",
         QT_TRANSLATE_NOOP("action","Append Text Frame")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-fretframe",
         QT_TRANSLATE_NOOP("action","Insert Fretboard Diagram Frame"),
         0,
         0,
         Icons::fframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "insert-vbox",
         QT_TRANSLATE_NOOP("action","Insert Vertical Frame"),
         QT_TRANSLATE_NOOP("action","Insert Vertical Frame"),
         QT_TRANSLATE_NOOP("action","Insert vertical frame"),
         Icons::vframe_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-hbox",
         QT_TRANSLATE_NOOP("action","Append Horizontal Frame")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "append-vbox",
         QT_TRANSLATE_NOOP("action","Append Vertical Frame")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "duplet",
         QT_TRANSLATE_NOOP("action","Duplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "triplet",
         QT_TRANSLATE_NOOP("action","Triplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "quadruplet",
         QT_TRANSLATE_NOOP("action","Quadruplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "quintuplet",
         QT_TRANSLATE_NOOP("action","Quintuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "sextuplet",
         QT_TRANSLATE_NOOP("action","Sextuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "septuplet",
         QT_TRANSLATE_NOOP("action","Septuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "octuplet",
         QT_TRANSLATE_NOOP("action","Octuplet")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "nonuplet",
         QT_TRANSLATE_NOOP("action","Nonuplet")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tuplet-dialog",
         QT_TRANSLATE_NOOP("action","Other..."),
         QT_TRANSLATE_NOOP("action","Other tuplets")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "note-longa",
         QT_TRANSLATE_NOOP("action","Longa"),
         QT_TRANSLATE_NOOP("action","Note duration: longa"),
         QT_TRANSLATE_NOOP("action","Longa"),
         Icons::longaUp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "note-breve",
         QT_TRANSLATE_NOOP("action","Double whole note"),
         QT_TRANSLATE_NOOP("action","Note duration: double whole"),
         QT_TRANSLATE_NOOP("action","Double whole note"),
         Icons::brevis_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-1",
         QT_TRANSLATE_NOOP("action","Whole note"),
         QT_TRANSLATE_NOOP("action","Note duration: whole"),
         QT_TRANSLATE_NOOP("action","Whole note"),
         Icons::note_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-2",
         QT_TRANSLATE_NOOP("action","Half note"),
         QT_TRANSLATE_NOOP("action","Note duration: half"),
         QT_TRANSLATE_NOOP("action","Half note"),
         Icons::note2_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-4",
         QT_TRANSLATE_NOOP("action","Quarter note"),
         QT_TRANSLATE_NOOP("action","Note duration: quarter"),
         QT_TRANSLATE_NOOP("action","Quarter note"),
         Icons::note4_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-8",
         QT_TRANSLATE_NOOP("action","Eighth note"),
         QT_TRANSLATE_NOOP("action","Note duration: eighth"),
         QT_TRANSLATE_NOOP("action","Eighth note"),
         Icons::note8_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-16",
         QT_TRANSLATE_NOOP("action","16th note"),
         QT_TRANSLATE_NOOP("action","Note duration: 16th"),
         QT_TRANSLATE_NOOP("action","16th note"),
         Icons::note16_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-32",
         QT_TRANSLATE_NOOP("action","32nd note"),
         QT_TRANSLATE_NOOP("action","Note duration: 32nd"),
         QT_TRANSLATE_NOOP("action","32nd note"),
         Icons::note32_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-64",
         QT_TRANSLATE_NOOP("action","64th note"),
         QT_TRANSLATE_NOOP("action","Note duration: 64th"),
         QT_TRANSLATE_NOOP("action","64th note"),
         Icons::note64_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM | STATE_NOTE_ENTRY_TAB,
         "pad-note-128",
         QT_TRANSLATE_NOOP("action","128th note"),
         QT_TRANSLATE_NOOP("action","Note duration: 128th"),
         QT_TRANSLATE_NOOP("action","128th note"),
         Icons::note128_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "pad-note-increase",
         QT_TRANSLATE_NOOP("action","Increase active duration"),
         QT_TRANSLATE_NOOP("action","Increase active duration")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY,
         "pad-note-decrease",
         QT_TRANSLATE_NOOP("action","Decrease active duration"),
         QT_TRANSLATE_NOOP("action","Decrease active duration")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dot",
         QT_TRANSLATE_NOOP("action","Augmentation dot"),
         QT_TRANSLATE_NOOP("action","Note duration: augmentation dot"),
         QT_TRANSLATE_NOOP("action","Augmentation dot"),
         Icons::dot_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "pad-dotdot",
         QT_TRANSLATE_NOOP("action","Double augmentation dot"),
         QT_TRANSLATE_NOOP("action","Note duration: double augmentation dot"),
         QT_TRANSLATE_NOOP("action","Double augmentation dot"),
         Icons::dotdot_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tie",
         QT_TRANSLATE_NOOP("action","Tie"),
         QT_TRANSLATE_NOOP("action","Note duration: tie"),
         QT_TRANSLATE_NOOP("action","Tie"),
         Icons::tie_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "pad-rest",
         QT_TRANSLATE_NOOP("action","Rest"),
         QT_TRANSLATE_NOOP("action","Note entry: rest"),
         QT_TRANSLATE_NOOP("action","Rest"),
         Icons::quartrest_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "sharp2",
         QT_TRANSLATE_NOOP("action","Double sharp"),
         QT_TRANSLATE_NOOP("action","Note entry: double sharp"),
         QT_TRANSLATE_NOOP("action","Double sharp"),
         Icons::sharpsharp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "sharp",
         QT_TRANSLATE_NOOP("action","Sharp"),
         QT_TRANSLATE_NOOP("action","Note entry: sharp"),
         QT_TRANSLATE_NOOP("action","Sharp"),
         Icons::sharp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "nat",
         QT_TRANSLATE_NOOP("action","Natural"),
         QT_TRANSLATE_NOOP("action","Note entry: natural"),
         QT_TRANSLATE_NOOP("action","Natural"),
         Icons::natural_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
       Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "flat",
         QT_TRANSLATE_NOOP("action","Flat"),
         QT_TRANSLATE_NOOP("action","Note entry: flat"),
         QT_TRANSLATE_NOOP("action","Flat"),
         Icons::flat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "flat2",
         QT_TRANSLATE_NOOP("action","Double flat"),
         QT_TRANSLATE_NOOP("action","Note entry: double flat"),
         QT_TRANSLATE_NOOP("action","Double flat"),
         Icons::flatflat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "acciaccatura",
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         QT_TRANSLATE_NOOP("action","Add acciaccatura"),
         QT_TRANSLATE_NOOP("action","Acciaccatura"),
         Icons::acciaccatura_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "appoggiatura",
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         QT_TRANSLATE_NOOP("action","Add appoggiatura"),
         QT_TRANSLATE_NOOP("action","Appoggiatura"),
         Icons::appoggiatura_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
        MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
       /* no stroke: 4th*/
        "grace4",
        QT_TRANSLATE_NOOP("action","Grace: quarter"),
        QT_TRANSLATE_NOOP("action","Add quarter grace note"),
        QT_TRANSLATE_NOOP("action","Grace: quarter"),
        Icons::grace4_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
        ),
     Shortcut(
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
        ),
     Shortcut(
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
        ),
        Shortcut(
          MsWidget::SCORE_TAB,
          STATE_NORMAL | STATE_NOTE_ENTRY,
        /* no stroke: Eighth*/
         "grace8after",
         QT_TRANSLATE_NOOP("action","Grace: eighth after"),
         QT_TRANSLATE_NOOP("action","Add Eighth grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: eighth after"),
         Icons::grace8after_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
       /* no stroke: 16th*/
         "grace16after",
         QT_TRANSLATE_NOOP("action","Grace: 16th after"),
         QT_TRANSLATE_NOOP("action","Add 16th grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: 16th after"),
         Icons::grace16after_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
       /* no stroke: 32nd*/
         "grace32after",
         QT_TRANSLATE_NOOP("action","Grace: 32nd after"),
         QT_TRANSLATE_NOOP("action","Add 32nd grace note after"),
         QT_TRANSLATE_NOOP("action","Grace: 32nd after"),
         Icons::grace32after_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-1",
         QT_TRANSLATE_NOOP("action","1"),
         QT_TRANSLATE_NOOP("action","Voice 1"),
         QT_TRANSLATE_NOOP("action","Voice 1")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-2",
         QT_TRANSLATE_NOOP("action","2"),
         QT_TRANSLATE_NOOP("action","Voice 2"),
         QT_TRANSLATE_NOOP("action","Voice 2")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-3",
         QT_TRANSLATE_NOOP("action","3"),
         QT_TRANSLATE_NOOP("action","Voice 3"),
         QT_TRANSLATE_NOOP("action","Voice 3")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "voice-4",
         QT_TRANSLATE_NOOP("action","4"),
         QT_TRANSLATE_NOOP("action","Voice 4"),
         QT_TRANSLATE_NOOP("action","Voice 4")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "midi-on",
         QT_TRANSLATE_NOOP("action","MIDI input"),
         QT_TRANSLATE_NOOP("action","Enable MIDI input"),
         QT_TRANSLATE_NOOP("action","Enable MIDI input"),
         Icons::midiin_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam-start",
         QT_TRANSLATE_NOOP("action","Beam start"),
         0,
         0,
         Icons::sbeam_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam-mid",
         QT_TRANSLATE_NOOP("action","Beam middle"),
         0,
         0,
         Icons::mbeam_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "no-beam",
         QT_TRANSLATE_NOOP("action","No beam"),
         0,
         0,
         Icons::nbeam_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam32",
         QT_TRANSLATE_NOOP("action","Beam 16th sub"),
         0,
         0,
         Icons::beam32_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "beam64",
         QT_TRANSLATE_NOOP("action","Beam 32nd sub"),
         0,
         0,
         Icons::beam64_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "auto-beam",
         QT_TRANSLATE_NOOP("action","Auto beam"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "fbeam1",
         QT_TRANSLATE_NOOP("action","Feathered beam, slower"),
         0,
         0,
         Icons::fbeam1_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "fbeam2",
         QT_TRANSLATE_NOOP("action","Feathered beam, faster"),
         0,
         0,
         Icons::fbeam2_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "toggle-palette",
         QT_TRANSLATE_NOOP("action","Palette"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "toggle-playpanel",
         QT_TRANSLATE_NOOP("action","Play Panel"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "toggle-selection-window",
         QT_TRANSLATE_NOOP("action","Selection Filter"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "toggle-navigator",
         QT_TRANSLATE_NOOP("action","Navigator"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "toggle-midiimportpanel",
         QT_TRANSLATE_NOOP("action","MIDI Import Panel"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
#ifdef Q_OS_MAC
         //Avoid conflict with M in text
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
#else
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
#endif
         "toggle-mixer",
         QT_TRANSLATE_NOOP("action","Mixer"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "toggle-transport",
         QT_TRANSLATE_NOOP("action","Transport"),
         QT_TRANSLATE_NOOP("action","Transport toolbar")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "toggle-noteinput",
         QT_TRANSLATE_NOOP("action","Note Input"),
         QT_TRANSLATE_NOOP("action","Note input toolbar")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "toggle-statusbar",
         QT_TRANSLATE_NOOP("action","Status Bar")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         "quit",
         QT_TRANSLATE_NOOP("action","Quit")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "mag",
         QT_TRANSLATE_NOOP("action","Zoom canvas")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "lyrics",
         QT_TRANSLATE_NOOP("action","Lyrics"),
         QT_TRANSLATE_NOOP("action","Add lyrics"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "tempo",
         QT_TRANSLATE_NOOP("action","Tempo Marking..."),
         QT_TRANSLATE_NOOP("action","Add tempo marking"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "system-text",
         QT_TRANSLATE_NOOP("action","System Text"),
         QT_TRANSLATE_NOOP("action","Add system text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "staff-text",
         QT_TRANSLATE_NOOP("action","Staff Text"),
         QT_TRANSLATE_NOOP("action","Add staff text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "frame-text",
         QT_TRANSLATE_NOOP("action","Text"),
         QT_TRANSLATE_NOOP("action","Add frame text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "title-text",
         QT_TRANSLATE_NOOP("action","Title"),
         QT_TRANSLATE_NOOP("action","Add title text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "subtitle-text",
         QT_TRANSLATE_NOOP("action","Subtitle"),
         QT_TRANSLATE_NOOP("action","Add subtitle text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "composer-text",
         QT_TRANSLATE_NOOP("action","Composer"),
         QT_TRANSLATE_NOOP("action","Add composer text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "poet-text",
         QT_TRANSLATE_NOOP("action","Lyricist"),
         QT_TRANSLATE_NOOP("action","Add lyricist text")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "part-text",
         QT_TRANSLATE_NOOP("action","Part Name"),
         QT_TRANSLATE_NOOP("action","Add part name")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "chord-text",
         QT_TRANSLATE_NOOP("action","Chord Symbol"),
         QT_TRANSLATE_NOOP("action","Add chord symbol")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rehearsalmark-text",
         QT_TRANSLATE_NOOP("action","Rehearsal Mark"),
         QT_TRANSLATE_NOOP("action","Add rehearsal mark")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "picture",
         QT_TRANSLATE_NOOP("action","Picture"),
         QT_TRANSLATE_NOOP("action","Add picture")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "play",
         QT_TRANSLATE_NOOP("action","Play"),
         QT_TRANSLATE_NOOP("action","Player play"),
         QT_TRANSLATE_NOOP("action","Start or stop playback"),
         Icons::play_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-prev-chord",
         QT_TRANSLATE_NOOP("action","Play Previous Chord"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-prev-measure",
         QT_TRANSLATE_NOOP("action","Play Previous Measure"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-next-chord",
         QT_TRANSLATE_NOOP("action","Play Next Chord"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "play-next-measure",
         QT_TRANSLATE_NOOP("action","Play Next Measure"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "seek-begin",
         QT_TRANSLATE_NOOP("action","Player Seek to Begin"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "rewind",
         QT_TRANSLATE_NOOP("action","Rewind"),
         QT_TRANSLATE_NOOP("action","Player rewind"),
         QT_TRANSLATE_NOOP("action","Rewind to start position"),
         Icons::start_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_PLAY,
         "seek-end",
         QT_TRANSLATE_NOOP("action","Player Seek to End")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "repeat",
         QT_TRANSLATE_NOOP("action","Play repeats"),
         QT_TRANSLATE_NOOP("action","Toggle repeats playback"),
         QT_TRANSLATE_NOOP("action","Play repeats"),
         Icons::repeat_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE | ShortcutFlags::A_CHECKED
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "pan",
         QT_TRANSLATE_NOOP("action","Pan"),
         QT_TRANSLATE_NOOP("action","Toggle pan score"),
         QT_TRANSLATE_NOOP("action","Pan score during playback"),
         Icons::pan_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE | ShortcutFlags::A_CHECKED
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "load-style",
         QT_TRANSLATE_NOOP("action","Load Style..."),
         QT_TRANSLATE_NOOP("action","Load style"),
         0,
         Icons::fileOpen_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "save-style",
         QT_TRANSLATE_NOOP("action","Save Style..."),
         QT_TRANSLATE_NOOP("action","Save style"),
         0,
         Icons::fileSave_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "select-all",
         QT_TRANSLATE_NOOP("action","Select All"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "transpose",
         QT_TRANSLATE_NOOP("action","&Transpose..."),
         QT_TRANSLATE_NOOP("action","Transpose"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "clef-violin",
         QT_TRANSLATE_NOOP("action","Treble Clef"),
         QT_TRANSLATE_NOOP("action","Add treble clef"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "clef-bass",
         QT_TRANSLATE_NOOP("action","Bass Clef"),
         QT_TRANSLATE_NOOP("action","Add bass clef"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x12",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-2"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x13",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-3"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x14",
         QT_TRANSLATE_NOOP("action","Exchange Voice 1-4"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x23",
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-3"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x24",
         QT_TRANSLATE_NOOP("action","Exchange Voice 2-4"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "voice-x34",
         QT_TRANSLATE_NOOP("action","Exchange Voice 3-4"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut (
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "concert-pitch",
         QT_TRANSLATE_NOOP("action","Concert Pitch"),
         QT_TRANSLATE_NOOP("action","Display in concert pitch"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "repeat-cmd",
         QT_TRANSLATE_NOOP("action","Repeat last command"),
         0,
         0,
         Icons::fileOpen_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-info",
         QT_TRANSLATE_NOOP("action","Info..."),
         QT_TRANSLATE_NOOP("action","Edit score info"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "system-break",
         QT_TRANSLATE_NOOP("action","Toggle System Break"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "page-break",
         QT_TRANSLATE_NOOP("action","Toggle Page Break"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "section-break",
         QT_TRANSLATE_NOOP("action","Toggle Section Break"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "edit-element",
         QT_TRANSLATE_NOOP("action","Edit Element")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_EDIT,
         "reset",
         QT_TRANSLATE_NOOP("action","Reset"),
         QT_TRANSLATE_NOOP("action","Reset user settings")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "debugger",
         QT_TRANSLATE_NOOP("action","Debugger")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "reset-stretch",
         QT_TRANSLATE_NOOP("action","Reset Stretch"),
         QT_TRANSLATE_NOOP("action","Reset measure stretch"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-invisible",
         QT_TRANSLATE_NOOP("action","Show Invisible"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-unprintable",
         QT_TRANSLATE_NOOP("action","Show Unprintable"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-frames",
         QT_TRANSLATE_NOOP("action","Show Frames"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "show-pageborders",
         QT_TRANSLATE_NOOP("action","Show Page Margins"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_SCORE | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_TEXT_EDIT | STATE_LYRICS_EDIT | STATE_HARMONY_FIGBASS_EDIT,
         "show-keys",
         QT_TRANSLATE_NOOP("action","Insert Special Characters..."),
         QT_TRANSLATE_NOOP("action","Insert Special Characters"),
         0,
         Icons::keys_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-1",
         QT_TRANSLATE_NOOP("action","Whole rest"),
         QT_TRANSLATE_NOOP("action","Note entry: whole rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-2",
         QT_TRANSLATE_NOOP("action","Half rest"),
         QT_TRANSLATE_NOOP("action","Note entry: half rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-4",
         QT_TRANSLATE_NOOP("action","Quarter rest"),
         QT_TRANSLATE_NOOP("action","Note entry: quarter rest")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "rest-8",
         QT_TRANSLATE_NOOP("action","Eighth rest"),
         QT_TRANSLATE_NOOP("action","Note entry: eighth rest")
         ),
      Shortcut(                     // mapped to undo in note entry mode
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "backspace",
         QT_TRANSLATE_NOOP("action","Backspace"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "find",
         QT_TRANSLATE_NOOP("action","Find")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY,
         "zoomin",
         QT_TRANSLATE_NOOP("action","Zoom In")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         // conflicts with Ctrl+- in edit mode to enter lyrics hyphen
         // STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,

         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "zoomout",
         QT_TRANSLATE_NOOP("action","Zoom Out")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "mirror-note",
         QT_TRANSLATE_NOOP("action","Mirror note head"),
         0,
         0,
         Icons::flip_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-style",
         QT_TRANSLATE_NOOP("action","General..."),
         QT_TRANSLATE_NOOP("action","Edit general style"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-text-style",
         QT_TRANSLATE_NOOP("action","Text..."),
         QT_TRANSLATE_NOOP("action","Edit text style"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "edit-harmony",
         QT_TRANSLATE_NOOP("action","Chord Symbols..."),
         QT_TRANSLATE_NOOP("action","Edit chord symbols style")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-similar",
         QT_TRANSLATE_NOOP("action","All Similar Elements"),
         QT_TRANSLATE_NOOP("action","Select all similar elements")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-similar-staff",
         QT_TRANSLATE_NOOP("action","All Similar Elements in Same Staff"),
         QT_TRANSLATE_NOOP("action","Select all similar elements in same staff")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "select-similar-range",
         QT_TRANSLATE_NOOP("action","All Similar Elements in Range Selection"),
         QT_TRANSLATE_NOOP("action","Select all similar elements in the range selection")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "synth-control",
         QT_TRANSLATE_NOOP("action","Synthesizer"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         "double-duration",
         QT_TRANSLATE_NOOP("action","Double duration"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM /*| STATE_NOTE_ENTRY_TAB*/,
         "half-duration",
         QT_TRANSLATE_NOOP("action","Half duration"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "repeat-sel",
         QT_TRANSLATE_NOOP("action","Repeat selection"),
         0,
         0,
         Icons::fileOpen_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "follow",
         QT_TRANSLATE_NOOP("action","Pan piano roll"),
         QT_TRANSLATE_NOOP("action","Toggle pan piano roll"),
         QT_TRANSLATE_NOOP("action","Pan roll during playback"),
         Icons::pan_ICON
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "split-h",
         QT_TRANSLATE_NOOP("action","Documents Side by Side"),
         QT_TRANSLATE_NOOP("action","Display documents side by side")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "split-v",
         QT_TRANSLATE_NOOP("action","Documents Stacked"),
         QT_TRANSLATE_NOOP("action","Display documents stacked")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "parts",
         QT_TRANSLATE_NOOP("action","Parts..."),
         QT_TRANSLATE_NOOP("action","Manage parts")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "enh-up",
         QT_TRANSLATE_NOOP("action","Enharmonic up")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY_PITCHED | STATE_NOTE_ENTRY_DRUM,
         "enh-down",
         QT_TRANSLATE_NOOP("action","Enharmonic down")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "revision",
         QT_TRANSLATE_NOOP("action","Create new revision")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_FOTO,
         "fotomode",
         QT_TRANSLATE_NOOP("action","Toggle screenshot mode"),
         0,
         0,
         Icons::fotomode_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
#ifdef OMR
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "show-omr",
         QT_TRANSLATE_NOOP("action","Show OMR image")
         ),
#endif
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL & (~STATE_TEXT_EDIT),
         "fullscreen",
         QT_TRANSLATE_NOOP("action","Full Screen")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "hraster",
         QT_TRANSLATE_NOOP("action","Enable snap to horizontal grid"),
         0,
         0,
         Icons::hraster_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "vraster",
         QT_TRANSLATE_NOOP("action","Enable snap to vertical grid"),
         0,
         0,
         Icons::vraster_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT,
         "config-raster",
         QT_TRANSLATE_NOOP("action","Configure grid")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NOTE_ENTRY,
         "repitch",
         QT_TRANSLATE_NOOP("action","Re-Pitch Mode"),
         QT_TRANSLATE_NOOP("action","Replace pitches without changing rhythms"),
         0,
         Icons::repitch_ICON,
         Qt::ApplicationShortcut,
         ShortcutFlags::A_CMD | ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "toogle-piano",
         QT_TRANSLATE_NOOP("action","Piano Keyboard")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         "media",
         QT_TRANSLATE_NOOP("action","Additional Media..."),
         QT_TRANSLATE_NOOP("action","Show media dialog")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "split-measure",
         QT_TRANSLATE_NOOP("action","Split Measure")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "join-measure",
         QT_TRANSLATE_NOOP("action","Join Measures")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         "page-settings",
         QT_TRANSLATE_NOOP("action","Page Settings..."),
         QT_TRANSLATE_NOOP("action","Page Settings")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL,
         "album",
         QT_TRANSLATE_NOOP("action","Album..."),
         QT_TRANSLATE_NOOP("action","Album"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "layer",
         QT_TRANSLATE_NOOP("action","Layers..."),
         QT_TRANSLATE_NOOP("action","Layers"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "next-score",
         QT_TRANSLATE_NOOP("action","Next Score"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "previous-score",
         QT_TRANSLATE_NOOP("action","Previous Score"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_INIT | STATE_DISABLED | STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_PLAY | STATE_FOTO,
         "musescore-connect",
         QT_TRANSLATE_NOOP("action", "MuseScore Connect"),
         0,
         0,
         Icons::community_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "plugin-creator",
         QT_TRANSLATE_NOOP("action", "Plugin Creator"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "plugin-manager",
         QT_TRANSLATE_NOOP("action", "Plugin Manager"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT | STATE_FOTO,
         "inspector",
         QT_TRANSLATE_NOOP("action","Inspector"),
         QT_TRANSLATE_NOOP("action","Show inspector")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "resource-manager",
         QT_TRANSLATE_NOOP("action", "Resource Manager"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
#ifdef OMR
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT,
         "omr",
         QT_TRANSLATE_NOOP("action","OMR Panel"),
         QT_TRANSLATE_NOOP("action","Show OMR Panel")
         ),
#endif
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "loop",
         QT_TRANSLATE_NOOP("action","Loop"),
         QT_TRANSLATE_NOOP("action","Toggle loop playback"),
         QT_TRANSLATE_NOOP("action","Loop playback"),
         Icons::loop_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "loop-in",
         QT_TRANSLATE_NOOP("action","Loop in"),
         QT_TRANSLATE_NOOP("action","Set loop In position"),
         QT_TRANSLATE_NOOP("action","Set loop In position"),
         Icons::loopIn_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY,
         "loop-out",
         QT_TRANSLATE_NOOP("action","Loop out"),
         QT_TRANSLATE_NOOP("action","Set loop Out position"),
         QT_TRANSLATE_NOOP("action","Set loop Out position"),
         Icons::loopOut_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT,
         "metronome",
         QT_TRANSLATE_NOOP("action","Metronome"),
         QT_TRANSLATE_NOOP("action","Toggle metronome playback"),
         QT_TRANSLATE_NOOP("action","Play metronome during playback"),
         Icons::metronome_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY | STATE_PLAY | STATE_EDIT | STATE_LYRICS_EDIT,
         "countin",
         QT_TRANSLATE_NOOP("action","Count-in"),
         QT_TRANSLATE_NOOP("action","Toggle count-in playback"),
         QT_TRANSLATE_NOOP("action","Play count-in at playback start"),
         Icons::countin_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CHECKABLE
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "figured-bass",
         QT_TRANSLATE_NOOP("action","Figured Bass"),
         QT_TRANSLATE_NOOP("action","Add figured bass"),
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "transpose-up",
         QT_TRANSLATE_NOOP("action","Transpose Up")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "transpose-down",
         QT_TRANSLATE_NOOP("action","Transpose Down")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "masterpalette",
         QT_TRANSLATE_NOOP("action","Master Palette..."),
         QT_TRANSLATE_NOOP("action","Show master palette"),
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "key-signatures",
         QT_TRANSLATE_NOOP("action","Key Signatures..."),
         QT_TRANSLATE_NOOP("action","Show key signature palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "time-signatures",
         QT_TRANSLATE_NOOP("action","Time Signatures..."),
         QT_TRANSLATE_NOOP("action","Show time signature palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "symbols",
         QT_TRANSLATE_NOOP("action","Symbols..."),
         QT_TRANSLATE_NOOP("action","Show symbol palette")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL,
         "viewmode",
         QT_TRANSLATE_NOOP("action","Toggle View Mode")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_LYRICS_EDIT,
         "next-lyric",
         QT_TRANSLATE_NOOP("action","Next syllable")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_LYRICS_EDIT,
         "prev-lyric",
         QT_TRANSLATE_NOOP("action","Previous syllable")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "toggle-visible",
         QT_TRANSLATE_NOOP("action","Toggle visibility")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "set-visible",
         QT_TRANSLATE_NOOP("action","Set visible")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "unset-visible",
         QT_TRANSLATE_NOOP("action","Set invisible")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-noteline",
         QT_TRANSLATE_NOOP("action","Note anchored Textline")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_NORMAL | STATE_LOCK,
         "lock",
         QT_TRANSLATE_NOOP("action","Lock Score")
         ),

      // TAB-specific actions
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,                     // use a STATE value which is never used: shortcut is never active
         "note-longa-TAB",
         QT_TRANSLATE_NOOP("action","Longa (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: longa (TAB)"),
         0,
         Icons::longaUp_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "note-breve-TAB",
         QT_TRANSLATE_NOOP("action","Double whole note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: double whole (TAB)"),
         QT_TRANSLATE_NOOP("action","Double whole note"),
         Icons::brevis_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-1-TAB",
         QT_TRANSLATE_NOOP("action","Whole note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: whole (TAB)"),
         QT_TRANSLATE_NOOP("action","Whole note"),
         Icons::note_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-2-TAB",
         QT_TRANSLATE_NOOP("action","Half note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: half (TAB)"),
         QT_TRANSLATE_NOOP("action","Half note"),
         Icons::note2_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-4-TAB",
         QT_TRANSLATE_NOOP("action","Quarter note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: quarter (TAB)"),
         QT_TRANSLATE_NOOP("action","Quarter note"),
         Icons::note4_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-8-TAB",
         QT_TRANSLATE_NOOP("action","Eighth note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: eighth (TAB)"),
         QT_TRANSLATE_NOOP("action","Eighth note"),
         Icons::note8_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-16-TAB",
         QT_TRANSLATE_NOOP("action","16th note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 16th (TAB)"),
         QT_TRANSLATE_NOOP("action","16th note"),
         Icons::note16_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-32-TAB",
         QT_TRANSLATE_NOOP("action","32nd note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 32nd (TAB)"),
         QT_TRANSLATE_NOOP("action","32nd note"),
         Icons::note32_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-64-TAB",
         QT_TRANSLATE_NOOP("action","64th note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 64th (TAB)"),
         QT_TRANSLATE_NOOP("action","64th note"),
         Icons::note64_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NEVER,
         "pad-note-128-TAB",
         QT_TRANSLATE_NOOP("action","128th note (TAB)"),
         QT_TRANSLATE_NOOP("action","Note duration: 128th (TAB)"),
         QT_TRANSLATE_NOOP("action","128th note"),
         Icons::note128_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "pad-note-increase-TAB",
         QT_TRANSLATE_NOOP("action","Increase active duration (TAB)"),
         QT_TRANSLATE_NOOP("action","Increase active duration (TAB)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "pad-note-decrease-TAB",
         QT_TRANSLATE_NOOP("action","Decrease active duration (TAB)"),
         QT_TRANSLATE_NOOP("action","Decrease active duration (TAB)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "rest-TAB",
         QT_TRANSLATE_NOOP("action","Rest (TAB)"),
         QT_TRANSLATE_NOOP("action","Enter rest (TAB)"),
         0,
         Icons::quartrest_ICON
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "pad-rest-TAB",
         QT_TRANSLATE_NOOP("action","Rest (TAB)"),
         QT_TRANSLATE_NOOP("action","Note entry: rest (TAB)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "string-above",
         QT_TRANSLATE_NOOP("action","String above (TAB)"),
         QT_TRANSLATE_NOOP("action","Select string above (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "string-below",
         QT_TRANSLATE_NOOP("action","String below (TAB)"),
         QT_TRANSLATE_NOOP("action","Select string below (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-0",
         QT_TRANSLATE_NOOP("action","Fret 0 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 0 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-1",
         QT_TRANSLATE_NOOP("action","Fret 1 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 1 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-2",
         QT_TRANSLATE_NOOP("action","Fret 2 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 2 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-3",
         QT_TRANSLATE_NOOP("action","Fret 3 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 3 on current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-4",
         QT_TRANSLATE_NOOP("action","Fret 4 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 4 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-5",
         QT_TRANSLATE_NOOP("action","Fret 5 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 5 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-6",
         QT_TRANSLATE_NOOP("action","Fret 6 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 6 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-7",
         QT_TRANSLATE_NOOP("action","Fret 7 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 7 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-8",
         QT_TRANSLATE_NOOP("action","Fret 8 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 8 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-9",
         QT_TRANSLATE_NOOP("action","Fret 9 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 9 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-10",
         QT_TRANSLATE_NOOP("action","Fret 10 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 10 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-11",
         QT_TRANSLATE_NOOP("action","Fret 11 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 11 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-12",
         QT_TRANSLATE_NOOP("action","Fret 12 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 12 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-13",
         QT_TRANSLATE_NOOP("action","Fret 13 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 13 of current string (TAB only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NOTE_ENTRY_TAB,
         "fret-14",
         QT_TRANSLATE_NOOP("action","Fret 14 (TAB)"),
         QT_TRANSLATE_NOOP("action","Add fret 14 of current string (TAB only)")
         ),

      // HARMONY / FIGURED BASS specific actions

      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-longa",
         QT_TRANSLATE_NOOP("action","Longa advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a longa (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-breve",
         QT_TRANSLATE_NOOP("action","Breve advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a double whole note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-1",
         QT_TRANSLATE_NOOP("action","Whole note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a whole note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-2",
         QT_TRANSLATE_NOOP("action","Half note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a half note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-4",
         QT_TRANSLATE_NOOP("action","Quarter note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a quarter note (F.B./Harm. only)")
        ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-8",
         QT_TRANSLATE_NOOP("action","Eighth note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of an eighth note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-16",
         QT_TRANSLATE_NOOP("action","16th note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a 16th note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-32",
         QT_TRANSLATE_NOOP("action","32nd note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a 32nd note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "advance-64",
         QT_TRANSLATE_NOOP("action","64th note advance (F.B./Harm.)"),
         QT_TRANSLATE_NOOP("action","Advance of a 64th note (F.B./Harm. only)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "prev-measure-TEXT",
         QT_TRANSLATE_NOOP("action","Previous measure (F.B./Harm.)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "next-measure-TEXT",
         QT_TRANSLATE_NOOP("action","Next measure (F.B./Harm.)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "prev-beat-TEXT",
         QT_TRANSLATE_NOOP("action","Previous beat (Chord symbol)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_HARMONY_FIGBASS_EDIT,
         "next-beat-TEXT",
         QT_TRANSLATE_NOOP("action","Next beat (Chord symbol)")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL | STATE_NOTE_ENTRY,
         "add-brackets",
         QT_TRANSLATE_NOOP("action","Add brackets to element"),
         0,
         0,
         Icons::brackets_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_NORMAL,
         "toggle-mmrest",
         QT_TRANSLATE_NOOP("action","Toggle create multimeasure rest"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::WindowShortcut,
         ShortcutFlags::A_CMD
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-b",
         QT_TRANSLATE_NOOP("action","bold face")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-i",
         QT_TRANSLATE_NOOP("action","italic")
         ),
      Shortcut(
         MsWidget::SCORE_TAB,
         STATE_TEXT_EDIT,
         "text-u",
         QT_TRANSLATE_NOOP("action","underline")
         ),
      Shortcut(
         MsWidget::MAIN_WINDOW,
         STATE_DISABLED | STATE_NORMAL,
         "startcenter",
         QT_TRANSLATE_NOOP("action","Toggle Startcenter"),
         0,
         0,
         Icons::Invalid_ICON,
         Qt::ApplicationShortcut
         ),
      // xml==0  marks end of list
      Shortcut(MsWidget::MAIN_WINDOW,0, 0, 0, 0)
      };
}

