
#ifndef __ENABLEPLAYWIDGET__
#define __ENABLEPLAYWIDGET__

/*
 This class was created for keyboard accessibility purposes.
 It should be used by widgets that want to allow score playback if they have focus, but not their children.
 Additionally, if one of its children widgets has focus and Escape is pressed the target widget will gain focus.
 The shortcut for this target widget's play action will always be the same as the global one ( getAction("play") )

 NOTE: The client is responsible for setting the appropriate Focus Policies for its controllers and forwards the
 calls of showEvent and eventFilter to their reference of EnablePlayForWidget object.

*/
namespace Ms {

class EnablePlayForWidget {
      QAction* _localPlayAction;
      QWidget* _target;

public:
      void showEvent(QShowEvent *);
      bool eventFilter(QObject* obj, QEvent* e);
      EnablePlayForWidget(QWidget* target);
      void connectLocalPlayToDifferentSlot(QObject* obj, const char* id);
};

}

#endif
