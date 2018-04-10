@Echo Off

attrib -s -h -r

Del *.Ncb
Del *.Opt
Del *.Plg
Del *.Aps
Del *.Scc
Del *.suo
Del *.xml
Del *.old
Del *.user
Del *.sdf

Del x64\Release\*.* /Q
Del Win32\Release\*.* /Q
Del Release\*.* /Q
Rd x64\Release
Rd Win32\Release

Del x64\Debug\*.* /Q
Del Win32\Debug\*.* /Q
Del Debug\*.* /Q
Rd x64\Debug
Rd Win32\Debug

Rd x64
Rd Win32

Del Setup\Output\*.* /Q
RD Setup\Output

Del Resources\*.aps

DeleteTree Win32 /Y /DelRoot
DeleteTree x64 /Y /DelRoot
DeleteTree IPCH /Y /DelRoot

Cls