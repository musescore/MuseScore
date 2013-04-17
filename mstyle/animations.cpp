//////////////////////////////////////////////////////////////////////////////
// oxygenanimations.cpp
// container for all animation engines
// -------------------
//
// Copyright (c) 2006, 2007 Riccardo Iaconelli <riccardo@kde.org>
// Copyright (c) 2006, 2007 Casper Boemann <cbr@boemann.dk>
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "animations.h"
#include "mconfig.h"

#define OxygenStyleConfigData_genericAnimationsEnabled      true
#define OxygenStyleConfigData_animationsEnabled             true
#define OxygenStyleConfigData_progressBarAnimationsEnabled  true
#define OxygenStyleConfigData_progressBarAnimated           true

//____________________________________________________________
Animations::Animations( QObject* parent ):
      QObject( parent ) {

      widgetEnabilityEngine_ = new WidgetStateEngine( this );
      spinBoxEngine_         = new SpinBoxEngine( this );
      comboBoxEngine_        = new WidgetStateEngine( this );
      toolButtonEngine_      = new WidgetStateEngine( this );
      toolBoxEngine_         = new ToolBoxEngine( this );

      registerEngine( splitterEngine_ = new SplitterEngine( this ) );
      registerEngine( dockSeparatorEngine_ = new DockSeparatorEngine( this ) );
      registerEngine( headerViewEngine_ = new HeaderViewEngine( this ) );
      registerEngine( widgetStateEngine_ = new WidgetStateEngine( this ) );
      registerEngine( lineEditEngine_ = new WidgetStateEngine( this ) );
      registerEngine( progressBarEngine_ = new ProgressBarEngine( this ) );
      registerEngine( menuBarEngine_ = new MenuBarEngineV1( this ) );
      registerEngine( menuEngine_ = new MenuEngineV1( this ) );
      registerEngine( scrollBarEngine_ = new ScrollBarEngine( this ) );
      registerEngine( sliderEngine_ = new SliderEngine( this ) );
      registerEngine( tabBarEngine_ = new TabBarEngine( this ) );
      registerEngine( toolBarEngine_ = new ToolBarEngine( this ) );
      registerEngine( mdiWindowEngine_ = new MdiWindowEngine( this ) );
      }

