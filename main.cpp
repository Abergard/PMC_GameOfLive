#include <windows.h>
#include <gl/gl.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

LRESULT CALLBACK WndProc(HWND hWnd,
                         UINT message,
                         WPARAM wParam,
                         LPARAM lParam);
WNDCLASSEX wc;

void EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC);
void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
void Display();
void DrawMap();
void DrawView(int start_pointX,
              int start_pointY,
              float RED,
              float GREEN,
              float BLUE,
              GLenum MODE,
              float scale);
int ShowAreaNumber(int x, int y)
{
    return ((x + 1000) / 2) * 10 + ((y - 980) / -20);
}
void randBlack();
void Clear();

unsigned int* pnTMP; // pomocnik
unsigned int* USEcolor;
unsigned int* NOTUSEcolor;
unsigned int LIFEcolor[10000];
unsigned int copyLIFEcolor[10000];

bool editionMode = true;
bool newMap = false;

////////////////////////////////////////////////////
struct ROZMIAR
{
    int x;
    int y;
} OKNO = {600, 600};
////////////////////////////////////////////////////

LPSTR NazwaKlasy = "GL tutorial";
MSG msg;
HWND g_hWnd;
HDC hDC;
HGLRC hRC;
BOOL quit = FALSE;

bool bSHIFT = false;
bool bCONTROL = false;

int defaultZoom = 40;

int selected_x = 0;
int selected_y = 0;

float camera_x = 0.0f;
float camera_y = 0.0f;
float camera_speed = 20.0f;

double ZOOM = 0;

const WORD ID_TIMER = 1;
int licznik = 50;
int time_count = 0;

int up = 0;
int up_right = 0;
int right = 0;
int down_right = 0;
int down = 0;
int down_left = 0;
int left = 0;
int up_left = 0;

////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrievInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = NazwaKlasy;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL,
                   "Rejestracja okna nie powiodla sie!",
                   "Register Error",
                   MB_OK | MB_ICONEXCLAMATION);
        return 1;
    }

    // WS_OVERLAPPEDWINDOW WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX / WS_CAPTION |
    // WS_POPUPWINDOW
    HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                               NazwaKlasy,
                               "GameOfLive",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               OKNO.x,
                               OKNO.y,
                               NULL,
                               NULL,
                               hInstance,
                               NULL);

    if (hWnd == NULL)
    {
        MessageBox(NULL,
                   "Proba utworzenia okna nie powiodla sie!",
                   "Create Error",
                   MB_ICONEXCLAMATION);
        return 1;
    }

    // timer
    if (SetTimer(hWnd, ID_TIMER, 10, NULL) == 0)
        MessageBox(hWnd, "Nie mozna utworzyc timera!", "Blad", MB_ICONSTOP);

    // rand seed
    srand(time(NULL));

    // enable OpenGL for the window
    EnableOpenGL(hWnd, &hDC, &hRC);

    // program main loop
    while (!quit)
    {
        // check for messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // handle or dispatch messages
            if (msg.message == WM_QUIT)
            {
                quit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            Display();
        }
    }
    // shutdown OpenGL
    DisableOpenGL(hWnd, hDC, hRC);
    // destroy the window explicitly
    DestroyWindow(hWnd);
    return msg.wParam;
}

void update()
{
    time_count = (time_count + 1) % licznik;
    if (editionMode == false && time_count == 0 && newMap == false)
    {
        int borderer;
        for (int i = 0; i < 10000; ++i)
        {
            borderer = 0;

            up = i;
            up_right = i;
            right = i;
            down_right = i;
            down = i;
            down_left = i;
            left = i;
            up_left = i;

            if (i % 100 == 0) // pierwsza linia od gory
            {
                up += 99;
                up_right += 99;
                up_left += 99;
                down_right += 1;
                down += 1;
                down_left += 1;
            }
            else if ((i + 1) % 100 == 0)
            {
                up -= 99;
                up_right -= 99;
                up_left -= 99;
                down_right -= 1;
                down -= 1;
                down_left -= 1;
            }
            else
            {
                up -= 1;
                up_right -= 1;
                up_left -= 1;
                down_right += 1;
                down += 1;
                down_left += 1;
            }

            // prawy sasiad
            if (i >= 9900) // prawy gorny rog
            {
                up_right -= 9900;
                right -= 9900;
                down_right -= 9900;
                down_left -= 100;
                left -= 100;
                up_left -= 100;
            }
            else if (i < 100) // lewy gorny rog
            {
                up_right += 100;
                right += 100;
                down_right += 100;
                down_left += 9900;
                left += 9900;
                up_left += 9900;
            }
            else // reszta
            {
                up_right += 100;
                right += 100;
                down_right += 100;
                down_left -= 100;
                left -= 100;
                up_left -= 100;
            }

            if (USEcolor[up] == 1) // gorny sasiad
                ++borderer;

            if (USEcolor[up_right] == 1) // prawy gorny sasiad
                ++borderer;

            if (USEcolor[right] == 1) // prawy sasiad
                ++borderer;

            if (USEcolor[down_right] == 1) // prawy dolny sasiad
                ++borderer;

            if (USEcolor[down] == 1) // dolny sasiad
                ++borderer;

            if (USEcolor[down_left] == 1) // dolny lewy sasiad
                ++borderer;

            if (USEcolor[left] == 1) // lewy sasiad
                ++borderer;

            if (USEcolor[up_left] == 1) // gorny lewy sasiad
                ++borderer;

            if (USEcolor[i] == 0)
            {
                if (borderer == 3)
                    NOTUSEcolor[i] = 1;
                else
                    NOTUSEcolor[i] = 0;
            }

            else if (borderer == 2 || borderer == 3)
                NOTUSEcolor[i] = 1;

            else
                NOTUSEcolor[i] = 0;
        }
        newMap = true;
    }
}

