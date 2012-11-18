; Windows Installer file for Cube Trains

[Setup]
AppName=Cube Trains
AppVersion=1.0.0
DefaultDirName={pf}\Cube Trains
DefaultGroupName=Cube Trains
UninstallDisplayIcon={app}\frogatto.exe
Compression=lzma2
SolidCompression=yes
OutputDir=c:\projects\cube trains\

[Files]
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\*.*"; DestDir: "{app}"; Excludes: "*.pdb,.*,std*.*";
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\data\*.*"; DestDir: "{app}\data"; Excludes: ".*"; Flags: recursesubdirs
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\images\*.*"; DestDir: "{app}\images"; Excludes: ".*"; Flags: recursesubdirs
Source: "C:\Projects\frogatto\vcredist_x86.exe"; DestDir: "{app}"; Flags: deleteafterinstall

; This is everything which is not nescessarily whats wanted
; Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\modules\cube_trains\*.*"; DestDir: "{app}\modules\cube_trains"; Excludes: ".*"; Flags: recursesubdirs

; These are more selective.
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\modules\cube_trains\*.*"; DestDir: "{app}\modules\cube_trains"; Excludes: ".*"; Excludes: "utils/*"; Flags: recursesubdirs
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\modules\cube_trains\data\*.*"; DestDir: "{app}\modules\cube_trains\data"; Excludes: ".*"; Flags: recursesubdirs
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\modules\cube_trains\images\*.*"; DestDir: "{app}\modules\cube_trains\images"; Excludes: ".*"; Flags: recursesubdirs
;Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\modules\cube_trains\locale\*.*"; DestDir: "{app}\modules\cube_trains\locale"; Excludes: ".*"; Flags: recursesubdirs
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\modules\cube_trains\sounds\*.*"; DestDir: "{app}\modules\cube_trains\sounds"; Excludes: ".*"; Flags: recursesubdirs
Source: "C:\Projects\frogatto-build\Frogatto\Win32\Release\modules\cube_trains\music\*.*"; DestDir: "{app}\modules\cube_trains\music"; Excludes: ".*"; Flags: recursesubdirs


[Icons]
Name: "{group}\Cube Trains"; Filename: "{app}\frogatto.exe"
Name: "{group}\Uninstall Cube Trains"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\vcredist_x86.exe"; Parameters: "/q"
