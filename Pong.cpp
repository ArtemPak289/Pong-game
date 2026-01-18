#include <chrono>
#include <iostream>
#include <sys/select.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

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
    }

    ~TerminalRawMode()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_);
    }

private:
    termios old_{};
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
            last = now;

            handleInput();
            update();
            render();

            if (score1_ >= WIN_SCORE || score2_ >= WIN_SCORE)
            {
                running_ = false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
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
    float speed_ = 0.5f;

private:
    static void clearScreen()
    {
        std::cout << "\033[2J\033[H";
    }

    void handleInput()
    {
        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        int ready = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
        if (ready <= 0)
            return;

        char ch;
        if (read(STDIN_FILENO, &ch, 1) <= 0)
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

    void update()
    {
        ball_.pos.x += ball_.vel.x * speed_;
        ball_.pos.y += ball_.vel.y * speed_;

        if (ball_.pos.y <= 0 || ball_.pos.y >= FIELD_HEIGHT - 1)
        {
            ball_.vel.y = -ball_.vel.y;
        }

        if ((int)ball_.pos.x == 2)
        {
            if ((int)ball_.pos.y >= p1_.y &&
                (int)ball_.pos.y < p1_.y + PADDLE_HEIGHT)
            {
                ball_.vel.x = -ball_.vel.x;
                speed_ *= 1.05f;
            }
        }

        if ((int)ball_.pos.x == FIELD_WIDTH - 3)
        {
            if ((int)ball_.pos.y >= p2_.y &&
                (int)ball_.pos.y < p2_.y + PADDLE_HEIGHT)
            {
                ball_.vel.x = -ball_.vel.x;
                speed_ *= 1.05f;
            }
        }

        if (ball_.pos.x < 0)
        {
            score2_++;
            speed_ = 1.0f;
            ball_.reset(+1);
        }

        if (ball_.pos.x > FIELD_WIDTH - 1)
        {
            score1_++;
            speed_ = 1.0f;
            ball_.reset(-1);
        }
    }

    void render()
    {
        clearScreen();

        for (int i = 0; i < FIELD_WIDTH + 2; ++i)
            std::cout << '#';
        std::cout << '\n';

        for (int y = 0; y < FIELD_HEIGHT; ++y)
        {
            std::cout << '#';

            for (int x = 0; x < FIELD_WIDTH; ++x)
            {
                if (x == (int)ball_.pos.x && y == (int)ball_.pos.y)
                {
                    std::cout << 'O';
                }
                else if (x == 1 && y >= p1_.y && y < p1_.y + PADDLE_HEIGHT)
                {
                    std::cout << '|';
                }
                else if (x == FIELD_WIDTH - 2 &&
                         y >= p2_.y && y < p2_.y + PADDLE_HEIGHT)
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

        for (int i = 0; i < FIELD_WIDTH + 2; ++i)
            std::cout << '#';
        std::cout << '\n';

        std::cout << "P1: " << score1_
                  << "   P2: " << score2_ << "\n";
        std::cout << "Controls: w/s and o/l | q = quit\n";
    }
};

int main()
{
    std::cout << "Terminal Pong (macOS)\n";
    std::cout << "Controls: w/s and o/l, q to quit\n";
    std::cout << "Press Enter to start...\n";
    std::cin.get();

    PongGame game;
    game.run();
    return 0;
}
