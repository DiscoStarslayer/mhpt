object ConnectForm: TConnectForm
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = #12469#12540#12496#12540#12395#25509#32154
  ClientHeight = 281
  ClientWidth = 425
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 67
    Top = 54
    Width = 52
    Height = 13
    Caption = #12469#12540#12496#12540#21517
  end
  object Label2: TLabel
    Left = 67
    Top = 81
    Width = 52
    Height = 13
    Caption = #12509#12540#12488#30058#21495
  end
  object Label3: TLabel
    Left = 67
    Top = 188
    Width = 52
    Height = 13
    Caption = #12509#12540#12488#30058#21495
  end
  object RadioConnect: TRadioButton
    Left = 24
    Top = 23
    Width = 113
    Height = 17
    Caption = #12469#12540#12496#12540#12395#25509#32154
    Checked = True
    TabOrder = 0
    TabStop = True
  end
  object RadioAccept: TRadioButton
    Left = 24
    Top = 157
    Width = 129
    Height = 17
    Caption = #12469#12540#12496#12540#12434#36215#21205#12377#12427
    TabOrder = 1
  end
  object EditServerName: TEdit
    Left = 125
    Top = 51
    Width = 209
    Height = 21
    TabOrder = 2
  end
  object EditServerPort: TEdit
    Left = 125
    Top = 78
    Width = 97
    Height = 21
    TabOrder = 3
  end
  object CheckBoxAutoConnect: TCheckBox
    Left = 125
    Top = 105
    Width = 164
    Height = 17
    Caption = #36215#21205#26178#12395#33258#21205#12391#25509#32154#12377#12427
    TabOrder = 4
  end
  object Button1: TButton
    Left = 240
    Top = 232
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    TabOrder = 5
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 329
    Top = 232
    Width = 75
    Height = 25
    Cancel = True
    Caption = #12461#12515#12531#12475#12523
    TabOrder = 6
    OnClick = Button2Click
  end
  object EditServerPort2: TEdit
    Left = 125
    Top = 185
    Width = 97
    Height = 21
    TabOrder = 7
  end
end
