// Snake Game using OOP: Classes - Game, Snake, Fruit
#include <iostream>      // For standard input/output
#include <termios.h>     // For terminal control (non-blocking input)
#include <unistd.h>      // For sleep/usleep and terminal control
#include <fcntl.h>       // For file descriptor control (_kbhit)
#include <cstdlib>       // For rand() and srand()
#include <ctime>         // For time() used in seeding randomness
#include <thread>        // For multithreading input
#include <vector>        // For dynamic tail representation

using namespace std;

// Game board dimensions
const int WIDTH = 30;
const int HEIGHT = 20;

// Terminal color codes for aesthetics
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string RESET = "\033[0m";

// ========== Utility Functions ========== //

// Disables stdout buffering so output appears instantly
void DisableBuffering() {
    setvbuf(stdout, NULL, _IONBF, 0);
}

// Non-blocking input detection (returns 1 if a key is pressed)
int _kbhit() {
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);           // Turn off canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int oldf = fcntl(STDIN_FILENO, F_GETFL);     // Get file descriptor flags
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);     // Restore terminal settings
    fcntl(STDIN_FILENO, F_SETFL, oldf);          // Restore file flags
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

// Reads a single character without waiting for Enter and without echoing
char _getch() {
    char buf = 0;
    termios old = {};
    tcgetattr(STDIN_FILENO, &old);
    termios newt = old;
    newt.c_lflag &= ~(ICANON | ECHO);            // Raw mode
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    read(STDIN_FILENO, &buf, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return buf;
}

// Direction enum for snake movement
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

// ========== Class: Fruit ========== //
class Fruit {
public:
    int x, y;

    // Constructor spawns fruit at random location
    Fruit() {
        Respawn();
    }

    // Places fruit at a new random location
    void Respawn() {
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
};

// ========== Class: Snake ========== //
class Snake {
public:
    int x, y;                       // Head position
    int score = 0;                  // Player score
    Direction dir = RIGHT;          // Current direction
    vector<pair<int, int>> tail;    // Tail segments

    // Initializes snake at center
    Snake() {
        x = WIDTH / 2;
        y = HEIGHT / 2;
        score = 0;
        dir = RIGHT;
        tail.clear();
    }

    // Handles movement and tail updates
    void Move() {
        if (dir == STOP) return;

        // Shift tail positions
        int prevX = x, prevY = y;
        for (auto& segment : tail) {
            swap(segment.first, prevX);
            swap(segment.second, prevY);
        }

        // Move the head in the current direction
        switch (dir) {
            case LEFT:  x--; break;
            case RIGHT: x++; break;
            case UP:    y--; break;
            case DOWN:  y++; break;
            default: break;
        }

        // Wall wrap-around
        if (x >= WIDTH) x = 0; else if (x < 0) x = WIDTH - 1;
        if (y >= HEIGHT) y = 0; else if (y < 0) y = HEIGHT - 1;
    }

    // Adds a new tail segment and increases score
    void Grow() {
        tail.push_back({x, y});
        score += 10;
    }

    // Checks if snake collides with itself
    bool CollidesWithSelf() const {
        for (const auto& segment : tail)
            if (segment.first == x && segment.second == y)
                return true;
        return false;
    }

    // Checks if snake's head is at fruit position
    bool EatsFruit(const Fruit& fruit) const {
        return x == fruit.x && y == fruit.y;
    }
};

// ========== Class: Game ========== //
class Game {
private:
    Snake snake;        // Player-controlled snake
    Fruit fruit;        // Fruit object
    bool gameOver = false;
    bool paused = false;

public:
    // Initializes game state
    Game() {
        srand(time(0));         // Seed RNG
        cout << "\033[2J";      // Clear terminal screen
    }

    // Threaded input loop: listens for keyboard commands
    void InputHandler() {
        while (!gameOver) {
            if (_kbhit()) {
                char key = _getch();
                switch (key) {
                    case 'a': if (snake.dir != RIGHT) snake.dir = LEFT; break;
                    case 'd': if (snake.dir != LEFT) snake.dir = RIGHT; break;
                    case 'w': if (snake.dir != DOWN) snake.dir = UP; break;
                    case 's': if (snake.dir != UP) snake.dir = DOWN; break;
                    case 'p': paused = !paused; break;
                    case 'x': gameOver = true; break;
                }
            }
            usleep(10000); // Avoid CPU overload
        }
    }

    // Renders the game board
    void Draw() {
        cout << "\033[H"; // Move cursor to top-left
        cout << (paused ? "Game Paused. Press 'p' to resume.\n" : "Snake Game\n");

        // Top border
        for (int i = 0; i < WIDTH + 2; i++) cout << "#";
        cout << "\n";

        // Grid body
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (j == 0) cout << "#"; // Left border

                // Head
                if (i == snake.y && j == snake.x)
                    cout << GREEN << "O" << RESET;

                // Fruit
                else if (i == fruit.y && j == fruit.x)
                    cout << RED << "F" << RESET;

                // Tail
                else {
                    bool printed = false;
                    for (auto& seg : snake.tail) {
                        if (seg.first == j && seg.second == i) {
                            cout << GREEN << "o" << RESET;
                            printed = true;
                            break;
                        }
                    }
                    if (!printed) cout << " ";
                }

                if (j == WIDTH - 1) cout << "#"; // Right border
            }
            cout << "\n";
        }

        // Bottom border
        for (int i = 0; i < WIDTH + 2; i++) cout << "#";
        cout << "\nScore: " << snake.score << endl;
        cout << "[WASD] to move | [P] Pause | [X] Exit\n";
        fflush(stdout);
    }

    // Updates game logic
    void Update() {
        if (paused) return;

        snake.Move();

        if (snake.CollidesWithSelf()) gameOver = true;
        if (snake.EatsFruit(fruit)) {
            snake.Grow();
            fruit.Respawn();
        }
    }

    // Controls the speed based on score
    void Delay() {
        int base = 200000, min = 70000;
        int delay = base - snake.score * 500;
        if (delay < min) delay = min;
        usleep(delay);
    }

    // Starts and runs the game loop
    void Run() {
        thread inputThread(&Game::InputHandler, this);

        while (!gameOver) {
            Draw();
            Update();
            Delay();
        }

        inputThread.join();  // Wait for input thread to finish
        cout << "\nGame Over! Final Score: " << snake.score << endl;
    }
};

// ========== Main Entry Point ========== //
int main() {
    DisableBuffering();  // Turn off buffering for smooth output
    Game game;           // Instantiate Game object
    game.Run();          // Start the game loop
    return 0;
}
