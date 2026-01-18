# Terminal Pong (C++)

Classic Pong game running directly in the terminal.
Fast input, smooth frame loop, ball acceleration, spin mechanics, and a simple AI opponent.

---

## 🎮 Features

* Real‑time keyboard input (no Enter required)
* ~60 FPS game loop
* Progressive ball speed increase
* Spin depending on hit position on the paddle
* Simple AI for Player 2 (can be replaced by manual control)
* Automatic terminal state recovery on exit
* Works in standard POSIX terminals

---

## ⌨️ Controls

**Player 1**

* `w` — move up
* `s` — move down

**Player 2**

* `o` — move up
* `l` — move down

**General**

* `q` — quit game

---

## 🛠️ Build & Run

### Linux / macOS / WSL

```bash
g++ pong.cpp -std=c++17 -O2 -Wall -Wextra -o pong
./pong
```

> Requires a terminal that supports ANSI escape codes and `termios`.

---

## 🧱 Project Structure

```
.
├── pong.cpp
└── README.md
```

---

## ⚙️ Technical Notes

* Uses non‑blocking input via `fcntl`.
* Terminal raw mode is managed using RAII.
* Frame timing is controlled with `std::chrono`.
* Rendering uses ANSI escape codes.

---

## 🚀 Ideas for Extension

* Add colors (ANSI escape sequences)
* Improve AI prediction logic
* Add pause and menu
* Add sound effects
* Network multiplayer
* Configurable field size

---

Have fun hacking the terminal 🎯
