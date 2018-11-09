#include "stdafx.h"
#include "FrameWnd.h"
#include <atlstr.h>
#include <sstream>
#include <ctime>
#include "GDIUtil.h"


using namespace Gdiplus;

#define  TIMER_MOUSE 1024
#define  TIMER_INIT_OCR 1025


jsValue JS_CALL js_msgBox(jsExecState es)
{
	const wchar_t* text = jsToStringW(es, jsArg(es, 0));
	const wchar_t* title = jsToStringW(es, jsArg(es, 1));

	HWND hwnd = CFrameWnd::MainWnd()->GetHWND();
	if (::IsWindow(hwnd) )
	{
		::PostMessage(hwnd, WM_USER_JS_NOTIFY, (WPARAM)text, (LPARAM)title);
	}
//	MessageBox(hwnd, text, title, 0);

	return jsUndefined();
}

void onURLChanged(const struct _wkeClientHandler* clientHandler, const wkeString URL)
{
//	MessageBox(hwnd, wkeToStringW(URL), _T(""), 0);
	HWND hwnd = CFrameWnd::MainWnd()->GetHWND();
	if (::IsWindow(hwnd) )
	{
		::PostMessage(hwnd, WM_USER_URL_CHANGE, (WPARAM)wkeToStringW(URL), (LPARAM)0);
	}
}

void onTitleChanged(const struct _wkeClientHandler* clientHandler, const wkeString title)
{
//	MessageBox(hwnd, wkeToStringW(title), _T(""), 0);
	HWND hwnd = CFrameWnd::MainWnd()->GetHWND();
	if (::IsWindow(hwnd) )
	{
		::PostMessage(hwnd, WM_USER_TITLE_CHANGE, (WPARAM)wkeToStringW(title), (LPARAM)0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////
//////////////////

CFrameWnd::CFrameWnd( LPCTSTR pszXMLPath )
	:  m_strXMLPath(pszXMLPath),
	m_pWke(NULL),
	m_pURLEdit(NULL),
	m_pLblStatusBar(NULL)
{
}

CFrameWnd * CFrameWnd::MainWnd()
{
	static CFrameWnd Frame(_T("Main_dlg.xml"));
	return &Frame;
}

LPCTSTR CFrameWnd::GetWindowClassName() const
{
	return _T("MainWnd");
}


CDuiString CFrameWnd::GetSkinFile()
{
	return m_strXMLPath;
}

CControlUI* CFrameWnd::CreateControl( LPCTSTR pstrClassName )
{
	if (_tcsicmp(pstrClassName, _T("wkeWebkit")) == 0)
		return  new CWkeWebkitUI;

	return NULL;
}

void CFrameWnd::InitWindow()
{
	CenterWindow();	
    std::wstring url = L"http://man.linuxde.net/screen";
	m_pURLEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("urlEdit")));
    if (m_pURLEdit)
    {
        m_pURLEdit->SetText(url.c_str());
    }
	//m_pLblStatusBar = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("statusbar")));
    m_pTipLabel = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("ocr_tip")));
	m_pWke = static_cast<CWkeWebkitUI*>(m_PaintManager.FindControl(_T("ie")));
	if (m_pWke)
	{
		//jsBindFunction("msgBox", js_msgBox, 2);//这里绑定js函数，让js主动调用c++函数
		static wkeClientHandler hander;        //网页标题改变和URL改变的回调
			hander.onTitleChanged = onTitleChanged;
			hander.onURLChanged = onURLChanged;
		m_pWke->SetClientHandler(&hander);
		//m_pWke->SetFile(_T("Html/index.html")/*msg.pSender->GetText().GetData()*/);
		m_pWke->SetURL(url.c_str()); //一个漂亮的网站，大家可以自己试试
	}

    ::SetTimer(m_hWnd, TIMER_INIT_OCR, 2000, NULL);
}

