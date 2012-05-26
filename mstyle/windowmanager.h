#ifndef oxygenwindowmanager_h
#define oxygenwindowmanager_h

//////////////////////////////////////////////////////////////////////////////
// oxygenwindowmanager.h
// pass some window mouse press/release/move event actions to window manager
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
//
// Largely inspired from BeSpin style
// Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
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

    class WindowManager: public QObject
    {

        Q_OBJECT

        public:

        //! constructor
        explicit WindowManager( QObject* );

        //! destructor
        virtual ~WindowManager( void )
        {}

        //! initialize
        /*! read relevant options from OxygenStyleConfigData */
        void initialize( void );

        //! register widget
        void registerWidget( QWidget* );

        //! unregister widget
        void unregisterWidget( QWidget* );

        //! event filter [reimplemented]
        virtual bool eventFilter( QObject*, QEvent* );

        protected:

        //! timer event,
        /*! used to start drag if button is pressed for a long enough time */
        void timerEvent( QTimerEvent* );

        //! mouse press event
        bool mousePressEvent( QObject*, QEvent* );

        //! mouse move event
        bool mouseMoveEvent( QObject*, QEvent* );

        //! mouse release event
        bool mouseReleaseEvent( QObject*, QEvent* );

        //!@name configuration
        //@{

        //! enable state
        bool enabled( void ) const
        { return enabled_; }

        //! enable state
        void setEnabled( bool value )
        { enabled_ = value; }

        //! returns true if window manager is used for moving
        bool useWMMoveResize( void ) const
        { return supportWMMoveResize() && useWMMoveResize_; }

        //! use window manager for moving, when available
        void setUseWMMoveResize( bool value )
        { useWMMoveResize_ = value; }

        //! drag mode
        int dragMode( void ) const
        { return dragMode_; }

        //! drag mode
        void setDragMode( int value )
        { dragMode_ = value; }

        //! drag distance (pixels)
        void setDragDistance( int value )
        { dragDistance_ = value; }

        //! drag delay (msec)
        void setDragDelay( int value )
        { dragDelay_ = value; }

        //! set list of whiteListed widgets
        /*!
        white list is read from options and is used to adjust
        per-app window dragging issues
        */
        void initializeWhiteList();

        //! set list of blackListed widgets
        /*!
        black list is read from options and is used to adjust
        per-app window dragging issues
        */
        void initializeBlackList( void );

        //@}

        //! returns true if widget is dragable
        bool isDragable( QWidget* );

        //! returns true if widget is dragable
        bool isBlackListed( QWidget* );

        //! returns true if widget is dragable
        bool isWhiteListed( QWidget* ) const;

        //! returns true if drag can be started from current widget
        bool canDrag( QWidget* );

        //! returns true if drag can be started from current widget and position
        /*! child at given position is passed as second argument */
        bool canDrag( QWidget*, QWidget*, const QPoint& );

        //! reset drag
        void resetDrag( void );

        //! start drag
        void startDrag( QWidget*, const QPoint& );

        //! returns true if window manager is used for moving
        /*! right now this is true only for X11 */
        bool supportWMMoveResize( void ) const;

        //!@name lock
        //@{

        void setLocked( bool value )
        { locked_ = value; }

        //! lock
        bool isLocked( void ) const
        { return locked_; }

        //@}

        private:

        //! enability
        bool enabled_;

        //! use WM moveResize
        bool useWMMoveResize_;

        //! drag mode
        int dragMode_;

        //! drag distance
        /*! this is copied from kwin::geometry */
        int dragDistance_;

        //! drag delay
        /*! this is copied from kwin::geometry */
        int dragDelay_;

        //! wrapper for exception id
        class ExceptionId: public QPair<QString, QString>
        {
            public:

            //! constructor
            ExceptionId( const QString& value )
            {
                const QStringList args( value.split( "@" ) );
                if( args.isEmpty() ) return;
                second = args[0].trimmed();
                if( args.size()>1 ) first = args[1].trimmed();
            }

            const QString& appName( void ) const
            { return first; }

            const QString& className( void ) const
            { return second; }

        };

        //! exception set
        typedef QSet<ExceptionId> ExceptionSet;

        //! list of white listed special widgets
        /*!
        it is read from options and is used to adjust
        per-app window dragging issues
        */
        ExceptionSet whiteList_;

        //! list of black listed special widgets
        /*!
        it is read from options and is used to adjust
        per-app window dragging issues
        */
        ExceptionSet blackList_;

        //! drag point
        QPoint dragPoint_;
        QPoint globalDragPoint_;

        //! drag timer
        QBasicTimer dragTimer_;

        //! target being dragged
        /*! QWeakPointer is used in case the target gets deleted while drag is in progress */
        QWeakPointer<QWidget> target_;

        //! true if drag is about to start
        bool dragAboutToStart_;

        //! true if drag is in progress
        bool dragInProgress_;

        //! true if drag is locked
        bool locked_;

        //! cursor override
        /*! used to keep track of application cursor being overridden when dragging in non-WM mode */
        bool cursorOverride_;

        //! provide application-wise event filter
        /*!
        it us used to unlock dragging and make sure event look is properly restored
        after a drag has occurred
        */
        class AppEventFilter: public QObject
        {

            public:

            //! constructor
            AppEventFilter( WindowManager* parent ):
                QObject( parent ),
                parent_( parent )
            {}

            //! event filter
            virtual bool eventFilter( QObject*, QEvent* );

            protected:

            //! application-wise event.
            /*! needed to catch end of XMoveResize events */
            bool appMouseEvent( QObject*, QEvent* );

            private:

            //! parent
            WindowManager* parent_;

        };

        //! application event filter
        AppEventFilter* appEventFilter_;

        //! allow access of all private members to the app event filter
        friend class AppEventFilter;

    };


#endif
