# Gaelic Clash

Gaelic Clash is a fast-paced Gaelic football simulator written in modern C++. You can experience the drama from the line in the classic terminal edition **or** watch it unfold in a brand-new Raylib-powered broadcast window.

## Modes & Features
- **Accelerated 70-minute match** that plays out in roughly two real-world minutes thanks to a virtual match clock and cinematic pacing.
- **Procedurally generated squads** with authentic Irish names, individual abilities, injuries, and scoring tallies.
- **Interactive tactics** each time your team attacks: gamble on goals, chip away with points, or slow things down to draw frees.
- **Adaptive AI coach** that reacts to the scoreline and remaining time when controlling the opposition.
- **Rich match commentary** with live scoreboards, halftime and full-time statistical reports, and top-scorer breakdowns.
- **Presentation mode (Raylib GUI)** with live commentary ticker, tactic buttons, and halftime/full-time overlays for couch spectating.

## Building & Running

### Quick CLI build (Linux & macOS)
```bash
cd source
make          # builds the text-mode Gaelic Clash
./GAA         # launches the interactive simulator in your terminal
```

### CMake workflow (all platforms)
Prefer the bundled **CMake presets** (CMake 3.23+) for a repeatable setup. They remember your configuration and are shared across scripts.

```bash
cmake --preset linux-release                  # configure (use windows-release/windows-gui-release on Windows)
cmake --build --preset linux-release          # build the CLI edition
cmake --build --preset windows-gui-release    # build the Raylib GUI on Windows
./build/linux-release/GaelicClash             # launch on Linux/macOS
```

If you would rather configure manually you can still run `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` and pass `-DENABLE_GUI=ON` to fetch and compile the Raylib presentation mode.

### Windows one-liner automation
Use the PowerShell helper to download a portable copy of CMake (when it is not already on your PATH), configure with the presets, build, and optionally package or launch the simulator:

```powershell
pwsh -ExecutionPolicy Bypass -File scripts/bootstrap-windows.ps1 -Gui -Package -Run
```

Add `-Gui` to include the Raylib front-end, `-Package` to produce the `.zip` plus NSIS installer, and `-Run` to launch the freshly built executable. The script expects the Visual Studio Build Tools with the C++ workload to be present and will remind you to install them if they are missing.

### Manual Windows workflow (if you prefer)
1. Install the **C++ Desktop Development** workload in Visual Studio *or* a CMake-capable toolchain such as MSYS2/MinGW.
2. Open **x64 Native Tools Command Prompt** (MSVC) or the relevant shell.
3. Configure and build via presets or the helper batch script:
   ```bat
   cmake --preset windows-gui-release
   cmake --build --preset windows-gui-release
   ```
   or
   ```bat
   scripts\build-windows.bat --gui --package  # --gui enables Raylib, --package adds installer/zip
   ```
4. Run the edition you want:
   ```bat
   build\windows-release\Release\GaelicClash.exe      # terminal play
   build\windows-release\Release\GaelicClashGUI.exe   # presentation window
   ```

### Portable Packages & Installer
Generate shareable bundles from any platform using the same presets:

```bash
cmake --preset linux-release
cmake --build --preset linux-release --target package
```

Swap `linux-release` for `windows-release` or `windows-gui-release` when running on Windows so the bundle matches the executable you built. When the GUI is enabled the installer/archives include both executables. On Windows the command above—or passing `-Package` to the PowerShell bootstrap script—produces a portable `.zip` and an **NSIS installer**. Linux and macOS users receive `.tar.gz` and `.zip` archives.

## Gameplay Tips
1. **Pick your side**: Choose whether to manage the home or away team and rename the counties in the setup screen.
2. **Set the tempo**: You can shorten each half (10–40 minutes of simulated time) to create a snappier or longer experience.
3. **Mind your tactics**:
   - *Go for goal* offers the fastest way to flip the scoreboard but risks turnovers.
   - *Take your points* is steady and safe for keeping the scoreboard ticking.
   - *Control the tempo* bleeds the clock, drawing frees when you are ahead.
4. **Watch the momentum**: Injuries, wides, and fouls are tracked and summarised at the break so you can adjust your style for the second half.
5. **In the GUI**: the tactics ribbon appears whenever you're on the ball—click a play to trigger the next highlight. During halftime/full-time you can relaunch the second half or jump back to the setup screen.

Have fun chasing Sam with Gaelic Clash! Contributions or tweaks are welcome—fire up an issue or hack away.
