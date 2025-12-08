// WindowsProject10.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "framework.h"
#include "WindowsProject10.h"
#include <cstdlib>      
#include <ctime>        /// rand(), srand(), time() 랜덤 함수
#include <vector>       /// 여러 개의 공 동적 관리
#include <cmath>        /// sqrt, fabs 등 수학 함수(제곱근, 절댓값) 사용 위해
using namespace std;

#define MAX_LOADSTRING 100

void SpawnNewBall();    /// 미리 함수 프로토타입 선언

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int); 
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT10, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT10));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT10));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT10);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    srand((unsigned)time(NULL));   /// 랜덤 초기화 . 랜덤 시드를 현재 시간으로 설정 . 실행할 때마다 매번 다른 랜덤 값이 나오도록
    SpawnNewBall();                /// 첫 공 생성 -> currentBall에 저장됨

    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//

/// ===== 전역 게임 변수 =====

/// 박스 영역
int boxLeft = 50;
int boxTop = 50;
int boxRight = 350;
int boxBottom = 550;

float gravity = 1.5f; /// 중력

struct Ball
{
    int type;       /// 1~11번 공 종류
    float x, y;     /// 공 중심
    float vx, vy;   /// 공 속도
    float radius;   /// 공 반지름
    bool isDropping;    /// 드롭(space)햇는가? false > 위에서 좌우 조종 중  true > 떨어져서 물리 연산 대상
    COLORREF color;     /// 공 색깔
};


/// 각 단계별 반지름

int mergeRadius[12] = {
    0,
    10, 15, 20,     // 1~3번
    30, 35, 40,     // 4~6번
    50, 55, 60,     // 7~9번
    65, 70          // 10~11번
};

COLORREF mergeColor[12] = {
    0,
    RGB(150,0,200),     //1 보라색
    RGB(255,0,0),       //2 빨간색
    RGB(255,140,0),     //3 주황색
    RGB(255,255,120),   //4 밝은 노란색
    RGB(200,255,200),   //5 옅은 초록색
    RGB(255,120,120),   //6 밝은 빨간색
    RGB(255,150,200),   //7 분홍색
    RGB(220,200,50),    //8 어두운 노란색
    RGB(200,180,120),   //9 밝은 갈색
    RGB(0,200,0),       //10 초록색
    RGB(0,120,0)        //11 어두운 초록색
};

vector<Ball> balls;   /// 내려온(굳혀진) 모든 공들 / 스페이스바로 떨어진 뒤 박스 안에 쌓이는 모든 공 저장하는 vector
Ball currentBall;     /// 현재 조종 중인 공 / 스페이스바 누르면 ball에 저장하고 새 공 한 개 생성

HDC hMemDC;         /// 메모리 DC
HBITMAP hBitmap;    /// 에 붙는 비트맵
RECT rt;            /// 창 클라이언트 영역 크기


// ==============================
//  새 공 생성 함수
// ==============================
void SpawnNewBall()     /// 새 공 생성
{
    int sizes[3] = { 10, 15, 20 };      /// 10. 15, 20 중 랜덤
    int idx = rand() % 3;
    int r = sizes[idx];

    currentBall.type = idx + 1; // 1, 2, 3번 공
    currentBall.radius = r;
    currentBall.x = (boxLeft + boxRight) / 2;
    currentBall.y = boxTop + r + 10;
    currentBall.vx = 0;
    currentBall.vy = 0;
    currentBall.isDropping = false;

    if (currentBall.type == 1) currentBall.color = RGB(150, 0, 200);
    if (currentBall.type == 2) currentBall.color = RGB(255, 0, 0);
    if (currentBall.type == 3) currentBall.color = RGB(255, 140, 0);
}



