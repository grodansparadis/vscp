;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
; NSIS installer script for VSCP ;
 
; (http://nsis.sourceforge.net)  ;
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!define PRODUCT_NAME "VSCP & friends"
;!define PRODUCT_VERSION '${VERSION}'
!define PRODUCT_VERSION '0.3.3'
!define PRODUCT_GROUP "Grodans Paradis AB"
!define PRODUCT_PUBLISHER "Grodans Paradis AB"
!define PRODUCT_WEB_SITE "http://www.vscp.org"
!define PRODUCT_DIR_REGKEY "Software\VSCP"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_ID "{309E1C57-5DE3-442e-8DB5-6417CFA53799}"
 
!define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_DIR_REGKEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Language"


;;;;;;;;;;;;;;;;;;;;;;;;;
 
; General configuration ;
 
;;;;;;;;;;;;;;;;;;;;;;;;;
 
 
 
Name "${PRODUCT_GROUP} ${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile vscp-${PRODUCT_VERSION}-win32.exe
InstallDir "$PROGRAMFILES\VSCP"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
!ifdef NSIS_LZMA_COMPRESS_WHOLE
SetCompressor lzma
!else
SetCompressor /SOLID lzma
!endif
;ShowInstDetails show
;ShowUnInstDetails show
SetOverwrite ifnewer
CRCCheck on
 
 
 
InstType "Recommended"
;InstType "Full" 
;InstType "Server"
;InstType "Client"
InstType /NOCUSTOM 


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
; NSIS Modern User Interface configuration ;
 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
 
 
; MUI 1.67 compatible ------
 
  !include "MUI.nsh"
 
; MUI Settings
 
  !define MUI_ABORTWARNING
  !define MUI_ICON "fatbee_v2.ico"
  !define MUI_UNICON "fatbee_v2.ico"
  !define MUI_COMPONENTSPAGE_SMALLDESC
 
 
 
; Installer pages
 
  ; Welcome page
 
    !define MUI_WELCOMEPAGE_TITLE_3LINES
    !insertmacro MUI_PAGE_WELCOME
 
  ; License page
 
    !insertmacro MUI_PAGE_LICENSE "COPYING.txt"
 
  ; Components page
 
    !insertmacro MUI_PAGE_COMPONENTS
 
  ; Directory page
 
    !insertmacro MUI_PAGE_DIRECTORY
 
  ; Instfiles page
 
    !insertmacro MUI_PAGE_INSTFILES
 
  ; Finish page
 
;    !define MUI_FINISHPAGE_RUN "$INSTDIR\vscpworks.exe"
    !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
    !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
    !define MUI_FINISHPAGE_LINK "Visit the VSCP Website"
    !define MUI_FINISHPAGE_LINK_LOCATION "http://www.vscp.org"
    !define MUI_FINISHPAGE_NOREBOOTSUPPORT
    !insertmacro MUI_PAGE_FINISH
 
 
; Uninstaller pages
 
	!insertmacro MUI_UNPAGE_CONFIRM
    !insertmacro MUI_UNPAGE_COMPONENTS
    !insertmacro MUI_UNPAGE_INSTFILES
    !insertmacro MUI_UNPAGE_FINISH
 
 
 
; Language files
 
  !insertmacro MUI_LANGUAGE "English" # first language is the default language
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Polish"
 
 
!insertmacro MUI_RESERVEFILE_LANGDLL
 
 
 
; Reserve files
 
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
 
 
 
; MUI end ------
 



;;;;;;;;;;;;;;;;;;;
 
; Extension lists ;
 
;;;;;;;;;;;;;;;;;;;
 
 
 
!macro MacroVscpWorksExtensions _action
 
	!insertmacro ${_action} ".vrx"
	!insertmacro ${_action} ".vtx"
 
!macroend
 
 


;;;;;;;;;;;;;;;;;;;;;;;;;;
 
; File type associations ;
 
;;;;;;;;;;;;;;;;;;;;;;;;;;
 
 
 
Function RegisterExtension
 
	; back up old value for extension $R0 (eg. ".opt")
	ReadRegStr $1 HKCR "$R0" ""
	StrCmp $1 "" NoBackup
	StrCmp $1 "VSCP$R0" "NoBackup"
	WriteRegStr HKCR "$R0" "VSCP.backup" $1
 
NoBackup:
 
	WriteRegStr HKCR "$R0" "" "VSCP$R0"
	ReadRegStr $0 HKCR "VSCP$R0" ""
	WriteRegStr HKCR "VSCP$R0" "" "VSCP Works data file ($R0)"
	WriteRegStr HKCR "VSCP$R0\shell" "" "Play"
	WriteRegStr HKCR "VSCP$R0\shell\Play\command" "" '"$INSTDIR\vscpworks.exe" --started-from-file "%1"'
	WriteRegStr HKCR "VSCP$R0\DefaultIcon" "" '"$INSTDIR\vscpworks.exe",0'
 
FunctionEnd
 
 
 
Function un.RegisterExtension
 
	;start of restore script
	ReadRegStr $1 HKCR "$R0" ""
	StrCmp $1 "VSCP$R0" 0 NoOwn ; only do this if we own it

    ReadRegStr $1 HKCR "$R0" "VSCP.backup"
    StrCmp $1 "" 0 Restore ; if backup="" then delete the whole key
    DeleteRegKey HKCR "$R0"
 
    Goto NoOwn
 
Restore:
 
    WriteRegStr HKCR "$R0" "" $1
    DeleteRegValue HKCR "$R0" "VSCP.backup"
 
NoOwn:
 
    DeleteRegKey HKCR "VSCP$R0" ;Delete key with association settings
 
FunctionEnd
 
 
 
!macro RegisterExtensionSection EXT
 
	Section ${EXT}
		SectionIn 1 3
		Push $R0
		StrCpy $R0 ${EXT}
		Call RegisterExtension
		Pop $R0
	SectionEnd
 
!macroend
 
 
 
!macro UnRegisterExtensionSection EXT
 
	Push $R0
	StrCpy $R0 ${EXT}
	Call un.RegisterExtension
	Pop $R0
 
!macroend
 
 
 
!macro WriteRegStrSupportedTypes EXT
 
	WriteRegStr HKCR Applications\vscpworks.exe\SupportedTypes ${EXT} ""
 
!macroend


 
 
Var UninstallLog
 
 
 
!macro OpenUninstallLog
 
	FileOpen $UninstallLog "$INSTDIR\uninstall.log" a
	FileSeek $UninstallLog 0 END
 
!macroend
 
 
 
!macro CloseUninstallLog
 
	FileClose $UninstallLog
	SetFileAttributes "$INSTDIR\uninstall.log" HIDDEN
 
!macroend
 
 
 
!macro InstallFile FILEREGEX
 
	File "${FILEREGEX}"
	!define Index 'Line${__LINE__}'
	FindFirst $0 $1 "$INSTDIR\${FILEREGEX}"
	StrCmp $0 "" "${Index}-End"
	"${Index}-Loop:"
		StrCmp $1 "" "${Index}-End"
		FileWrite $UninstallLog "$1$\r$\n"
		FindNext $0 $1
		Goto "${Index}-Loop"
	"${Index}-End:"
	!undef Index
 
!macroend
 
 
 
!macro InstallFolder FOLDER
 
	File /r "${FOLDER}"
	Push "${FOLDER}"
	Call InstallFolderInternal
 
!macroend
 
 
 
Function InstallFolderInternal
 
	Pop $9
	!define Index 'Line${__LINE__}'
	FindFirst $0 $1 "$INSTDIR\$9\*"
	StrCmp $0 "" "${Index}-End"
	"${Index}-Loop:"
		StrCmp $1 "" "${Index}-End"
		StrCmp $1 "." "${Index}-Next"
		StrCmp $1 ".." "${Index}-Next"
		IfFileExists "$9\$1\*" 0 "${Index}-Write"
			Push $0
			Push $9
			Push "$9\$1"
			Call InstallFolderInternal
			Pop $9
			Pop $0
			Goto "${Index}-Next"
		"${Index}-Write:"
		FileWrite $UninstallLog "$9\$1$\r$\n"
    "${Index}-Next:"
    FindNext $0 $1
    Goto "${Index}-Loop"
	"${Index}-End:"
	!undef Index
 
FunctionEnd
 



;;;;;;;;;;;;;;;;;;;;;;
 
; Installer sections ;
 
;;;;;;;;;;;;;;;;;;;;;;
 
 
 
Section "Support components (required)" SEC01
 
	SectionIn 1 2 3 4 RO
 
	SetShellVarContext current
	SetOutPath "$INSTDIR"
 
 
	!insertmacro OpenUninstallLog
 
	!insertmacro InstallFile files\vscpworks.exe
	!insertmacro InstallFile *.txt
	!insertmacro InstallFile files\mkpasswd.exe
	!insertmacro InstallFile files\iflist.exe
 
    !insertmacro InstallFolder files\doc
	!insertmacro InstallFolder files\work
  
	; Shortcut to VSCP web site
	WriteIniStr "$INSTDIR\VSCP & Friends Website.url" "InternetShortcut" "URL" \
		"${PRODUCT_WEB_SITE}"
 
	FileWrite $UninstallLog "${PRODUCT_GROUP} Website.url$\r$\n"
 
	; Shortcut to VSCP docs on web site
	WriteIniStr "$INSTDIR\VSCP Application Documentation.url" "InternetShortcut" "URL" \
		"${PRODUCT_WEB_SITE}/wiki/doku.php?id=canal_vscp_applications"
 
	FileWrite $UninstallLog "Documentation.url$\r$\n"
  
	SetOutPath $SYSDIR
  
	; Install wx dll's
  	File  files\wx\*
	
	SetOutPath "$INSTDIR"
	
	; Install VSCP Works default configuration file
	SetShellVarContext current
;	IfFileExists "$APPDATA\vscpworks\vscpworks.conf" VSCPWORKS_CONF_PRESENT
	CreateDirectory "$APPDATA\vscpworks"
	SetOutPath "$APPDATA\vscpworks"
  	File  files\vscpworks\*
	
;VSCPWORKS_CONF_PRESENT:	
	
	; Install VSCP Works default configuration file
	SetShellVarContext all
	CreateDirectory "$APPDATA\vscp"
	SetOutPath "$APPDATA\vscp"
  	File  files\vscpd\*
		
	!insertmacro CloseUninstallLog
 
	; Add VSCP Works to "recomended programs" for the following extensions
	WriteRegStr HKCR Applications\vscpworks.exe "*.mdf" ""
	WriteRegStr HKCR Applications\vscpworks.exe "FriendlyAppName" "VSCP Works"
 
 
SectionEnd
 
 
 
Section "Start Menu Shortcut" SEC02
 
	SectionIn 1 2 3 4
 
	CreateDirectory "$SMPROGRAMS\VSCP & Friends"
	CreateShortCut "$SMPROGRAMS\VSCP & Friends\README.lnk" "$INSTDIR\README.txt" ""
	CreateShortCut "$SMPROGRAMS\VSCP & Friends\VSCP & Friends Site.lnk" "$INSTDIR\VSCP & Friends Website.url" ""
	CreateShortCut "$SMPROGRAMS\VSCP & Friends\VSCP application documentation.lnk" "$INSTDIR\VSCP Application Documentation.url" ""
	
	CreateShortCut "$SMPROGRAMS\VSCP & Friends\VSCP Works.lnk" "$INSTDIR\vscpworks.exe" ""
	CreateShortCut "$SMPROGRAMS\VSCP & Friends\VSCP Daemon.lnk" "$INSTDIR\vscpd.exe" ""
    
SectionEnd
 
 
 
Section "Desktop Shortcut" SEC03
 
	SectionIn 1 2 3 4
 
	CreateShortCut "$DESKTOP\VSCP Works.lnk" "$INSTDIR\vscpworks.exe" ""
	CreateShortCut "$DESKTOP\VSCP Daemon.lnk" "$INSTDIR\vscpd.exe" ""
 
SectionEnd
 
 
Section "VSCP Works" SEC04
 
	SectionIn 1 2 4
  
	SetOutPath "$INSTDIR"
	!insertmacro OpenUninstallLog
	!insertmacro InstallFile files\vscpworks.exe
	!insertmacro CloseUninstallLog
 
SectionEnd
 
 
Section "VSCP Server" SEC05
 
	SectionIn 1 2 3
  
	SetOutPath "$INSTDIR"
	!insertmacro OpenUninstallLog	
	!insertmacro InstallFile files\vscpservice.exe
	!insertmacro InstallFile files\vscpd.exe
	!insertmacro CloseUninstallLog 
	
	; Install the service
	Exec "$INSTDIR\vscpservice.exe -i"
 
SectionEnd 

 

Section "Drivers" SEC06
 
	SectionIn 1 2 3 4
  
	SetOutPath "$INSTDIR"
	!insertmacro OpenUninstallLog	
	!insertmacro InstallFolder files\drivers
	!insertmacro CloseUninstallLog
 
SectionEnd
 
 
Section "Development tools & examples" SEC07
 
	SectionIn 1 2 3 4
  
	SetOutPath "$INSTDIR"
	!insertmacro InstallFolder files\examples
	!insertmacro InstallFolder files\include
	!insertmacro InstallFolder files\lib
	!insertmacro InstallFolder files\cpp
	RegDLL "$INSTDIR\lib\axvlc.dll"
 
SectionEnd 

 
; Installer section descriptions
 
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
 
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} \
    "Required low level drivers and components."
 
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} \
    "Adds icons to your start menu for easy access"
 
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} \
    "Adds icon to your desktop for easy access"
 
 !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} \
    "Components needed to use the VSCP Works client application"
	
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC05} \
    "Components needed to unse the VSCP Server."
	
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC06} \
	"Drivers"
		
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC07} \
	"Components needed for development."		
 