void updateKey(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case '1':
        if (editionMode == true)
        {
            MessageBox(NULL, "Only 2 cell will die", NULL, MB_OK);
            USEcolor[ShowAreaNumber(selected_x, selected_y)] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) + 1] = 1;
        }
        break;

    case '2':
        if (editionMode == true)
        {
            MessageBox(NULL,
                       "Cell with 2 or 3 neighbours will live to next "
                       "generation\n",
                       NULL,
                       MB_OK);
            USEcolor[ShowAreaNumber(selected_x, selected_y) - 1] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y)] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) + 100] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) + 1] = 1;
        }
        break;

    case '3':
        if (editionMode == true)
        {
            MessageBox(NULL,
                       "Cell with more than 3 neighbours will die",
                       NULL,
                       MB_OK);
            USEcolor[ShowAreaNumber(selected_x, selected_y)] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) + 1] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) - 1] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) + 100] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) - 100] = 1;
        }
        break;

    case '4':
        if (editionMode == true)
        {
            MessageBox(NULL,
                       "If empyt cell have excatly 3 live neighbours, "
                       "becomes a live cell",
                       NULL,
                       MB_OK);
            USEcolor[ShowAreaNumber(selected_x, selected_y) + 1] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) + 100] = 1;
            USEcolor[ShowAreaNumber(selected_x, selected_y) - 100] = 1;
        }
        break;

    case VK_OEM_COMMA:
        if (licznik > 1)
            --licznik;
        break;

    case VK_OEM_PERIOD:
        if (licznik < 100)
            ++licznik;
        break;

    case VK_ESCAPE:
        PostQuitMessage(0);
        break;

    case VK_RETURN:
        if (editionMode == true)
            randBlack();
        break;

    case VK_BACK:
        if (editionMode == true)
            Clear();
        break;

    case VK_SHIFT:
        bSHIFT = true;
        break;

    case VK_NUMPAD0:
        if (editionMode)
            USEcolor[ShowAreaNumber(selected_x, selected_y)] = 0;
        break;

    case VK_NUMPAD5:
        if (editionMode)
            USEcolor[ShowAreaNumber(selected_x, selected_y)] = 1;
        break;

    case VK_UP:
        if (bSHIFT)
        {
            camera_y -= camera_speed;
        }
        else
        {
            int tmp_y = selected_y;
            selected_y = (selected_y + 1020) % 2000 - 1000;
            if (selected_y == (ZOOM + defaultZoom - camera_y) ||
                (selected_y * tmp_y) < 0)
            {
                camera_y = -1 * selected_y;
            }
        }
        break;

    case VK_DOWN:
        if (bSHIFT)
        {
            camera_y += camera_speed;
        }
        else
        {
            int tmp_y = selected_y;
            selected_y = (selected_y + -1000) % 2000 + 980;
            if (selected_y == (-ZOOM - defaultZoom - 20 - camera_y) ||
                (selected_y * tmp_y) < 0)
            {
                camera_y = -1 * selected_y;
            }
        }
        break;

    case VK_LEFT:
        if (bSHIFT)
        {
            camera_x += camera_speed;
        }
        else
        {
            int tmp_x = selected_x;
            selected_x = (selected_x + -1000) % 2000 + 980;
            if (selected_x == (-ZOOM - defaultZoom - 20 - camera_x) ||
                (selected_x * tmp_x) < 0)
            {
                camera_x = -1 * selected_x;
            }
        }
        break;

    case VK_RIGHT:
        if (bSHIFT)
        {
            camera_x -= camera_speed;
        }
        else
        {
            int tmp_x = selected_x;
            selected_x = (selected_x + 1020) % 2000 - 1000;
            if (selected_x == (ZOOM + defaultZoom - camera_x) ||
                (selected_x * tmp_x) < 0)
            {
                camera_x = -1 * selected_x;
            }
        }
        break;

    case VK_ADD:
        if (bCONTROL)
        {
            if (ZOOM > -20)
                ZOOM -= 20;
        }
        break;

    case VK_SUBTRACT:
        if (bCONTROL)
        {
            if (ZOOM < 1000)
                ZOOM += 20;
        }
        break;

    case VK_CONTROL:
        bCONTROL = true;
        break;

    case VK_SPACE:
        editionMode = !editionMode;
        break;
    }
    PostMessage(hWnd, WM_SIZE, wParam, lParam);
}
////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        PostMessage(hWnd, WM_SIZE, wParam, lParam);
        return 0;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_TIMER:
        update();
        return 0;

    case WM_KEYDOWN:
        updateKey(hWnd, message, wParam, lParam);
        return 0;

    case WM_SIZE:
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(-defaultZoom - ZOOM,
                defaultZoom + ZOOM,
                -defaultZoom - ZOOM,
                defaultZoom + ZOOM,
                -1,
                1);
        glTranslatef(camera_x, camera_y, 0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        return 0;

    case WM_KEYUP:
        switch (wParam)
        {
        case VK_SHIFT:
            bSHIFT = false;
            break;

        case VK_CONTROL:
            bCONTROL = false;
            break;
        }
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

////////////////////////////////////////////////////
// Enable OpenGL
void EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int format;

    // get the device context (DC)
    *hDC = GetDC(hWnd);

    // set the pixel format for the DC
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    format = ChoosePixelFormat(*hDC, &pfd);
    SetPixelFormat(*hDC, format, &pfd);

    // create and enable the render context (RC)
    *hRC = wglCreateContext(*hDC);
    wglMakeCurrent(*hDC, *hRC);

    for (int i = 0; i < 10000; ++i)
    {
        LIFEcolor[i] = 0;
        copyLIFEcolor[i] = 0;
    }

    USEcolor = LIFEcolor;
    NOTUSEcolor = copyLIFEcolor;

    glPointSize(1.0f);
    glLineWidth(2.0f);
}

