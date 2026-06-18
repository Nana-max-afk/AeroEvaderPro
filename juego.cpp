#include "juego.h"
#include <algorithm>
#include <chrono>
#include <conio.h>
#include <cstdlib>
#include <iostream>
#include <random>
#include <thread>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

Juego::Juego(int ancho, int alto)
    : anchoPantalla(ancho), altoPantalla(alto), puntaje(0), vidas(3), nivel(1),
      jugadorX(ancho / 2), jugadorY(alto - 3), juegoActivo(false),
      velocidadGeneral(175), combustible(100), combo(0), tiempoMilisegundos(0),
      escudoActivo(false), duracionEscudo(0), turboActivo(false),
      duracionTurbo(0), cantidadEscudos(0), cantidadTurbos(0),
      mensajeNotificacion(""), timerNotificacion(0) {}

void Juego::iniciar(const std::string &nombreJugador) {
  jugadorNombre = nombreJugador;
  puntaje = 0;
  vidas = 3;
  nivel = 1;
  combustible = 100;
  combo = 0;
  tiempoMilisegundos = 0;
  escudoActivo = false;
  duracionEscudo = 0;
  turboActivo = false;
  duracionTurbo = 0;
  cantidadEscudos = 0;
  cantidadTurbos = 0;
  mensajeNotificacion = "";
  timerNotificacion = 0;
  jugadorX = anchoPantalla / 2;
  jugadorY = altoPantalla - 3;
  juegoActivo = true;
  obstaculos.clear();
  inicializarEstrellas();

#ifdef _WIN32
  // Habilitar soporte para secuencias ANSI y ocultar cursor
  HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  if (hConsoleOut != INVALID_HANDLE_VALUE) {
    GetConsoleMode(hConsoleOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsoleOut, dwMode);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsoleOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsoleOut, &cursorInfo);
  }
#endif

  mostrarPantallaInicio();
  std::this_thread::sleep_for(std::chrono::milliseconds(1200));
  mostrarPantallaCarga();

  // Limpieza total del terminal antes de entrar al bucle de juego
  std::system("cls");

  auto ultimoTiempo = std::chrono::steady_clock::now();
  while (juegoActivo) {
    auto ahora = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                     ahora - ultimoTiempo)
                     .count();
    int actualVelocidad =
        turboActivo ? std::max(20, velocidadGeneral - 30) : velocidadGeneral;
    if (delta >= actualVelocidad) {
      ultimoTiempo = ahora;
      actualizarTiempo(static_cast<int>(delta));
      procesarEntrada();
      generarObstaculos();
      actualizarEscena();
      dibujarEscena();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(8));
  }

#ifdef _WIN32
  // Restaurar visibilidad del cursor al salir del juego
  if (hConsoleOut != INVALID_HANDLE_VALUE) {
    CONSOLE_CURSOR_INFO cursorInfoRest;
    GetConsoleCursorInfo(hConsoleOut, &cursorInfoRest);
    cursorInfoRest.bVisible = TRUE;
    SetConsoleCursorInfo(hConsoleOut, &cursorInfoRest);
  }
#endif

  // Limpieza total del terminal antes del informe final
  std::system("cls");
  mostrarInformeFinal();
}

