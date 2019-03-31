#include "WinToast/src/wintoastlib.h"


class WinToastHandlerExample : public WinToastLib::IWinToastHandler {
public:
	WinToastHandlerExample()
	{

	}
	// Public interfaces
	virtual void toastActivated(int) const
	{

	}
	virtual void toastActivated() const
	{

	}
	virtual void toastDismissed(WinToastDismissalReason state) const
	{

	}
	virtual void toastFailed() const
	{

	}
};



std::shared_ptr<WinToastHandlerExample> ToastHandler;
void ToastInit(const wchar_t* ttitle)
{
	// Toast
	using namespace WinToastLib;
	if (WinToast::isCompatible())
	{
		WinToast::instance()->setAppName(ttitle);
		const auto aumi = WinToast::configureAUMI(L"FluentTorrent", ttitle, L"", L"");
		WinToast::instance()->setAppUserModelId(aumi);
		if (WinToast::instance()->initialize())
		{
			ToastHandler = std::make_shared<WinToastHandlerExample>();
		}
	}

}

void ShowToast(const wchar_t* ttitle,const wchar_t* msg)
{
	using namespace WinToastLib;
	WinToastTemplate templ = WinToastTemplate(WinToastTemplate::Text02);
	templ.setTextField(ttitle, WinToastTemplate::FirstLine);
	templ.setTextField(msg, WinToastTemplate::SecondLine);
	WinToast::instance()->showToast(templ, ToastHandler.get());
}

