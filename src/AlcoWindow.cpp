
#include "AlcoWindow.hpp"

namespace alco::window
{
	Window::~Window()
	{
		this->Close();
		if (m_thread.joinable()) m_thread.join();
	}

	void Window::Open(std::wstring title)
	{
		this->Close();

		WNDCLASS wc = { };
		wc.lpfnWndProc = m_WndProc;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.lpszClassName = L"ALCO_WINDOW_LIBRARY_CLASS_2215";
		RegisterClass(&wc);

		m_hwnd = CreateWindow(
			wc.lpszClassName,
			title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			800,
			600,
			nullptr,
			nullptr,
			GetModuleHandle(nullptr),
			nullptr
		);

		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

		ShowWindow(m_hwnd, SW_SHOWDEFAULT);
		UpdateWindow(m_hwnd);

		m_running = true;
	}

	void Window::OpenAsync(std::wstring title)
	{
		if (m_thread.joinable()) m_thread.join();
		m_thread = std::thread([this, &title]()
		{
			this->Open(title);
			while (this->IsRunning()) this->m_Update(1);
		});
		m_async = true;
		while (!m_running);
	}

	void Window::Update()
	{
		if (m_async) return;
		this->m_Update(0);
	}

	void Window::Close()
	{
		m_async = false;
		m_running = false;
		DestroyWindow(m_hwnd);
	}

	void Window::SetSize(POINT size)
	{
		RECT rect = { 0, 0, size.x, size.y };
		AdjustWindowRect(&rect, GetWindowLong(m_hwnd, GWL_STYLE), false);
		SetWindowPos(m_hwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		if (m_onSize) m_onSize(size);
	}

	void Window::SetPosition(POINT pos) const
	{
		SetWindowPos(m_hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void Window::SetSizeAndPosition(POINT size, POINT pos)
	{
		this->SetSize(size);
		this->SetPosition(pos);
	}

	void Window::SetTitle(std::wstring title) const
	{
		SetWindowTextW(m_hwnd, title.c_str());
	}

	void Window::SetResizeCallback(std::function<void(POINT)> callback)
	{
		m_onSize = callback;
		if (!m_onSize) return;
		m_onSize(this->GetSize());
	}

	void Window::SetCloseCallback(std::function<bool()> callback)
	{
		m_onClose = callback;
	}

	HWND Window::GetHWND() const
	{
		return m_hwnd;
	}

	POINT Window::GetSize() const
	{
		RECT rect;
		GetClientRect(m_hwnd, &rect);
		return { rect.right - rect.left, rect.bottom - rect.top };
	}

	POINT Window::GetPosition() const
	{
		RECT rect;
		GetWindowRect(m_hwnd, &rect);
		return { rect.left, rect.top };
	}

	std::wstring Window::GetTitle() const
	{
		std::wstring title;
		title.resize(GetWindowTextLengthW(m_hwnd) + 1);
		GetWindowTextW(m_hwnd, const_cast<LPWSTR>(title.c_str()), static_cast<int>(title.size()));
		return title;
	}

	bool Window::IsRunning() const
	{
		return m_running;
	}

	bool Window::IsForeground() const
	{
		return m_hwnd == GetForegroundWindow();
	}

	void Window::m_Update(bool waitMSG)
	{
		MSG msg = {};
		if (waitMSG)
		{
			GetMessageW(&msg, nullptr, 0, 0);
		}
		else
		{
			PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
		}
		if (msg.message == WM_QUIT) m_running = false;

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	LRESULT _stdcall Window::m_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Window* pthis = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (!pthis) return DefWindowProc(hwnd, msg, wParam, lParam);

		switch (msg)
		{
		case WM_SIZE:
		{
			POINT size = pthis->GetSize();
			if (wParam == SIZE_MINIMIZED) return 0;
			if (size.x == pthis->m_size.x && size.y == pthis->m_size.y) return 0;
			if (pthis->m_onSize) { pthis->m_onSize(size); }
			pthis->m_size = size;
			return 0;
		}
		case WM_CLOSE:
		{
			if (pthis->m_onClose && !pthis->m_futureRunning)
			{
				pthis->m_onCloseFuture = std::async(std::launch::async, pthis->m_onClose);
				pthis->m_futureRunning = true;
				return 0;
			}
			PostQuitMessage(0);
			return 0;
		}
		}

		if (pthis->m_futureRunning)
		{
			if (pthis->m_onCloseFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			{
				pthis->m_futureRunning = false;
				if (pthis->m_onCloseFuture.get()) PostQuitMessage(0);
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}