!insertmacro MUI_FUNCTION_DESCRIPTION_END
 
 
 
Function .onInit
 
	ReadRegStr $R0  ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
		"UninstallString"
 
	StrCmp $R0 "" done
 
	MessageBox MB_YESNO|MB_ICONEXCLAMATION \
		"VSCP & Friends has already been installed. $\nDo you want to remove \
		the previous version before installing $(^Name) ?" \
		IDNO done
 
  
;Run the uninstaller
 
;uninst:
 
	ClearErrors
	ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
done:
 
	!insertmacro MUI_LANGDLL_DISPLAY
 
FunctionEnd
 
 
 
Section -Post
 
	; Install VC runtimes
	ExecWait '"$INSTDIR\work\vcredist_x86.exe"'
	
	; Install winpcap library
	ExecWait '"$INSTDIR\work\WinPcap_4_1_2.exe"'
	
	; Remove the work folder
	RMDir /r $INSTDIR\work
	
	WriteUninstaller "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "InstallDir" $INSTDIR
	WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Version" "${PRODUCT_VERSION}"
	WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\vscpworks.exe"
 
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
		"DisplayName" "$(^Name)"
 
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
		"UninstallString" "$INSTDIR\uninstall.exe"
 
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
		"DisplayIcon" "$INSTDIR\vscpworks.exe"
 
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
		"DisplayVersion" "${PRODUCT_VERSION}"
 
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
		"URLInfoAbout" "${PRODUCT_WEB_SITE}"
 
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
		"Publisher" "${PRODUCT_PUBLISHER}"
 
