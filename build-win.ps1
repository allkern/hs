$VERSION_TAG = git describe --always --tags --abbrev=0
$COMMIT_HASH = git rev-parse --short HEAD
$OS_INFO = (Get-WMIObject win32_operatingsystem).caption + " " + (Get-WMIObject win32_operatingsystem).version + " " + (Get-WMIObject win32_operatingsystem).OSArchitecture

md -Force -Path bin > $null

c++ main.cpp -o "bin/hs.exe" -std=c++20 `
    -DOS_VERSION="`"$($OS_INFO)`"" `
    -DHS_VERSION="`"$($VERSION_TAG)`"" `
    -DHS_COMMIT_HASH="`"$($COMMIT_HASH)`""