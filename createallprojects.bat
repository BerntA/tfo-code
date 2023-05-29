pushd %~dp0
devtools\bin\vpc.exe +everything /mksln everything.sln /define:VS2022
popd

pause
