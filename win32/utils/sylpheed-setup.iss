; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Sylpheed-Claws
AppVerName=Sylpheed-0.7.6-Claws-18 (Win32)
AppPublisher=The friendly Sylpheed hackers
AppPublisherURL=http://claws-w32.sourceforge.net
AppSupportURL=http://claws-w32.sourceforge.net
AppUpdatesURL=http://claws-w32.sourceforge.net
DefaultDirName={pf}\Sylpheed-Claws
DefaultGroupName=Sylpheed-Claws
AllowNoIcons=yes
;AlwaysCreateUninstallIcon=yes
LicenseFile=D:\_pak\Sylpheed.076.claws\doc\COPYING
InfoAfterFile=D:\_pak\Sylpheed.076.claws\doc\README
; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

[Types]
Name: "full";                   Description: "Full installation"
Name: "compact";                Description: "Compact installation"
Name: "custom";                 Description: "Custom installation";                     Flags: iscustom;

[Components]
Name: "main";                   Description: "Sylpheed program files";                  Types: full compact custom;     Flags: fixed;
Name: "settings";               Description: "Sylpheed registry settings";              Types: full compact custom;
Name: "help";                   Description: "Sylpheed help files";                     Types: full custom;
Name: "themes";                 Description: "Sylpheed theme pack";                     Types: full;
Name: "gtk";                    Description: "Gtk+ libraries (customized build, v1.3 + v2.0)";  Types: full compact custom;
Name: "gpg";                    Description: "GnuPG 1.0.6-2 (installs to C:\gnupg)";    Types: full;

[Tasks]
Name: "desktopicon";            Description: "Create a &desktop icon";                          GroupDescription: "Additional icons:"
Name: "quicklaunchicon";        Description: "Create a &Quick Launch icon";                     GroupDescription: "Additional icons:";

Name: "reg_sylpheed";           Description: "Create Sylpheed &registry key (recommended)";     GroupDescription: "Registry settings:"; Components: settings;
Name: "reg_defaultmailer";      Description: "Set Sylpheed default &mailer for mailto: links";  GroupDescription: "Registry settings:"; Components: settings;
Name: "reg_gpgme";              Description: "Create &GPGME registry key (needed fpr GnuPG)";   GroupDescription: "Registry settings:"; Components: settings;

Name: "reg_gnupg";              Description: "Create Gnu&PG registry key";                      GroupDescription: "Registry settings:"; Components: gpg;

[Files]
Source: "D:\_pak\Sylpheed.076.claws\bin\sylpheed.exe";          DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;
Source: "D:\_pak\Sylpheed.076.claws\bin\fnmatch.dll";           DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;
Source: "D:\_pak\Sylpheed.076.claws\bin\libcompface.dll";       DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;
Source: "D:\_pak\Sylpheed.076.claws\bin\libkcc.dll";            DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;
Source: "D:\_pak\Sylpheed.076.claws\bin\regex.dll";             DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;
Source: "D:\_pak\Sylpheed.076.claws\bin\w32lib.dll";            DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;
Source: "D:\_pak\Sylpheed.076.claws\bin\iconv.dll";             DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;

Source: "D:\_pak\Sylpheed.076.claws\bin\libgthread-2.0-0.dll";  DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite; Components: gtk
Source: "D:\_pak\Sylpheed.076.claws\bin\libgdk-0.dll";          DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite; Components: gtk
Source: "D:\_pak\Sylpheed.076.claws\bin\libglib-2.0-0.dll";     DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite; Components: gtk
Source: "D:\_pak\Sylpheed.076.claws\bin\libgmodule-2.0-0.dll";  DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite; Components: gtk
Source: "D:\_pak\Sylpheed.076.claws\bin\libgobject-2.0-0.dll";  DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite; Components: gtk
Source: "D:\_pak\Sylpheed.076.claws\bin\libgtk-0.dll";          DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite; Components: gtk
Source: "D:\_pak\Sylpheed.076.claws\bin\libintl-1.dll";         DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite; Components: gtk

