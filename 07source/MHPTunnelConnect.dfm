object ConnectForm: TConnectForm
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Connect to the server'
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
    Caption = 'Server name'
  end
  object Label2: TLabel
    Left = 67
    Top = 81
    Width = 52
    Height = 13
    Caption = 'Port number'
  end
  object Label3: TLabel
    Left = 67
    Top = 188
    Width = 52
    Height = 13
    Caption = 'Port number'
  end
  object RadioConnect: TRadioButton
    Left = 24
    Top = 23
    Width = 113
    Height = 17
    Caption = 'Connect to the server'
    Checked = True
    TabOrder = 0
    TabStop = True
  end
  object RadioAccept: TRadioButton
    Left = 24
    Top = 157
    Width = 129
    Height = 17
    Caption = 'Start the server'
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
    Caption = 'Connect automatically when you start'
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
    Caption = 'Cancel'
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
