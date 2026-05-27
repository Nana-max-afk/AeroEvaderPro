#include "SistemaUsuarios.h"
#include "juego.h"
#include "juego2d.h"
#include <iostream>
#include <limits>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#include <windows.h>
#undef CloseWindow
#undef ShowCursor
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

// ─── Utilidades de Terminal ───────────────────────────────────────────────────

void establecerTemaConsola() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE) {
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    // Fondo negro profundo con texto predeterminado blanco
    std::system("color 07");
#endif
}

int obtenerAnchoConsola() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#endif
    return 80;
}

int obtenerAltoConsola() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#endif
    return 25;
}

// ─── Encabezado y Menú ───────────────────────────────────────────────────────

void mostrarEncabezado() {
    int termW = obtenerAnchoConsola();
    int termH = obtenerAltoConsola();
    int margenIzq = std::max(0, (termW - 82) / 2);
    int margenSup = std::max(0, (termH - 28) / 2);
    std::string pad(margenIzq, ' ');
    std::string cn = "\x1B[0m";

    for (int i = 0; i < margenSup; ++i) std::cout << "\n";

    std::cout << pad << "\x1B[96m╔══════════════════════════════════════════════════════════════════════════════════╗\n" << cn;
    std::cout << pad << "\x1B[96m║\x1B[0m" << "  \x1B[90m·  ★  ·  ·    ·  ·    ·    ★  ·  ·  ·   ★  ·    ·  ·    ·  ★  ·  ·    ·  ·  ·  \x1B[0m" << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m╠══════════════════════════════════════════════════════════════════════════════════╣\n" << cn;
    std::cout << pad << "\x1B[96m║\x1B[0m" << "        \x1B[93m █████╗ ███████╗██████╗  ██████╗     ███████╗██╗   ██╗ █████╗  \x1B[0m       " << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║\x1B[0m" << "        \x1B[93m██╔══██╗██╔════╝██╔══██╗██╔═══██╗    ██╔════╝██║   ██║██╔══██╗ \x1B[0m       " << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║\x1B[0m" << "        \x1B[93m███████║█████╗  ██████╔╝██║   ██║    █████╗  ██║   ██║███████║ \x1B[0m       " << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║\x1B[0m" << "        \x1B[93m██╔══██║██╔══╝  ██╔══██╗██║   ██║    ██╔══╝  ╚██╗ ██╔╝██╔══██║ \x1B[0m       " << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║\x1B[0m" << "        \x1B[93m██║  ██║███████╗██║  ██║╚██████╔╝    ███████╗ ╚████╔╝ ██║  ██║ \x1B[0m       " << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║\x1B[0m" << "        \x1B[93m╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝ ╚═════╝     ╚══════╝  ╚═══╝  ╚═╝  ╚═╝ \x1B[0m       " << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m╠══════════════════════════════════════════════════════════════════════════════════╣\n" << cn;

    // Nave pequeña decorativa inline
    std::cout << pad << "\x1B[96m║  \x1B[91m◄\x1B[37m══\x1B[90m█\x1B[94m██\x1B[96m█\x1B[94m██\x1B[90m█\x1B[37m══\x1B[91m►   \x1B[90mPilota tu nave a través del sector galáctico y sobrevive.   \x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║  \x1B[90m█\x1B[91m▒\x1B[93m▓\x1B[91m▒\x1B[90m█          \x1B[37mEvade asteroides, destruye drones y recoge power-ups.      \x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m╠══════════════════════════════════════════════════════════════════════════════════╣\n" << cn;
}

void mostrarMenuPrincipal(const std::string &pad) {
    std::string cn = "\x1B[0m";
    std::cout << pad << "\x1B[96m║  \x1B[92m[1]\x1B[37m Iniciar Sesion          \x1B[96m║\x1B[37m  \x1B[92m[2]\x1B[37m Registrarse                                  \x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║  \x1B[93m[3]\x1B[37m Records Globales        \x1B[96m║\x1B[37m  \x1B[93m[4]\x1B[37m Estadisticas Personales                      \x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║  \x1B[95m[5]\x1B[37m Historial de Partidas   \x1B[96m║\x1B[37m  \x1B[95m[6]\x1B[37m Informacion del Juego                        \x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m║  \x1B[90m[7]\x1B[37m Creditos y Creadores    \x1B[96m║\x1B[37m  \x1B[91m[8]\x1B[37m Salir del Sistema                            \x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m╠══════════════════════════════════════════════════════════════════════════════════╣\n" << cn;
    std::cout << pad << "\x1B[96m║  \x1B[93mCOMANDO \x1B[96m»\x1B[0m ";
}

void cerrarMenuLinea(const std::string &pad) {
    std::string cn = "\x1B[0m";
    std::cout << "\n";
    std::cout << pad << "\x1B[96m╚══════════════════════════════════════════════════════════════════════════════════╝\n" << cn;
}

// ─── Paneles de Contenido ─────────────────────────────────────────────────────

void mostrarPanelBordeado(const std::string &pad, const std::string &titulo, const std::vector<std::string> &lineas) {
    std::string cn = "\x1B[0m";
    std::cout << "\n";
    std::cout << pad << "\x1B[96m╔══════════════════════════════════════════════════════════════════════════════════╗\n" << cn;
    // Centrar título
    int relleno = 80 - (int)titulo.size();
    std::cout << pad << "\x1B[96m║\x1B[0m" << std::string(relleno / 2, ' ') << "\x1B[93m" << titulo << cn << std::string(relleno - relleno / 2, ' ') << "\x1B[96m║\n" << cn;
    std::cout << pad << "\x1B[96m╠══════════════════════════════════════════════════════════════════════════════════╣\n" << cn;
    for (const auto &l : lineas) {
        int esp = 80 - (int)l.size();
        if (esp < 0) esp = 0;
        std::cout << pad << "\x1B[96m║\x1B[37m " << l.substr(0, 79) << std::string(std::min(esp, 79 - (int)l.size()), ' ') << "\x1B[96m║\n" << cn;
    }
    std::cout << pad << "\x1B[96m╚══════════════════════════════════════════════════════════════════════════════════╝\n" << cn;
}

void mostrarInformacionJuego(const std::string &pad) {
    mostrarPanelBordeado(pad, " INFORMACION DE AERO EVADER PRO ", {
        "  Aero Evader Pro es un shooter arcade espacial retro-futurista.",
        "  Cada nivel intensifica la lluvia de asteroides, drones y cometas.",
        "  Gestiona tu combustible: se agota con el tiempo.",
        "  Recoge cometas [C] para recuperar combustible y ganar puntos.",
        "  Los power-ups [S] y [T] se guardan en inventario.",
        "  Presiona S para activar el Escudo, T para el Turbo.",
        "  El Escudo absorbe un impacto de meteoro o drone.",
        "  El Turbo acelera el refresco del juego pero gasta mas combustible.",
        "  Cada 200 pts subes de nivel: los enemigos son mas rapidos.",
        "  El combo se activa al recoger 5+ cometas seguidas (+2 pts/tick).",
    });
}

void mostrarGuia(const std::string &pad) {
    mostrarPanelBordeado(pad, " GUIA DE SOBREVIVENCIA ", {
        "  Flecha Izquierda / Derecha : mueve tu nave.",
        "  ESC                        : salir de la mision.",
        "  S / s                      : activar escudo (si tienes en inventario).",
        "  T / t                      : activar turbo (si tienes en inventario).",
        "",
        "  Meteoro [M]   → evita el impacto o pierde una vida.",
        "  Drone   [D]   → evita el impacto o pierde una vida.",
        "  Cometa  [C]   → recoge para ganar puntos y combustible.",
        "  Escudo  [S]   → recoge y guarda en inventario.",
        "  Turbo   [T]   → recoge y guarda en inventario.",
    });
}

void mostrarCreditos(const std::string &pad) {
    mostrarPanelBordeado(pad, " CREDITOS Y CREADORES ", {
        "  Desarrollador:  [Tu Nombre]",
        "  Proyecto:       Aero Evader Pro",
        "  Materia:        Programacion I",
        "  Universidad:    [Nombre de la Universidad]",
        "",
        "  Tecnologias: C++17 | MSVC | Windows Console API | ANSI/UTF-8",
        "  Graficos:    Unicode Block Art + ANSI 16-color",
        "",
        "  Gracias por jugar. El sector galactico te necesita, piloto.",
    });
}

// ─── Entrada y Main ───────────────────────────────────────────────────────────

int obtenerOpcionValida() {
    int opcion = 0;
    if (!(std::cin >> opcion)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return opcion;
}

void esperarEnter(const std::string &pad) {
    std::string cn = "\x1B[0m";
    std::cout << "\n" << pad << "  \x1B[90mPresiona Enter para continuar...\x1B[0m ";
    std::cin.get();
}

int main() {
    establecerTemaConsola();
    SistemaUsuarios sistema("usuarios.txt", "records.txt");
    Juego juego(78, 24);

    while (true) {
        std::system("cls");

        int termW = obtenerAnchoConsola();
        int margenIzq = std::max(0, (termW - 82) / 2);
        std::string pad(margenIzq, ' ');
        std::string cn = "\x1B[0m";

        mostrarEncabezado();
        mostrarMenuPrincipal(pad);

        int opcion = obtenerOpcionValida();
        cerrarMenuLinea(pad);

        if (opcion == 1) {
            std::string nombre, contrasena;
            std::cout << pad << "\n\x1B[96m  Nombre de usuario:\x1B[97m ";
            std::getline(std::cin, nombre);
            std::cout << pad << "\x1B[96m  Contrasena:\x1B[97m ";
            std::getline(std::cin, contrasena);

            if (sistema.iniciarSesion(nombre, contrasena)) {
                std::cout << "\n" << pad << "\x1B[92m  ✓ Bienvenido, " << nombre << ". Preparando cabina...\x1B[0m\n";
                esperarEnter(pad);
                Juego2D juego2d(800, 600);
                juego2d.iniciar(nombre);
                int puntajeFinal = juego2d.obtenerPuntaje();
                sistema.registrarRecord(nombre, puntajeFinal);
                sistema.actualizarPuntajeActual(puntajeFinal);
                esperarEnter(pad);
            } else {
                std::cout << "\n" << pad << "\x1B[91m  ✕ Error: usuario o contrasena incorrectos.\x1B[0m\n";
                esperarEnter(pad);
            }
        } else if (opcion == 2) {
            std::string nombre, contrasena;
            std::cout << pad << "\n\x1B[96m  Nuevo nombre de usuario:\x1B[97m ";
            std::getline(std::cin, nombre);
            std::cout << pad << "\x1B[96m  Nueva contrasena:\x1B[97m ";
            std::getline(std::cin, contrasena);
            if (sistema.registrarUsuario(nombre, contrasena)) {
                std::cout << "\n" << pad << "\x1B[92m  ✓ Usuario registrado con exito. Ya puedes iniciar sesion.\x1B[0m\n";
            } else {
                std::cout << "\n" << pad << "\x1B[91m  ✕ No se pudo registrar. Usuario ya existe o datos invalidos (sin espacios).\x1B[0m\n";
            }
            esperarEnter(pad);
        } else if (opcion == 3) {
            std::system("cls");
            sistema.mostrarRecords();
            esperarEnter(pad);
        } else if (opcion == 4) {
            std::system("cls");
            sistema.mostrarEstadisticasUsuario();
            esperarEnter(pad);
        } else if (opcion == 5) {
            std::system("cls");
            sistema.mostrarHistorialUsuario();
            esperarEnter(pad);
        } else if (opcion == 6) {
            std::system("cls");
            mostrarInformacionJuego(pad);
            mostrarGuia(pad);
            esperarEnter(pad);
        } else if (opcion == 7) {
            std::system("cls");
            mostrarCreditos(pad);
            esperarEnter(pad);
        } else if (opcion == 8) {
            std::system("cls");
            int termW2 = obtenerAnchoConsola();
            int termH2 = obtenerAltoConsola();
            int mIzq = std::max(0, (termW2 - 82) / 2);
            int mSup = std::max(0, (termH2 - 5) / 2);
            std::string pad2(mIzq, ' ');
            for (int i = 0; i < mSup; ++i) std::cout << "\n";
            std::cout << pad2 << "\x1B[96m╔══════════════════════════════════════════════════════════════════════════════════╗\n\x1B[0m";
            std::cout << pad2 << "\x1B[96m║\x1B[0m" << "       \x1B[93mGracias por pilotar Aero Evader Pro. Hasta la proxima, piloto.\x1B[0m       " << "\x1B[96m║\n\x1B[0m";
            std::cout << pad2 << "\x1B[96m╚══════════════════════════════════════════════════════════════════════════════════╝\n\x1B[0m";
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            break;
        } else {
            std::cout << "\n" << pad << "\x1B[91m  ✕ Opcion invalida. Intenta de nuevo.\x1B[0m\n";
            esperarEnter(pad);
        }
    }

    return 0;
}