Source: "D:\_pak\Sylpheed.076.claws\doc\*.*";                   DestDir: "{app}\doc";                   CopyMode: alwaysoverwrite
Source: "D:\_pak\Sylpheed.076.claws\doc\faq\*.*";               DestDir: "{app}\doc\faq";               CopyMode: alwaysoverwrite; Flags: recursesubdirs; Components: help
Source: "D:\_pak\Sylpheed.076.claws\doc\manual\*.*";            DestDir: "{app}\doc\manual";            CopyMode: alwaysoverwrite; Flags: recursesubdirs; Components: help
Source: "D:\_pak\Sylpheed.076.claws\etc\*.*";                   DestDir: "{app}\etc";                   CopyMode: alwaysoverwrite; Flags: recursesubdirs
Source: "D:\_pak\Sylpheed.076.claws\locale\*.*";                DestDir: "{app}\locale";                CopyMode: alwaysoverwrite; Flags: recursesubdirs
Source: "D:\_pak\Sylpheed.076.claws\themes\*.*";                DestDir: "{app}\themes";                CopyMode: alwaysoverwrite; Flags: recursesubdirs; Components: themes
; Claws specific
Source: "D:\_pak\Sylpheed.076.claws\bin\libeay32.dll";          DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;
Source: "D:\_pak\Sylpheed.076.claws\bin\ssleay32.dll";          DestDir: "{app}\bin";                   CopyMode: alwaysoverwrite;

Source: "D:\_pak\gnupg\*.*";                                    DestDir: "C:\gnupg";                    CopyMode: onlyifdoesntexist; Components: gpg

[INI]
Filename: "{app}\doc\gtk-w32.url";                              Section: "InternetShortcut"; Key: "URL"; String: "http://www.gimp.org/~tml/gimp/win32/"
Filename: "{app}\doc\sylpheed.url";                             Section: "InternetShortcut"; Key: "URL"; String: "http://sylpheed.good-day.net"
Filename: "{app}\doc\sylpheed-w32-en.url";                      Section: "InternetShortcut"; Key: "URL"; String: "http://claws-w32.sf.net"
Filename: "{app}\doc\sylpheed-w32-jp.url";                      Section: "InternetShortcut"; Key: "URL"; String: "http://www2.odn.ne.jp/munesato/sylpheed"
Filename: "{app}\doc\sylpheed-claws.url";                       Section: "InternetShortcut"; Key: "URL"; String: "http://sylpheed-claws.sf.net"
Filename: "{app}\doc\claws-sf-project.url";                     Section: "InternetShortcut"; Key: "URL"; String: "http://sf.net/projects/sylpheed-claws"
Filename: "{app}\doc\claws-cvs.url";                            Section: "InternetShortcut"; Key: "URL"; String: "http://cvs.sf.net/cgi-bin/viewcvs.cgi/sylpheed-claws/sylpheed-claws/"
Filename: "{app}\doc\claws-cvs-w32.url";                        Section: "InternetShortcut"; Key: "URL"; String: "http://cvs.sf.net/cgi-bin/viewcvs.cgi/sylpheed-claws/sylpheed-claws/?only_with_tag=win32"
Filename: "{app}\doc\claws-patches.url";                        Section: "InternetShortcut"; Key: "URL"; String: "http://www.thewildbeast.co.uk/sylpheed/"

[Icons]
Name: "{userdesktop}\Sylpheed-Claws";                           Filename: "{app}\bin\sylpheed.exe"; MinVersion: 4,4; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Sylpheed-Claws"; Filename: "{app}\bin\sylpheed.exe"; MinVersion: 4,4; Tasks: quicklaunchicon

Name: "{group}\Sylpheed-Claws";                                 Filename: "{app}\bin\sylpheed.exe"
Name: "{group}\Uninstall Sylpheed for Win32";                   Filename: "{uninstallexe}"
Name: "{group}\Sylpheed-FAQ";                                   Filename: "{app}\doc\faq\en\sylpheed-faq.html";         Components: help;
Name: "{group}\Sylpheed-Manual";                                Filename: "{app}\doc\manual\en\sylpheed.html";          Components: help;

Name: "{group}\links\Sylpheed";                                 Filename: "{app}\doc\sylpheed.url"
Name: "{group}\links\Sylpheed (Win32) english";                 Filename: "{app}\doc\sylpheed-w32-en.url"
Name: "{group}\links\Sylpheed (Win32) japanese";                Filename: "{app}\doc\sylpheed-w32-jp.url"
Name: "{group}\links\Gtk+ for Win32";                           Filename: "{app}\doc\gtk-w32.url"
Name: "{group}\links\Sylpheed-Claws";                           Filename: "{app}\doc\sylpheed-claws.url"
Name: "{group}\links\Sylpheed-Claws (Win32)";                   Filename: "{app}\doc\sylpheed-claws-w32.url"
Name: "{group}\links\Sylpheed-Claws SourceForge project";       Filename: "{app}\doc\claws-sf-project.url"
Name: "{group}\links\Sylpheed-Claws CVS tree";                  Filename: "{app}\doc\claws-cvs.url"
Name: "{group}\links\Sylpheed-Claws CVS tree (Win32)";          Filename: "{app}\doc\claws-cvs-w32.url"
Name: "{group}\links\Sylpheed-patches";                         Filename: "{app}\doc\claws-patches.url"