void Juego::mostrarPantallaInicio() const {
  std::system("cls");

  int termW = obtenerAnchoTerminal();
  int termH = obtenerAltoTerminal();
  int margenIzq = std::max(0, (termW - 82) / 2);
  int margenSup = std::max(0, (termH - 22) / 2);
  std::string pad(margenIzq, ' ');
  std::string cn = "\x1B[0m";

  for (int i = 0; i < margenSup; ++i)
    std::cout << "\n";

  // Banner pixel art nave + título
  std::cout << pad
            << "\x1B["
               "96m╔═══════════════════════════════════════════════════════════"
               "═════════════════════════════════════╗\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "  \x1B[90m·  ·  ·    ·   · ·    ·    ·  ·   ·    ·  ·   ·  ·   "
               "·    ·   ·  ·   ·  · ·  \x1B[0m"
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "      \x1B[93m █████╗ ███████╗██████╗  ██████╗     ███████╗██╗ "
               "  ██╗ █████╗ \x1B[0m      "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "      \x1B[93m██╔══██╗██╔════╝██╔══██╗██╔═══██╗    ██╔════╝██║ "
               "  ██║██╔══██╗\x1B[0m      "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "      \x1B[93m███████║█████╗  ██████╔╝██║   ██║    █████╗  ██║ "
               "  ██║███████║\x1B[0m      "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "      \x1B[93m██╔══██║██╔══╝  ██╔══██╗██║   ██║    ██╔══╝  "
               "╚██╗ ██╔╝██╔══██║\x1B[0m      "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "      \x1B[93m██║  ██║███████╗██║  ██║╚██████╔╝    ███████╗ "
               "╚████╔╝ ██║  ██║\x1B[0m      "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "      \x1B[93m╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝ ╚═════╝     ╚══════╝  "
               "╚═══╝  ╚═╝  ╚═╝\x1B[0m      "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═════════════════════════════════════╣\n"
            << cn;

  // Ficha de piloto
  std::string pilotoLinea = "  PILOTO: " + jugadorNombre;
  int espacioPiloto = 80 - (int)pilotoLinea.size() - 14;
  if (espacioPiloto < 0)
    espacioPiloto = 0;
  std::cout << pad << "\x1B[96m║ \x1B[97m" << pilotoLinea
            << std::string(espacioPiloto, ' ')
            << "\x1B[93mMISION: ARENA ESPACIAL \x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;

  // Nave pixel art en la pantalla de inicio
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "                               \x1B[91m▲\x1B[0m                "
               "                              "
            << "\x1B[96m║\n"
            << cn;
  std::cout
      << pad << "\x1B[96m║\x1B[0m"
      << "                             \x1B[37m/\x1B[96m█\x1B[37m\\\x1B[0m     "
         "                                        "
      << "\x1B[96m║\n"
      << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "                "
               "\x1B[91m◄\x1B[37m══\x1B[90m█\x1B[94m██\x1B[96m█\x1B[94m██\x1B["
               "90m█\x1B[37m══\x1B[91m►\x1B[0m                                 "
               "          "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "                      "
               "\x1B[90m█\x1B[91m▒\x1B[93m▓\x1B[91m▒\x1B[90m█\x1B[0m           "
               "                                   "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;

  // Info de controles en 2 columnas
  std::cout << pad
            << "\x1B[96m║ \x1B[90m← →\x1B[37m Mover nave    \x1B[90mS\x1B[37m "
               "Activar Escudo    \x1B[90mT\x1B[37m Activar Turbo    "
               "\x1B[90mESC\x1B[37m Salir         \x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B[96m║ \x1B[96m[C]\x1B[37m Cometa → puntos+combustible   "
               "\x1B[96m[S]\x1B[37m Escudo → protección   \x1B[96m[T]\x1B[37m "
               "Turbo → velocidad  \x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "            \x1B[93m★\x1B[0m  Preparando sistemas de vuelo. "
               "Despegue inminente...  \x1B[93m★\x1B[0m           "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╚═══════════════════════════════════════════════════════════"
               "═══════════════════════╝\n"
            << cn;
}

void Juego::mostrarPantallaCarga() const {
  std::system("cls");

  int termW = obtenerAnchoTerminal();
  int termH = obtenerAltoTerminal();
  int margenIzq = std::max(0, (termW - 82) / 2);
  int margenSup = std::max(0, (termH - 12) / 2);
  std::string pad(margenIzq, ' ');
  std::string cn = "\x1B[0m";

  for (int i = 0; i < margenSup; ++i)
    std::cout << "\n";

  std::cout << pad
            << "\x1B["
               "96m╔═══════════════════════════════════════════════════════════"
               "═══════════════════════╗\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "                      \x1B[93m INICIALIZANDO HIPERESPACIO "
               "\x1B[0m                          "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "          \x1B[90m·  ★  ·  ·  ·   *   ·  ·   *   ·  ·  ·   ★  "
               "·  ·  *  ·  ·  ·  ★  ·\x1B[0m          "
            << "\x1B[96m║\n"
            << cn;

  // Barra de carga animada
  std::cout << pad << "\x1B[96m║\x1B[0m  \x1B[37m Cargando: [\x1B[0m";
  std::cout.flush();

  const int totalSegmentos = 60;
  for (int i = 0; i < totalSegmentos; ++i) {
    // Color dinámico según progreso
    if (i < 20) {
      std::cout << "\x1B[91m█"; // Rojo inicio
    } else if (i < 40) {
      std::cout << "\x1B[93m█"; // Amarillo medio
    } else {
      std::cout << "\x1B[92m█"; // Verde final
    }
    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }

  std::cout << "\x1B[0m\x1B[37m] \x1B[92m100%\x1B[0m  \x1B[96m║\n";

  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "          \x1B[90m·  ★  ·  ·  ·   *   ·  ·   *   ·  ·  ·   ★  "
               "·  ·  *  ·  ·  ·  ★  ·\x1B[0m          "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "         \x1B[92m✓\x1B[37m Motores encendidos    "
               "\x1B[92m✓\x1B[37m Escudos calibrados    \x1B[92m✓\x1B[37m "
               "Rumbo establecido   \x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╚═══════════════════════════════════════════════════════════"
               "═══════════════════════╝\n"
            << cn;

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Juego::inicializarEstrellas() {
  estrellas.clear();
  std::default_random_engine generador(
      (unsigned)std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<int> distrX(0, anchoPantalla - 1);
  std::uniform_int_distribution<int> distrY(0, altoPantalla - 1);
  std::uniform_int_distribution<int> distrCapa(1, 3);

  for (int i = 0; i < 70; ++i) {
    Estrella estrella{distrX(generador), distrY(generador),
                      distrCapa(generador)};
    estrellas.push_back(estrella);
  }
}

void Juego::actualizarEstrellas() {
  static int tickContador = 0;
  tickContador++;

  static std::default_random_engine generador(
      (unsigned)std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<int> distrX(0, anchoPantalla - 1);

  for (auto &estrella : estrellas) {
    bool mover = false;
    if (estrella.capa == 3) {
      mover = true; // Capa rápida se mueve cada tick
    } else if (estrella.capa == 2 && tickContador % 2 == 0) {
      mover = true; // Capa media se mueve cada 2 ticks
    } else if (estrella.capa == 1 && tickContador % 3 == 0) {
      mover = true; // Capa lenta se mueve cada 3 ticks
    }

    if (mover) {
      estrella.y += 1;
      if (estrella.y >= altoPantalla) {
        estrella.y = 0;
        estrella.x = distrX(generador);
      }
    }
  }
}

void Juego::dibujarEstrellas(std::vector<std::vector<Pixel>> &pantalla) const {
  for (const auto &estrella : estrellas) {
    if (estrella.y >= 0 && estrella.y < altoPantalla && estrella.x >= 0 &&
        estrella.x < anchoPantalla) {
      std::string brillo = ".";
      std::string color = "\x1B[90m"; // Gris oscuro por defecto

      if (estrella.capa == 3) {
        brillo = "*";
        color = "\x1B[97m"; // Blanco brillante para estrellas cercanas
      } else if (estrella.capa == 2) {
        brillo = "+";
        color = "\x1B[37m"; // Blanco apagado para estrellas medias
      } else {
        brillo = ".";
        color = "\x1B[90m"; // Gris oscuro para estrellas lejanas
      }

      pantalla[estrella.y][estrella.x].caracter = brillo;
      pantalla[estrella.y][estrella.x].colorAnsi = color;
    }
  }
}

void Juego::dibujarJugador(std::vector<std::vector<Pixel>> &pantalla) const {
  // Dibujo del escudo aura alrededor de la nave
  if (escudoActivo) {
    std::string colorEscudo = "\x1B[96m"; // Cian brillante
    for (int dx = -7; dx <= 7; ++dx) {
      int x = jugadorX + dx;
      int yTop = jugadorY - 4;
      if (x >= 0 && x < anchoPantalla && yTop >= 0 && yTop < altoPantalla) {
        pantalla[yTop][x].caracter = "▄";
        pantalla[yTop][x].colorAnsi = colorEscudo;
      }
      int yBottom = jugadorY + 1;
      if (x >= 0 && x < anchoPantalla && yBottom >= 0 &&
          yBottom < altoPantalla) {
        pantalla[yBottom][x].caracter = "▀";
        pantalla[yBottom][x].colorAnsi = colorEscudo;
      }
    }
    for (int dy = -4; dy <= 1; ++dy) {
      int y = jugadorY + dy;
      int xLeft = jugadorX - 7;
      if (xLeft >= 0 && xLeft < anchoPantalla && y >= 0 && y < altoPantalla) {
        pantalla[y][xLeft].caracter = "█";
        pantalla[y][xLeft].colorAnsi = colorEscudo;
      }
      int xRight = jugadorX + 7;
      if (xRight >= 0 && xRight < anchoPantalla && y >= 0 && y < altoPantalla) {
        pantalla[y][xRight].caracter = "█";
        pantalla[y][xRight].colorAnsi = colorEscudo;
      }
    }
  }

  auto dibujaNavePixel = [&](int rx, int ry, const std::string &caracter,
                             const std::string &color) {
    int x = jugadorX + rx;
    int y = jugadorY + ry;
    if (x >= 0 && x < anchoPantalla && y >= 0 && y < altoPantalla) {
      pantalla[y][x].caracter = caracter;
      pantalla[y][x].colorAnsi = color;
    }
  };

  // Fila -3: Punta / Cockpit (Nose cone)
  dibujaNavePixel(0, -3, "▲", "\x1B[91m"); // Rojo brillante

  // Fila -2: Cabina
  dibujaNavePixel(-1, -2, "/", "\x1B[37m");
  dibujaNavePixel(0, -2, "█", "\x1B[96m"); // Vidrio cian brillante
  dibujaNavePixel(1, -2, "\\", "\x1B[37m");

  // Fila -1: Cuerpo de la nave y alas
  dibujaNavePixel(-6, -1, "◄", "\x1B[91m"); // Extremo de ala izquierda
  dibujaNavePixel(-5, -1, "═", "\x1B[37m");
  dibujaNavePixel(-4, -1, "═", "\x1B[37m");
  dibujaNavePixel(-3, -1, "█", "\x1B[90m"); // Metal oscuro
  dibujaNavePixel(-2, -1, "█", "\x1B[94m"); // Azul metálico
  dibujaNavePixel(-1, -1, "█", "\x1B[94m");
  dibujaNavePixel(0, -1, "█", "\x1B[96m"); // Centro cian
  dibujaNavePixel(1, -1, "█", "\x1B[94m");
  dibujaNavePixel(2, -1, "█", "\x1B[94m");
  dibujaNavePixel(3, -1, "█", "\x1B[90m");
  dibujaNavePixel(4, -1, "═", "\x1B[37m");
  dibujaNavePixel(5, -1, "═", "\x1B[37m");
  dibujaNavePixel(6, -1, "►", "\x1B[91m"); // Extremo de ala derecha

  // Fila 0: Motores y Fuego propulsor
  dibujaNavePixel(-2, 0, "█", "\x1B[90m");
  dibujaNavePixel(-1, 0, "▒", "\x1B[91m"); // Fuego rojo
  dibujaNavePixel(0, 0, "▓", "\x1B[93m");  // Fuego naranja/amarillo
  dibujaNavePixel(1, 0, "▒", "\x1B[91m");
  dibujaNavePixel(2, 0, "█", "\x1B[90m");

  // Si el turbo está activo, añadir efectos de propulsión gigante en las alas
  if (turboActivo) {
    dibujaNavePixel(-4, 0, "▼", "\x1B[93m");
    dibujaNavePixel(-1, 1, "▼", "\x1B[97m"); // Flama extra
    dibujaNavePixel(0, 1, "█", "\x1B[93m");
    dibujaNavePixel(1, 1, "▼", "\x1B[97m");
    dibujaNavePixel(4, 0, "▼", "\x1B[93m");
  }
}

void Juego::dibujarObstaculo(std::vector<std::vector<Pixel>> &pantalla,
                             const Obstaculo &obs) const {
  auto dibujaPixel = [&](int rx, int ry, const std::string &caracter,
                         const std::string &color) {
    int x = obs.x + rx;
    int y = obs.y + ry;
    if (x >= 0 && x < anchoPantalla && y >= 0 && y < altoPantalla) {
      pantalla[y][x].caracter = caracter;
      pantalla[y][x].colorAnsi = color;
    }
  };

  switch (obs.tipo) {
  case TipoObstaculo::Meteoro:
    // Roca pesada y rugosa en rojo óxido/marrón
    dibujaPixel(-1, 0, "▞", "\x1B[31m");
    dibujaPixel(0, 0, "█", "\x1B[31m");
    dibujaPixel(1, 0, "▚", "\x1B[31m");
    dibujaPixel(-1, 1, "▚", "\x1B[31m");
    dibujaPixel(0, 1, "█", "\x1B[31m");
    dibujaPixel(1, 1, "▞", "\x1B[31m");
    break;
  case TipoObstaculo::Drone:
    // Drone robótico militar verde con alas amarillas
    dibujaPixel(-2, 0, "▞", "\x1B[92m");
    dibujaPixel(-1, 0, "█", "\x1B[92m");
    dibujaPixel(0, 0, "█", "\x1B[92m");
    dibujaPixel(1, 0, "█", "\x1B[92m");
    dibujaPixel(2, 0, "▚", "\x1B[92m");

    dibujaPixel(-3, 1, "◄", "\x1B[93m");
    dibujaPixel(-2, 1, "═", "\x1B[93m");
    dibujaPixel(-1, 1, "═", "\x1B[93m");
    dibujaPixel(0, 1, "█", "\x1B[92m");
    dibujaPixel(1, 1, "═", "\x1B[93m");
    dibujaPixel(2, 1, "═", "\x1B[93m");
    dibujaPixel(3, 1, "►", "\x1B[93m");
    break;
  case TipoObstaculo::Cometa:
    // Cometa cósmico con estela degradada
    dibujaPixel(-4, 0, "░", "\x1B[94m"); // Cola azul oscuro
    dibujaPixel(-3, 0, "▒", "\x1B[96m"); // Cola cian claro
    dibujaPixel(-2, 0, "▓", "\x1B[96m");
    dibujaPixel(-1, 0, "█", "\x1B[97m"); // Cabezal blanco
    dibujaPixel(0, 0, "★", "\x1B[93m");  // Núcleo estrella dorada
    dibujaPixel(1, 0, "═", "\x1B[95m");  // Punta magenta
    dibujaPixel(2, 0, "═", "\x1B[95m");
    break;
  case TipoObstaculo::PowerUpEscudo:
    // Contenedor holográfico cian
    dibujaPixel(-2, 0, "║", "\x1B[96m");
    dibujaPixel(-1, 0, "[", "\x1B[96m");
    dibujaPixel(0, 0, "S", "\x1B[97m");
    dibujaPixel(1, 0, "]", "\x1B[96m");
    dibujaPixel(2, 0, "║", "\x1B[96m");
    break;
  case TipoObstaculo::PowerUpTurbo:
    // Contenedor combustible naranja/rojo
    dibujaPixel(-2, 0, "║", "\x1B[91m");
    dibujaPixel(-1, 0, "[", "\x1B[91m");
    dibujaPixel(0, 0, "T", "\x1B[93m");
    dibujaPixel(1, 0, "]", "\x1B[91m");
    dibujaPixel(2, 0, "║", "\x1B[91m");
    break;
  }
}

void Juego::limpiarPantalla() const {
#ifdef _WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsole != INVALID_HANDLE_VALUE) {
    COORD home = {0, 0};
    SetConsoleCursorPosition(hConsole, home);
    return;
  }
#endif
  std::cout << "\x1B[2J\x1B[H";
}

void Juego::dibujarEscena() const {
  // Inicializar matriz de Pixels
  std::vector<std::vector<Pixel>> pantalla(altoPantalla,
                                           std::vector<Pixel>(anchoPantalla));

  dibujarEstrellas(pantalla);
  dibujarParticulas(pantalla); // Dibujar partículas de explosiones
  for (const auto &obs : obstaculos) {
    dibujarObstaculo(pantalla, obs);
  }
  dibujarJugador(pantalla);

  // Obtener dimensiones del terminal para centrado dinámico
  int termAncho = obtenerAnchoTerminal();
  int termAlto = obtenerAltoTerminal();

  int margenIzquierdo = std::max(0, (termAncho - 80) / 2);
  int margenSuperior = std::max(0, (termAlto - 36) / 2);
  std::string margenStr(margenIzquierdo, ' ');

  std::string salida;
  // Reservar espacio suficiente para los caracteres de la pantalla más los
  // códigos ANSI de color
  salida.reserve((anchoPantalla + 30 + margenIzquierdo) * (altoPantalla + 25));

  limpiarPantalla();

  // Inyectar margen superior
  for (int i = 0; i < margenSuperior; ++i) {
    salida += "\n";
  }

  // Calcular colorMarco de forma reactiva (Cyberpunk HUD)
  std::string colorMarco = "\x1B[96m"; // Cian neón por defecto
  if (vidas <= 1 || combustible <= 20) {
    // Alerta roja parpadeante
    if ((tiempoMilisegundos / 200) % 2 == 0) {
      colorMarco = "\x1B[91m"; // Rojo brillante
    } else {
      colorMarco = "\x1B[31m"; // Rojo oscuro
    }
  } else if (turboActivo) {
    colorMarco = "\x1B[93m"; // Amarillo brillante
  } else if (escudoActivo) {
    colorMarco = "\x1B[94m"; // Azul brillante
  }

  std::string colorNormal = "\x1B[0m";

  // Borde superior
  salida += margenStr + colorMarco + "╔" + std::string(anchoPantalla, '═') +
            "╗\n" + colorNormal;

  std::string lineaTitulo = "  AERO EVADER PRO  ";
  int relleno = anchoPantalla - (int)lineaTitulo.size();
  if (relleno < 0)
    relleno = 0;

  salida += margenStr + colorMarco + "║" + colorNormal +
            std::string(relleno / 2, ' ') + "\x1B[93m" + lineaTitulo +
            colorNormal + std::string(relleno - relleno / 2, ' ') + colorMarco +
            "║\n" + colorNormal;
  salida += margenStr + colorMarco + "╠" + std::string(anchoPantalla, '═') +
            "╣\n" + colorNormal;

  std::string leyenda =
      " [M] Asteroide   [C] Cometa   [D] Drone   [S] Escudo   [T] Turbo ";
  if ((int)leyenda.size() < anchoPantalla) {
    leyenda += std::string(anchoPantalla - (int)leyenda.size(), ' ');
  }
  salida += margenStr + colorMarco + "║" + "\x1B[37m" +
            leyenda.substr(0, anchoPantalla) + colorMarco + "║\n" + colorNormal;

  // Barra de notificación de eventos en juego
  if (!mensajeNotificacion.empty() && timerNotificacion > 0) {
    // Calcular ancho visible aproximado del mensaje sin códigos ANSI
    // (aproximado) El mensaje ya incluye padding de 2 espacios por lado
    std::string notifLinea = mensajeNotificacion;
    // Centrar el mensaje dentro del ancho del campo
    int anchoVisible = 0;
    bool enCodigo = false;
    for (char c : notifLinea) {
      if (c == '\x1B') {
        enCodigo = true;
        continue;
      }
      if (enCodigo) {
        if (c == 'm')
          enCodigo = false;
        continue;
      }
      anchoVisible++;
    }
    int paddingLeft = std::max(0, (anchoPantalla - anchoVisible) / 2);
    int paddingRight = std::max(0, anchoPantalla - anchoVisible - paddingLeft);
    std::string notifFila = std::string(paddingLeft, ' ') + notifLinea +
                            std::string(paddingRight, ' ');
    salida += margenStr + colorMarco + "║" + "\x1B[40m" +
              notifFila.substr(0, notifFila.size()) + colorMarco + "║\n" +
              colorNormal;
  } else {
    // Separador normal cuando no hay notificación
    salida += margenStr + colorMarco + "╠" + std::string(anchoPantalla, '═') +
              "╣\n" + colorNormal;
  }

  // Pintar la pantalla celda por celda cuidando de no alterar las dimensiones
  for (int y = 0; y < altoPantalla; ++y) {
    salida += margenStr + colorMarco + "║" + colorNormal;
    for (int x = 0; x < anchoPantalla; ++x) {
      if (!pantalla[y][x].colorAnsi.empty()) {
        salida += pantalla[y][x].colorAnsi;
        salida += pantalla[y][x].caracter;
        salida += colorNormal;
      } else {
        salida += pantalla[y][x].caracter;
      }
    }
    salida += colorMarco + "║\n" + colorNormal;
  }
  salida += margenStr + colorMarco + "╚" + std::string(anchoPantalla, '═') +
            "╝\n" + colorNormal;

  // Acoplar la barra de estado completa directamente en el buffer de salida
  mostrarBarraEstado(salida);

  // Una sola escritura atómica de toda la escena para evitar parpadeos
  std::cout << salida;
}

void Juego::mostrarBarraEstado(std::string &salida) const {
  // Calcular colorMarco de forma reactiva en el HUD
  std::string colorMarco = "\x1B[96m"; // Cian neón por defecto
  if (vidas <= 1 || combustible <= 20) {
    if ((tiempoMilisegundos / 200) % 2 == 0) {
      colorMarco = "\x1B[91m"; // Rojo brillante
    } else {
      colorMarco = "\x1B[31m"; // Rojo oscuro
    }
  } else if (turboActivo) {
    colorMarco = "\x1B[93m"; // Amarillo brillante
  } else if (escudoActivo) {
    colorMarco = "\x1B[94m"; // Azul brillante
  }

  std::string colorNormal = "\x1B[0m";
  std::string colorGris = "\x1B[90m";

  int termAncho = obtenerAnchoTerminal();
  int margenIzquierdo = std::max(0, (termAncho - 80) / 2);
  std::string margenStr(margenIzquierdo, ' ');

  salida += margenStr + colorMarco + "╠" + std::string(anchoPantalla, '═') +
            "╣\n" + colorNormal;

  // Linea 1: Piloto y Nivel
  std::string pilotoTxt = " Piloto: " + jugadorNombre;
  std::string nivelTxt = "Nivel: " + std::to_string(nivel) + " ";
  int espacio1 =
      anchoPantalla - (int)pilotoTxt.size() - (int)nivelTxt.size() - 2;
  if (espacio1 < 0)
    espacio1 = 0;
  salida += margenStr + colorMarco + "║ " + colorNormal + "\x1B[97m" +
            pilotoTxt + colorNormal + std::string(espacio1, ' ') + "\x1B[93m" +
            nivelTxt + colorNormal + colorMarco + " ║\n" + colorNormal;

  // Linea 2: Puntaje y Vidas (corazones ♥)
  std::string puntajeTxt = " Puntaje: " + std::to_string(puntaje);
  std::string corazones = "Vidas: ";
  for (int i = 0; i < vidas; ++i) {
    corazones += "♥ ";
  }
  // El carácter "♥" y el espacio ocupan 2 caracteres visibles en pantalla
  int visibleVidasLen = 7 + (vidas * 2);
  int espacio2 = anchoPantalla - (int)puntajeTxt.size() - visibleVidasLen - 2;
  if (espacio2 < 0)
    espacio2 = 0;

  salida += margenStr + colorMarco + "║ " + colorNormal + "\x1B[92m" +
            puntajeTxt + colorNormal + std::string(espacio2, ' ') + "\x1B[91m" +
            corazones + colorNormal + colorMarco + " ║\n" + colorNormal;

  // Linea 3: Combustible (barra gráfica) y Tiempo
  int celdasActivas = std::min(15, (combustible * 15) / 140);
  if (celdasActivas < 0)
    celdasActivas = 0;
  std::string combustibleBarra = " Combustible: [";
  std::string colorCombustible = "\x1B[92m"; // Verde por defecto
  if (combustible < 20) {
    colorCombustible = "\x1B[91m"; // Rojo
  } else if (combustible < 50) {
    colorCombustible = "\x1B[93m"; // Amarillo
  }

  std::string bar;
  for (int i = 0; i < 15; ++i) {
    if (i < celdasActivas)
      bar += "█";
    else
      bar += "░";
  }

  std::string combustibleTexto = "⛽ " + std::to_string(combustible) + "% ";
  std::string tiempoTxt =
      "Tiempo: " + std::to_string(tiempoMilisegundos / 1000) + "s ";

  int visibleCombustibleLen =
      14 + 15 + 1 + combustibleTexto.size(); // Largo de la barra
  int espacio3 =
      anchoPantalla - visibleCombustibleLen - (int)tiempoTxt.size() - 2;
  if (espacio3 < 0)
    espacio3 = 0;

  salida += margenStr + colorMarco + "║" + colorNormal + colorCombustible +
            combustibleBarra + bar + "] " + combustibleTexto + colorNormal +
            std::string(espacio3, ' ') + "\x1B[95m" + tiempoTxt + colorNormal +
            colorMarco + " ║\n" + colorNormal;

  // Linea 4: Inventario de Powerups y Estados activos
  int espacio4 =
      anchoPantalla -
      53; // Espaciado fijo calculado para calzar exactamente con el ancho
  if (espacio4 < 0)
    espacio4 = 0;

  salida += margenStr + colorMarco + "║" + colorNormal + "\x1B[96m" +
            " [S] Escudos: " + std::to_string(cantidadEscudos) + " (" +
            (escudoActivo ? "\x1B[96mACTIVO" : "\x1B[90mLISTO") + "\x1B[96m)" +
            colorNormal + std::string(espacio4, ' ') + "\x1B[93m" +
            " [T] Turbos: " + std::to_string(cantidadTurbos) + " (" +
            (turboActivo ? "\x1B[93mACTIVO" : "\x1B[90mLISTO") + "\x1B[93m) " +
            colorNormal + colorMarco + "║\n" + colorNormal;

  // Linea 5: Controles de ayuda
  std::string controles = " Controles: <- / ->  |  S = ACTIVAR ESCUDO  |  T = "
                          "ACTIVAR TURBO  |  ESC = SALIR ";
  int espacio5 = anchoPantalla - (int)controles.size();
  if (espacio5 < 0)
    espacio5 = 0;
  salida += margenStr + colorMarco + "║" + colorGris + controles +
            std::string(espacio5, ' ') + colorMarco + "║\n" + colorNormal;
  salida += margenStr + colorMarco + "╚" + std::string(anchoPantalla, '═') +
            "╝\n" + colorNormal;
}

void Juego::procesarEntrada() {
  const int radioNave = 6; // Modificado para evitar el recorte visual de las
                           // alas en los límites izquierdo/derecho
  while (_kbhit()) {
    int tecla = _getch();
    if (tecla == 0 || tecla == 224) {
      tecla = _getch();
      if (tecla == 75) {
        jugadorX = std::max(radioNave, jugadorX - 2);
      } else if (tecla == 77) {
        jugadorX = std::min(anchoPantalla - radioNave - 1, jugadorX + 2);
      }
    } else if (tecla == 27) { // ESC
      juegoActivo = false;
    } else if (tecla == 's' || tecla == 'S') { // Activar Escudo manual
      if (cantidadEscudos > 0 && !escudoActivo) {
        escudoActivo = true;
        duracionEscudo = 14;
        cantidadEscudos--;
      }
    } else if (tecla == 't' || tecla == 'T') { // Activar Turbo manual
      if (cantidadTurbos > 0 && !turboActivo) {
        turboActivo = true;
        duracionTurbo = 18;
        cantidadTurbos--;
      }
    }
  }
}

void Juego::generarObstaculos() {
  static std::default_random_engine generador(
      (unsigned)std::chrono::system_clock::now().time_since_epoch().count());
  static std::uniform_int_distribution<int> distrChance(0, 99);
  static std::uniform_int_distribution<int> distrVel(1, 2);

  int maxObstaculos = std::min(18, 6 + nivel * 2);
  if (obstaculos.size() > maxObstaculos) {
    return;
  }

  int chance = distrChance(generador);
  int umbral = std::min(32, 18 + nivel * 2);
  if (chance < umbral) {
    Obstaculo nuevo;
    nuevo.y = 0;
    nuevo.velocidad = std::min(4, 1 + (nivel / 4) + distrVel(generador));

    TipoObstaculo tipo;
    int minX = 1;
    int maxX = anchoPantalla - 2;

    // Limitar la coordenada de spawn x según el ancho del obstáculo para evitar
    // salirse de los límites
    if (chance < 4) {
      tipo = TipoObstaculo::PowerUpEscudo;
      minX = 2;
      maxX = anchoPantalla - 3; // Ancho 5
    } else if (chance < 8) {
      tipo = TipoObstaculo::PowerUpTurbo;
      minX = 2;
      maxX = anchoPantalla - 3; // Ancho 5
    } else if (chance < 24) {
      tipo = TipoObstaculo::Cometa;
      minX = 4;
      maxX = anchoPantalla - 3; // Ancho 7
    } else if (chance < 50) {
      tipo = TipoObstaculo::Drone;
      minX = 3;
      maxX = anchoPantalla - 4; // Ancho 7
    } else {
      tipo = TipoObstaculo::Meteoro;
      minX = 1;
      maxX = anchoPantalla - 2; // Ancho 3
    }

    std::uniform_int_distribution<int> distrX(minX, maxX);
    nuevo.x = distrX(generador);
    nuevo.tipo = tipo;
    obstaculos.push_back(nuevo);
  }
}

void Juego::actualizarEscena() {
  actualizarEstrellas();
  actualizarParticulas();
  for (auto &obs : obstaculos) {
    obs.y += obs.velocidad;
  }

  obstaculos.erase(std::remove_if(obstaculos.begin(), obstaculos.end(),
                                  [&](const Obstaculo &obs) {
                                    if (obs.y > altoPantalla - 1) {
                                      if (obs.tipo == TipoObstaculo::Meteoro ||
                                          obs.tipo == TipoObstaculo::Drone) {
                                        puntaje += 15;
                                      } else if (obs.tipo ==
                                                 TipoObstaculo::Cometa) {
                                        puntaje += 12;
                                      }
                                      return true;
                                    }
                                    return false;
                                  }),
                   obstaculos.end());

  for (auto it = obstaculos.begin(); it != obstaculos.end();) {
    // Límites de colisión de la nave (13 x 4)
    int naveIzq = jugadorX - 6;
    int naveDer = jugadorX + 6;
    int naveSup = jugadorY - 3;
    int naveInf = jugadorY;

    // Límites del obstáculo según su tipo real
    int obsIzq = it->x;
    int obsDer = it->x;
    int obsSup = it->y;
    int obsInf = it->y;

    switch (it->tipo) {
    case TipoObstaculo::Meteoro:
      obsIzq = it->x - 1;
      obsDer = it->x + 1;
      obsSup = it->y;
      obsInf = it->y + 1;
      break;
    case TipoObstaculo::Drone:
      obsIzq = it->x - 3;
      obsDer = it->x + 3;
      obsSup = it->y;
      obsInf = it->y + 1;
      break;
    case TipoObstaculo::Cometa:
      obsIzq = it->x - 4;
      obsDer = it->x + 2;
      obsSup = it->y;
      obsInf = it->y;
      break;
    case TipoObstaculo::PowerUpEscudo:
    case TipoObstaculo::PowerUpTurbo:
      obsIzq = it->x - 2;
      obsDer = it->x + 2;
      obsSup = it->y;
      obsInf = it->y;
      break;
    }

    // Detección de colisión exacta por superposición de áreas (Bounding Box)
    bool colision = (obsIzq <= naveDer && obsDer >= naveIzq &&
                     obsSup <= naveInf && obsInf >= naveSup);

    if (colision) {
      switch (it->tipo) {
      case TipoObstaculo::Meteoro:
      case TipoObstaculo::Drone:
        if (escudoActivo) {
          crearExplosion(it->x, it->y, "\x1B[96m", 12);
          escudoActivo = false;
          duracionEscudo = 0;
          puntaje += 10;
          mensajeNotificacion =
              "  \x1B[96m★ ESCUDO ABSORBE EL IMPACTO! +10 ★\x1B[0m  ";
          timerNotificacion = 6;
        } else {
          crearExplosion(it->x, it->y, "\x1B[91m", 20);
          vidas -= 1;
          combo = 0;
          mensajeNotificacion = "  \x1B[91m✕ IMPACTO! -1 VIDA ✕\x1B[0m  ";
          timerNotificacion = 8;
        }
        break;
      case TipoObstaculo::Cometa:
        crearExplosion(it->x, it->y, "\x1B[93m", 12);
        puntaje += 30;
        combo += 1;
        combustible = std::min(130, combustible + 12);
        mensajeNotificacion =
            "  \x1B[93m★ COMETA! +30 pts +12% combustible ★\x1B[0m  ";
        timerNotificacion = 5;
        break;
      case TipoObstaculo::PowerUpEscudo:
        crearExplosion(it->x, it->y, "\x1B[96m", 12);
        cantidadEscudos =
            std::min(5, cantidadEscudos + 1); // Guardar en inventario
        puntaje += 25;
        mensajeNotificacion =
            "  \x1B[96m◈ ESCUDO RECOGIDO! Presiona S para activar ◈\x1B[0m  ";
        timerNotificacion = 7;
        break;
      case TipoObstaculo::PowerUpTurbo:
        crearExplosion(it->x, it->y, "\x1B[93m", 12);
        cantidadTurbos =
            std::min(5, cantidadTurbos + 1); // Guardar en inventario
        puntaje += 25;
        mensajeNotificacion =
            "  \x1B[93m◈ TURBO RECOGIDO! Presiona T para activar ◈\x1B[0m  ";
        timerNotificacion = 7;
        break;
      }
      it = obstaculos.erase(it);
    } else {
      ++it;
    }
  }

  if (duracionEscudo > 0) {
    duracionEscudo -= 1;
    if (duracionEscudo == 0) {
      escudoActivo = false;
    }
  }
  if (duracionTurbo > 0) {
    duracionTurbo -= 1;
    if (duracionTurbo == 0) {
      turboActivo = false;
    }
  }

  int gastoCombustible = 1 + (nivel / 4);
  if (combustible > 0) {
    combustible -= gastoCombustible;
  } else {
    vidas -= 1;
    combustible = 20;
    combo = 0;
  }

  if (combo >= 5) {
    puntaje += 2;
    if (timerNotificacion == 0) {
      mensajeNotificacion = "  \x1B[92m✦ COMBO x" + std::to_string(combo) +
                            "! Bonus activo ✦\x1B[0m  ";
      timerNotificacion = 3;
    }
  }

  // Aviso de combustible bajo
  if (combustible <= 20 && combustible > 0 && timerNotificacion == 0) {
    mensajeNotificacion =
        "  \x1B[91m⚠ COMBUSTIBLE CRITICO! Recoge cometas ⚠\x1B[0m  ";
    timerNotificacion = 4;
  }

  // Decrement timer notificacion
  if (timerNotificacion > 0) {
    timerNotificacion--;
    if (timerNotificacion == 0) {
      mensajeNotificacion = "";
    }
  }

  actualizarNivel();
  if (vidas <= 0) {
    juegoActivo = false;
  }
}

void Juego::actualizarNivel() {
  int nuevoNivel = puntaje / 200 + 1;
  if (nuevoNivel != nivel) {
    nivel = nuevoNivel;
    if (velocidadGeneral > 30) {
      velocidadGeneral -= 6;
    }
    if (nivel % 3 == 0) {
      combustible = std::min(140, combustible + 15);
    }
  }
}

char Juego::dibujarObjeto(const Obstaculo &obs) const {
  switch (obs.tipo) {
  case TipoObstaculo::Meteoro:
    return 'M';
  case TipoObstaculo::Drone:
    return 'D';
  case TipoObstaculo::Cometa:
    return 'C';
  case TipoObstaculo::PowerUpEscudo:
    return 'S';
  case TipoObstaculo::PowerUpTurbo:
    return 'T';
  default:
    return 'X';
  }
}

void Juego::mostrarInformeFinal() const {
  std::system("cls");

  int termW = obtenerAnchoTerminal();
  int termH = obtenerAltoTerminal();
  int margenIzq = std::max(0, (termW - 82) / 2);
  int margenSup = std::max(0, (termH - 26) / 2);
  std::string pad(margenIzq, ' ');
  std::string cn = "\x1B[0m";

  // Calificar rendimiento
  std::string rango;
  std::string colorRango;
  if (puntaje >= 2000) {
    rango = "  S - LEGENDARIO  ";
    colorRango = "\x1B[93m";
  } else if (puntaje >= 1200) {
    rango = "  A - EXCELENTE   ";
    colorRango = "\x1B[96m";
  } else if (puntaje >= 700) {
    rango = "  B - BIEN HECHO  ";
    colorRango = "\x1B[92m";
  } else if (puntaje >= 300) {
    rango = "  C - ACEPTABLE   ";
    colorRango = "\x1B[93m";
  } else {
    rango = "  D - NECESITAS   ENTRENAR  ";
    colorRango = "\x1B[91m";
  }

  for (int i = 0; i < margenSup; ++i)
    std::cout << "\n";

  std::cout << pad
            << "\x1B["
               "96m╔═══════════════════════════════════════════════════════════"
               "═══════════════════════╗\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "                                                               "
               "                 "
            << "\x1B[96m║\n"
            << cn;
  if (vidas > 0) {
    std::cout << pad << "\x1B[96m║\x1B[0m"
              << "                       \x1B[92m  ★  MISION COMPLETADA  ★  "
                 "\x1B[0m                         "
              << "\x1B[96m║\n"
              << cn;
  } else {
    std::cout << pad << "\x1B[96m║\x1B[0m"
              << "                       \x1B[91m  ✕  NAVE DESTRUIDA  ✕  "
                 "\x1B[0m                              "
              << "\x1B[96m║\n"
              << cn;
  }
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "                                                               "
               "                 "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;

  // Rango / Calificación
  std::cout << pad << "\x1B[96m║ \x1B[37mRANGO DE MISION:  " << colorRango
            << "[ " << rango << " ]" << cn
            << std::string(std::max(0, 39 - (int)rango.size()), ' ')
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;

  // Ficha del piloto
  std::cout << pad
            << "\x1B[96m║ \x1B[90mPILOTO\x1B[0m                                "
               "                                         \x1B[96m║\n"
            << cn;
  auto lineaStat = [&](const std::string &etiq, const std::string &valor,
                       const std::string &color) {
    int espacios = 76 - (int)etiq.size() - (int)valor.size();
    if (espacios < 1)
      espacios = 1;
    std::cout << pad << "\x1B[96m║  \x1B[90m" << etiq << "\x1B[0m"
              << std::string(espacios, ' ') << color << valor << "  \x1B[96m║\n"
              << cn;
  };

  lineaStat("Nombre:", jugadorNombre, "\x1B[97m");
  lineaStat("Puntaje final:", std::to_string(puntaje) + " pts", "\x1B[93m");
  lineaStat("Nivel alcanzado:", "Nivel " + std::to_string(nivel), "\x1B[96m");

  // Vidas como corazones
  std::string corazonesStr;
  for (int i = 0; i < vidas; ++i)
    corazonesStr += "♥ ";
  if (corazonesStr.empty())
    corazonesStr = "✕ Sin vidas";
  lineaStat("Vidas restantes:", corazonesStr, "\x1B[91m");

  lineaStat("Combustible final:", std::to_string(combustible) + "%",
            combustible > 50   ? "\x1B[92m"
            : combustible > 20 ? "\x1B[93m"
                               : "\x1B[91m");
  lineaStat("Tiempo de mision:",
            std::to_string(tiempoMilisegundos / 1000) + " segundos",
            "\x1B[95m");
  lineaStat("Combo maximo:", "x" + std::to_string(combo),
            combo >= 5 ? "\x1B[92m" : "\x1B[37m");
  lineaStat("Escudos restantes:",
            std::to_string(cantidadEscudos) + " en inventario", "\x1B[96m");
  lineaStat("Turbos restantes:",
            std::to_string(cantidadTurbos) + " en inventario", "\x1B[93m");

  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;

  // Trucos y consejos según rango
  if (puntaje < 300) {
    std::cout
        << pad
        << "\x1B[96m║ \x1B[93m★\x1B[37m TIP: Recoge cometas para recargar "
           "combustible y ganar puntos extra.           \x1B[96m║\n"
        << cn;
  } else if (puntaje < 700) {
    std::cout
        << pad
        << "\x1B[96m║ \x1B[93m★\x1B[37m TIP: Guarda el escudo para situaciones "
           "criticas y no lo actives de entrada.   \x1B[96m║\n"
        << cn;
  } else {
    std::cout << pad
              << "\x1B[96m║ \x1B[92m★\x1B[37m Excelente trabajo, piloto. El "
                 "sector necesita a los mejores como tu.         \x1B[96m║\n"
              << cn;
  }

  std::cout << pad
            << "\x1B["
               "96m╠═══════════════════════════════════════════════════════════"
               "═══════════════════════╣\n"
            << cn;
  std::cout << pad << "\x1B[96m║\x1B[0m"
            << "              \x1B[90mPresiona cualquier tecla para volver al "
               "menu...\x1B[0m                   "
            << "\x1B[96m║\n"
            << cn;
  std::cout << pad
            << "\x1B["
               "96m╚═══════════════════════════════════════════════════════════"
               "═══════════════════════╝\n"
            << cn;

  while (!_kbhit()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  _getch();
}

void Juego::actualizarTiempo(int deltaMs) {
  tiempoMilisegundos += deltaMs;
  if (tiempoMilisegundos >= 800) {
    int bonos = tiempoMilisegundos / 800;
    puntaje += 2 * bonos;
    tiempoMilisegundos %= 800;
  }
}

int Juego::obtenerPuntaje() const { return puntaje; }

int Juego::obtenerAnchoTerminal() const {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
  }
#endif
  return 80;
}

int Juego::obtenerAltoTerminal() const {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  }
#endif
  return 25;
}

void Juego::crearExplosion(int x, int y, const std::string &colorBase,
                           int cantidad) {
  std::default_random_engine generador(
      (unsigned)std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<double> distrV(-1.2, 1.2);
  std::uniform_int_distribution<int> distrVida(3, 7);

  std::vector<std::string> caracteres = {"*", "+", ".", "o", "x"};
  std::uniform_int_distribution<int> distrChar(0, (int)caracteres.size() - 1);

  for (int i = 0; i < cantidad; ++i) {
    Particula p;
    p.x = x;
    p.y = y;
    p.vx = distrV(generador);
    p.vy = distrV(generador) - 0.3; // Pequeño impulso hacia arriba
    p.vida = distrVida(generador);
    p.caracter = caracteres[distrChar(generador)];
    p.colorAnsi = colorBase;
    particulas.push_back(p);
  }
}

void Juego::actualizarParticulas() {
  for (auto &p : particulas) {
    p.x += p.vx;
    p.y += p.vy;
    p.vida--;
  }
  particulas.erase(std::remove_if(particulas.begin(), particulas.end(),
                                  [&](const Particula &p) {
                                    return p.vida <= 0 || p.x < 0 ||
                                           p.x >= anchoPantalla || p.y < 0 ||
                                           p.y >= altoPantalla;
                                  }),
                   particulas.end());
}

void Juego::dibujarParticulas(std::vector<std::vector<Pixel>> &pantalla) const {
  for (const auto &p : particulas) {
    int px = static_cast<int>(p.x);
    int py = static_cast<int>(p.y);
    if (px >= 0 && px < anchoPantalla && py >= 0 && py < altoPantalla) {
      pantalla[py][px].caracter = p.caracter;
      pantalla[py][px].colorAnsi = p.colorAnsi;
    }
  }
}
