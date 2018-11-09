#pragma once

const char TM_TICKER = 1;

class CWkeWebkitUI;
class CWkeWebkitWnd : public CWindowWnd ,protected wkeBufHandler 
{
	friend class CWkeWebkitUI;
public:
	CWkeWebkitWnd();
	~CWkeWebkitWnd();

	void Init(CWkeWebkitUI* pOwner);
	wkeWebView	GetWebView();
	LPCTSTR GetWindowClassName() const;
	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled); 
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled); 
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnImeStartComposition(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);	
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
	virtual void onBufUpdated (const HDC hdc,int x, int y, int cx, int cy);

protected:
	CWkeWebkitUI* m_pOwner;
	wkeWebView	 m_pWebView;
	std::wstring		 m_strUrl;	
	bool		 m_bInit;

};


class CWkeWebkitUI :public CControlUI
{
	friend class CWkeWebkitWnd;
public:
	CWkeWebkitUI(void);
	~CWkeWebkitUI(void);

	static void WkeWebkit_Init();
	static void WkeWebkit_Shutdown();
	
	LPCTSTR	GetClass() const override;
	LPVOID	GetInterface(LPCTSTR pstrName) override;

	void	SetVisible(bool bVisible) override;
	void	SetInternVisible(bool bVisible = true) override;
	void	DoInit() override;
	void	SetPos(RECT rc) override;	// ��Щ���SetPos�������ҵĲ�һ������Ҫ����ʹ�õĿ��޸��������
	void	DoEvent(TEventUI& event) override;	
	void	SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

	void	SetURL(std::wstring strValue);
	void	SetFile(std::wstring strValue);
    std::wstring RunJS(std::wstring strValue);
	void	SetClientHandler(const wkeClientHandler* handler);

	void	GoBack();
	void	GoForward();
    HWND    GetWebKitWindow();

protected:
	CWkeWebkitWnd *m_pWindow;
};
