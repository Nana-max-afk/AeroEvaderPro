@echo off
rem Configurar la terminal para soporte UTF-8 (gráficos y bloques)
chcp 65001 >nul

rem Configurar el tamaño ideal de la ventana (100 columnas, 40 líneas)
mode con: cols=100 lines=40

rem Título de la ventana
title Aero Evader Pro - Consola Arcade

rem Iniciar el ejecutable del juego
if exist "AeroEvaderPro.exe" (
    AeroEvaderPro.exe
) else (
    echo No se encontro AeroEvaderPro.exe. Compila el juego primero usando build.bat.
    pause
)
exit
