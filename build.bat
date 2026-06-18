@echo off
setlocal

rem Configurar el entorno de Visual Studio 2022 Build Tools instalado
set "VSINSTALL=C:\Progra~2\Microsoft Visual Studio\2022\BuildTools"

if not exist "%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat" (
  echo No se encontro vcvarsall.bat en %VSINSTALL%.
  echo Asegurate de que el C++ Workload esta instalado en Visual Studio Installer.
  pause
  exit /b 1
)

echo.
echo ===================================================
echo Compilando Aero Evader Pro - Nivel Premium Raylib
echo ===================================================
echo.

call "%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 goto :fail

cl.exe /EHsc /O2 /MD main.cpp juego2d.cpp SistemaUsuarios.cpp /I raylib\raylib-6.0_win64_msvc16\include /Fe:AeroEvaderPro.exe /link /LIBPATH:raylib\raylib-6.0_win64_msvc16\lib raylib.lib gdi32.lib user32.lib shell32.lib winmm.lib opengl32.lib /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
if errorlevel 1 goto :fail

echo.
echo [EXITO] Compilacion completada con exito!
echo Ejecutable creado: AeroEvaderPro.exe
echo.
pause
exit /b 0

:fail
echo.
echo [ERROR] Compilacion fallida. Revisa los mensajes de error.
echo.
pause
exit /b 1
