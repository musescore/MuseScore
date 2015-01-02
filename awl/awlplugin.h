//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

#ifndef __AWLPLUGIN_H__
#define __AWLPLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>

//---------------------------------------------------------
//   AwlPlugin
//---------------------------------------------------------

class AwlPlugin : public QDesignerCustomWidgetInterface {
	Q_INTERFACES(QDesignerCustomWidgetInterface)
      bool m_initialized;

   public:
    	AwlPlugin() : m_initialized(false) { }
	bool isContainer() const     { return false;         }
    	bool isInitialized() const   { return m_initialized; }
    	QIcon icon() const           { return QIcon();       }
    	virtual QString codeTemplate() const { return QString();     }
    	QString whatsThis() const    { return QString();     }
    	QString toolTip() const      { return QString();     }
    	QString group() const        { return "MusE Awl Widgets"; }
	void initialize(QDesignerFormEditorInterface *) {
		if (m_initialized)
			return;
		m_initialized = true;
		}
      };

//---------------------------------------------------------
//   KnobPlugin
//---------------------------------------------------------

class KnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	KnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return QString("awl/knob.h"); }
      QString name() const        { return "Awl::Knob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   VolKnobPlugin
//---------------------------------------------------------

class VolKnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	VolKnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/volknob.h"; }
      QString name() const { return "Awl::VolKnob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   PanKnobPlugin
//---------------------------------------------------------

class PanKnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	PanKnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/panknob.h"; }
      QString name() const { return "Awl::PanKnob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   MidiPanKnobPlugin
//---------------------------------------------------------

class MidiPanKnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	MidiPanKnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/midipanknob.h"; }
      QString name() const { return "Awl::MidiPanKnob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   ColorLabelPlugin
//---------------------------------------------------------

class ColorLabelPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	ColorLabelPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/colorlabel.h"; }
      QString name() const { return "Awl::ColorLabel"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   SliderPlugin
//---------------------------------------------------------

class SliderPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
      SliderPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/slider.h"; }
      QString name() const { return "Awl::Slider"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   VolSliderPlugin
//---------------------------------------------------------

class VolSliderPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
      VolSliderPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/volslider.h"; }
      QString name() const { return "Awl::VolSlider"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   MeterSliderPlugin
//---------------------------------------------------------

class MeterSliderPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
      MeterSliderPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/mslider.h"; }
      QString name() const { return "Awl::MeterSlider"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   DenominatorSpinBox
//---------------------------------------------------------

class DenominatorSpinBoxPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
      DenominatorSpinBoxPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/denomspinbox.h"; }
      QString name() const { return "Awl::DenominatorSpinBox"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   PitchLabelPlugin
//---------------------------------------------------------

class PitchLabelPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
      PitchLabelPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/pitchlabel.h"; }
      QString name() const { return "Awl::PitchLabel"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   PitchEditPlugin
//---------------------------------------------------------

class PitchEditPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
      PitchEditPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/pitchedit.h"; }
      QString name() const { return "Awl::PitchEdit"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   AwlPlugins
//---------------------------------------------------------

class AwlPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface {
      Q_OBJECT
      Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
      Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

   public:
      QList<QDesignerCustomWidgetInterface*> customWidgets() const;
      };


#endif

