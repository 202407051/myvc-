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

/// 박스 영역
int boxLeft = 50;
int boxTop = 50;
int boxRight = 350;
int boxBottom = 550;

int floorHeight = 30;
int floorTop = boxBottom;
int floorBottom = boxBottom + floorHeight;

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

    // 원하는 클라이언트 영역 크기
    RECT rc = { 0, 0, boxRight + 100, boxBottom + 50}; // +여유
    DWORD style = WS_OVERLAPPEDWINDOW;

    // 이 스타일에 맞도록 전체 창 크기 계산
    AdjustWindowRectEx(&rc, style, TRUE, 0);

    // 계산된 값 → 창 크기
    int winW = rc.right - rc.left;
    int winH = rc.bottom - rc.top;

    // 계산된 크기로 생성
    HWND hWnd = CreateWindowW(
        szWindowClass,
        szTitle,
        style,
        CW_USEDEFAULT,
        0,
        winW,       // 계산된 너비
        winH,       // 계산된 높이
        nullptr, nullptr, hInstance, nullptr
    );


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


bool showLine = false;         // 기준선 보이기 여부
bool isGameOver = false;       // 게임 오버 상태

int lineY;                     // WM_SIZE나 InitInstance에서 계산

int score = 0; /// 점수 변수
int GetScoreFor(int type)   /// 점수 계산 함수
{
    return type * (type + 1) / 2;
}

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
Ball nextBall;

HDC hMemDC;         /// 메모리 DC
HBITMAP hBitmap;    /// 에 붙는 비트맵
RECT rt;            /// 창 클라이언트 영역 크기


// 랜덤 공 하나를 만들어 주는 공통 함수 (currentBall, nextBall 둘 다 여기 사용)
void MakeRandomBall(Ball& b)
{
    int sizes[3] = { 10, 15, 20 };      /// 10. 15, 20 중 랜덤
    int idx = rand() % 3; /// 0, 1, 2
    int r = sizes[idx]; /// r= 10, 15, 20 중 하나

    b.type = idx + 1; /// 1, 2, 3번 공
    b.radius = r; /// 반지름
    b.x = (boxLeft + boxRight) / 2;   /// x위치: 박스 중앙
    b.y = boxTop + r + 10;            /// y위치: 위쪽에서 살짝 내려
    b.vx = 0;
    b.vy = 0;
    b.isDropping = false;             /// drop되지 않음

    if (b.type == 1) b.color = RGB(150, 0, 200);    /// 색깔 지정
    if (b.type == 2) b.color = RGB(255, 0, 0);
    if (b.type == 3) b.color = RGB(255, 140, 0);
}


void SpawnNewBall()     /// 새 공 생성
{
    static bool first = true;   /// 첫 호출인지 여부

    if (first)
    {
        /// 처음 한 번은 currentBall, nextBall 둘 다 랜덤 생성
        MakeRandomBall(currentBall);
        MakeRandomBall(nextBall);
        first = false;
    }
    else
    {
        /// 그 다음부터는 nextBall을 currentBall로 가져오고,
        /// 새로운 nextBall을 하나 더 만든다.
        currentBall = nextBall;

        /// currentBall 위치는 항상 박스 위쪽 중앙에서 시작
        currentBall.x = (boxLeft + boxRight) / 2;
        currentBall.y = boxTop + currentBall.radius + 10;
        currentBall.vx = 0;
        currentBall.vy = 0;
        currentBall.isDropping = false;

        /// 새로 떨어질 다음 공 생성
        MakeRandomBall(nextBall);
    }
}


