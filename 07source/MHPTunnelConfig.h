//---------------------------------------------------------------------------

#ifndef MHPTunnelConfigH
#define MHPTunnelConfigH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TConfigForm : public TForm
{
__published:	// Management components IDE
	TButton *ButtonOK;
	TButton *ButtonCancel;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *Label5;
	TEdit *EditNickName;
	TComboBox *ComboBoxDevice;
	TCheckBox *CheckBoxAutoDevice;
	TRadioButton *RadioSSID1;
	TButton *Button1;
	TRadioButton *RadioSSID2;
	TEdit *EditSSIDInterval;
	TRadioButton *RadioLog1;
	TRadioButton *RadioLog2;
	TButton *Button2;
	TLabel *Label6;
	TCheckBox *CheckBoxUseTaskTray;
	TCheckBox *CheckBoxIgnoreCall;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall ButtonOKClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall ComboBoxDeviceChange(TObject *Sender);
	void __fastcall ButtonCancelClick(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
private:	// User-declared
	int deviceNum;
	char* deviceName[20];
public:		// User-declared
	__fastcall TConfigForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConfigForm *ConfigForm;
//---------------------------------------------------------------------------
#endif
