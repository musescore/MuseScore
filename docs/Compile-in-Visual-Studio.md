# Compiling in Visual Studio

This guide shows how to build MuseScore on Windows using Visual Studio 2022:

1. **Prerequisites**
   - Install Visual Studio 2022 with the **Desktop development with C++** workload.
   - Install CMake (version ≥ 3.21).
   - Install Git.

2. **Clone the repo**
   ```powershell
   git clone --recursive https://github.com/musescore/MuseScore.git
   cd MuseScore
   ```

3. **Optional: Disable CMake Presets in VS**
   If you have **Always use CMake Presets** enabled, Visual Studio will ignore your `CMakeSettings.json`
   and generate a `CMakePresets.json` instead—so you won’t see the expected `msvc.build_x64` folder.
   To restore the default behavior:
   1. In Visual Studio go to **Tools → Options → CMake → General**.  
   2. Under **CMake configuration file**, select **Never use CMake Presets**.  
   3. Reload your “Standard Build” configuration; you’ll again get the `msvc.build_x64` folder.

4. **Configure**
   ```powershell
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 `
     -DCMAKE_BUILD_TYPE=RelWithDebInfo
   ```

5. **Open & Build**
   1. Double-click `MuseScore.sln` in the `build` folder to open the solution in Visual Studio.
   2. Select the **Debug** configuration from the toolbar.
   3. Choose **Build → Build Solution** (or press Ctrl+Shift+B).

6. **Run & Debug**
   - Press **F5** to launch MuseScore under the debugger.
   - Set breakpoints in C++ or QML and step through as needed.

_See also the Qt Creator guide at https://github.com/musescore/MuseScore/wiki/Compile-in-Qt-Creator_

