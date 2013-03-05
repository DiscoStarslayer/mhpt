//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include "MHPTunnelConnect.h"
#include "tunnel-client.h"
#include "cTextCommand.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConnectForm *ConnectForm;
//---------------------------------------------------------------------------
__fastcall TConnectForm::TConnectForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TConnectForm::FormCreate(TObject *Sender)
{
	cTextCommand cmd;
	this->EditServerName->Text = cmd.getSetting("Server");

	this->EditServerPort->Text = cmd.getSetting("Port");
	this->EditServerPort2->Text = this->EditServerPort->Text;

	if (strcmp(cmd.getSetting("AutoConnect"), "false")==0) {
		this->CheckBoxAutoConnect->Checked = false;
	} else {
		this->CheckBoxAutoConnect->Checked = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TConnectForm::Button2Click(TObject *Sender)
{
	this->Close();
}
//---------------------------------------------------------------------------
void __fastcall TConnectForm::Button1Click(TObject *Sender)
{
	cTextCommand cmd;
	cmd.execute("/close");
	char tmp[100];
	if (this->RadioConnect->Checked) {
		if (this->CheckBoxAutoConnect->Checked) {
			cmd.execute("/set AutoConnect true");
		} else {
			cmd.execute("/set AutoConnect false");
		}
		snprintf(tmp, sizeof(tmp), "/connect %s:%s",
			this->EditServerName->Text.c_str(),
			this->EditServerPort->Text.c_str());
		cmd.execute(tmp);
		//TextCommand("/users");
	} else {
		cmd.execute("/set AutoConnect false");
		snprintf(tmp, sizeof(tmp), "/openserver %s",
			this->EditServerPort2->Text.c_str());
		cmd.execute(tmp);
	}
	this->Close();
}
//---------------------------------------------------------------------------
