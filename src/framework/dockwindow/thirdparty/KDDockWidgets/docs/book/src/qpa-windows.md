# Windows

KDDockWidgets not only works fine on `Windows` it also implements some native features:
- Aero-snap support (drag to screen edges to partially-maximize windows)
- Support for native Windows drop shadow around the frame
- Support mixing with MFC event loop

## Known issues

- Turning off "Show window contents while dragging" is not supported, as Qt won't receive
  any mouse or window move event until mouse is released.

Note: `MinGW` compiler is not tested, but probably works.<br>
Note: KDDW is tested with Windows 10 and 11. `Windows 7` can be made to work, upon request.
