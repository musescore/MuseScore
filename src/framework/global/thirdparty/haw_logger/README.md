Simple logger
-------------

Simple, convinient, thread safe and flexible logger with Qt support
(ported from https://github.com/igorkorsukov/qzebradev)

Features:
* Stream input
* Many destinations 
* Log levels, messages types, messages tags
* Very small overhead for disabled debug (with use macro) 
* Catch Qt messages
* Custom output format
* Custom messages types
* Filter by type

[Example](tests/main.cpp)

To use Logger within your software project include the Logger source into your project

Source:
* logger.h/cpp - logger and base stuff
* logdefdest.h/cpp - default destinations for console and file 
* log_example.h - macro for simple use logger

or include `logger.cmake` in the cmake project

Change log_example.h as you see fit