LRESULT CFrameWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    switch (uMsg)
    {
    case WM_USER_TITLE_CHANGE:
    {
        bHandled = TRUE;
        return OnTitleChanged(*this, wParam, lParam);
    }
    case WM_USER_URL_CHANGE:
    {
        bHandled = TRUE;
        return OnURLChanged(*this, wParam, lParam);
    }

    case WM_USER_JS_NOTIFY:
    {
        bHandled = TRUE;
        return OnJsNotify(*this, wParam, lParam);
    }
    case WM_RBUTTONDOWN : {
        
        
        return 1;
    }
    //case WM_MOUSEMOVE: {
    //    if (m_pTipLabel->IsVisible()) {
    //        m_pTipLabel->SetVisible(false);
    //    }

    //    return 1;
    //}
    case WM_TIMER:
    {
        if (wParam == TIMER_INIT_OCR) {
            if (!ocr_chi_sim_.Init(L"chi_sim")/* || !ocr_eng_.Init(L"eng")*/)
            {
                ::MessageBox(m_hWnd, L"ocr init failed.", L"OCR", MB_OK);
            }

            ::KillTimer(m_hWnd, TIMER_INIT_OCR);
            ::SetTimer(m_hWnd, TIMER_MOUSE, 2000, NULL);
        }
        else if (wParam == TIMER_MOUSE) {
            POINT pt;
            ::GetCursorPos(&pt);
            if (mouse_pt_.x == pt.x && mouse_pt_.y == pt.y)
            {
                RECT main_rect;
                ::GetWindowRect(m_hWnd, &main_rect);

                RECT pos;
                pos.left = pt.x - main_rect.left;
                pos.top = pt.y - main_rect.top;
                pos.right = pos.left + m_pTipLabel->GetFixedWidth();
                pos.bottom = pos.top + m_pTipLabel->GetFixedHeight();

                //m_pTipLabel->SetPos(pos);
                //if (!m_pTipLabel->IsVisible()) {
                //    m_pTipLabel->SetVisible(true);
                //}

                RECT web_kit_rect;
                ::GetWindowRect(m_pWke->GetWebKitWindow(), &web_kit_rect);
                int text_area_height = 50;
                int text_area_width = 200;
                RECT text_area;
                text_area.top = pt.y - web_kit_rect.top - text_area_height;
                text_area.left = pt.x - web_kit_rect.left;
                text_area.right = text_area.left + text_area_width;
                text_area.bottom = text_area.top + text_area_height;

                Gdiplus::Graphics graphic(m_pWke->GetWebKitWindow(), FALSE);
                GraphicsPath gpath;
                gpath.AddLine(text_area.left, text_area.top, text_area.right, text_area.top);
                gpath.AddLine(text_area.right, text_area.top, text_area.right, text_area.bottom);
                gpath.AddLine(text_area.right, text_area.bottom, text_area.left, text_area.bottom);
                gpath.AddLine(text_area.left, text_area.bottom, text_area.left, text_area.top);
                gpath.CloseFigure();
                Gdiplus::Pen pen(Gdiplus::Color(255, 0, 255, 0));
                graphic.DrawPath(&pen, &gpath);

                text_area.left *= 2;
                text_area.top *= 2;
                text_area.right *= 2;
                text_area.bottom *= 2;
                std::wstring wtext = ocr_chi_sim_.GetRecognizedText(text_area);
                for (auto& x : wtext)
                {
                    if (x == L'\n') {
                        x = L' ';
                    }
                }
                m_pTipLabel->SetText(wtext.c_str());
            }

            mouse_pt_ = pt;
        }
        
        return 1;
    }
    }
    return 0;
}

void CFrameWnd::Notify( TNotifyUI& msg )
{
	if( msg.sType == _T("click") ) 
	{	
		if( msg.pSender->GetName() == _T("btnBack") ) 
		{
			//m_pWke->GoBack();
            WCHAR buffer[MAX_PATH] = { 0 };
            ::GetModuleFileName(NULL, buffer, MAX_PATH);
            std::wstring path(buffer);
            path = path.substr(0, path.find_last_of(L"\\"));
            path += L"\\temp";
            std::wstringstream stream;
            DWORD stamp = std::time(nullptr);
            //stream << path << L"\\" << stamp << L".bmp";
            //std::wstring bmp_file_path = stream.str();
            //bool succeed = CaptureAnImage(m_pWke->GetWebKitWindow(), bmp_file_path.c_str());
            //if (succeed) {
            //    path = bmp_file_path.substr(0, bmp_file_path.find_last_of(L"\\"));
            //    stream.str(L"");
            //    //stream << path << L"\\" << stamp << L".jpg";
            //    stream << path << L"\\2.jpg";
            //    succeed = ConvertBMP2JPG(stream.str(), bmp_file_path);
            //    if (succeed) {
            //        std::string jpg_file = CW2AEX<>(stream.str().c_str(), CP_ACP);
            //        bool succeed = ocr_.RecognizeImage(jpg_file);
            //        if (!succeed)
            //        {
            //            ::MessageBox(m_hWnd, L"ocr recognize image failed.", L"OCR", MB_OK);
            //        }
            //    }

            //    ::DeleteFile(bmp_file_path.c_str());
            //}

            stream << path << L"\\" << stamp << L".jpg";
            std::wstring jpg_file_path = stream.str();
            bool succeed = CaptureAnImage(m_pWke->GetWebKitWindow(), jpg_file_path);
            if (succeed) {
                if (!ocr_chi_sim_.RecognizeImage(jpg_file_path) /*|| !ocr_eng_.RecognizeImage(jpg_file_path)*/)
                {
                    ::MessageBox(m_hWnd, L"ocr recognize image failed.", L"OCR", MB_OK);
                }
                //::DeleteFile(jpg_file_path.c_str());
            }
		}
		if( msg.pSender->GetName() == _T("btnforward") ) 
		{
			//m_pWke->GoForward();
            GDIUtil::ClearRGB(L"G:\\learn\\github\\ocr\\WebOCR\\Demo\\duilib_webkit\\duilib_wke\\Bin\\imagecircle2.png");
		}
		if( msg.pSender->GetName() == _T("btnGo") ) 
		{
			if (m_pURLEdit)
				m_pWke->SetURL(m_pURLEdit->GetText().GetData());
		}
		if( msg.pSender->GetName() == _T("btnRefresh") ) 
		{
				std::wstring strRet = m_pWke->RunJS(m_pURLEdit->GetText().GetData());
				MessageBox(m_hWnd, strRet.c_str(), _T(""), 0);
		}

	}
	else if( msg.sType == _T("return") ) 
	{
		if( msg.pSender->GetName() == _T("urlEdit") ) 
		{
			if (m_pURLEdit)
				m_pWke->SetURL(m_pURLEdit->GetText().GetData());
		}
	}

	__super::Notify(msg);
}

