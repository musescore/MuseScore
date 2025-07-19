# Crash dumps

## Background
It’s an unfortunate truth that any large piece of software will contain bugs that will cause it to occasionally crash. Even in the absence of bugs, software incompatibilities can cause program instability.

Fixing bugs and incompatibilities in client software that ships to millions of users around the world is a daunting task. User reports and manual reproduction of crashes can work, but even given a user report, often times the problem is not readily reproducible. This is for various reasons, such as e.g. system version or third-party software incompatibility, or the problem can happen due to a race of some sort. Users are also unlikely to report problems they encounter, and user reports are often of poor quality, as unfortunately most users don’t have experience with making good bug reports.

Automated crash reports have been the best solution to the problem so far, as this relieves the burden of manual reporting from users, while capturing the hardware and software state at the time of crash.

## Capturing crash dumps
For capturing crash dumps we use [Crashpad](https://chromium.googlesource.com/crashpad/crashpad/+/master/README.md).  
Crashpad is a client-side library that focuses on capturing machine and program state in a postmortem crash report, and transmitting this report to a backend server - a “collection server”. The Crashpad library is embedded by the client application. Conceptually, Crashpad breaks down into the handler and the client. The handler runs in a separate process from the client or clients. It is responsible for snapshotting the crashing client process’ state on a crash, saving it to a crash dump, and transmitting the crash dump to an upstream server. Clients register with the handler to allow it to capture and upload their crashes.

### Crashpad client
The Crashpad client is integrated into the Diagnostics module. It is initialized when the application starts and launches a `crashpad_handler` (see `DiagnosticsModule::onInit`).
  
### Crashpad handler  
The Crashpad handler prebuilt for all platforms and installed with MuseScore (see `src/diagnostics/crashpad_handler`).
  
### Save and send dumps
Crash dumps saving into logs directory.  
The path is different for each platform:  
TODO  
  
Also, dumps automatically sent to the crash reports server (we use Sentry), if a user accepted sending.
  
## View locally the crash report
To view the crash report needs debug symbols, otherwise, we will not see the name of the function, there will only be addressed.  
  
### Generate debug symbols
To get the debug symbols, you need the `dump_syms` utility. It can be downloaded from [this](https://github.com/musescore/crashpad_fork/tree/main/prebuilds/breakpad) repository for your platform. You need to put it in some directory and add this directory to the system `path`, or you can specify the path to `dump_syms` explicitly when calling the script.    
Also, you need the installed `python2` and `bash` shell (for Windows you can use `git bash`)  
  
To generate symbols, you need to call the `generate_syms.sh` script (see `generate_syms.sh -h` for help)   
   
For all builds on CI, symbols are generated automatically and saved to the `symbols` directory in the archive with the artifact.  
  
Symbols **must fully match** the build on which the crash occurred and the dump was created. Otherwise, the names of the functions will not be shown, or not what it should actually be shown.  
   
### View report 
To view the dump report, you need the `minidump_stackwalk` utility. It can be downloaded from [this](https://github.com/musescore/crashpad_fork/tree/main/prebuilds/breakpad) repository for your platform. You need to put it in some directory and add this directory to the system `path`, or you can specify the path to `minidump_stackwalk` explicitly when calling the script.    
Also, you need the `bash` shell (for Windows you can use `git bash`)    

To view the dump report, you need to call the `view_dump.sh` script (see `view_dump.sh -h` for help)       

