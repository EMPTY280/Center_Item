#include "MyImage.h"
#include "DSpriteManager.h"

#define MAX_FRAME 6

/*
* 캐릭터 스프라이트 정보에 머리(모자를 쓸 위치)의 좌표를 추가로 저장
* 
* 모자 스프라이트를 불러옴. (센터점 이용 + 1 프레임)
* 
* 캐릭터 스프라이트를 그린 후 머리 위치에 모자 스프라이트를 그리기
*/

#pragma region
Graphics *g_BackBuffer;
Graphics *g_MainBuffer;
Bitmap    *g_Bitmap;
Pen		   *g_pPen;
#pragma endregion
CMyImage g_myImage;

Rect g_aniRect[MAX_FRAME];
int     g_aniDirection = 0;
int     g_aniIndex = 0;
Point g_ptCenter[MAX_FRAME];

bool g_isCenter = false;
Point g_headPos[MAX_FRAME]; // 머리의 위치


CMyImage g_hatImage; // 모자 이미지
Rect g_hatRect; // 모자의 rect
Point g_hatCenter; // 모자의 중심정

Point characterPos = { 500, 600 };

#pragma region
DWORD g_interval = 0;

void OnUpdate(HWND hWnd, DWORD tick);
void CreateBuffer(HWND hWnd, HDC hDC);
void ReleaseBuffer(HWND hWnd, HDC hDC);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#pragma endregion
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,     int nCmdShow)
{
#pragma region
	WNDCLASS   wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
    wndclass.hCursor = NULL;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "myGame";
	
	if(RegisterClass(&wndclass) == 0)
	{
		return 0;		
	}

	RECT rc = { 0, 0, 1024, 768 };
	::AdjustWindowRect(&rc,
		WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX,
		FALSE);

	HWND hwnd = CreateWindow("myGame", "Game Window",
		WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX,
		100, 100, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInstance, NULL);

	if(hwnd == NULL)
	{
		return 0;
	}

	HDC hDC = GetDC(hwnd);


	CreateBuffer(hwnd, hDC);

	g_pPen = new Pen(Color(255, 0, 0), 1.0f);
#pragma endregion

	g_myImage.Load("./Data/Image/mushroom.png");

	int frame, center, left, top, width, height, xCenter, yCenter, xHead, yHead;

	FILE *fp = fopen("mushroom.txt", "rt");
	fscanf(fp, "%d %d", &frame, &center);
	
	for (int i = 0; i < frame; i++)
	{
		if (center == 0)
		{
			fscanf(fp, "%d %d %d %d", &left, &top, &width, &height);
			g_aniRect[i].X = left;
			g_aniRect[i].Y = top;
			g_aniRect[i].Width = width;
			g_aniRect[i].Height = height;

			g_isCenter = false;
		}
		else
		{
			fscanf(fp, "%d %d %d %d %d %d %d %d", &left, &top, &width, &height, &xCenter, &yCenter, &xHead, &yHead);
			g_aniRect[i].X = left;
			g_aniRect[i].Y = top;
			g_aniRect[i].Width = width;
			g_aniRect[i].Height = height;
			g_ptCenter[i].X = xCenter;
			g_ptCenter[i].Y = yCenter;
			g_headPos[i].X = xHead;
			g_headPos[i].Y = yHead;

			g_isCenter = true;
		}
	}
	fclose(fp);

	// 모자 스프라이트 불러오기
	g_hatImage.Load("./Data/Image/hat.png");
	fp = fopen("hat.txt", "rt");
	fscanf(fp, "%d %d %d %d %d %d", &left, &top, &width, &height, &xCenter, &yCenter);
	g_hatRect.X = left;
	g_hatRect.Y = top;
	g_hatRect.Width = width;
	g_hatRect.Height = height;
	g_hatCenter.X = xCenter;
	g_hatCenter.Y = yCenter;
	fclose(fp);

#pragma region
	ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

	MSG msg;
	DWORD tick = GetTickCount();
	while(1)
	{	
		//윈도우 메세지가 있을경우 메세지를 처리한다.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else //메세지가 없을 경우 게임 루프를 실행한다.
		{		
			DWORD curTick = GetTickCount();
			OnUpdate(hwnd, curTick - tick);
			tick = curTick;

			
			g_MainBuffer->DrawImage(g_Bitmap, 0, 0);
		}
	}

	delete g_pPen;
	ReleaseBuffer(hwnd, hDC);

	return 0;
#pragma endregion
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}


	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CreateBuffer(HWND hWnd, HDC hDC)
{
	GdiplusStartupInput			gdiplusStartupInput;
	ULONG_PTR					gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	RECT rc;
	GetClientRect(hWnd, &rc);

	g_Bitmap = new Bitmap(rc.right - rc.left, rc.bottom - rc.top);
	g_BackBuffer = new Graphics(g_Bitmap);
	g_BackBuffer->SetPageUnit(Gdiplus::Unit::UnitPixel);
	
	g_MainBuffer = new Graphics(hDC);
	g_MainBuffer->SetPageUnit(Gdiplus::Unit::UnitPixel);
}

void ReleaseBuffer(HWND hWnd, HDC hDC)
{
	ReleaseDC(hWnd, hDC);

	delete g_Bitmap;
	delete g_BackBuffer;
	delete g_MainBuffer;
}

void OnUpdate(HWND hWnd, DWORD tick)
{
	if(hWnd == NULL)
		return;

	Color color(255, 255, 255);
	g_BackBuffer->Clear(color);

	g_interval += tick;

	if (g_isCenter == false)
	{
		g_myImage.Draw(g_BackBuffer, 100, 100,
			g_aniRect[g_aniIndex].X,
			g_aniRect[g_aniIndex].Y,
			g_aniRect[g_aniIndex].Width,
			g_aniRect[g_aniIndex].Height);
	}
	else
	{
		Point center = g_ptCenter[g_aniIndex];
		center.X = characterPos.X - center.X;
		center.Y = characterPos.Y - center.Y;
		g_myImage.Draw(g_BackBuffer, center.X, center.Y,
			g_aniRect[g_aniIndex].X,
			g_aniRect[g_aniIndex].Y,
			g_aniRect[g_aniIndex].Width,
			g_aniRect[g_aniIndex].Height);

		// 모자 그리기
		Point hatCenter = g_hatCenter;
		hatCenter.X = center.X - hatCenter.X + g_headPos[g_aniIndex].X;
		hatCenter.Y = center.Y - hatCenter.Y + g_headPos[g_aniIndex].Y;

		g_hatImage.Draw(g_BackBuffer, hatCenter.X, hatCenter.Y,
			g_hatRect.X, g_hatRect.Y,
			g_hatRect.Width, g_hatRect.Height);
	}

	if (g_interval > 200)
	{
		g_interval = 0;
		g_aniIndex++;
		if (g_aniIndex >= MAX_FRAME)
			g_aniIndex = 0;
	}

}