/// *** 물리 연산, 충돌, 합체, 현재 공 처리 *** ///
void Update()
{
    if (isGameOver)
        return;

    // 1) '이미 떨어져서 쌓인 공들' 처리
    for (auto& b : balls)
    {
        if (b.isDropping)   /// 떨어졌을 경우
        {
            b.vy += gravity;    /// 매 프레임 중력 적용
            b.y += b.vy;    /// 속도만큼 위치(y) 이동

            float bottom = boxBottom - b.radius;    /// 박스 바닥에 딱 닿는 위치
            if (b.y > bottom)
            {
                b.y = bottom;   /// y를 바닥으로 고정
                b.vy *= -0.2f;  /// 작게 통통 튀기
                if (fabs(b.vy) < 0.5f) b.vy = 0;    /// 속도 <0.5f 면 0으로 만들어 멈춤
            }
        }

        for (int iter = 0; iter < 10; iter++)
        {
            for (auto& b : balls)
            {
                if (b.x - b.radius < boxLeft)
                    b.x = boxLeft + b.radius;
                if (b.x + b.radius > boxRight)
                    b.x = boxRight - b.radius;
                if (b.y + b.radius > boxBottom)
                    b.y = boxBottom - b.radius;
                if (b.y - b.radius < boxTop)
                    b.y = boxTop + b.radius;
            }
        }
    }

    // 2) 공들끼리 충돌 처리
    for (int iter = 0; iter < 10; iter++)   // ⭐⭐ 반복 안정화
    {
        for (int i = 0; i < balls.size(); i++)
        {
            for (int j = i + 1; j < balls.size(); j++)
            {
                Ball& A = balls[i];
                Ball& B = balls[j];

                float dx = B.x - A.x;
                float dy = B.y - A.y;
                float dist = sqrt(dx * dx + dy * dy);
                float minDist = A.radius + B.radius;    /// 반지름 합

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

            if (dist < A.radius + B.radius + 0.5) /// 두 공이 붙어 있고
            {
                if (A.type == B.type)   /// 같은 번호의 공이면 합체
                {
                    int newType = A.type + 1;   /// 새 공: 한 단계 더 큰 공
                    if (newType > 11) newType = 11; /// 최대 11단계

                    score += GetScoreFor(newType); /// 점수 증가

                    // 새로운 공 생성
                    Ball newBall;
                    newBall.type = newType;
                    newBall.radius = mergeRadius[newType];
                    newBall.color = mergeColor[newType];
                    newBall.x = (A.x + B.x) / 2;    /// 위치: A와 B 중간
                    newBall.y = (A.y + B.y) / 2;
                    newBall.vx = 0;
                    newBall.vy = 0;
                    newBall.isDropping = true;

                    // 기존 둘 제거
                    balls.erase(balls.begin() + j);
                    balls.erase(balls.begin() + i);

                    // 새 공 삽입
                    balls.push_back(newBall);

                    // 반드시 i-- 필요: 다음 루프에서 건너뛰지 않게
                    i--;

                    break;
                }
            }
        }
    }


    // 3) 현재 조종 가능한 공 처리
    if (!currentBall.isDropping)    /// 박스 밖으로 못 나가게 체크
    {
        currentBall.x += currentBall.vx;

        if (currentBall.x - currentBall.radius < boxLeft)
            currentBall.x = boxLeft + currentBall.radius;

        if (currentBall.x + currentBall.radius > boxRight)
            currentBall.x = boxRight - currentBall.radius;
    }
    else    /// 없어도? 되긴 해
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

    /// === 기준선 표시 조건 ===

    // 쌓인 공의 가장 위 y 구하기
    float highestY = boxBottom;

    for (auto& b : balls)
    {
        // ⭐ 떨어지는 중이거나, 아직 흔들리는 공은 제외!
        if (fabs(b.vy) > 0.1f)
            continue;

        if (b.y - b.radius < highestY)
            highestY = b.y - b.radius;
    }

    // 기준선 보이기 시작 조건 (단 한 번만)
    {
        float thresholdY = boxTop + (boxBottom - boxTop) * 0.3f;  /// 전체 높이의 30% 지점

        if (highestY < thresholdY)
            showLine = true;      /// 이제부터 기준선 보임
        else
            showLine = false;
    }

    // === 게임 오버 조건 ===
    // ⭐ showLine이 true이고, 안정된 공이 기준선 넘어갔을 때만
    if (showLine && !isGameOver && (highestY < lineY))
    {
        isGameOver = true;
    }
}


// ==============================
//  Render
// ==============================
void Render(HDC hdc, HWND hWnd)
{
    GetClientRect(hWnd, &rt);

    if (hMemDC == NULL)     /// 더블 버퍼링 > 깜빡임 줄이기
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

    // 바닥(추가된 공간) 그리기
    int floorHeight = 20;
    RECT floorRect = { boxLeft, boxBottom, boxRight -1, boxBottom + floorHeight };

    // 바닥 채우기 (흰색)
    HBRUSH floorBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hMemDC, &floorRect, floorBrush);
    DeleteObject(floorBrush);

    HPEN thickPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));  // 두께 6
    HPEN oldPen2 = (HPEN)SelectObject(hMemDC, thickPen);

    MoveToEx(hMemDC, floorRect.left, floorRect.top, NULL);
    LineTo(hMemDC, floorRect.right, floorRect.top);
    LineTo(hMemDC, floorRect.right, floorRect.bottom);
    LineTo(hMemDC, floorRect.left, floorRect.bottom);
    LineTo(hMemDC, floorRect.left, floorRect.top);

    SelectObject(hMemDC, oldPen2);
    DeleteObject(thickPen);

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

    /// 2-1) 다음 공 미리보기 (사각 박스 오른쪽)
    {
        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(0, 0, 0));

        RECT nextLabel = { boxRight + 20, boxTop + 5, boxRight + 60, boxTop + 20 };
        DrawText(hMemDC, L"NEXT", -1, &nextLabel,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        int px = boxRight + 40;      /// 미리보기 중심 x 위치 (박스 오른쪽 여유 공간)
        int py = boxTop + 50;        /// 미리보기 중심 y 위치

        HBRUSH prevBr = CreateSolidBrush(nextBall.color);
        HBRUSH prevOld = (HBRUSH)SelectObject(hMemDC, prevBr);

        Ellipse(hMemDC,
            px - nextBall.radius, py - nextBall.radius,
            px + nextBall.radius, py + nextBall.radius
        );

        SelectObject(hMemDC, prevOld);
        DeleteObject(prevBr);
    }


    if (showLine && !isGameOver)
    {
        HPEN pen = CreatePen(PS_DOT, 2, RGB(255, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hMemDC, pen);

        MoveToEx(hMemDC, boxLeft, lineY, NULL);
        LineTo(hMemDC, boxRight, lineY);

        SelectObject(hMemDC, oldPen);
        DeleteObject(pen);
    }

    // 점수 출력 (우상단)
    SetBkMode(hMemDC, TRANSPARENT);     // 글자 배경 투명
    SetTextColor(hMemDC, RGB(0, 0, 0));   // 글자 검정
    HFONT hFont = CreateFont(
        28, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        HANGUL_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        L"맑은 고딕"
    );
    HFONT oldF = (HFONT)SelectObject(hMemDC, hFont);

    // 점수를 문자열로 변환
    wchar_t scoreText[32];
    wsprintf(scoreText, L"%d", score);

    // 사각형 내부 우상단 영역
    RECT scoreRect = { boxRight - 70, boxTop + 10, boxRight - 10, boxTop + 40 };

    // 중앙 정렬
    DrawText(hMemDC, scoreText, -1, &scoreRect,
        DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hMemDC, oldF);
    DeleteObject(hFont);

    if (isGameOver)
    {
        RECT popup;
        popup.left = boxLeft + 20;
        popup.right = boxRight - 20;
        popup.top = boxTop + 200;
        popup.bottom = boxTop + 350;

        // 버튼 영역
        RECT restartBtn;
        restartBtn.left = popup.left + 40;    // ★ 수정
        restartBtn.right = popup.right - 40;  // ★ 수정
        restartBtn.top = popup.bottom - 60;   // ★ 수정
        restartBtn.bottom = popup.bottom - 20;// ★ 수정

        // 버튼 박스 그리기
        FrameRect(hMemDC, &restartBtn, (HBRUSH)GetStockObject(BLACK_BRUSH));

        // 텍스트
        DrawText(hMemDC, L"[ restart ]", -1, &restartBtn,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        HBRUSH br = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hMemDC, &popup, br);
        DeleteObject(br);

        // 테두리
        FrameRect(hMemDC, &popup, (HBRUSH)GetStockObject(BLACK_BRUSH));

        // 텍스트
        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(0, 0, 0));

        RECT text;
        text = popup;

        text.top += 20;

        DrawText(hMemDC, L"GAME OVER", -1, &text,
            DT_CENTER | DT_TOP | DT_SINGLELINE);

        WCHAR scoreMsg[64];
        swprintf(scoreMsg, 64, L"score: %d", score);

        text.top += 50;

        DrawText(hMemDC, scoreMsg, -1, &text,
            DT_CENTER | DT_TOP | DT_SINGLELINE);

        text.top += 45;

        DrawText(hMemDC, L"[ restart ]", -1, &text,
            DT_CENTER | DT_TOP | DT_SINGLELINE);
    }

    // 더블버퍼 → 실제 화면 복사
    BitBlt(hdc, 0, 0, rt.right, rt.bottom, hMemDC, 0, 0, SRCCOPY);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 16, NULL); // 60fps 타이머
        lineY = boxTop + 60;
        break;

    case WM_TIMER:
        Update();   /// 타이머 울릴 때마다 update(물리연산) 계산
        InvalidateRect(hWnd, NULL, FALSE);  /// 화면 갱신 요청
        break;

    case WM_KEYDOWN:
        if (isGameOver) break;
        switch (wParam)
        {
        case VK_LEFT:
            if (!currentBall.isDropping)    /// isDropping = false
                currentBall.vx = -5;    /// 왼쪽으로 이동
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

    case WM_KEYUP:  /// 키에서 손을 떼면 좌우 이동 멈춤
        if (isGameOver) break;
        switch (wParam)
        {
        case VK_LEFT:
        case VK_RIGHT:
            if (!currentBall.isDropping)
                currentBall.vx = 0;
            break;
        }
        break;

    case WM_LBUTTONDOWN:
    {
        if (isGameOver)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            RECT restartBtn;
            restartBtn.left = boxLeft + 40;      // ★ Render와 동일 좌표
            restartBtn.right = boxRight - 40;
            restartBtn.top = boxTop + 290;
            restartBtn.bottom = boxTop + 330;

            // ★ restart 버튼 영역 클릭만 재시작
            if (x > restartBtn.left && x < restartBtn.right &&
                y > restartBtn.top && y < restartBtn.bottom)
            {
                balls.clear();
                score = 0;
                isGameOver = false;
                showLine = false;
                SpawnNewBall();
            }
        }
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
