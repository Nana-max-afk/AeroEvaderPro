@echo off
rem Iniciar el ejecutable del juego en modo GUI
if exist "AeroEvaderPro.exe" (
    start AeroEvaderPro.exe
) else (
    echo No se encontro AeroEvaderPro.exe. Compila el juego primero usando build.bat.
    pause
)
exit
