<p align="center">
  <img src="docs/assets/logo-placeholder.png" alt="MuseSketch" width="120" />
</p>

<h1 align="center">🎼 MuseSketch</h1>

<p align="center">
  <strong>Sketch motifs. Shape ideas. Build counterpoint. Anywhere.</strong>
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#how-it-works">How It Works</a> •
  <a href="#getting-started">Getting Started</a> •
  <a href="#building">Building</a> •
  <a href="#architecture">Architecture</a> •
  <a href="#roadmap">Roadmap</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-iOS%20%7C%20Android%20%7C%20macOS-blue" alt="Platforms" />
  <img src="https://img.shields.io/badge/Qt-6.6-green" alt="Qt Version" />
  <img src="https://img.shields.io/badge/C++-17-orange" alt="C++ Standard" />
  <img src="https://img.shields.io/badge/License-GPL--3.0-purple" alt="License" />
</p>

---

## ✨ What is MuseSketch?

**MuseSketch** is a mobile-first classical composition app that lets you capture musical ideas as **motifs**, develop them through transformations, and assemble them into **SATB chorales and contrapuntal textures** — all from your phone or tablet.

Unlike traditional notation apps that force you to tap tiny notes on a staff, MuseSketch uses **intuitive touch gestures** to shape your musical ideas. Draw melodic contours, tap scale degrees, set rhythms — and let the app handle the notation.

> 🎹 Perfect for composers, music students, choir arrangers, and anyone who thinks in motifs.

---

## 🎯 Features

### 🎨 **Intuitive Motif Creation**
Create melodies by tapping **scale degree pads** (1-7) — no need to know exactly which note you want. The app maps your ideas to the current key automatically.

### 🎵 **Flexible Rhythm System**
Build rhythmic patterns with whole, half, quarter, eighth, and sixteenth notes. Add ties and rests with a tap.

### 🎼 **Intelligent SATB Generation**
Transform any melody into a full **Soprano, Alto, Tenor, Bass** chorale with one button. The partwriting engine follows classical voice-leading rules automatically.

### 🎹 **Real-Time Playback**
Hear your ideas instantly with built-in audio synthesis. Play individual motifs or full SATB arrangements.

### 📂 **Organize with Sketches & Sections**
- **Sketches** contain your musical ideas for a project
- **Motif Library** stores reusable melodic fragments
- **Sections** let you arrange motifs into longer pieces

### 📤 **Professional Export**
Export your work as:
- **MIDI** — for DAWs and synthesizers
- **MusicXML** — for Finale, Sibelius, Dorico, and MuseScore
- More formats coming soon

---

## 🎬 How It Works

### 1️⃣ Create a Motif

```
┌─────────────────────────────────────┐
│  Tap scale degrees to build melody  │
│                                     │
│    ┌───┬───┬───┬───┬───┬───┬───┐    │
│    │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │    │
│    └───┴───┴───┴───┴───┴───┴───┘    │
│                                     │
│  Set rhythm: ♩ ♩ ♪ ♪ 𝅗𝅥              │
└─────────────────────────────────────┘
```

### 2️⃣ Build a Section

Place your motifs on a timeline, set the length (up to 16 bars), and see the melody visualized.

### 3️⃣ Generate SATB

Select "SATB Chorale" texture and tap **Generate Partwriting**. The engine creates alto, tenor, and bass voices following classical voice-leading principles.

### 4️⃣ Play & Export

Preview your composition with real-time audio, then export to MIDI or MusicXML for further refinement.

---

## 🚀 Getting Started

### Prerequisites

- **Qt 6.5+** with modules: Core, Quick, Multimedia, Svg, Xml
- **CMake 3.16+**
- **C++17** compatible compiler

### Quick Start (macOS)

```bash
# Clone the repository
git clone https://github.com/musescore/MuseScore.git
cd MuseScore/MuseSketch

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build . -j8

# Run
./app/musesketch.app/Contents/MacOS/musesketch
```

