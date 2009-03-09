# The NSIS (http://nsis.sourceforge.net) install script.
# This script is BSD licensed.

!include LogicLib.nsh
!include MUI2.nsh

!define VERSION "0.0.0"
!define QUADVERSION "0.0.0.0"

outFile "unbound_setup_${VERSION}.exe"
Name "Unbound"

# default install directory
installDir "$PROGRAMFILES\Unbound"
installDirRegKey HKLM "Software\Unbound" "InstallLocation"
RequestExecutionLevel admin
SetCompressor /solid /final lzma
#give credits to Nullsoft: BrandingText ""
VIAddVersionKey "ProductName" "Unbound"
VIAddVersionKey "CompanyName" "NLnet Labs"
VIAddVersionKey "FileDescription" "(un)install the unbound DNS resolver"
VIAddVersionKey "LegalCopyright" "Copyright 2009, NLnet Labs"
VIAddVersionKey "FileVersion" "${QUADVERSION}"
VIAddVersionKey "ProductVersion" "${QUADVERSION}"
VIProductVersion "${QUADVERSION}"

# Global Variables
Var StartMenuFolder

# user interface pages to show
# for the plain UI (using MUI2 now)
#Page license
#Page directory
#Page instfiles
#UninstPage uninstConfirm
#UninstPage instfiles

# use ReserveFile for files required before actual installation
# makes the installer start faster
ReserveFile "..\LICENSE"

#!define MUI_ICON "combined.ico"
#!define MUI_UNICON "combined.ico"
!define MUI_ICON "${NSISDIR}\contrib\graphics\icons\orange-install-nsis.ico"
!define MUI_UNICON "${NSISDIR}\contrib\graphics\icons\orange-uninstall-nsis.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "setup_top.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "setup_left.bmp"
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
#!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Unbound"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Unbound"
!insertmacro MUI_PAGE_STARTMENU UnboundStartMenu $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English" 

# default section, one per component, we have one component.
section "Unbound section"
	# copy files
	setOutPath $INSTDIR
	File "..\LICENSE"
	File "..\unbound.exe"
	File "..\unbound-checkconf.exe"
	File "..\unbound-control.exe"
	File "..\unbound-host.exe"
	File "..\unbound-service-install.exe"
	File "..\unbound-service-remove.exe"
	File "unbound-website.url"
	File "service.conf"
	File "..\doc\example.conf"

	# store installation folder
	WriteRegStr HKLM "Software\Unbound" "InstallLocation" $INSTDIR

	# uninstaller
	WriteUninstaller "uninst.exe"

	# register uninstaller
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound" "DisplayName" "Unbound"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound" "UninstallString" "$\"$INSTDIR\uninst.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound" "QuietUninstallString" "$\"$INSTDIR\uninst.exe$\" /S"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound" "NoRepair" "1"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound" "URLInfoAbout" "http://unbound.net"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound" "Publisher" "NLnet Labs"

	# start menu items
	!insertmacro MUI_STARTMENU_WRITE_BEGIN UnboundStartMenu
	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\unbound.net website.lnk" "$INSTDIR\unbound-website.url" "" "$INSTDIR\unbound.exe" "" "" "" "Visit the unbound website"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninst.exe" "" "" "" "" "" "Uninstall unbound"
	!insertmacro MUI_STARTMENU_WRITE_END

	# install service entry
	ExecWait '"$INSTDIR\unbound-service-install.exe"'

sectionEnd

# uninstaller section
section "un.Unbound section"
	# uninstall service entry
	ExecWait '"$INSTDIR\unbound-service-remove.exe"'
	# deregister uninstall
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Unbound"
	Delete "$INSTDIR\uninst.exe"   # delete self
	Delete "$INSTDIR\LICENSE"
	Delete "$INSTDIR\unbound.exe"
	Delete "$INSTDIR\unbound-checkconf.exe"
	Delete "$INSTDIR\unbound-control.exe"
	Delete "$INSTDIR\unbound-host.exe"
	Delete "$INSTDIR\unbound-service-install.exe"
	Delete "$INSTDIR\unbound-service-remove.exe"
	Delete "$INSTDIR\unbound-website.url"
	Delete "$INSTDIR\service.conf"
	Delete "$INSTDIR\example.conf"
	RmDir "$INSTDIR"

	# start menu items
	!insertmacro MUI_STARTMENU_GETFOLDER UnboundStartMenu $StartMenuFolder
	Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\unbound.net website.lnk"
	RMDir "$SMPROGRAMS\$StartMenuFolder"

	DeleteRegKey /ifempty HKLM "Software\Unbound"
sectionEnd
