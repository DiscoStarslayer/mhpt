object ConfigForm: TConfigForm
  Left = 183
  Top = 200
  BorderStyle = bsDialog
  Caption = Setting
  ClientHeight = 313
  ClientWidth = 456
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 24
    Top = 24
    Width = 54
    Height = 13
    Caption = 'Nickname'
  end
  object Label2: TLabel
    Left = 24
    Top = 64
    Width = 62
    Height = 13
    Caption = 'Radio device'
  end
  object Label3: TLabel
    Left = 24
    Top = 131
    Width = 81
    Height = 13
    Caption = 'SSID Automatic search'
  end
  object Label4: TLabel
    Left = 24
    Top = 163
    Width = 82
    Height = 13
    Caption = 'Search interval (milliseconds)'
  end
  object Label5: TLabel
    Left = 24
    Top = 236
    Width = 52
    Height = 13
    Caption = 'Log Output'
    Visible = False
  end
  object Label6: TLabel
    Left = 24
    Top = 200
    Width = 55
    Height = 13
    Caption = 'Other Settings'
  end
  object ButtonOK: TButton
    Left = 271
    Top = 262
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    TabOrder = 10
    OnClick = ButtonOKClick
  end
  object ButtonCancel: TButton
    Left = 352
    Top = 262
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    TabOrder = 9
    OnClick = ButtonCancelClick
  end
  object EditNickName: TEdit
    Left = 152
    Top = 21
    Width = 177
    Height = 21
    TabOrder = 0
  end
  object ComboBoxDevice: TComboBox
    Left = 152
    Top = 61
    Width = 266
    Height = 21
    ItemHeight = 13
    TabOrder = 1
    Text = 'It is not connected (chat only)'
    OnChange = ComboBoxDeviceChange
    Items.Strings = (
      'It is not connected (chat only)')
  end
  object CheckBoxAutoDevice: TCheckBox
    Left = 152
    Top = 88
    Width = 153
    Height = 17
    Caption = 'Auto-connect at startup'
    TabOrder = 2
  end
  object RadioSSID1: TRadioButton
    Left = 152
    Top = 130
    Width = 113
    Height = 17
    Caption = 'Automatic search'
    TabOrder = 4
  end
  object Button1: TButton
    Left = 368
    Top = 88
    Width = 50
    Height = 21
    Caption = 'Reset'
    TabOrder = 3
    OnClick = Button1Click
  end
  object RadioSSID2: TRadioButton
    Left = 271
    Top = 130
    Width = 130
    Height = 17
    Caption = 'Do not perform an automatic search'
    TabOrder = 5
  end
  object EditSSIDInterval: TEdit
    Left = 152
    Top = 160
    Width = 113
    Height = 21
    TabOrder = 6
  end
  object RadioLog1: TRadioButton
    Left = 24
    Top = 255
    Width = 113
    Height = 17
    Caption = 'Make'
    TabOrder = 11
    Visible = False
  end
  object RadioLog2: TRadioButton
    Left = 24
    Top = 266
    Width = 113
    Height = 17
    Caption = 'Not'
    TabOrder = 12
    Visible = False
  end
  object Button2: TButton
    Left = 311
    Top = 88
    Width = 51
    Height = 21
    Caption = 'Test'
    TabOrder = 13
    OnClick = Button2Click
  end
  object CheckBoxUseTaskTray: TCheckBox
    Left = 152
    Top = 199
    Width = 296
    Height = 17
    Caption = 'Balloon notification message displays an icon in the task tray'
    TabOrder = 7
  end
  object CheckBoxIgnoreCall: TCheckBox
    Left = 152
    Top = 222
    Width = 287
    Height = 17
    Caption = 'I do not display a pop-up message when it receives a call'
    TabOrder = 8
  end
end
