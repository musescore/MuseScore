# Async primitives

Convenient, thread-safe, flexible and simple used async primitives   
Requires C++17 and higher.

Features:
* Channel - channel for asynchronous interaction or communication between threads, inspired by channel from GoLang.
* Promise - promise to return a result from asynchronous operations or other threads, inspired by promise from JS.
* Notification - for notification of something
* Async - call function on next event loop or in another thread

These primitives are intended to:
* To separate dependencies between sender and subscriber (like Qt signals/slots approach, just more convenient and safer)
* Building responsive asynchronous UI applications (like JS approach)
* Communication between threads (like GoLang approach)


[Example](example/main.cpp)

Used in at least two private commercial projects and two open source
[MuseScore](https://github.com/musescore/MuseScore) and [Audacity](https://github.com/audacity/audacity)

## Integration 

### Add source 
To use Async within your software project include the Async source into your project
See and include `async/async.cmake` in the cmake project (see [example/CMakeLists.txt](example/CMakeLists.txt))

### Add aliases 
Recommended add own aliases to use async, see [example/async](example/async)

### Add call processEvents   
To use channels in one main thread, nothing is required, just direct calls will occur, like callbacks.   
   
For communication between threads or for using `Async` - calling a function on the next event loop, integration with the main event loop and the event loop of other threads is required.   
   
There are two options for integration with the main eventloop:   
1. If it is not possible to directly modify the body of the event loop, 
then you can, for example, use a timer to call `processMessages`

2. If you can directly add your call to the body of the event loop, then it’s easier to just add the call `processMessages`, like:
```
// event loop 
while (running) {
    app::async::processEvents();
    ...
}
```

## ChangeLog

### v1.4
* New non-blocking implementation (a lot of thanks for the review [Casper Jeukendrup](https://github.com/cbjeukendrup))

### v1.3
* Fixes related to communication between threads

### v1.2
* Added the possibility to have more than one parameter (thanks [Casper Jeukendrup](https://github.com/cbjeukendrup))

### v1.1
* Ported to std

### v1.0
* Implemented with Qt 
