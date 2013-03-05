object ConfigForm: TConfigForm
  Left = 183
  Top = 200
  BorderStyle = bsDialog
  Caption = #35373#23450
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
    Caption = #12491#12483#12463#12493#12540#12512
  end
  object Label2: TLabel
    Left = 24
    Top = 64
    Width = 62
    Height = 13
    Caption = #28961#32218#12487#12496#12452#12473
  end
  object Label3: TLabel
    Left = 24
    Top = 131
    Width = 81
    Height = 13
    Caption = 'SSID'#12398#33258#21205#26908#32034
  end
  object Label4: TLabel
    Left = 24
    Top = 163
    Width = 82
    Height = 13
    Caption = #26908#32034#38291#38548'('#12511#12522#31186')'
  end
  object Label5: TLabel
    Left = 24
    Top = 236
    Width = 52
    Height = 13
    Caption = #12525#12464#12398#20986#21147
    Visible = False
  end
  object Label6: TLabel
    Left = 24
    Top = 200
    Width = 55
    Height = 13
    Caption = #12381#12398#20182#35373#23450
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
    Caption = #12461#12515#12531#12475#12523
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
    Text = #25509#32154#12375#12394#12356' ('#12481#12515#12483#12488#12398#12415')'
    OnChange = ComboBoxDeviceChange
    Items.Strings = (
      #25509#32154#12375#12394#12356'('#12481#12515#12483#12488#12398#12415')')
  end
  object CheckBoxAutoDevice: TCheckBox
    Left = 152
    Top = 88
    Width = 153
    Height = 17
    Caption = #36215#21205#26178#12395#33258#21205#25509#32154#12377#12427
    TabOrder = 2
  end
  object RadioSSID1: TRadioButton
    Left = 152
    Top = 130
    Width = 113
    Height = 17
    Caption = #33258#21205#26908#32034#12434#34892#12358
    TabOrder = 4
  end
  object Button1: TButton
    Left = 368
    Top = 88
    Width = 50
    Height = 21
    Caption = #12522#12475#12483#12488
    TabOrder = 3
    OnClick = Button1Click
  end
  object RadioSSID2: TRadioButton
    Left = 271
    Top = 130
    Width = 130
    Height = 17
    Caption = #33258#21205#26908#32034#12434#34892#12431#12394#12356
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
    Caption = #12377#12427
    TabOrder = 11
    Visible = False
  end
  object RadioLog2: TRadioButton
    Left = 24
    Top = 266
    Width = 113
    Height = 17
    Caption = #12375#12394#12356
    TabOrder = 12
    Visible = False
  end
  object Button2: TButton
    Left = 311
    Top = 88
    Width = 51
    Height = 21
    Caption = #12486#12473#12488
    TabOrder = 13
    OnClick = Button2Click
  end
  object CheckBoxUseTaskTray: TCheckBox
    Left = 152
    Top = 199
    Width = 296
    Height = 17
    Caption = #12479#12473#12463#12488#12524#12452#12395#12450#12452#12467#12531#12434#34920#31034#12375#12513#12483#12475#12540#12472#12434#12496#12523#12540#12531#12391#36890#30693
    TabOrder = 7
  end
  object CheckBoxIgnoreCall: TCheckBox
    Left = 152
    Top = 222
    Width = 287
    Height = 17
    Caption = '<call>'#12434#21463#20449#26178#12395#12509#12483#12503#12450#12483#12503#12513#12483#12475#12540#12472#12434#34920#31034#12375#12394#12356
    TabOrder = 8
  end
end
