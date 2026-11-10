# Snake Game

A terminal Snake game written in C++ in [`main2.cpp`](main2.cpp). Steer a snake around a 30 × 20 board, collect fruit for points, and avoid its own tail. The snake wraps through walls instead of dying at the border.

## Build and play

The source uses POSIX terminal APIs (`termios`, `unistd`, and `fcntl`) and ANSI color/positioning codes. Use a macOS or Linux terminal with a C++ compiler and thread support:

```bash
git clone https://github.com/kanchanpotdar/Snake-Game.git
cd Snake-Game
c++ -std=c++17 -pthread main2.cpp -o snake
./snake
```

| Key | Action |
| --- | --- |
| `W` / `A` / `S` / `D` | Move up / left / down / right |
| `P` | Pause or resume |
| `X` | Exit the game |

Type the lowercase keys shown in the source (`w`, `a`, `s`, `d`, `p`, `x`); the terminal does not require Enter. The program blocks an immediate reversal into the opposite direction. Run it in an interactive terminal; redirected input or a non-POSIX terminal may not work. An exit or self-collision ends the game and prints the final score.

## How it works

`Fruit` chooses a random board coordinate. `Snake` stores the head, direction, score, and a vector of tail segments. On each update, tail positions shift forward, the head moves one cell, and coordinates wrap at board edges. Eating fruit grows the tail by one segment and adds 10 points. Hitting the tail ends the game.

`Game` draws the board and runs the update loop while a separate thread polls the keyboard. The loop starts with a 200 ms delay between frames; the delay falls as score increases, down to a 70 ms minimum. The console draws the head and tail in green and fruit in red using ANSI escape codes.

## Current limits

- This is a single-file terminal game; there are no external assets, build system, or automated tests.
- Fruit can respawn on the snake because placement does not check occupied cells.
- Input and game loops access direction, pause, and game-over state from different threads without synchronization. Behavior can therefore be unpredictable under concurrent access.
- The terminal settings are changed briefly while polling keys and reading input. Interrupting the process may leave the terminal display in an awkward state.

The source passed a syntax-only compile with `c++ -std=c++17 -pthread -Wall -Wextra -fsyntax-only main2.cpp` on macOS. Interactive gameplay was not exercised during this documentation update.
