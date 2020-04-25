# MuseScore Framework

Framework (engine, platform) contains an independent domain modules for developing a MuseScore application.
The framework includes modules such as:

* Audio
* Graphics
* Fonts
* UI Engine
* Modularity
* System
* and other

It’s like it’s a third-party engine, like Unity, Unreal Engine, etc.

And should not contain or depend on such models as:

* Notation (libmscore, view)
* Sequencer (view)
* Piano roll
* Mixer (view)
* User acount
* and etc
