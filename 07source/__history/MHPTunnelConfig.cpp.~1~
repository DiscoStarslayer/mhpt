//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include "MHPTunnelConfig.h"
#include "MHPTunnelMain.h"
#include "tunnel-client.h"
#include "cTextCommand.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfigForm *ConfigForm;
//---------------------------------------------------------------------------
__fastcall TConfigForm::TConfigForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

int select;
int prev;

void __fastcall TConfigForm::FormCreate(TObject *Sender)
{
	cTextCommand cmd;
	select = 0;
	prev = 0;
//	TextCommand("/get Device");
	char nowDevice[100];
	strcpy(nowDevice, cmd.getSetting("Device"));

	cmd.execute("/listdevice");
	const char * device = cmd.nextResult();

	int i = 0;
	while (device != NULL) {
		this->deviceName[i] = (char *)malloc(strlen(device) + 1);
		strcpy(this->deviceName[i], device);
		i++;
		device = cmd.nextResult();
	}
	this->deviceNum = i;
	for(i = 0; i < deviceNum; i++) {
		char command[100];
		snprintf(command, sizeof(command), "/devicedesc %s", deviceName[i]);
		cmd.execute(command);
		const char * desc = cmd.nextResult();
		this->ComboBoxDevice->AddItem(desc, (TObject*)deviceName[i]);
		if (strcmp(deviceName[i], nowDevice) == 0) {
			select = i + 1;
			if (GetAdapterStatus() > 0) {
				prev = i + 1;
			}
			this->ComboBoxDevice->Text = desc;
		}
	}

//	TextCommand("/get NickName");
	this->EditNickName->Text = cmd.getSetting("NickName");

	if (MHPTunnelForm->GetBoolSetting("SSIDAutoSence", true)) {
		this->RadioSSID1->Checked = true;
	} else {
		this->RadioSSID2->Checked = true;
	}

//	TextCommand("/get SSIDAutoSenceInterval");
	this->EditSSIDInterval->Text = cmd.getSetting("SSIDAutoSenceInterval");

	this->CheckBoxAutoDevice->Checked = MHPTunnelForm->GetBoolSetting("AutoDevice", true);
	this->CheckBoxIgnoreCall->Checked = MHPTunnelForm->GetBoolSetting("IgnoreCall", false);
//	this->CheckBoxUseBalloon->Checked = MHPTunnelForm->GetBoolSetting("UseBalloon", true);
	this->CheckBoxUseTaskTray->Checked = MHPTunnelForm->GetBoolSetting("UseTaskTray", true);
}
//---------------------------------------------------------------------------


void __fastcall TConfigForm::ButtonOKClick(TObject *Sender)
{
	cTextCommand cmd;
	if (this->EditNickName->Text == "") {
		MessageBox(this->Handle, "Please enter a nickname", "Error", MB_OK);
	} else {
		if (prev != select) {
			cmd.execute("/closedevice");
			int i = select;
			if (i > 0) {
				i--;
				char tmp[100];
				snprintf(tmp, sizeof(tmp), "/opendevice %s", deviceName[i]);
				cmd.execute(tmp);
			}
			prev = select;
		}
		cmd.setSetting("NickName", this->EditNickName->Text.c_str());
		cmd.setSetting("SSIDAutoSenceInterval", this->EditSSIDInterval->Text.c_str());
		if (this->RadioSSID1->Checked) {
			cmd.setSetting("SSIDAutoSence", "true");
		} else {
			cmd.setSetting("SSIDAutoSence", "false");
		}
		if (this->CheckBoxAutoDevice->Checked) {
			cmd.setSetting("AutoDevice", "true");
		} else {
			cmd.setSetting("AutoDevice", "false");
		}

		MHPTunnelForm->SetBoolSetting("IgnoreCall", this->CheckBoxIgnoreCall->Checked);
 //		MHPTunnelForm->SetBoolSetting("UseBalloon", this->CheckBoxUseBalloon->Checked);
		MHPTunnelForm->SetBoolSetting("UseTaskTray", this->CheckBoxUseTaskTray->Checked);

		MHPTunnelForm->TrayIcon->Visible = MHPTunnelForm->GetBoolSetting("UseTaskTray", true);
		this->Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TConfigForm::Button1Click(TObject *Sender)
{
	cTextCommand cmd;
// 	if (prev != select) {
		cmd.execute("/closedevice");
		int i = select;
		if (i > 0) {
			i--;
			char tmp[100];
			snprintf(tmp, sizeof(tmp), "/opendevice %s", deviceName[i]);
			cmd.execute(tmp);
			prev = select;
		}
//	}
}
//---------------------------------------------------------------------------

void __fastcall TConfigForm::ComboBoxDeviceChange(TObject *Sender)
{
	select = this->ComboBoxDevice->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TConfigForm::ButtonCancelClick(TObject *Sender)
{
	this->Close();
}
//---------------------------------------------------------------------------

void __fastcall TConfigForm::Button2Click(TObject *Sender)
{
	cTextCommand cmd;
	if (prev != select) {
		cmd.execute("/closedevice");
		int i = select;
		if (i > 0) {
			i--;
			char tmp[100];
			snprintf(tmp, sizeof(tmp), "/opendevice %s", deviceName[i]);
			cmd.execute(tmp);
			prev = select;
		}
		Sleep(500);
	}
	if (GetAdapterStatus() == 0) {
		MessageBox(this->Handle, "Please open the device", "Test failed", MB_OK);
		return;
	}
	if (GetAdapterStatus() < 3) {
		MessageBox(this->Handle, "It is unable to connect to the PSP. \nPlease try again since the beginning of the meeting place.", "Test failed", MB_OK);
		return;
	}
	cmd.execute("/test");
	while (cmd.nextResult() != NULL) {
	}
	Sleep(500);
	cmd.execute("/test");
	const char * result = cmd.nextResult();
	int count = 0;
	sscanf(result, "%d", &count);
	if (count == 0 || count > 2000) {
		MessageBox(this->Handle, "I failed to transmit packets. \n Is this device may not be able to connect a PSP.", "Test failed", MB_OK);
		return;
	} else {
		MessageBox(this->Handle, "I was successful in sending and receiving packets.", "Successful Test", MB_OK);
		return;
	}
}
//---------------------------------------------------------------------------