[Registry]
Root: HKCU; Subkey: "Software\Sylpheed";                        Flags: uninsdeletekey; Tasks: reg_sylpheed
Root: HKCU; Subkey: "Software\Sylpheed";                                                        ValueType: string;      ValueName: "InstalledDir";      ValueData: "{app}";             Tasks: reg_sylpheed
Root: HKCU; Subkey: "Software\Software\GNU\GnuPG";                                              ValueType: string;      ValueName: "gpgProgram";        ValueData: "C:\gnupg\gpg.exe";  Tasks: reg_gpgme
Root: HKCU; Subkey: "Software\Software\GNU\GnuPG";                                              ValueType: string;      ValueName: "InstalledDir";      ValueData: "C:\gnupg";  Components: gpg

Root: HKCR; Subkey: "mailto";                                   Flags: uninsclearvalue; Tasks: reg_defaultmailer
Root: HKCR; Subkey: "mailto";                                                                   ValueType: string;      ValueName: "";                  ValueData: "URL:MailTo-Protocol";                       Flags: uninsclearvalue; Tasks: reg_defaultmailer
Root: HKCR; Subkey: "mailto";                                                                   ValueType: string;      ValueName: "URL Protocol";      ValueData: "";                                          Flags: uninsclearvalue; Tasks: reg_defaultmailer
Root: HKCR; Subkey: "mailto";                                                                   ValueType: binary;      ValueName: "EditFlags";         ValueData: "02 00 00 00";                               Flags: uninsclearvalue; Tasks: reg_defaultmailer
Root: HKCR; Subkey: "mailto\DefaultIcon";                                                       ValueType: string;      ValueName: "";                  ValueData: "{app}\bin\sylpheed.exe,0";                  Flags: uninsclearvalue; Tasks: reg_defaultmailer
Root: HKCR; Subkey: "mailto\shell\open\command";                                                ValueType: string;      ValueName: "";                  ValueData: "{app}\bin\sylpheed.exe --compose ""%1""";   Flags: uninsclearvalue; Tasks: reg_defaultmailer

Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed";           Flags: uninsdeletekey; Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail";                                                    ValueType: string;      ValueName: "";                  ValueData: "Sylpheed";                                  Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed";                                           ValueType: string;      ValueName: "";                  ValueData: "Sylpheed";                                  Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed";                                           ValueType: string;      ValueName: "DLLPath";           ValueData: "";                                          Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed\Protocols\mailto";                          ValueType: string;      ValueName: "";                  ValueData: "URL:MailTo-Protokoll";                      Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed\Protocols\mailto";                          ValueType: string;      ValueName: "URL Protocol";      ValueData: "";                                          Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed\Protocols\mailto";                          ValueType: binary;      ValueName: "EditFlags";         ValueData: "02 00 00 00";                               Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed\Protocols\mailto\DefaultIcon";              ValueType: string;      ValueName: "";                  ValueData: "{app}\bin\sylpheed.exe";                    Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed\Protocols\mailto\shell\open\command";       ValueType: string;      ValueName: "";                  ValueData: "{app}\bin\sylpheed.exe --compose ""%1""";   Tasks: reg_defaultmailer
Root: HKLM; Subkey: "SOFTWARE\Clients\Mail\Sylpheed\shell\mailto\shell\open\command";           ValueType: string;      ValueName: "";                  ValueData: "{app}\bin\sylpheed.exe";                    Tasks: reg_defaultmailer

;[Run]
;Filename: "{app}\bin\sylpheed.exe";    Description: "Launch Sylpheed-Claws"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\doc\sylpheed.url"
Type: files; Name: "{app}\doc\sylpheed-w32-en.url"
Type: files; Name: "{app}\doc\sylpheed-w32-jp.url"
Type: files; Name: "{app}\doc\sylpheed-claws.url"
Type: files; Name: "{app}\doc\claws-sf-project.url"
Type: files; Name: "{app}\doc\claws-cvs.url"
Type: files; Name: "{app}\doc\claws-cvs-w32.url"
Type: files; Name: "{app}\doc\claws-patches.url"
Type: files; Name: "{app}\doc\gtk-w32.url"
Type: dirifempty; Name: "{app}\bin\"
Type: dirifempty; Name: "{app}\doc\"
Type: dirifempty; Name: "{app}"

