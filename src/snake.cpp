#include <hurdygurdy.hpp>

using namespace hg;

enum State {
    State_title,
    State_game,
    State_gameOver,
};

constexpr u32 width = 28;
constexpr u32 height = 21;

Rng rng{trueRandom()};

struct Title {
    Layer2D layer{};

    Title() noexcept
    {
        layer.drawText(
            "Snake",
            getDefaultFont(),
            Vec4{1},
            {
                {width / 2.0f - 8.0f, height / 4.0f},
                {INFINITY, height / 2.0f}
            },
            height / 35.0f);
    }

    State update(Renderer2D& renderer, const Window& window)
    {
        if (window.isButtonDown(Button_space))
            return State_game;

        renderer.queueLayer(layer);

        return State_title;
    }
};

struct Point {
    i32 x, y;
};

struct Game {
    static constexpr f64 speed = 0.08f;
    f64 timeTilTick = speed;

    Layer2D layer{};

    Point fruit = {(i32)(rng.next() % width), (i32)(rng.next() % height)};
    Point head = {width / 2, height / 2};
    Point vel = {1, 0};

    Array<Point> snake{};

    Game() noexcept
    {
        snake.push(head);
    }

    State update(Renderer2D& renderer, const Window& window, f64 delta)
    {
        if (window.isButtonDown(Button_w) || window.isButtonDown(Button_up))
        {
            if (vel.y != 1)
            {
                vel.x = 0;
                vel.y = -1;
            }
        }
        else if (window.isButtonDown(Button_a) || window.isButtonDown(Button_left))
        {
            if (vel.x != 1)
            {
                vel.x = -1;
                vel.y = 0;
            }
        }
        else if (window.isButtonDown(Button_s) || window.isButtonDown(Button_down))
        {
            if (vel.y != -1)
            {
                vel.x = 0;
                vel.y = 1;
            }
        }
        else if (window.isButtonDown(Button_d) || window.isButtonDown(Button_right))
        {
            if (vel.x != -1)
            {
                vel.x = 1;
                vel.y = 0;
            }
        }

        timeTilTick -= delta;
        if (timeTilTick < 0)
        {
            timeTilTick += speed;

            head.x += vel.x;
            head.y += vel.y;

            if (head.x < 0)
                head.x += (i32)width;
            else if (head.x >= (i32)width)
                head.x -= (i32)width;

            if (head.y < 0)
                head.y += (i32)height;
            else if (head.y >= (i32)height)
                head.y -= (i32)height;

            if (head.x == fruit.x && head.y == fruit.y)
            {
                while (head.x == fruit.x && head.y == fruit.y)
                {
                    fruit.x = (i32)(rng.next() % width);
                    fruit.y = (i32)(rng.next() % height);
                }
                snake.push(head);
            }
            else
            {
                for (u32 i = 0; i < snake.count - 1; ++i)
                {
                    if (snake[i].x == head.x && snake[i].y == head.y)
                    {
                        *this = {};
                        return State_gameOver;
                    }

                    snake[i] = snake[i + 1];
                }
                snake[snake.count - 1] = head;
            }

            layer.clear();

            for (u32 i = 0; i < snake.count; ++i)
            {
                Vec2 pos = {(f32)snake[i].x, (f32)snake[i].y};
                layer.drawRect({0, 1, 0, 1}, {pos, pos + Vec2{1}});
            }

            Vec2 pos = {(f32)fruit.x, (f32)fruit.y};
            layer.drawRect({1, 0, 0, 1}, {pos, pos + Vec2{1}});
        }

        renderer.queueLayer(layer);

        return State_game;
    }
};

struct GameOver {
    Layer2D layer{};

    GameOver() noexcept
    {
        layer.drawText(
            "Game Over",
            getDefaultFont(),
            Vec4{1},
            {
                {width / 2.0f - 8.0f, height / 4.0f},
                {INFINITY, height / 2.0f}
            },
            height / 35.0f);
    }

    State update(Renderer2D& renderer, const Window& window)
    {
        if (window.isButtonDown(Button_space))
            return State_game;

        renderer.queueLayer(layer);

        return State_gameOver;
    }
};

int main()
{
    HG_LOG("begun\n");

    HurdyGurdy hg = init().expect("Could not initialize HurdyGurdy");

    Window window = Window::create("Snake", 1200, 800).expect("Could not create window");
    Renderer2D renderer{window.imageFormat()};
    Camera camera{};

    Title title{};
    Game game{};
    GameOver gameOver{};
 
    State state = State_title;

    HG_LOG("finished init\n");

    Clock clock{};
    for (;;)
    {
beginFrame:
        f64 delta = clock.tick();
        processEvents();

        HG_LOG("processed events\n");

        if (wasQuit() || window.wasClosed())
            goto quit;

        HG_LOG("did not quit\n");

        camera.setOrthographic(width, height, (f32)window.width() / (f32)window.height());
        camera.update();

        HG_LOG("update camera\n");

        switch (state)
        {
        case State_title:
        {
            HG_LOG("state title\n");
            state = title.update(renderer, window);
            HG_LOG("updated title\n");
            if (state != State_title)
                goto beginFrame;
            HG_LOG("state remains title\n");
        } break;
        case State_game:
        {
            state = game.update(renderer, window, delta);
            if (state != State_game)
                goto beginFrame;
        } break;
        case State_gameOver:
        {
            state = gameOver.update(renderer, window);
            if (state != State_gameOver)
                goto beginFrame;
        } break;
        }

        HG_LOG("updated\n");

        Window* windows[] = {&window};
        GpuCmd* cmd = gpuBeginFrame(windows);
        if (window.imageView() != nullptr)
        {
            GpuRenderAttachment colorAttachment{};
            colorAttachment.image = window.imageView();

            GpuRenderPass pass{};
            pass.colorAttachments = {&colorAttachment, 1};

            gpuBeginRenderPass(cmd, pass);

            renderer.render(cmd, camera);

            gpuEndRenderPass(cmd);
        }
        gpuEndFrame(cmd);

        HG_LOG("renderer\n");
    }
quit:
    gpuWaitIdle();

    HG_LOG("quit\n");
}

