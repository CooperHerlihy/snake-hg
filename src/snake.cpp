#include "hurdygurdy.hpp"

#include <random>

static u32 seed = std::random_device{}();
static u32 pos = 0;

static u32 rng() {
    return pos = hgNoise(seed, pos);
}

int main()
{
    hgInit();
    hgDefer(hgDeinit());

    HgWindow* window = hgWindowCreate("Snake", 1200, 800, nullptr);
    hgDefer(hgWindowDestroy(window));

    HgCamera camera = hgCameraCreate();
    hgDefer(hgCameraDestroy(&camera));

    hgRendererInit2D(hgWindowImageFormat(window));
    hgDefer(hgRendererDeinit2D());

    HgLayer2D snakeLayer = hgLayerCreate2D();
    hgDefer(hgLayerDestroy2D(&snakeLayer));

    constexpr u32 width = 28;
    constexpr u32 height = 21;

    struct Point {
        i32 x, y;
    };

    Point head{width / 2, height / 2};
    Point vel{1, 0};

    Point fruit{(i32)(rng() % width), (i32)(rng() % height)};

    HgArray<Point> snake = hgArrayCreate<Point>();
    hgDefer(hgArrayDestroy(&snake));

    *hgArrayPush(&snake) = head;

    f64 speed = 0.08f;
    f64 timeTilTick = speed;

    HgClock clock{};
    hgClockTick(&clock);
    for (;;)
    {
        f64 delta = hgClockTick(&clock);
        hgProcessEvents();

        if (hgWasQuit() || hgWindowWasClosed(window))
            goto quit;

        hgCameraSetOrthographic(&camera, width, height, (f32)hgWindowWidth(window) / (f32)hgWindowHeight(window));
        hgCameraUpdate(&camera);

        if (hgIsButtonDown(window, HgButton_w) || hgIsButtonDown(window, HgButton_up))
        {
            if (vel.y != 1)
            {
                vel.x = 0;
                vel.y = -1;
            }
        }
        else if (hgIsButtonDown(window, HgButton_a) || hgIsButtonDown(window, HgButton_left))
        {
            if (vel.x != 1)
            {
                vel.x = -1;
                vel.y = 0;
            }
        }
        else if (hgIsButtonDown(window, HgButton_s) || hgIsButtonDown(window, HgButton_down))
        {
            if (vel.y != -1)
            {
                vel.x = 0;
                vel.y = 1;
            }
        }
        else if (hgIsButtonDown(window, HgButton_d) || hgIsButtonDown(window, HgButton_right))
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
                    fruit.x = (i32)(rng() % width);
                    fruit.y = (i32)(rng() % height);
                }

                *hgArrayPush(&snake) = head;
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

            hgLayerClear2D(&snakeLayer);

            for (u32 i = 0; i < snake.count; ++i)
            {
                hgDrawRect2D(&snakeLayer, HgVec4{0, 1, 0, 1}, {HgVec2{(f32)snake[i].x, (f32)snake[i].y}, HgVec2{1, 1}});
            }

            hgDrawRect2D(&snakeLayer, HgVec4{1, 0, 0, 1}, {HgVec2{(f32)fruit.x, (f32)fruit.y}, HgVec2{1, 1}});
        }

        HgGpuCmd* cmd = hgGpuFrameBegin(&window, 1);
        if (hgWindowImageView(window) != nullptr)
        {
            HgGpuRenderAttachment windowAttachment{};
            windowAttachment.image = hgWindowImageView(window);

            HgGpuRenderPass pass{};
            pass.colorAttachments = &windowAttachment;
            pass.colorAttachmentCount = 1;

            hgGpuRenderPassBegin(cmd, &pass);

            hgRenderLayer2D(cmd, &camera, &snakeLayer);

            hgGpuRenderPassEnd(cmd);
        }
        hgGpuFrameEnd(cmd);
    }
quit:
    hgGpuWaitIdle();
}

