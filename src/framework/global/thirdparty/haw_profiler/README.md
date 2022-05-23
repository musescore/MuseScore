Simple profiler
---------------
Simple, embedded profiler with very small overhead  
(ported from https://github.com/igorkorsukov/qzebradev)
  
Features:
* Embedded profiler (can run anywhere and anytime)
* Function duration measure
* Steps duration measure
* Very small overhead
* Enabled / disabled on compile time and run time
* Thread safe (without use mutex)
* Custom data printer

[Example](tests/main.cpp)

To use Profiler within your software project include the Profiler source into your project

Source:
* profiler.h/cpp - profiler and macros

or see and include `src/profiler.cmake` in the cmake project
