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

    enum Point {
        Point_empty = 0,
        Point_snake,
        Point_fruit,
    };

    constexpr u32 width = 28;
    constexpr u32 height = 21;
    Point points[width][height]{};
    points[rng() % width][rng() % height] = Point_fruit;

    struct Snake {
        i32 x, y;
        i32 vx, vy;
    };

    Snake head{20, 14, 1, 0};

    HgQueue<Snake> snake = hgQueueCreate<Snake>();
    hgDefer(hgQueueDestroy(&snake));

    hgQueuePushBack(&snake, head);

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

        camera.type = HgCameraType_orthographic;
        camera.orthographic.left = 0;
        camera.orthographic.right = width;
        camera.orthographic.top = 0;
        camera.orthographic.bottom = height;
        camera.orthographic.near = 0;
        camera.orthographic.far = 1;

        f32 aspect = (f32)hgWindowWidth(window) / (f32)hgWindowHeight(window);
        if (aspect > (f32)width / (f32)height)
        {
            f32 margin = aspect - (f32)width / (f32)height;
            camera.orthographic.left -= margin * width / 2.0f;
            camera.orthographic.right += margin * width / 2.0f;
        }
        else
        {
            f32 margin = (f32)hgWindowHeight(window) / (f32)hgWindowWidth(window) - 3.0f / 4.0f;
            camera.orthographic.top -= margin * height / 2.0f;
            camera.orthographic.bottom += margin * height / 2.0f;
        }

        hgCameraUpdate(&camera);

        if (hgIsButtonDown(window, HgButton_w) || hgIsButtonDown(window, HgButton_up))
        {
            if (head.vy != 1)
            {
                head.vx = 0;
                head.vy = -1;
            }
        }
        else if (hgIsButtonDown(window, HgButton_a) || hgIsButtonDown(window, HgButton_left))
        {
            if (head.vx != 1)
            {
                head.vx = -1;
                head.vy = 0;
            }
        }
        else if (hgIsButtonDown(window, HgButton_s) || hgIsButtonDown(window, HgButton_down))
        {
            if (head.vy != -1)
            {
                head.vx = 0;
                head.vy = 1;
            }
        }
        else if (hgIsButtonDown(window, HgButton_d) || hgIsButtonDown(window, HgButton_right))
        {
            if (head.vx != -1)
            {
                head.vx = 1;
                head.vy = 0;
            }
        }

        timeTilTick -= delta;
        if (timeTilTick < 0)
        {
            timeTilTick += speed;

            head.x += head.vx;
            head.y += head.vy;

            if (head.x < 0)
                head.x += (i32)width;
            else if (head.x >= (i32)width)
                head.x -= (i32)width;

            if (head.y < 0)
                head.y += (i32)height;
            else if (head.y >= (i32)height)
                head.y -= (i32)height;

            Point p = points[head.x][head.y];
            if (p == Point_snake)
                goto quit;

            hgQueuePushBack(&snake, head);
            points[head.x][head.y] = Point_snake;

            if (p == Point_fruit)
            {
                u32 x, y;
                do
                {
                    x = rng() % width;
                    y = rng() % height;
                }
                while (points[x][y] == Point_snake);
                points[x][y] = Point_fruit;
            }
            else
            {
                Snake tail = hgQueuePopFront(&snake);
                points[tail.x][tail.y] = Point_empty;
            }

            hgLayerClear2D(&snakeLayer);

            for (u32 x = 0; x < width; ++x)
            {
                for (u32 y = 0; y < height; ++y)
                {
                    if (points[x][y] == Point_empty)
                    {
                        continue;
                    }
                    else if (points[x][y] == Point_snake)
                    {
                        hgDrawRect2D(&snakeLayer, HgVec4{0, 1, 0, 1}, {HgVec2{(f32)x, (f32)y}, HgVec2{1, 1}});
                    }
                    else if (points[x][y] == Point_fruit)
                    {
                        hgDrawRect2D(&snakeLayer, HgVec4{1, 0, 0, 1}, {HgVec2{(f32)x, (f32)y}, HgVec2{1, 1}});
                    }
                }
            }
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

            hgGpuSetViewport(cmd, 0, 0, (f32)hgWindowWidth(window), (f32)hgWindowHeight(window));
            hgGpuSetScissor(cmd, 0, 0, hgWindowWidth(window), hgWindowHeight(window));

            hgRenderLayer2D(cmd, &camera, &snakeLayer);

            hgGpuRenderPassEnd(cmd);

            HgGpuImageBarrier presentBarrier{};
            presentBarrier.image = hgWindowImageView(window);
            presentBarrier.nextLayout = HgGpuLayout_presentSrc;

            hgGpuMemoryBarrier(cmd, nullptr, 0, &presentBarrier, 1);
        }
        hgGpuFrameEnd(cmd);
    }
quit:
    hgGpuWaitIdle();
}