SectionEnd
 
 
 
;;;;;;;;;;;;;;;;;;;;;;;;
 
; Uninstaller sections ;
 
;;;;;;;;;;;;;;;;;;;;;;;;
 
 
 
; TrimNewlines (copied from NSIS documentation)
; input, top of stack  (e.g. whatever$\r$\n)
; output, top of stack (replaces, with e.g. whatever)
; modifies no other variables.
 
 
 
Function un.TrimNewlines
 
 Exch $R0
 Push $R1
 Push $R2
 StrCpy $R1 0
 
 loop:
 
   IntOp $R1 $R1 - 1
   StrCpy $R2 $R0 1 $R1
   StrCmp $R2 "$\r" loop
   StrCmp $R2 "$\n" loop
   IntOp $R1 $R1 + 1
   IntCmp $R1 0 no_trim_needed
   StrCpy $R0 $R0 $R1
 
 no_trim_needed:
 
   Pop $R2
   Pop $R1
   Exch $R0
 
FunctionEnd
 
 
 
Function un.RemoveEmptyDirs
 
  Pop $9
  !define Index 'Line${__LINE__}'
  FindFirst $0 $1 "$INSTDIR$9*"
  StrCmp $0 "" "${Index}-End"
  
  "${Index}-Loop:"
    StrCmp $1 "" "${Index}-End"
    StrCmp $1 "." "${Index}-Next"
    StrCmp $1 ".." "${Index}-Next"
      Push $0
      Push $1
      Push $9
      Push "$9$1\"
      Call un.RemoveEmptyDirs
      Pop $9
      Pop $1
      Pop $0
    "${Index}-Remove:"
 
    RMDir "$INSTDIR$9$1"
 
    "${Index}-Next:"
 
    FindNext $0 $1
    Goto "${Index}-Loop"
 
  "${Index}-End:"
 
  FindClose $0
  !undef Index
 
