#include <Windows.h>
#include <stdbool.h>

#define WIDTH 720
#define HEIGHT 720
#define BLOCK_COUNT 1
#define BLOCK_SIZE 48
#define GAP_SIZE 1
#define BOUNCE_DIRECTION 'y'
#define FIELD_SIZE HEIGHT
#define COLOUR_INCREMENT 1

BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER),WIDTH,HEIGHT,1,32,BI_RGB };
HWND window;
HDC WindowDC;

typedef struct {
    unsigned char b, g, r, a;
}pixel;

typedef struct {
    int x, y;
}vector2;

typedef struct {
    int x, y, vel_x, vel_y;
}entity;

enum {
    BLUE, GREEN, RED
};

pixel vram[HEIGHT * WIDTH];
entity block[BLOCK_COUNT];
char dominant_colour;
pixel colour;

int windowMessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDCLASSA window_class = { .lpfnWndProc = windowMessageHandler,.lpszClassName = "class",.lpszMenuName = "class" };
unsigned int screen_width;
unsigned int screen_height;
int tick_count;

int windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_QUIT:
    case WM_CLOSE:
        ExitProcess(0);
    }
    return DefWindowProcA(window, msg, wParam, lParam);
}

void drawrectangle(int x, int y, int width, int height, pixel colour)
{
    int tempwidth = width;

    width += x;
    height += y;

    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }

    if (width > WIDTH)
    {
        width = WIDTH - 1;
    }
    if (height > HEIGHT)
    {
        height = HEIGHT - 1;
    }
    for (; y < height; y++)
    {
        for (; x < width; x++)
        {
            vram[x + y * WIDTH] = colour;
        }
        x -= tempwidth;
    }
}

bool aabb(vector2 pos_a, vector2 pos_b, int width_a, int heigth_a, int width_b, int heigth_b)
{
    width_a /= 2;
    width_b /= 2;
    heigth_a /= 2;
    heigth_b /= 2;

    bool x1 = pos_a.x - width_a <= pos_b.x + width_b;
    bool x2 = pos_a.x + width_a >= pos_b.x - width_b;
    bool y1 = pos_a.y - heigth_a <= pos_b.y + heigth_b;
    bool y2 = pos_a.y + heigth_a >= pos_b.y - heigth_b;

    return (x1 && x2 && y1 && y2);
}

void init()
{
    memset(vram, WIDTH * HEIGHT * sizeof(pixel), 255);
    block[0] = (entity){ 0, 0, 8, 5 };
    //block[1] = (entity){ 0, HEIGHT - BLOCK_SIZE, 10, -6 };
}

void physics()
{
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        block[i].x += block[i].vel_x;
        block[i].y += block[i].vel_y;

        for (int j = 0; j < BLOCK_COUNT; j++)
        {
            if (aabb((vector2) { block[i].x, block[i].y }, (vector2) { block[j].x, block[j].y }, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE))
            {
                if (BOUNCE_DIRECTION == 'x')
                {
                    block[i].vel_x *= -1;
                    block[j].vel_x *= -1;
                    continue;
                }
                block[i].vel_y *= -1;
                block[j].vel_y *= -1;
            }
        }
        
        if (block[i].x < 0)
        {
            block[i].x = 0;
            block[i].vel_x *= -1;
        }
        if (block[i].x > FIELD_SIZE - BLOCK_SIZE)
        {
            block[i].x = FIELD_SIZE - BLOCK_SIZE;
            block[i].vel_x *= -1;
        }
        if (block[i].y < 0)
        {
            block[i].y = 0;
            block[i].vel_y *= -1;
        }
        if (block[i].y > FIELD_SIZE - BLOCK_SIZE)
        {
            block[i].y = FIELD_SIZE - BLOCK_SIZE;
            block[i].vel_y *= -1;
        }
    }
    tick_count++;
}

void set_colour()
{
    switch (dominant_colour)
    {
    case BLUE:
        colour.b -= COLOUR_INCREMENT;
        colour.g += COLOUR_INCREMENT;
        if (colour.b <= 4)
        {
            dominant_colour = GREEN;
        }
        break;
    case GREEN:
        colour.g -= COLOUR_INCREMENT;
        colour.r += COLOUR_INCREMENT;
        if (colour.g <= 4)
        {
            dominant_colour = RED;
        }
        break;
    case RED:
        colour.r -= COLOUR_INCREMENT;
        colour.b += COLOUR_INCREMENT;
        if (colour.r <= 4)
        {
            dominant_colour = BLUE;
        }
        break;
    }
}

void loop()
{
    init();
    for (;;)
    {
        physics();
        set_colour();
        for (int i = 0; i < BLOCK_COUNT; i++)
        {
            drawrectangle(block[i].x, block[i].y, BLOCK_SIZE, BLOCK_SIZE,(pixel) { 0, 0, 0 });
            drawrectangle(block[i].x + GAP_SIZE, block[i].y + GAP_SIZE, BLOCK_SIZE - GAP_SIZE * 2, BLOCK_SIZE - GAP_SIZE * 2, colour);
        }
        StretchDIBits(WindowDC, 0, 0, screen_height, screen_height, 0, 0, WIDTH, HEIGHT, vram, &bmi, 0, SRCCOPY);
        Sleep(25 - tick_count / 75 < 5 ? 5 : 25 - tick_count / 75);
    }
}

void main() 
{
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);
    RegisterClassA(&window_class);
    window = CreateWindowExA(0, "class", "hello", WS_VISIBLE | WS_POPUP, 0, 0, screen_height, screen_height, 0, 0, window_class.hInstance, 0);
    WindowDC = GetDC(window);
    MSG message;

#pragma comment(lib,"winmm")
    timeBeginPeriod(1);
    CreateThread(0, 0, loop, 0, 0, 0);

    while (GetMessageA(&message, window, 0, 0)) 
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}