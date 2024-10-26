# Event Filters

Since KDDW works with non-Qt, we can't use Qt event filters directly, so we added an abstraction layer.<br>
Event filters implement `EventFilterInterface`.
For Qt, we have a `GlobalEventFilter` class, which calls `qGuiApp->installEventFilter`, then forwards events
to all registered `EventFilterInterface`.

We have the following global event filters:

- DockRegistry singleton
  Catches expose events to maintain floating window z-order.
  Catches clicking on a MDI dock widget, to raise it.
  For hiding the auto-hide sidebar overlay when clicking elsewhere.

- FallbackMouseGrabber
  For platforms that don't support grabbing the mouse. Mostly for QtQuick to workaround bugs.

- Some wayland code

- WidgetResizeHandler when used by MDI, or if EGLFS
  For resizing MDI dock widgets when mouse goes near borders.


# mdi raise()

Clicking on a MDI dockwidget will raise it.<br>
This is tested by `tst_mdiZorder()`.<br>
Actual raising is done by `DockRegistry::onMouseButtonPress()`, which is called by our global event filter.