//____________________________________________________________
void Animations::setupEngines( void ) {

            {
            // default enability, duration and maxFrame
            bool animationsEnabled = MgStyleConfigData::animationsEnabled;

            // enability
            widgetEnabilityEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            widgetStateEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            comboBoxEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            toolButtonEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            toolBoxEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            lineEditEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            splitterEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            scrollBarEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            sliderEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            spinBoxEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            tabBarEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            dockSeparatorEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            headerViewEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );
            mdiWindowEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::genericAnimationsEnabled );

            progressBarEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::progressBarAnimationsEnabled );
            progressBarEngine_->setBusyIndicatorEnabled( animationsEnabled &&  MgStyleConfigData::progressBarAnimated );

            // menubar engine
            int menuBarAnimationType( MgStyleConfigData::menuBarAnimationType );
            if ( menuBarAnimationType == MgStyleConfigData::MB_FADE && !qobject_cast<MenuBarEngineV1*>( menuBarEngine_ ) ) {
                  if ( menuBarEngine_ ) {

                        MenuBarEngineV1* newEngine = new MenuBarEngineV1( this, menuBarEngine_ );
                        registerEngine( newEngine );
                        menuBarEngine_->deleteLater();
                        menuBarEngine_ = newEngine;

                        }
                  else registerEngine( menuBarEngine_ = new MenuBarEngineV1( this ) );

                  }
            else if ( menuBarAnimationType == MgStyleConfigData::MB_FOLLOW_MOUSE && !qobject_cast<MenuBarEngineV2*>( menuBarEngine_ ) ) {

                  if ( menuBarEngine_ ) {

                        MenuBarEngineV2* newEngine = new MenuBarEngineV2( this, menuBarEngine_ );
                        registerEngine( newEngine );
                        menuBarEngine_->deleteLater();
                        menuBarEngine_ = newEngine;

                        }
                  else registerEngine( menuBarEngine_ = new MenuBarEngineV1( this ) );

                  }

            // menu engine
            int menuAnimationType( MgStyleConfigData::menuAnimationType );
            if ( menuAnimationType == MgStyleConfigData::ME_FADE && !qobject_cast<MenuEngineV1*>( menuEngine_ ) ) {

                  if ( menuEngine_ ) {

                        MenuEngineV1* newEngine = new MenuEngineV1( this, menuEngine_ );
                        registerEngine( newEngine );
                        menuEngine_->deleteLater();
                        menuEngine_ = newEngine;

                        }
                  else registerEngine( menuEngine_ = new MenuEngineV1( this ) );

                  }
            else if ( menuAnimationType == MgStyleConfigData::ME_FOLLOW_MOUSE && !qobject_cast<MenuEngineV2*>( menuEngine_ ) ) {

                  if ( menuEngine_ ) {

                        MenuEngineV2* newEngine = new MenuEngineV2( this, menuEngine_ );
                        registerEngine( newEngine );
                        menuEngine_->deleteLater();
                        menuEngine_ = newEngine;

                        }
                  else registerEngine( menuEngine_ = new MenuEngineV1( this ) );

                  }

            menuBarEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::menuBarAnimationsEnabled && menuBarAnimationType != MgStyleConfigData::MB_NONE );
            menuEngine_->setEnabled( animationsEnabled &&  MgStyleConfigData::menuAnimationsEnabled && menuAnimationType != MgStyleConfigData::ME_NONE );

            // toolbar engine
            int toolBarAnimationType( MgStyleConfigData::toolBarAnimationType );
            if ( toolBarAnimationType == MgStyleConfigData::TB_NONE || toolBarAnimationType == MgStyleConfigData::TB_FOLLOW_MOUSE ) {

                  // disable toolbar engine
                  toolBarEngine_->setEnabled( animationsEnabled && toolBarAnimationType == MgStyleConfigData::TB_FOLLOW_MOUSE );

                  // unregister all toolbuttons that belong to a toolbar
                  foreach( QWidget * widget, widgetStateEngine_->registeredWidgets( AnimationHover | AnimationFocus ) ) {
                        if ( qobject_cast<QToolButton*>( widget ) && qobject_cast<QToolBar*>( widget->parentWidget() ) ) {
                              widgetStateEngine_->unregisterWidget( widget );
                              }
                        }

                  }
            else if ( toolBarAnimationType == MgStyleConfigData::TB_FADE ) {

                  // disable toolbar engine
                  toolBarEngine_->setEnabled( false );

                  // retrieve all registered toolbars
                  BaseEngine::WidgetList widgets( toolBarEngine_->registeredWidgets() );
                  foreach( QWidget * widget, widgets ) {
                        // get all toolbuttons
                        foreach( QObject * child, widget->children() ) {
                              if ( QToolButton* toolButton = qobject_cast<QToolButton*>( child ) ) {
                                    widgetStateEngine_->registerWidget( toolButton, AnimationHover );
                                    }
                              }
                        }

                  }

            }


            {

            // durations
            widgetEnabilityEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            widgetStateEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            comboBoxEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            toolButtonEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            toolBoxEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            lineEditEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            splitterEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            scrollBarEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            sliderEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            spinBoxEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            tabBarEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            dockSeparatorEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            headerViewEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            mdiWindowEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);

            progressBarEngine_->setDuration( MgStyleConfigData::progressBarAnimationsDuration);
            progressBarEngine_->setBusyStepDuration( MgStyleConfigData::progressBarBusyStepDuration);

            toolBarEngine_->setDuration( MgStyleConfigData::genericAnimationsDuration);
            toolBarEngine_->setFollowMouseDuration( MgStyleConfigData::toolBarAnimationsDuration);

            menuBarEngine_->setDuration( MgStyleConfigData::menuBarAnimationsDuration);
            menuBarEngine_->setFollowMouseDuration( MgStyleConfigData::menuBarFollowMouseAnimationsDuration);

            menuEngine_->setDuration( MgStyleConfigData::menuAnimationsDuration);
            menuEngine_->setFollowMouseDuration( MgStyleConfigData::menuFollowMouseAnimationsDuration);
            }

      }

//---------------------------------------------------------
//   registerWidget
//---------------------------------------------------------

