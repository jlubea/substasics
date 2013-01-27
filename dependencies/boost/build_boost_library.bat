rem
rem This batch file should be run from the visual studio command prompt
rem

SET SUBSTASICS_BOOST_OPTIONS=--with-system --with-date_time --with-filesystem --with-chrono define=BOOST_NO_COMPILER_TLS define=BOOST_FILESYSTEM_VERSION=3 --build-type=complete

.\b2 %SUBSTASICS_BOOST_OPTIONS% variant=debug address-model=32

rem mkdir -p stage/lib/debug/x86
