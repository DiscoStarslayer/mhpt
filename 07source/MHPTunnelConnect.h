//---------------------------------------------------------------------------

#ifndef MHPTunnelConnectH
#define MHPTunnelConnectH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TConnectForm : public TForm
{
__published:	// Management components IDE
	TRadioButton *RadioConnect;
	TRadioButton *RadioAccept;
	TEdit *EditServerName;
	TLabel *Label1;
	TLabel *Label2;
	TEdit *EditServerPort;
	TCheckBox *CheckBoxAutoConnect;
	TButton *Button1;
	TButton *Button2;
	TLabel *Label3;
	TEdit *EditServerPort2;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
private:	// User-declared
public:		// User-declared
	__fastcall TConnectForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConnectForm *ConnectForm;
//---------------------------------------------------------------------------
#endif