////////////////////////////////////////////////////
// Disable OpenGL
void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    USEcolor = NULL;
    NOTUSEcolor = NULL;
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

////////////////////////////////////////////////////
void Display()
{
    if (newMap == true)
    {
        pnTMP = USEcolor;
        USEcolor = NOTUSEcolor;
        NOTUSEcolor = pnTMP;
        pnTMP = NULL;
        newMap = false;
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    DrawMap();

    if (editionMode)
        DrawView(selected_x, selected_y, 0, 0, 1, GL_LINE, 0);
    else
        DrawView(selected_x, selected_y, 1, 0, 0, GL_LINE, 0);

    glBegin(GL_POINTS);
    glColor3d(0.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();

    SwapBuffers(hDC);
}

////////////////////////////////////////////////////
void DrawMap()
{
    int nrArea = 0;
    for (int x = -1000; x < 1000; x += 20)
    {
        for (int y = 980; y >= -1000; y -= 20)
        {
            if (USEcolor[nrArea] == 0)
                DrawView(x, y, 0.9f, 0.9f, 0.9f, GL_LINE, 0.6);
            else
            {
                float color;
                if (USEcolor[nrArea] == 1)
                    color = 0;

                DrawView(x, y, color, color, color, GL_FILL, 0.6);
            }
            ++nrArea;
        }
    }
}

////////////////////////////////////////////////////
void DrawView(int start_pointX,
              int start_pointY,
              float RED,
              float GREEN,
              float BLUE,
              GLenum MODE,
              float scale)
{
    glPolygonMode(GL_FRONT_AND_BACK, MODE);
    glBegin(GL_QUAD_STRIP);
    glColor3f(RED, GREEN, BLUE);
    glVertex2f(start_pointX + scale, start_pointY + scale);
    glVertex2f(start_pointX - scale + 20.0f, start_pointY + scale);
    glVertex2f(start_pointX + scale, start_pointY - scale + 20.0f);
    glVertex2f(start_pointX - scale + 20.0f, start_pointY - scale + 20.0f);

    glEnd();
}

////////////////////////////////////////////////////
void randBlack()
{
    for (int i = 0; i < 10000; ++i)
    {
        if ((rand() % 6 + 1 * rand()) % 10 == 3)
            USEcolor[i] = 1;
    }
}

////////////////////////////////////////////////////
void Clear()
{
    for (int i = 0; i < 10000; ++i)
    {
        USEcolor[i] = 0;
    }
}