// ==============================
//  Update
// ==============================
void Update()
{
    // 1) 이미 떨어져서 쌓인 공들 처리
    for (auto& b : balls)
    {
        if (b.isDropping)
        {
            b.vy += gravity;
            b.y += b.vy;

            float bottom = boxBottom - b.radius;
            if (b.y > bottom)
            {
                b.y = bottom;
                b.vy *= -0.5f;
                if (fabs(b.vy) < 0.5f) b.vy = 0;
            }
        }

        // ===== 좌우 벽 충돌 처리 추가 =====
        if (b.x - b.radius < boxLeft)
            b.x = boxLeft + b.radius;

        if (b.x + b.radius > boxRight)
            b.x = boxRight - b.radius;

        // (필요하다면) 박스 위쪽도 막음
        if (b.y - b.radius < boxTop)
            b.y = boxTop + b.radius;
    }

    // 2) 공들끼리 충돌 처리 (강한 안정화 버전)
    for (int i = 0; i < balls.size(); i++)
    {
        for (int j = i + 1; j < balls.size(); j++)
        {
            Ball& A = balls[i];
            Ball& B = balls[j];

            float dx = B.x - A.x;
            float dy = B.y - A.y;
            float dist = sqrt(dx * dx + dy * dy);
            float minDist = A.radius + B.radius;

            if (dist < minDist)
            {
                if (dist < 0.0001f) dist = 0.0001f; // 보호

                float overlap = minDist - dist;

                // 충돌 방향 단위 벡터
                float nx = dx / dist;
                float ny = dy / dist;

                // ===== 겹침을 100% 제거 =====
                float push = overlap * 1.0f;

                // 너무 많이 흔들리지 않게 밑에 있는 공은 덜 움직이게 (노란색 ★ 중요)
                float A_weight = 0.3f;
                float B_weight = 0.7f;

                A.x -= nx * push * A_weight;
                A.y -= ny * push * A_weight;

                B.x += nx * push * B_weight;
                B.y += ny * push * B_weight;

                // ===== 속도 감쇠 — 딱딱하고 안정적인 느낌 =====
                A.vx *= 0.1f;
                A.vy *= 0.1f;
                B.vx *= 0.1f;
                B.vy *= 0.1f;

                // ===== 거의 멈추면 고정 =====
                if (fabs(A.vy) < 0.1f) A.vy = 0;
                if (fabs(B.vy) < 0.1f) B.vy = 0;
            }
        }
    }


    // ====== 공끼리 합쳐지는 로직 ======
    for (int i = 0; i < balls.size(); i++)
    {
        for (int j = i + 1; j < balls.size(); j++)
        {
            Ball& A = balls[i];
            Ball& B = balls[j];

            float dx = B.x - A.x;
            float dy = B.y - A.y;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < A.radius + B.radius)
            {
                // 같은 번호의 공만 합쳐짐
                if (A.type == B.type)
                {
                    int newType = A.type + 1;
                    if (newType > 11) newType = 11;

                    // 새로운 공 생성
                    Ball newBall;
                    newBall.type = newType;
                    newBall.radius = mergeRadius[newType];
                    newBall.color = mergeColor[newType];
                    newBall.x = (A.x + B.x) / 2;
                    newBall.y = (A.y + B.y) / 2;
                    newBall.vx = 0;
                    newBall.vy = 0;
                    newBall.isDropping = true;

                    // 기존 둘 제거
                    balls.erase(balls.begin() + j);
                    balls.erase(balls.begin() + i);

                    // 새 공 삽입
                    balls.push_back(newBall);

                    // 반드시 i-- 필요
                    i--;

                    break;
                }
            }
        }
    }


    // 3) 현재 조종 가능한 공 처리
    if (!currentBall.isDropping)
    {
        currentBall.x += currentBall.vx;

        if (currentBall.x - currentBall.radius < boxLeft)
            currentBall.x = boxLeft + currentBall.radius;

        if (currentBall.x + currentBall.radius > boxRight)
            currentBall.x = boxRight - currentBall.radius;
    }
    else
    {
        currentBall.vy += gravity;
        currentBall.y += currentBall.vy;

        float bottom = boxBottom - currentBall.radius;
        if (currentBall.y > bottom)
        {
            currentBall.y = bottom;
            currentBall.vy *= -0.5f;
            if (fabs(currentBall.vy) < 0.5f) currentBall.vy = 0;
        }
    }
}


// ==============================
//  Render
// ==============================
void Render(HDC hdc, HWND hWnd)
{
    GetClientRect(hWnd, &rt);

    if (hMemDC == NULL)
    {
        hMemDC = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
        SelectObject(hMemDC, hBitmap);
    }

    // (1) 전체 배경을 흰색으로 칠하기
    HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hMemDC, &rt, bg);
    DeleteObject(bg);

    // (2) 박스(네모칸) 테두리만 그리기
    HPEN boxPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(hMemDC, boxPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hMemDC, GetStockObject(NULL_BRUSH));

    Rectangle(hMemDC, boxLeft, boxTop, boxRight, boxBottom);

    SelectObject(hMemDC, oldBrush);
    SelectObject(hMemDC, oldPen);
    DeleteObject(boxPen);

    // 1) 떨어진 공들 그리기
    for (auto& b : balls)
    {
        HBRUSH br = CreateSolidBrush(b.color);
        HBRUSH old = (HBRUSH)SelectObject(hMemDC, br);

        Ellipse(hMemDC,
            b.x - b.radius, b.y - b.radius,
            b.x + b.radius, b.y + b.radius
        );

        SelectObject(hMemDC, old);
        DeleteObject(br);
    }

    // 2) 현재 공 그리기
    HBRUSH br = CreateSolidBrush(currentBall.color);
    HBRUSH old = (HBRUSH)SelectObject(hMemDC, br);

    Ellipse(hMemDC,
        currentBall.x - currentBall.radius, currentBall.y - currentBall.radius,
        currentBall.x + currentBall.radius, currentBall.y + currentBall.radius
    );

    SelectObject(hMemDC, old);
    DeleteObject(br);

    // (추가됨) 더블버퍼 → 실제 화면 복사
    BitBlt(hdc, 0, 0, rt.right, rt.bottom, hMemDC, 0, 0, SRCCOPY);
}



// ==============================
//  WndProc
// ==============================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 16, NULL); // 60fps 타이머
        break;

    case WM_TIMER:
        Update();
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT:
            if (!currentBall.isDropping)
                currentBall.vx = -5;
            break;

        case VK_RIGHT:
            if (!currentBall.isDropping)
                currentBall.vx = 5;
            break;

        case VK_SPACE:
            if (!currentBall.isDropping)
            {
                currentBall.isDropping = true;

                // 떨어지는 공을 리스트에 넣음
                balls.push_back(currentBall);

                // 새 공 즉시 생성
                SpawnNewBall();
            }
            break;
        }
        break;

    case WM_KEYUP:  // ← 기존 위치가 잘못되어 있었음 → 올바르게 분리됨
        switch (wParam)
        {
        case VK_LEFT:
        case VK_RIGHT:
            if (!currentBall.isDropping)
                currentBall.vx = 0;
            break;
        }
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Render(hdc, hWnd);

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



// ==============================
//  About Dialog
// ==============================
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
