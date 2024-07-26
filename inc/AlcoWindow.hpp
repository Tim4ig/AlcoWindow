
#pragma once

#include <string>
#include <thread>
#include <functional>
#include <future>

#ifndef UNICODE
#define UNICODE 1
#endif // !UNICODE

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace alco::window
{
	class Window
	{
	public:
		~Window();

		void Close();
		void Update();
		void Open(std::wstring title);
		void OpenAsync(std::wstring title);

		void SetSize(POINT size);
		void SetPosition(POINT pos) const;
		void SetSizeAndPosition(POINT size, POINT pos);
		void SetTitle(std::wstring title) const;

		void SetResizeCallback(std::function<void(POINT)> callback);
		void SetCloseCallback(std::function<bool()> callback);

		HWND GetHWND() const;
		POINT GetSize() const;
		POINT GetPosition() const;
		std::wstring GetTitle() const;

		bool IsRunning() const;
		bool IsForeground() const;
	private:
		HWND m_hwnd = nullptr;
		std::thread m_thread;
		
		bool m_running = false;
		bool m_async = false;

		std::function<void(POINT)> m_onSize;
		std::function<bool()> m_onClose;
		std::future<bool> m_onCloseFuture;
		bool m_futureRunning = false;

		POINT m_size = {};

		void m_Update(bool waitMSG);
		static LRESULT __stdcall m_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}
