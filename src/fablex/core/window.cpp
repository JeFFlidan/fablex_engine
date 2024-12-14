#include "window.h"
#include <winuser.h>

namespace fe
{

LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            // Setup user value
            LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            Window* window = reinterpret_cast<Window*>(lpCreateStruct->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            break;
        }
        case WM_DESTROY:
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::init(const WindowCreateInfo& createInfo)
{
    m_windowInfo.width = createInfo.width;
    m_windowInfo.height = createInfo.height;

#ifdef WIN32
    m_win32WindowData.className = "Fablex Window";
    m_win32WindowData.hInstance = GetModuleHandle(nullptr);

    WNDCLASS wndClass{};
    wndClass.lpszClassName = m_win32WindowData.className;
    wndClass.hInstance = m_win32WindowData.hInstance;
    wndClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.lpfnWndProc = window_proc;

    RegisterClass(&wndClass);

    DWORD style = WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;

    RECT rect;
	rect.left = 100;
	rect.top = 100;
	rect.right = rect.left + m_windowInfo.width;
	rect.bottom = rect.top + m_windowInfo.height;

	AdjustWindowRect(&rect, style, false);
	LPVOID lpData = static_cast<LPVOID>(this);

	HWND hWnd = CreateWindowEx(
		0,
		m_win32WindowData.className,
		createInfo.windowTitle.c_str(),
		style,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		m_win32WindowData.hInstance,
		lpData
	);

    m_windowInfo.win32Window.hWnd = hWnd;

	ShowWindow(hWnd, SW_SHOW);
#endif // WIN32
}

bool Window::process_message()
{
#ifdef WIN32
    MSG msg{};

   	while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
	{
		if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
		{
			m_isRunning = false;
			return false;
		}
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif // WIN32

    return true;
}

void Window::close()
{
#ifdef WIN32
    DestroyWindow((HWND)m_windowInfo.win32Window.hWnd);
#endif // WIN32
}

void Window::set_width(uint32 width)
{
    if (width >= m_windowInfo.minWidth || width <= m_windowInfo.maxWidth)
    {
        m_windowInfo.width = width;
    }
}

void Window::set_height(uint32 height)
{
    if (height >= m_windowInfo.minHeight || height <= m_windowInfo.maxHeight)
    {
        m_windowInfo.height = height;
    }
}

}