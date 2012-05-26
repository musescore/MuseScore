// krazy:excludeall=qclasses

//////////////////////////////////////////////////////////////////////////////
// oxygentransitions.cpp
// container for all transition engines
// -------------------
//
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

#include "transitions.h"
#include "mconfig.h"


    //________________________________________________________--
    Transitions::Transitions( QObject* parent ):
        QObject( parent )
    {

        registerEngine( comboBoxEngine_ = new ComboBoxEngine( this ) );
        registerEngine( labelEngine_ = new LabelEngine( this ) );
        registerEngine( lineEditEngine_ = new LineEditEngine( this ) );
        registerEngine( stackedWidgetEngine_ = new StackedWidgetEngine( this ) );

    }

    //________________________________________________________--
    void Transitions::setupEngines( void )
    {

        // default enability, duration and maxFrame
        bool animationsEnabled( MgStyleConfigData::animationsEnabled );

        // enability
        comboBoxEngine().setEnabled( animationsEnabled && MgStyleConfigData::comboBoxTransitionsEnabled );
        labelEngine().setEnabled( animationsEnabled && MgStyleConfigData::labelTransitionsEnabled );
        lineEditEngine().setEnabled( animationsEnabled && MgStyleConfigData::lineEditTransitionsEnabled );
        stackedWidgetEngine().setEnabled( animationsEnabled && MgStyleConfigData::stackedWidgetTransitionsEnabled );

        // durations
        comboBoxEngine().setDuration( MgStyleConfigData::comboBoxTransitionsDuration );
        labelEngine().setDuration( MgStyleConfigData::labelTransitionsDuration );
        lineEditEngine().setDuration( MgStyleConfigData::lineEditTransitionsDuration );
        stackedWidgetEngine().setDuration( MgStyleConfigData::stackedWidgetTransitionsDuration );

    }

    //____________________________________________________________
    void Transitions::registerWidget( QWidget* widget ) const
    {

        if( !widget ) return;

        if( QLabel* label = qobject_cast<QLabel*>( widget ) ) {

            // do not animate labels from tooltips
            if( widget->window() && widget->window()->windowFlags().testFlag( Qt::ToolTip ) ) return;
            else if( widget->window() && widget->window()->inherits( "KWin::GeometryTip" ) ) return;
            else labelEngine().registerWidget( label );

        } else if( QComboBox* comboBox = qobject_cast<QComboBox*>( widget ) ) {

            comboBoxEngine().registerWidget( comboBox );

        } else if( QLineEdit* lineEdit = qobject_cast<QLineEdit*>( widget ) ) {

            lineEditEngine().registerWidget( lineEdit );

        } else if( QStackedWidget* stack = qobject_cast<QStackedWidget*>( widget ) ) {

            stackedWidgetEngine().registerWidget( stack );

        }

    }

    //____________________________________________________________
    void Transitions::unregisterWidget( QWidget* widget ) const
    {

        if( !widget ) return;

        // the following allows some optimization of widget unregistration
        // it assumes that a widget can be registered atmost in one of the
        // engines stored in the list.
        foreach( const BaseEngine::Pointer& engine, engines_ )
        { if( engine && engine.data()->unregisterWidget( widget ) ) break; }

    }

