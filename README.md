# 2048 - Qt GUI Puzzle Game

## 👨‍💻 Authors
* Kaan Aydeniz
* Bartu Batur Sevindik

## 📜 About the Project
This project is a C++ implementation of the classic 2048 sliding-tile puzzle game, featuring a fully interactive Graphical User Interface (GUI) built with the **Qt 6 Framework**. The objective of the game is to slide and merge numbered tiles on a grid to reach the target tile of 2048. 

The application goes beyond the classic mechanics by introducing multiple game modes, an unlimited undo system, and an event-driven architecture that manages real-time interactions, animations, and game states.

## ✨ Key Features
* **Event-Driven UI:** Developed using Qt Widgets (`QWidget`, `QGridLayout`, `QMessageBox`), the interface captures real-time keyboard inputs (Arrow keys or WASD) and button clicks via Qt's robust Signal-Slot mechanism.
* **Advanced State Management (Undo System):** The game features an unlimited Undo functionality. Before every valid move, the current board state and score are captured and pushed onto a `QStack<GameState>`. Players can infinitely revert their moves back to the beginning of the session.
* **Custom Game Modes:**
  * **Normal Mode:** The classic 2048 experience.
  * **Hard Mode:** A time-pressure variant using `QTimer`. If the player remains inactive for 5 seconds, the system automatically calculates all valid moves and randomly executes one.
  * **Unlimited Mode:** Allows the player to continue combining tiles beyond the 2048 target.
  * **Unlimited Hard Mode:** Combines both the 5-second time pressure and infinite gameplay.
* **Dynamic Styling & Rendering:** Grid cells dynamically update their background colors, text colors, and font properties based on the tile's exponential value, matching the classic game's aesthetic.

## 🛠️ Build and Execution

The project uses CMake for build configuration. You can build and run the game using the following commands:

**1. Configure the project:**
```bash
cmake -S . -B build
```

**2. Compile the executable:**
```bash
cmake --build build
```

**3. Run the game:**
```bash
./build/2048
```

*(Alternatively, you can use the provided verification script: `./scripts/verify_build.sh`)*

## 🎮 Controls
* **Movement:** `Up`, `Down`, `Left`, `Right` (or `W`, `A`, `S`, `D`)
* **Undo Move:** `U` key (or click the "Undo" button)
* **Restart Game:** `R` key (or click the "Restart" button)
* **Change Mode:** Click the "Mode" button to open the selection dialog.

## 🗂️ Project Structure
* `src/main.cpp`: The entry point that initializes the `QApplication` and the main event loop.
* `src/mainwindow.h` / `mainwindow.cpp`: Contains the core game logic, UI rendering, signal-slot connections, matrix manipulation (slide & merge algorithms), and timer-based events.
* `CMakeLists.txt`: Project configuration and Qt module linking.

## ⚠️ Troubleshooting for macOS / Custom Qt Installations
If CMake fails to find the Qt6 package during the configuration step, you may need to manually specify your Qt directory path. You can do this by passing the `CMAKE_PREFIX_PATH` flag:

```bash
# Example for Homebrew installations on Apple Silicon:
cmake -S . -B build -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt"

# Example for official Qt Installer installations:
cmake -S . -B build -DCMAKE_PREFIX_PATH="~/Qt/6.x.x/macos"