### Mobile Builds

See [MOBILE_BUILD.md](docs/MOBILE_BUILD.md) for detailed iOS and Android build instructions.

---

## 🏗️ Building

### macOS (Desktop)

```bash
mkdir build && cd build
cmake ..
cmake --build . -j8
```

### iOS Simulator

```bash
./scripts/build-ios.sh simulator debug
open build-ios-simulator-debug/MuseSketch.xcodeproj
# Press Cmd+R in Xcode to run
```

### Android

```bash
export ANDROID_SDK_ROOT=~/Library/Android/sdk
export ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/29.0.14206865
./scripts/build-android.sh release
```

---

## 🧠 Architecture

MuseSketch follows a clean separation between **core logic** (C++) and **UI** (QML).

```
MuseSketch/
├── app/                    # QML UI layer
│   ├── Main.qml           # App shell & navigation
│   ├── MotifShapeView.qml # Motif creation screen
│   ├── SectionEditorView.qml # Section arrangement
│   └── components/        # Reusable UI components
│
├── core/                   # C++ business logic
│   ├── Motif.*            # Motif data model
│   ├── Section.*          # Section & arrangement
│   ├── PartwritingEngine.* # SATB voice generation
│   ├── AudioEngine.*      # Real-time synthesis
│   └── ExportEngine.*     # MIDI/MusicXML export
│
└── docs/                   # Documentation
    ├── PRD.md             # Product requirements
    ├── ROADMAP.md         # Development roadmap
    └── MOBILE_BUILD.md    # Mobile build guide
```

### Key Components

| Component | Purpose |
|-----------|---------|
| `SketchManager` | Central data management for sketches, motifs, and sections |
| `MotifEditorController` | Handles motif creation and editing logic |
| `PartwritingEngine` | Generates SATB voices with proper voice-leading |
| `AudioEngine` | Real-time audio synthesis with multiple voice patches |
| `ExportEngine` | Exports to MIDI and MusicXML formats |

### Design Principles

- **Motifs as First-Class Objects** — Everything starts with a motif
- **Notation as Output, Not Interface** — Create music intuitively, render notation automatically
- **Mobile-First** — Touch-native gestures, not tiny staff editing
- **Classical Voice-Leading** — The partwriting engine follows traditional rules

---

## 🗺️ Roadmap

### ✅ Completed

- [x] Motif creation with scale degree pads
- [x] Rhythm system (whole → sixteenth notes)
- [x] Sketch & section management
- [x] SATB chorale generation
- [x] Real-time audio playback
- [x] MIDI & MusicXML export
- [x] iOS & Android build support

### 🔜 Coming Soon

- [ ] Motif transformations (inversion, retrograde, transposition)
- [ ] Imitative counterpoint textures
- [ ] Chord symbol input
- [ ] Cloud sync between devices
- [ ] MuseScore Studio integration

### 🔮 Future

- [ ] Audio recording for motif capture
- [ ] AI-assisted harmonization suggestions
- [ ] Collaborative editing

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [PRD.md](docs/PRD.md) | Product requirements document |
| [ROADMAP.md](docs/ROADMAP.md) | Detailed development roadmap |
| [MOBILE_BUILD.md](docs/MOBILE_BUILD.md) | iOS & Android build guide |
| [PARTWRITING_ENGINE.md](docs/PARTWRITING_ENGINE.md) | SATB generation algorithm |

---

## 🤝 Contributing

We welcome contributions! Here's how to get started:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

Please read our [Contributing Guide](CONTRIBUTING.md) for details on our code of conduct and development process.

---

## 📄 License

MuseSketch is part of the MuseScore project and is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

- **[MuseScore](https://musescore.org)** — The world's most popular free music notation software
- **[Qt](https://qt.io)** — Cross-platform application framework
- Built with ❤️ for musicians everywhere

---

<p align="center">
  <strong>🎵 Start sketching your musical ideas today!</strong>
</p>

