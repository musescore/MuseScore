//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: icons.h 5246 2012-01-24 18:48:55Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __ICONS_H__
#define __ICONS_H__

namespace Ms {

extern void genIcons();

enum { longaUp_ICON, brevis_ICON, note_ICON, note2_ICON, note4_ICON, note8_ICON, note16_ICON,
      note32_ICON, note64_ICON, note128_ICON,
      natural_ICON, sharp_ICON, sharpsharp_ICON, flat_ICON, flatflat_ICON,
      quartrest_ICON, dot_ICON, dotdot_ICON,
      flip_ICON,
      undo_ICON, redo_ICON, cut_ICON, copy_ICON, paste_ICON, print_ICON, clef_ICON,
      midiin_ICON, speaker_ICON, start_ICON, play_ICON, repeat_ICON, pan_ICON,
      sbeam_ICON, mbeam_ICON, nbeam_ICON, beam32_ICON, beam64_ICON, abeam_ICON, fbeam1_ICON, fbeam2_ICON,
      fileOpen_ICON, fileNew_ICON, fileSave_ICON, fileSaveAs_ICON,
      window_ICON, acciaccatura_ICON, appoggiatura_ICON,
      grace4_ICON, grace8b_ICON, grace16_ICON, grace32_ICON, noteEntry_ICON,
      keys_ICON, tie_ICON,
      textBold_ICON, textItalic_ICON, textUnderline_ICON,
      textLeft_ICON, textCenter_ICON, textRight_ICON, textTop_ICON, textBottom_ICON, textVCenter_ICON, textBaseline_ICON,
      textSuper_ICON, textSub_ICON,
      fotomode_ICON,
      hraster_ICON, vraster_ICON, repitch_ICON,
      formatListUnordered_ICON, formatListOrdered_ICON,
      formatIndentMore_ICON, formatIndentLess_ICON, community_ICON,
      loop_ICON, loopIn_ICON, loopOut_ICON, metronome_ICON,
      vframe_ICON, hframe_ICON, tframe_ICON, fframe_ICON, measure_ICON, checkmark_ICON,
      helpContents_ICON, goHome_ICON, goPrevious_ICON, goNext_ICON, viewRefresh_ICON,
      brackets_ICON,
      voice1_ICON, voice2_ICON, voice3_ICON, voice4_ICON,
      ICONS
      };

extern QIcon* icons[ICONS];

} // namespace Ms
#endif

