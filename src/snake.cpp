#include "hurdygurdy.hpp"

using namespace hg;

static Rng rng{trueRandom()};

int main()
{
    HurdyGurdy hg = init().expect("Could not initialize HurdyGurdy");

    Window window = Window::create("Snake", 1200, 800, {}).expect("Could not create window");

    Camera camera = Camera::create();

    initRenderer2D(window.imageFormat());
    HG_DEFER(deinitRenderer2D());

    Layer2D snakeLayer = Layer2D::create();

    constexpr u32 width = 28;
    constexpr u32 height = 21;

    struct Point {
        i32 x, y;
    };

    Point head{width / 2, height / 2};
    Point vel{1, 0};

    Point fruit{(i32)(rng.next() % width), (i32)(rng.next() % height)};

    Array<Point> snake{};
    snake.push(head);

    f64 speed = 0.08f;
    f64 timeTilTick = speed;

    Clock clock{};
    for (;;)
    {
        f64 delta = clock.tick();
        processEvents();

        if (wasQuit() || window.wasClosed())
            goto quit;

        camera.setOrthographic(width, height, (f32)window.width() / (f32)window.height());
        camera.update();

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
                        goto quit;

                    snake[i] = snake[i + 1];
                }
                snake[snake.count - 1] = head;
            }

            snakeLayer.clear();

            for (u32 i = 0; i < snake.count; ++i)
            {
                Vec2 pos = {(f32)snake[i].x, (f32)snake[i].y};
                snakeLayer.drawRect({0, 1, 0, 1}, {pos, pos + Vec2{1}});
            }

            Vec2 pos = {(f32)fruit.x, (f32)fruit.y};
            snakeLayer.drawRect({1, 0, 0, 1}, {pos, pos + Vec2{1}});
        }

        Window* windows[] = {&window};
        GpuCmd* cmd = gpuFrameBegin(windows);
        if (window.imageView() != nullptr)
        {
            GpuRenderAttachment windowAttachment{};
            windowAttachment.image = window.imageView();

            GpuRenderPass pass{};
            pass.colorAttachments = {&windowAttachment, 1};

            gpuRenderPassBegin(cmd, pass);

            snakeLayer.render(cmd, &camera);

            gpuRenderPassEnd(cmd);
        }
        gpuFrameEnd(cmd);
    }
quit:
    gpuWaitIdle();
}

