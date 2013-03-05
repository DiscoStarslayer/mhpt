//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------
USEFORM("MHPTunnelMain.CPP", MHPTunnelForm);
USEFORM("MHPTunnelConfig.cpp", ConfigForm);
USEFORM("MHPTunnelConnect.cpp", ConnectForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Application->Initialize();
	Application->Title = "MHPTunnel";
	Application->CreateForm(__classid(TMHPTunnelForm), &MHPTunnelForm);
		Application->CreateForm(__classid(TConfigForm), &ConfigForm);
		Application->CreateForm(__classid(TConnectForm), &ConnectForm);
		Application->Run();

	return 0;
}
//---------------------------------------------------------------------
