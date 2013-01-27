rem
rem This batch file should be run from the visual studio command prompt
rem

del /F /S /Q stage\lib\*

SET SUBSTASICS_BOOST_OPTIONS=--with-system --with-date_time --with-filesystem --with-chrono define=BOOST_NO_COMPILER_TLS define=BOOST_FILESYSTEM_VERSION=3 --build-type=complete

.\b2 %SUBSTASICS_BOOST_OPTIONS% variant=debug address-model=32

mkdir stage\lib\x86\debug
move /Y stage\lib\*.* stage\lib\x86\debug

.\b2 %SUBSTASICS_BOOST_OPTIONS% variant=release address-model=32

mkdir stage\lib\x86\release
move /Y stage\lib\*.* stage\lib\x86\release

.\b2 %SUBSTASICS_BOOST_OPTIONS% variant=debug address-model=64

mkdir stage\lib\x64\debug
move /Y stage\lib\*.* stage\lib\x64\debug

.\b2 %SUBSTASICS_BOOST_OPTIONS% variant=debug address-model=64

mkdir stage\lib\x64\release
move /Y stage\lib\*.* stage\lib\x64\release