void Animations::registerWidget( QWidget* widget ) const {
      if (!widget)
            return;

      // these are needed to not register animations for kwin widgets
      if (widget->objectName() == "decoration widget")
            return;
      if (widget->inherits("KCommonDecorationButton"))
            return;
      if (widget->inherits("QShapedPixmapWidget"))
            return;

      // all widgets are registered to the enability engine.
      widgetEnabilityEngine().registerWidget( widget, AnimationEnable );

      // install animation timers
      // for optimization, one should put with most used widgets here first
      if ( qobject_cast<QToolButton*>(widget) ) {

            toolButtonEngine().registerWidget( widget, AnimationHover );
            bool isInToolBar( qobject_cast<QToolBar*>(widget->parent()) );
            if ( isInToolBar ) {

                  if ( MgStyleConfigData::toolBarAnimationType == MgStyleConfigData::TB_FADE ) {
                        widgetStateEngine().registerWidget( widget, AnimationHover );
                        }

                  }
            else widgetStateEngine().registerWidget( widget, AnimationHover | AnimationFocus );

            }
      else if ( qobject_cast<QAbstractButton*>(widget) ) {

            if ( qobject_cast<QToolBox*>( widget->parent() ) ) {
                  toolBoxEngine().registerWidget( widget );
                  }

            widgetStateEngine().registerWidget( widget, AnimationHover | AnimationFocus );

            }
      else if ( qobject_cast<QDial*>(widget) ) {

            widgetStateEngine().registerWidget( widget, AnimationHover | AnimationFocus );

            }

      // groupboxes
      else if ( QGroupBox* groupBox = qobject_cast<QGroupBox*>( widget ) ) {
            if ( groupBox->isCheckable() ) {
                  widgetStateEngine().registerWidget( widget, AnimationHover | AnimationFocus );
                  }
            }

      // scrollbar
      else if ( qobject_cast<QScrollBar*>( widget ) ) {
            scrollBarEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QSlider*>( widget ) ) {
            sliderEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QProgressBar*>( widget ) ) {
            progressBarEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QSplitterHandle*>( widget ) ) {
            splitterEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QMainWindow*>( widget ) ) {
            dockSeparatorEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QHeaderView*>( widget ) ) {
            headerViewEngine().registerWidget( widget );
            }
      // menu
      else if ( qobject_cast<QMenu*>( widget ) ) {
            menuEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QMenuBar*>( widget ) ) {
            menuBarEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QTabBar*>( widget ) ) {
            tabBarEngine().registerWidget( widget );
            }
      else if ( qobject_cast<QToolBar*>( widget ) ) {
            toolBarEngine().registerWidget( widget );
            }

      // editors
      else if ( qobject_cast<QComboBox*>( widget ) ) {
            comboBoxEngine().registerWidget( widget, AnimationHover );
            lineEditEngine().registerWidget( widget, AnimationHover | AnimationFocus );
            }
      else if ( qobject_cast<QSpinBox*>( widget ) ) {
            spinBoxEngine().registerWidget( widget );
            lineEditEngine().registerWidget( widget, AnimationHover | AnimationFocus );
            }
      else if ( qobject_cast<QLineEdit*>( widget ) ) {
            lineEditEngine().registerWidget( widget, AnimationHover | AnimationFocus );
            }
      else if ( qobject_cast<QTextEdit*>( widget ) ) {
            lineEditEngine().registerWidget( widget, AnimationHover | AnimationFocus );
            }

      // lists
      else if ( qobject_cast<QAbstractItemView*>( widget ) || widget->inherits("Q3ListView") ) {
            lineEditEngine().registerWidget( widget, AnimationHover | AnimationFocus );
            }

      // mdi subwindows
      else if ( qobject_cast<QMdiSubWindow*>( widget ) ) {
            mdiWindowEngine().registerWidget( widget );
            }

      return;

      }

void Animations::unregisterWidget( QWidget* widget ) const {
      if ( !widget ) return;

      /*
      these are the engines that have not been stored
      inside the list, because they can be register widgets in combination
      with other engines
      */
      widgetEnabilityEngine().unregisterWidget( widget );
      spinBoxEngine().unregisterWidget( widget );
      comboBoxEngine().unregisterWidget( widget );
      toolButtonEngine().unregisterWidget( widget );
      toolBoxEngine().unregisterWidget( widget );

      // the following allows some optimization of widget unregistration
      // it assumes that a widget can be registered atmost in one of the
      // engines stored in the list.
      foreach( const BaseEngine::Pointer & engine, engines_ ) {
            if ( engine && engine.data()->unregisterWidget( widget ) ) break;
            }

      }

//_______________________________________________________________
void Animations::unregisterEngine( QObject* object ) {
      int index( engines_.indexOf( qobject_cast<BaseEngine*>(object) ) );
      if ( index >= 0 ) engines_.removeAt( index );
      }

//_______________________________________________________________
void Animations::registerEngine( BaseEngine* engine ) {
      engines_.push_back( engine );
      connect( engine, SIGNAL( destroyed( QObject* ) ), this, SLOT( unregisterEngine( QObject* ) ) );
      }