LRESULT CFrameWnd::OnTitleChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	m_Title = (LPCTSTR)wParam;
	TCHAR szText[MAX_PATH] = {0};
	_stprintf(szText, _T("网站标题：%s     网站网址:%s"),m_Title.c_str(),m_URL.c_str());
	//m_pLblStatusBar->SetText(szText);
	return 0 ;
}

LRESULT CFrameWnd::OnURLChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	m_URL = (LPCTSTR)wParam;
	TCHAR szText[MAX_PATH] = {0};
	_stprintf(szText, _T("网站标题：%s     网站网址:%s"),m_Title.c_str(),m_URL.c_str());
	//m_pLblStatusBar->SetText(szText);
	return 0 ;
}

LRESULT CFrameWnd::OnJsNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	//MessageBox(m_hWnd, (LPCTSTR)wParam, (LPCTSTR)lParam, 0);
	return 0;
}

void CFrameWnd::OnFinalMessage(HWND hWnd) {
    ::KillTimer(m_hWnd, TIMER_MOUSE);
}

bool CFrameWnd::CaptureAnImage(HWND hWnd, const std::wstring& destFile)
{
    CLSID clsid;
    int ret = -1;
    if (-1 != GDIUtil::GetEncoderClsid(L"image/jpeg", &clsid))
    {
        bool succeed = false;
        HDC hdcWindow;
        HDC hdcMemDC = NULL;
        HBITMAP hbmScreen = NULL;
        BITMAP bmpScreen;

        // Retrieve the handle to a display device context for the client 
        // area of the window. 
        //hdcScreen = GetDC(NULL);
        hdcWindow = GetDC(hWnd);

        // Create a compatible DC which is used in a BitBlt from the window DC
        hdcMemDC = CreateCompatibleDC(hdcWindow);

        if (!hdcMemDC)
        {
            MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
            return false;
        }

        // Get the client area for size calculation
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // Create a compatible bitmap from the Window DC
        hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

        if (!hbmScreen)
        {
            MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
            return false;
        }

        // Select the compatible bitmap into the compatible memory DC.
        SelectObject(hdcMemDC, hbmScreen);

        // Bit block transfer into our compatible memory DC.
        if (!BitBlt(hdcMemDC,
            0, 0,
            rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
            hdcWindow,
            0, 0,
            SRCCOPY))
        {
            MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
            return false;
        }

        // Get the BITMAP from the HBITMAP
        GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

        GdiplusStartupInput startupInput;
        ULONG_PTR token;
        GdiplusStartup(&token, &startupInput, NULL);
        {
            Gdiplus::Bitmap gdi_bitmap(hbmScreen, NULL);
            UINT scaled_width = gdi_bitmap.GetWidth() * 2;
            UINT scaled_height = gdi_bitmap.GetHeight() * 2;
            Gdiplus::Bitmap gdi_bitmap_x2(scaled_width, scaled_height, gdi_bitmap.GetPixelFormat());
            Gdiplus::Graphics graphics(&gdi_bitmap_x2);
            graphics.DrawImage(&gdi_bitmap, 0, 0, scaled_width, scaled_height);
            Status status = gdi_bitmap_x2.Save(destFile.c_str(), &clsid, NULL);
            if (!status) {
                return true;
            }
        }

        
    }

    return false;

#if 0
    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char *lpbitmap = (char *)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap,
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // A file is created, this is where we will save the screen capture.
    HANDLE hFile = CreateFile(destFile.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    // Add the size of the headers to the size of the bitmap to get the total file size
    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    //Size of the file
    bmfHeader.bfSize = dwSizeofDIB;

    //bfType must always be BM for Bitmaps
    bmfHeader.bfType = 0x4D42; //BM   

    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    //Unlock and Free the DIB from the heap
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    //Close the handle for the file that was created
    CloseHandle(hFile);

    succeed = true;

    //Clean up
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    //ReleaseDC(NULL, hdcScreen);
    ReleaseDC(hWnd, hdcWindow);


    return succeed;
#endif
}


