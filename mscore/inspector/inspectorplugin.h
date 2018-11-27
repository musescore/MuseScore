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

#ifndef __INSPECTORPLUGIN_H__
#define __INSPECTORPLUGIN_H__

#include <QtUiPlugin/QDesignerCustomWidgetInterface>

namespace Ms {
      struct MScore {
            static QColor selectColor[4];
            static void init();
            };

      struct Preferences {
            int getInt(QString) const;
            bool isThemeDark() const;
            };
      }

//---------------------------------------------------------
//   InspectorPlugin
//---------------------------------------------------------

class InspectorPlugin : public QDesignerCustomWidgetInterface {
	Q_INTERFACES(QDesignerCustomWidgetInterface)
      bool m_initialized;

   public:
    	InspectorPlugin() : m_initialized(false) { }
	bool isContainer() const     { return false;         }
    	bool isInitialized() const   { return m_initialized; }
    	QIcon icon() const           { return QIcon();       }
    	virtual QString codeTemplate() const { return QString();     }
    	QString whatsThis() const    { return QString();     }
    	QString toolTip() const      { return QString();     }
    	QString group() const        { return "MuseScore Inspector Widgets"; }
	void initialize(QDesignerFormEditorInterface *) {
		if (m_initialized)
			return;
		m_initialized = true;
		}
      };

//---------------------------------------------------------
//   FontStyleSelectPlugin
//---------------------------------------------------------

class FontStyleSelectPlugin : public QObject, public InspectorPlugin {
      Q_OBJECT

   public:
     	FontStyleSelectPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return QString("inspector/fontStyleSelect.h"); }
      QString name() const        { return "Ms::FontStyleSelect"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   ResetButtonPlugin
//---------------------------------------------------------

class ResetButtonPlugin : public QObject, public InspectorPlugin {
      Q_OBJECT

   public:
     	ResetButtonPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return QString("inspector/resetButton.h"); }
      QString name() const        { return "Ms::ResetButton"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorPlugins
//---------------------------------------------------------

class InspectorPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface {
      Q_OBJECT
      Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
      Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

   public:
      InspectorPlugins();
      QList<QDesignerCustomWidgetInterface*> customWidgets() const;
      };


#endif

