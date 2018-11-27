//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "fontStyleSelect.h"
#include "resetButton.h"
#include "inspectorplugin.h"
#include "icons.h"

#include <QtCore/QtPlugin>

namespace Ms {
      QString iconPath = QString(":/data/icons/");
      QColor  MScore::selectColor[4];

      void MScore::init()
            {
            selectColor[0].setNamedColor("#1259d0");   //blue
            selectColor[1].setNamedColor("#009234");   //green
            selectColor[2].setNamedColor("#c04400");   //orange
            selectColor[3].setNamedColor("#70167a");   //purple
            }
      Preferences preferences;

      int Preferences::getInt(QString) const
            {
            return 20;
            }
      bool Preferences::isThemeDark() const { return false; }
      }

QWidget* FontStyleSelectPlugin::createWidget(QWidget* parent)
	{
      return new Ms::FontStyleSelect(parent);
      }

QWidget* ResetButtonPlugin::createWidget(QWidget* parent)
	{
      return new Ms::ResetButton(parent);
      }

//---------------------------------------------------------
//   customWidgets
//---------------------------------------------------------

QList<QDesignerCustomWidgetInterface*> InspectorPlugins::customWidgets() const
	{
	QList<QDesignerCustomWidgetInterface*> plugins;
            plugins
               << new FontStyleSelectPlugin
               << new ResetButtonPlugin
               ;
      return plugins;
	}

InspectorPlugins::InspectorPlugins()
      {
      Ms::MScore::init();
      Ms::genIcons();
      }


// Q_EXPORT_PLUGIN(InspectorPlugins)

