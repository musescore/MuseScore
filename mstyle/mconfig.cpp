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

#include "mconfig.h"

bool MgStyleConfigData::genericAnimationsEnabled = true;
bool MgStyleConfigData::animationsEnabled = true;
bool MgStyleConfigData::progressBarAnimationsEnabled = true;
bool MgStyleConfigData::progressBarAnimated = true;

int  MgStyleConfigData::menuBarAnimationType = MgStyleConfigData::MB_FADE;
int  MgStyleConfigData::menuAnimationType = MgStyleConfigData::ME_FADE;

int  MgStyleConfigData::genericAnimationsDuration = 150;
int  MgStyleConfigData::toolBarAnimationsDuration = 50;
int  MgStyleConfigData::menuAnimationsDuration = 150;
int  MgStyleConfigData::progressBarAnimationsDuration;
int  MgStyleConfigData::progressBarBusyStepDuration = 50;
int  MgStyleConfigData::menuBarAnimationsDuration = 150;
int  MgStyleConfigData::menuFollowMouseAnimationsDuration = 40;
int  MgStyleConfigData::menuBarFollowMouseAnimationsDuration = 80;
bool MgStyleConfigData::menuBarAnimationsEnabled = true;
bool MgStyleConfigData::menuAnimationsEnabled = true;
int  MgStyleConfigData::toolBarAnimationType = MgStyleConfigData::TB_FADE;

//int  MgStyleConfigData::menuHighlightMode = MgStyleConfigData::MM_STRONG;
int  MgStyleConfigData::menuHighlightMode = MgStyleConfigData::MM_SUBTLE;
bool MgStyleConfigData::tabSubtleShadow = true;
bool MgStyleConfigData::showMnemonics = true;
int  MgStyleConfigData::scrollBarAddLineButtons = 2;
int  MgStyleConfigData::scrollBarSubLineButtons = 1;
int  MgStyleConfigData::tabStyle = MgStyleConfigData::TS_SINGLE;
bool MgStyleConfigData::viewDrawFocusIndicator = true;
bool MgStyleConfigData::cacheEnabled = true;
int  MgStyleConfigData::maxCacheSize = 512;
bool MgStyleConfigData::widgetExplorerEnabled = false;
bool MgStyleConfigData::drawWidgetRects = true;
int  MgStyleConfigData::scrollBarWidth  = 15;

bool MgStyleConfigData::comboBoxTransitionsEnabled = false;
bool MgStyleConfigData::labelTransitionsEnabled = true;
bool MgStyleConfigData::lineEditTransitionsEnabled = true;
bool MgStyleConfigData::stackedWidgetTransitionsEnabled = false;

int  MgStyleConfigData::comboBoxTransitionsDuration = 75;
int  MgStyleConfigData::labelTransitionsDuration = 75;
int  MgStyleConfigData::lineEditTransitionsDuration = 150;
int  MgStyleConfigData::stackedWidgetTransitionsDuration = 150;

bool MgStyleConfigData::toolTipDrawStyledFrames = true;