FunctionEnd
 
 
 
Section "Uninstall" SEC91
 
	SectionIn 1 2 3 RO
	SetShellVarContext all
 
;  !insertmacro MacroAllExtensions DeleteContextMenu
;  !insertmacro MacroAllExtensions UnRegisterExtensionSection
;  !insertmacro DeleteContextMenu "Directory"
 
 
	RMDir "$SMPROGRAMS\VSCP"
	RMDir /r $SMPROGRAMS\VSCP
 
 
 
	FileOpen $UninstallLog "$INSTDIR\uninstall.log" r
 
UninstallLoop:
 
    ClearErrors
    FileRead $UninstallLog $R0
    IfErrors UninstallEnd
    Push $R0
    Call un.TrimNewLines
    Pop $R0
    Delete "$INSTDIR\$R0"
    Goto UninstallLoop
 
UninstallEnd:
 
	FileClose $UninstallLog
	Delete "$INSTDIR\uninstall.log"
	Delete "$INSTDIR\uninstall.exe"
	
	Delete "$INSTDIR\vscpworks.exe"
	Delete "$INSTDIR\vscpd.exe"
	Delete "$INSTDIR\vscpservice.exe"
	
	RMDir /r "$INSTDIR\drivers\"
	RMDir /r "$INSTDIR\doc\*"
	RMDir /r "$INSTDIR\examples\"
	RMDir /r "$INSTDIR\include\"
	RMDir /r "$INSTDIR\lib\"
	RMDir /r "$INSTDIR\work\"
	RMDir /r "$INSTDIR\cpp\"
	Push "\"
	Call un.RemoveEmptyDirs
	RMDir "$INSTDIR"
 
	DeleteRegKey HKLM Software\VSCP
	DeleteRegKey HKCR Applications\vscpworks.exe
 
	DeleteRegKey HKLM \
		"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
 
	Delete "$DESKTOP\VSCP Works.lnk"
 
	DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
	DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
	SetAutoClose true
 
SectionEnd
 
 
 
 
; Uninstaller section descriptions
 
!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
 
	!insertmacro MUI_DESCRIPTION_TEXT ${SEC91} \
		"Uninstall VSCP & Friends and all its components"
 
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END
 
 
 
Function un.onUninstSuccess
 
	HideWindow
	MessageBox MB_ICONINFORMATION|MB_OK \
		"$(^Name) was successfully removed from your computer."
 
FunctionEnd
 
 
 
Function un.onInit
 
  !insertmacro MUI_UNGETLANGUAGE
 
FunctionEnd

