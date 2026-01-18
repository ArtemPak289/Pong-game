#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

constexpr int FIELD_WIDTH = 80;
constexpr int FIELD_HEIGHT = 24;
constexpr int PADDLE_HEIGHT = 4;
constexpr int WIN_SCORE = 15;

class TerminalRawMode
{
public:
    TerminalRawMode()
    {
        tcgetattr(STDIN_FILENO, &old_);
        termios raw = old_;
        raw.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        // non‑blocking stdin
        oldFlags_ = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldFlags_ | O_NONBLOCK);
    }

    ~TerminalRawMode()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_);
        fcntl(STDIN_FILENO, F_SETFL, oldFlags_);
    }

private:
    termios old_{};
    int oldFlags_{};
};

struct Vec2
{
    float x{0}, y{0};
};

struct Paddle
{
    int y = FIELD_HEIGHT / 2 - PADDLE_HEIGHT / 2;

    void moveUp()
    {
        if (y > 0)
            y--;
    }

    void moveDown()
    {
        if (y < FIELD_HEIGHT - PADDLE_HEIGHT)
            y++;
    }
};

struct Ball
{
    Vec2 pos{FIELD_WIDTH / 2.0f, FIELD_HEIGHT / 2.0f};
    Vec2 vel{1.0f, 0.6f};

    void reset(int dir)
    {
        pos = {FIELD_WIDTH / 2.0f, FIELD_HEIGHT / 2.0f};
        vel = {1.0f * dir, 0.6f};
    }
};

class PongGame
{
public:
    void run()
    {
        TerminalRawMode raw;
        clearScreen();

        using clock = std::chrono::steady_clock;
        auto last = clock::now();

        while (running_)
        {
            auto now = clock::now();
            std::chrono::duration<float> dt = now - last;
            last = now;

            handleInput();
            update(dt.count());
            render();

            if (score1_ >= WIN_SCORE || score2_ >= WIN_SCORE)
            {
                running_ = false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }

        clearScreen();
        std::cout << "Game Over!\n";
        if (score1_ > score2_)
            std::cout << "Player 1 wins!\n";
        else
            std::cout << "Player 2 wins!\n";
    }

private:
    Paddle p1_;
    Paddle p2_;
    Ball ball_;

    int score1_{0};
    int score2_{0};
    bool running_{true};

    float speedMultiplier_ = 1.0f;

private:
    static void clearScreen()
    {
        std::cout << "\033[2J\033[H";
    }

    void handleInput()
    {
        int ch = getchar();
        if (ch == EOF)
            return;

        switch (ch)
        {
        case 'w':
            p1_.moveUp();
            break;
        case 's':
            p1_.moveDown();
            break;
        case 'o':
            p2_.moveUp();
            break;
        case 'l':
            p2_.moveDown();
            break;
        case 'q':
            running_ = false;
            break;
        default:
            break;
        }
    }

    void update(float dt)
    {
        // Move ball
        ball_.pos.x += ball_.vel.x * speedMultiplier_;
        ball_.pos.y += ball_.vel.y * speedMultiplier_;

        // Top / bottom collision
        if (ball_.pos.y <= 0 || ball_.pos.y >= FIELD_HEIGHT - 1)
        {
            ball_.vel.y = -ball_.vel.y;
        }

        // Left paddle collision
        if (static_cast<int>(ball_.pos.x) == 2)
        {
            if (static_cast<int>(ball_.pos.y) >= p1_.y &&
                static_cast<int>(ball_.pos.y) < p1_.y + PADDLE_HEIGHT)
            {

                ball_.vel.x = -ball_.vel.x;
                addSpin(p1_);
                speedUp();
            }
        }

        // Right paddle collision
        if (static_cast<int>(ball_.pos.x) == FIELD_WIDTH - 3)
        {
            if (static_cast<int>(ball_.pos.y) >= p2_.y &&
                static_cast<int>(ball_.pos.y) < p2_.y + PADDLE_HEIGHT)
            {

                ball_.vel.x = -ball_.vel.x;
                addSpin(p2_);
                speedUp();
            }
        }

        // Score
        if (ball_.pos.x < 0)
        {
            score2_++;
            speedMultiplier_ = 1.0f;
            ball_.reset(+1);
        }

        if (ball_.pos.x > FIELD_WIDTH - 1)
        {
            score1_++;
            speedMultiplier_ = 1.0f;
            ball_.reset(-1);
        }

        // Simple AI for Player 2 if you comment manual controls
        aiMove();
    }

    void aiMove()
    {
        int target = static_cast<int>(ball_.pos.y);
        int center = p2_.y + PADDLE_HEIGHT / 2;

        if (target < center)
            p2_.moveUp();
        else if (target > center)
            p2_.moveDown();
    }

    void addSpin(const Paddle &paddle)
    {
        int paddleCenter = paddle.y + PADDLE_HEIGHT / 2;
        float diff = ball_.pos.y - paddleCenter;
        ball_.vel.y += diff * 0.05f; // spin
    }

    void speedUp()
    {
        speedMultiplier_ *= 1.05f; // progressive difficulty
    }

    void render()
    {
        clearScreen();

        // Top border
        for (int i = 0; i < FIELD_WIDTH + 2; ++i)
            std::cout << '#';
        std::cout << '\n';

        for (int y = 0; y < FIELD_HEIGHT; ++y)
        {
            std::cout << '#';

            for (int x = 0; x < FIELD_WIDTH; ++x)
            {
                if (x == static_cast<int>(ball_.pos.x) &&
                    y == static_cast<int>(ball_.pos.y))
                {
                    std::cout << 'O';
                }
                else if (x == 1 && y >= p1_.y && y < p1_.y + PADDLE_HEIGHT)
                {
                    std::cout << '|';
                }
                else if (x == FIELD_WIDTH - 2 && y >= p2_.y && y < p2_.y + PADDLE_HEIGHT)
                {
                    std::cout << '|';
                }
                else if (x == FIELD_WIDTH / 2)
                {
                    std::cout << ':';
                }
                else
                {
                    std::cout << ' ';
                }
            }

            std::cout << "#\n";
        }

        // Bottom border
        for (int i = 0; i < FIELD_WIDTH + 2; ++i)
            std::cout << '#';
        std::cout << '\n';

        std::cout << "P1: " << score1_ << "   P2: " << score2_ << '\n';
        std::cout << "Controls: P1(w/s), P2(o/l), q = quit" << '\n';
    }
};

int main()
{
    std::cout << "Terminal Pong (C++)\n";
    std::cout << "Press w/s and o/l to move paddles. q to quit.\n";
    std::cout << "First to " << WIN_SCORE << " wins.\n";
    std::cout << "Press Enter to start..." << std::endl;
    std::cin.get();

    PongGame game;
    game.run();
    return 0;
}
