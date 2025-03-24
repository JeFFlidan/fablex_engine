#include "window.h"
#include <winuser.h>

namespace fe
{

LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

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
        case WM_SIZE:
        {
            window->set_width(LOWORD(lParam));
            window->set_height(HIWORD(lParam));
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::init(const WindowCreateInfo& createInfo)
{    
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

	LPVOID lpData = static_cast<LPVOID>(this);

	HWND hWnd = CreateWindowEx(
		0,
		m_win32WindowData.className,
		createInfo.windowTitle.c_str(),
		style,
		0,
		0,
		800,
		800,
		nullptr,
		nullptr,
		m_win32WindowData.hInstance,
		lpData
	);

    m_windowInfo.win32Window.hWnd = hWnd;

	ShowWindow(hWnd, SW_MAXIMIZE);
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