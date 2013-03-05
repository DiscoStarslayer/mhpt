//----------------------------------------------------------------------------
#ifndef MHPTunnelMainH
#define MHPTunnelMainH
//----------------------------------------------------------------------------
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <StdCtrls.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
#include <Controls.hpp>
#include <Forms.hpp>
#include <Graphics.hpp>
#include <Classes.hpp>
#include <Windows.hpp>
#include <System.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <StdActns.hpp>
#include <ToolWin.hpp>
#include <XPMan.hpp>
#include <AppEvnts.hpp>
#include <OleCtrls.hpp>
#include <SHDocVw.hpp>
//----------------------------------------------------------------------------
class TMHPTunnelForm : public TForm
{
__published:
	TStatusBar *StatusBar;
	TMainMenu *MainMenu;
	TMenuItem *MenuConnect;
	TMenuItem *MenuConnectArena;
	TMenuItem *MenuDisconnect;
	TMenuItem *MenuSep1;
	TMenuItem *MenuQuit;
	TMenuItem *MenuSetting;
	TMenuItem *MenuSettingUser;
	TMenuItem *MenuSep2;
	TMenuItem *MenuClearConsole;
	TMenuItem *MenuHelp;
	TMenuItem *MenuVersion;
	TXPManifest *XPManifest1;
	TTrayIcon *TrayIcon;
	TListView *ListUsers;
	TImageList *ImageList;
	TPanel *PanelBottom;
	TButton *ButtonSend;
	TEdit *CommandBox;
	TSplitter *Splitter;
	TPanel *InfoPanel;
	TSplitter *Splitter1;
	TListView *ListPSP;
	TApplicationEvents *ApplicationEvents;
	TMenuItem *MenuSep3;
	TMenuItem *MenuOnlineManual;
	TPopupMenu *PopupMenu;
	TMenuItem *PopupCopy;
	TMenuItem *PopupPaste;
	TWebBrowser *WebBrowser;
	TMenuItem *PopupPasteHTML;
	TRichEdit *RichConsole;
	TMemo *ConsoleBox;
	void __fastcall MenuQuitClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall ButtonSendClick(TObject *Sender);
	void __fastcall MenuVersionClick(TObject *Sender);
	void __fastcall MenuClearConsoleClick(TObject *Sender);
	void __fastcall MenuDisconnectClick(TObject *Sender);
	void __fastcall MenuSettingUserClick(TObject *Sender);
	void __fastcall MenuConnectArenaClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall TrayIconClick(TObject *Sender);
	void __fastcall CommandBoxKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall CommandBoxKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall CommandBoxKeyPress(TObject *Sender, char &Key);
	void __fastcall ApplicationEventsMessage(tagMSG &Msg, bool &Handled);
	void __fastcall MenuOnlineManualClick(TObject *Sender);
	void __fastcall PopupCopyClick(TObject *Sender);
	void __fastcall PopupPasteClick(TObject *Sender);
	void __fastcall ListUsersDblClick(TObject *Sender);
	void __fastcall PopupPasteHTMLClick(TObject *Sender);
private:
	void AddHistory(const char * command);
	int historyIndex;
	static const int historyMax = 40;
	char * history[historyMax];
	char * historyTemp;
	bool isIMEInputNow(HWND hWnd);
	HANDLE hMonitor;
	void __fastcall CommandBoxPaste();
public:
	void __fastcall ApplicationDeactivate(TObject *Sender);
	const char * GetSetting(const char * key);
	bool GetBoolSetting(const char * key, bool def);
	void SetSetting(const char * key, const char * value);
	void SetBoolSetting(const char * key, bool value);
	virtual __fastcall TMHPTunnelForm(TComponent *AOwner);
	TListItem * FindItem(const char * uid);
	TListItem * FindItemPSP(const char * uid);
	bool closed;
	void UpdateUserList();
	void addLine(const char *, const char *);
	void clearConsole();
	static const int CONSOLE_MEMO = 0;
	static const int CONSOLE_RICH = 1;
	static const int CONSOLE_WEB = 2;
	int ConsoleType;
	void __fastcall WndProc(Messages::TMessage &Message);
};
//----------------------------------------------------------------------------
extern TMHPTunnelForm *MHPTunnelForm;
//----------------------------------------------------------------------------
#endif
