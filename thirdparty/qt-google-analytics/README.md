qt-google-analytics
================

Qt5 classes for providing google analytics usage in a Qt/QML application.

## Building
Include ```qt-google-analytics.pri``` in your .pro file.

## Using
Please make sure you have set your application information using ```QApplication::setApplicationName``` and ```QApplication::setApplicationVersion```.

### In C++:
```
GAnalytics tracker("UA-my-id");
tracker.sendScreenView("Main Screen");
```

### In QtQuick:
Register the class on the C++ side using ```qmlRegisterType<GAnalytics>("analytics", 0, 1, "Tracker");```
```
Tracker {
  id: tracker
  trackingID: "UA-my-id"
}

[...]
tracker.sendScreenView("Main Screen")
```

There is also an example application in the examples folder.

## License
Copyright (c) 2014-2019, University of Applied Sciences Augsburg.
All rights reserved. Distributed under the terms and conditions of the BSD License. See separate LICENSE.txt.
