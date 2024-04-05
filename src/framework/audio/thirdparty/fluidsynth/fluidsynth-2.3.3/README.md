# FluidSynth

| | Build Status |
|---|---|
| <img src="https://www.kernel.org/theme/images/logos/tux.png" height="30" alt=""> **Linux** (CI) | [![FluidSynth Linux](https://github.com/FluidSynth/fluidsynth/workflows/FluidSynth%20Linux/badge.svg)](https://github.com/FluidSynth/fluidsynth/actions?query=workflow%3A%22FluidSynth+Linux%22) |
| <img src="https://upload.wikimedia.org/wikipedia/commons/3/35/Obs-logo.png" height=30 alt=""> **Linux** (Packaging) | [![build result](https://build.opensuse.org/projects/home:derselbst:anmp/packages/fluidsynth/badge.svg?type=default)](https://build.opensuse.org/package/show/home:derselbst:anmp/fluidsynth)
| <img src="https://raw.githubusercontent.com/docker-library/docs/781049d54b1bd9b26d7e8ad384a92f7e0dcb0894/alpine/logo.png" height="25" alt=""> **Alpine** (musl) | [![Fluidsynth Alpine](https://dev.azure.com/tommbrt/tommbrt/_apis/build/status%2FFluidSynth.fluidsynth.alpine?branchName=master)](https://dev.azure.com/tommbrt/tommbrt/_build/latest?definitionId=12&branchName=alpine) |
| <img src="https://www.freebsd.org/gifs/daemon_hammer.jpg" height="30" alt=""> **FreeBSD** | [![Build Status](https://api.cirrus-ci.com/github/FluidSynth/fluidsynth.svg?branch=master)](https://cirrus-ci.com/github/FluidSynth/fluidsynth) |
| <img src="https://www.microsoft.com/windows/favicon.ico" height="25" alt=""> **Windows** (CI) | [![Build Status](https://dev.azure.com/tommbrt/tommbrt/_apis/build/status/FluidSynth.fluidsynth.Win?branchName=master)](https://dev.azure.com/tommbrt/tommbrt/_build/latest?definitionId=3&branchName=master) |
| <img src="https://www.microsoft.com/windows/favicon.ico" height="25" alt=""> **Windows** (vcpkg) | [![Build Status](https://dev.azure.com/tommbrt/tommbrt/_apis/build/status/FluidSynth.fluidsynth.vcpkg?branchName=master)](https://dev.azure.com/tommbrt/tommbrt/_build/latest?definitionId=6&branchName=master) |
| <img src="https://www.apple.com/favicon.ico" height="30" alt=""> **MacOSX** | [![Build Status](https://dev.azure.com/tommbrt/tommbrt/_apis/build/status/FluidSynth.fluidsynth.macOS?branchName=master)](https://dev.azure.com/tommbrt/tommbrt/_build/latest?definitionId=5&branchName=master) |
| <img src="https://www.android.com/favicon.ico" height="30" alt=""> **Android** | [![Build Status](https://dev.azure.com/tommbrt/tommbrt/_apis/build/status/FluidSynth.fluidsynth.Android?branchName=master)](https://dev.azure.com/tommbrt/tommbrt/_build/latest?definitionId=4&branchName=master) |


#### FluidSynth is a cross-platform, real-time software synthesizer based on the Soundfont 2 specification.

FluidSynth generates audio by reading and handling MIDI events from MIDI input devices by using a [SoundFont](https://github.com/FluidSynth/fluidsynth/wiki/SoundFont). It is the software analogue of a MIDI synthesizer. FluidSynth can also play MIDI files.

[![SonarQube Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=FluidSynth_fluidsynth&metric=alert_status)](https://sonarcloud.io/dashboard?id=FluidSynth_fluidsynth) [![OHLOH Project Stats](https://www.openhub.net/p/fluidsynth/widgets/project_thin_badge?format=gif)](https://www.openhub.net/p/fluidsynth)


## Documentation

The central place for documentation and further links is our **wiki** here at GitHub:

#### https://github.com/FluidSynth/fluidsynth/wiki

If you are missing parts of the documentation, let us know by writing to our mailing list.
Of course, you are welcome to edit and improve the wiki yourself. All you need is an account at GitHub. Alternatively, you may send an EMail to our mailing list along with your suggested changes. Further information about the mailing list is available in the wiki as well.

Latest information about FluidSynth is also available on the web site at https://www.fluidsynth.org/.

## License

The source code for FluidSynth is distributed under the terms of the [GNU Lesser General Public License](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html), see the [LICENSE](https://github.com/FluidSynth/fluidsynth/blob/master/LICENSE) file. To better understand the conditions how FluidSynth can be used in e.g. commercial or closed-source projects, please refer to the [LicensingFAQ in our wiki](https://github.com/FluidSynth/fluidsynth/wiki/LicensingFAQ).

## Building from source

For information on how to build FluidSynth from source, please [refer to our wiki](https://github.com/FluidSynth/fluidsynth/wiki/BuildingWithCMake).

## Links

- FluidSynth's Home Page, https://www.fluidsynth.org

- FluidSynth's wiki, https://github.com/FluidSynth/fluidsynth/wiki

- FluidSynth's API documentation, https://www.fluidsynth.org/api/

---

## Historical background

### Why did we do it

The synthesizer grew out of a project, started by Samuel Bianchini and
Peter Hanappe, and later joined by Johnathan Lee, that aimed at
developing a networked multi-user game.

Sound (and music) was considered a very important part of the game. In
addition, users had to be able to extend the game with their own
sounds and images. Johnathan Lee proposed to use the Soundfont
standard combined with intelligent use of midifiles. The arguments
were:

- Wavetable synthesis is low on CPU usage, it is intuitive and it can
  produce rich sounds

- Hardware acceleration is possible if the user owns a Soundfont
  compatible soundcard (important for games!)

- MIDI files are small and Soundfont2 files can be made small thru the
  intelligent use of loops and wavetables. Together, they are easier to
  downloaded than MP3 or audio files.

- Graphical editors are available for both file format: various
  Soundfont editors are available on PC and on Linux (Smurf!), and
  MIDI sequencers are available on all platforms.

It seemed like a good combination to use for an (online) game. 

In order to make Soundfonts available on all platforms (Linux, Mac,
and Windows) and for all sound cards, we needed a software Soundfont
synthesizer. That is why we developed FluidSynth.

### Design decisions

The synthesizer was designed to be as self-contained as possible for
several reasons:

- It had to be multi-platform (Linux, macOS, Win32). It was therefore
  important that the code didn't rely on any platform-specific
  library.

- It had to be easy to integrate the synthesizer modules in various
  environments, as a plugin or as a dynamically loadable object. I
  wanted to make the synthesizer available as a plugin (jMax, LADSPA,
  Xmms, WinAmp, Director, ...); develop language bindings (Python,
  Java, Perl, ...); and integrate it into (game) frameworks (Crystal
  Space, SDL, ...). For these reasons I've decided it would be easiest
  if the project stayed very focused on its goal (a Soundfont
  synthesizer), stayed small (ideally one file) and didn't dependent
  on external code.
