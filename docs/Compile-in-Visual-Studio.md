# Compiling in Visual Studio

This guide shows how to build MuseScore on Windows using Visual Studio 2022:

1. **Prerequisites**
   - Install Visual Studio 2022 with "Desktop development with C++" workload.
   - Install CMake (>=3.21).
   - Install Git.

2. **Clone the repo**
   ```powershell
   git clone --recursive https://github.com/musescore/MuseScore.git
   cd MuseScore
   2.1 **Optional: Disable CMake Presets in VS**  
        If you’ve got “Always use CMake Presets” enabled, Visual Studio will ignore your `CMakeSettings.json` and generate a `CMakePresets.json` instead—so you won’t see the expected `msvc.build_x64` folder. To restore the default behavior:

        1. In VS, go **Tools → Options → CMake → General**.  
        2. Under **CMake configuration file**, select **Never use CMake Presets**.  
        3. Reload your “Standard Build” configuration and you’ll again get the `msvc.build_x64` folder as documented.  