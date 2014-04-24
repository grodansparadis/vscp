VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3210
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6795
   LinkTopic       =   "Form1"
   ScaleHeight     =   3210
   ScaleWidth      =   6795
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox Status 
      Height          =   375
      Left            =   120
      TabIndex        =   18
      Text            =   "Text1"
      Top             =   1320
      Width           =   4815
   End
   Begin VB.PictureBox Picture10 
      Height          =   495
      Left            =   2880
      Picture         =   "Form1.frx":0000
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   17
      Top             =   2640
      Width           =   495
   End
   Begin VB.PictureBox Picture9 
      Height          =   495
      Left            =   2280
      Picture         =   "Form1.frx":0442
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   16
      Top             =   2640
      Width           =   495
   End
   Begin VB.PictureBox Picture8 
      Height          =   495
      Left            =   1680
      Picture         =   "Form1.frx":0884
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   15
      Top             =   2640
      Width           =   495
   End
   Begin VB.PictureBox Picture7 
      Height          =   495
      Left            =   1080
      Picture         =   "Form1.frx":0CC6
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   14
      Top             =   2640
      Width           =   495
   End
   Begin VB.PictureBox Picture6 
      Height          =   495
      Left            =   6240
      Picture         =   "Form1.frx":1108
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   13
      Top             =   2160
      Width           =   495
   End
   Begin VB.Timer Timer1 
      Left            =   120
      Top             =   2760
   End
   Begin VB.TextBox txtNickNameID 
      Height          =   285
      Left            =   6240
      TabIndex        =   11
      Text            =   "000"
      Top             =   1560
      Width           =   375
   End
   Begin VB.CommandButton Command6 
      Caption         =   "Init"
      Height          =   375
      Left            =   6240
      TabIndex        =   10
      Top             =   2760
      Width           =   495
   End
   Begin VB.PictureBox Picture5 
      Height          =   495
      Left            =   5760
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   9
      Top             =   120
      Width           =   495
   End
   Begin VB.PictureBox Picture4 
      Height          =   495
      Left            =   4440
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   8
      Top             =   120
      Width           =   495
   End
   Begin VB.PictureBox Picture3 
      Height          =   495
      Left            =   3120
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   7
      Top             =   120
      Width           =   495
   End
   Begin VB.PictureBox Picture2 
      Height          =   495
      Left            =   1800
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   6
      Top             =   120
      Width           =   495
   End
   Begin VB.PictureBox Picture1 
      Height          =   495
      Left            =   480
      ScaleHeight     =   435
      ScaleWidth      =   435
      TabIndex        =   5
      Top             =   120
      Width           =   495
   End
   Begin VB.CommandButton Command5 
      Caption         =   "Button5"
      Height          =   375
      Left            =   5400
      TabIndex        =   4
      Top             =   840
      Width           =   1215
   End
   Begin VB.CommandButton Command4 
      Caption         =   "Button4"
      Height          =   375
      Left            =   4080
      TabIndex        =   3
      Top             =   840
      Width           =   1215
   End
   Begin VB.CommandButton Command3 
      Caption         =   "Button3"
      Height          =   375
      Left            =   2760
      TabIndex        =   2
      Top             =   840
      Width           =   1215
   End
   Begin VB.CommandButton Command2 
      Caption         =   "Button2"
      Height          =   375
      Left            =   1440
      TabIndex        =   1
      Top             =   840
      Width           =   1215
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Button1"
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   840
      Width           =   1215
   End
   Begin MSWinsockLib.Winsock Winsock1 
      Left            =   120
      Top             =   2280
      _ExtentX        =   741
      _ExtentY        =   741
      _Version        =   393216
      Protocol        =   1
      RemoteHost      =   "255.255.255.255"
      RemotePort      =   46657
      LocalPort       =   46657
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "Nickname ID :"
      Height          =   255
      Left            =   5040
      TabIndex        =   12
      Top             =   1560
      Width           =   1095
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Command1_Click()
Winsock1.SendData "This is a test"
End Sub

Private Sub Text1_Change()

End Sub

Private Sub Winsock1_DataArrival(ByVal bytesTotal As Long)
    Dim strData As String

    'MsgBox ("Data n=" + Str(bytesTotal))
    Winsock1.GetData strData
    Status.Text = strData
    
End Sub

