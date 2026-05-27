@echo off
setlocal

rem Usa el entorno de Build Tools instalado directamente
set "VSINSTALL=C:\Progra~2\Microsoft Visual Studio\2022\BuildTools"

if not exist "%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat" (
  echo No se encontro vcvarsall.bat en %VSINSTALL%.
  echo Asegurate de que el workload C++ esta instalado.
  pause
  exit /b 1
)

echo Usando VS en: %VSINSTALL%
call "%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 goto :fail

cl.exe /EHsc main.cpp juego.cpp SistemaUsuarios.cpp /Fe:AeroEvaderPro.exe
if errorlevel 1 goto :fail

echo Compilacion completada: AeroEvaderPro.exe
pause
exit /b 0

:fail
echo Compilacion fallida.
pause
exit /b 1
