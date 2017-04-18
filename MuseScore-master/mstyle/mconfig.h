//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __MCONFIG_H__
#define __MCONFIG_H__

//---------------------------------------------------------
//   MgStyleConfigData
//---------------------------------------------------------

struct MgStyleConfigData {
      enum {
            MB_NONE,
            MB_FOLLOW_MOUSE,
            MB_FADE,
            ME_NONE,
            ME_FADE,
            ME_FOLLOW_MOUSE,
            TB_NONE,
            TB_FADE,
            TB_FOLLOW_MOUSE
            };
      enum {
            MM_DARK,
            MM_STRONG,
            MM_SUBTLE,
            };
      enum {
            TS_SINGLE,
            TS_PLAIN
            };
      enum {                  // drag mode
            WD_FULL,
            WD_MINIMAL
            };

      static bool genericAnimationsEnabled;
      static int genericAnimationsDuration;
      static int toolBarAnimationsDuration;
      static int menuAnimationsDuration;
      static int progressBarAnimationsDuration;
      static int progressBarBusyStepDuration;
      static int menuBarAnimationsDuration;
      static int menuFollowMouseAnimationsDuration;
      static int menuBarFollowMouseAnimationsDuration;

      static bool animationsEnabled;
      static bool progressBarAnimationsEnabled;
      static bool progressBarAnimated;
      static int menuBarAnimationType;
      static int menuAnimationType;
      static bool menuBarAnimationsEnabled;
      static bool menuAnimationsEnabled;
      static int toolBarAnimationType;

      static int menuHighlightMode;
      static bool tabSubtleShadow;
      static bool showMnemonics;
      static int scrollBarAddLineButtons;
      static int scrollBarSubLineButtons;
      static int scrollBarWidth;
      static int tabStyle;
      static bool viewDrawFocusIndicator;
      static bool cacheEnabled;
      static int maxCacheSize;
      static bool widgetExplorerEnabled;
      static bool drawWidgetRects;

      static bool comboBoxTransitionsEnabled;
      static bool labelTransitionsEnabled;
      static bool lineEditTransitionsEnabled;
      static bool stackedWidgetTransitionsEnabled;

      static int comboBoxTransitionsDuration;
      static int labelTransitionsDuration;
      static int lineEditTransitionsDuration;
      static int stackedWidgetTransitionsDuration;

      static bool toolTipDrawStyledFrames;

      };
#endif

