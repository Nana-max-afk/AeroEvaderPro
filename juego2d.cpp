#include "juego2d.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <thread>
#include <iostream>
#include <conio.h>

// Helper para colores ANSI en la consola al informar
#define ANSI_CYAN "\x1B[96m"
#define ANSI_RED "\x1B[91m"
#define ANSI_GREEN "\x1B[92m"
#define ANSI_YELLOW "\x1B[93m"
#define ANSI_RESET "\x1B[0m"

Juego2D::Juego2D(int ancho, int alto)
    : anchoPantalla(ancho), altoPantalla(alto), puntaje(0), vidas(3), nivel(1),
      jugadorVelocidad(380.0f), juegoActivo(false), velocidadJuegoBase(100.0f),
      combustible(100), combo(0), tiempoTranscurrido(0.0f), escudoActivo(false),
      duracionEscudo(0.0f), turboActivo(false), duracionTurbo(0.0f),
      cantidadEscudos(0), cantidadTurbos(0), mensajeNotificacion(""), timerNotificacion(0.0f) {
    
    jugadorPos = { (float)ancho / 2.0f, (float)alto - 80.0f };
}

void Juego2D::iniciar(const std::string &nombreJugador) {
    jugadorNombre = nombreJugador;
    puntaje = 0;
    vidas = 3;
    nivel = 1;
    combustible = 100;
    combo = 0;
    tiempoTranscurrido = 0.0f;
    escudoActivo = false;
    duracionEscudo = 0.0f;
    turboActivo = false;
    duracionTurbo = 0.0f;
    cantidadEscudos = 0;
    cantidadTurbos = 0;
    mensajeNotificacion = "";
    timerNotificacion = 0.0f;
    
    jugadorPos = { (float)anchoPantalla / 2.0f, (float)altoPantalla - 90.0f };
    juegoActivo = true;
    
    obstaculos.clear();
    particulas.clear();
    inicializarEstrellas();

    // Inicializar ventana Raylib
    InitWindow(anchoPantalla, altoPantalla, "Aero Evader Pro - 2D Edition");
    SetTargetFPS(60);

    // Pequeño banner de notificación inicial
    mensajeNotificacion = "SISTEMAS INICIALIZADOS - PILOTO: " + jugadorNombre;
    timerNotificacion = 4.0f;

    // Bucle principal de Raylib a 60 FPS fijos
    while (!WindowShouldClose() && juegoActivo) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f; // Limitar picos de lag

        procesarEntrada();
        generarObstaculos();
        actualizarEscena(dt);
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        dibujarFondo();
        dibujarEstrellas();
        dibujarParticulas();
        
        for (const auto &obs : obstaculos) {
            dibujarObstaculo(obs);
        }
        
        dibujarJugador();
        dibujarHUD();
        
        EndDrawing();
    }

    CloseWindow();
    
    // Informe final en consola de forma espectacular
    std::system("cls");
    
    // Calificar rendimiento
    std::string rango;
    std::string colorRango;
    if (puntaje >= 2000)      { rango = "S - LEGENDARIO"; colorRango = ANSI_YELLOW; }
    else if (puntaje >= 1200) { rango = "A - EXCELENTE"; colorRango = ANSI_CYAN; }
    else if (puntaje >= 700)  { rango = "B - BIEN HECHO"; colorRango = ANSI_GREEN; }
    else if (puntaje >= 300)  { rango = "C - ACEPTABLE"; colorRango = ANSI_YELLOW; }
    else                      { rango = "D - NECESITAS ENTRENAR"; colorRango = ANSI_RED; }

    std::cout << ANSI_CYAN << "╔══════════════════════════════════════════════════════════════════════════════════╗\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "║                                                                                ║\n" << ANSI_RESET;
    if (vidas > 0) {
        std::cout << ANSI_CYAN << "║" << ANSI_GREEN << "                         ★  MISION COMPLETADA (ESC)  ★                          " << ANSI_CYAN << "║\n" << ANSI_RESET;
    } else {
        std::cout << ANSI_CYAN << "║" << ANSI_RED << "                            ✕  NAVE DESTRUIDA  ✕                             " << ANSI_CYAN << "║\n" << ANSI_RESET;
    }
    std::cout << ANSI_CYAN << "║                                                                                ║\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "╠══════════════════════════════════════════════════════════════════════════════════╣\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "║ " << ANSI_RESET << "RANGO DE MISION:  " << colorRango << "[ " << rango << " ]" << ANSI_RESET << std::string(38 - rango.size(), ' ') << ANSI_CYAN << "║\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "╠══════════════════════════════════════════════════════════════════════════════════╣\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "║ " << ANSI_RESET << "ESTADISTICAS FINALES DEL PILOTO:                                               " << ANSI_CYAN << "║\n" << ANSI_RESET;
    
    auto lineaStat = [](const std::string &etiq, const std::string &valor, const std::string &color) {
        int espacios = 74 - etiq.size() - valor.size();
        if (espacios < 1) espacios = 1;
        std::cout << ANSI_CYAN << "║  " << ANSI_RESET << etiq << std::string(espacios, ' ') << color << valor << ANSI_RESET << "  " << ANSI_CYAN << "║\n" << ANSI_RESET;
    };
    
    lineaStat("Nombre de Piloto:", jugadorNombre, ANSI_RESET);
    lineaStat("Puntaje Final:", std::to_string(puntaje) + " pts", ANSI_YELLOW);
    lineaStat("Nivel de Amenaza:", "Nivel " + std::to_string(nivel), ANSI_CYAN);
    
    std::string corazonesStr;
    for (int i = 0; i < vidas; ++i) corazonesStr += "♥ ";
    if (corazonesStr.empty()) corazonesStr = "✕ Sin Vidas";
    lineaStat("Vidas Restantes:", corazonesStr, ANSI_RED);
    
    lineaStat("Combustible Restante:", std::to_string(combustible) + "%", combustible > 50 ? ANSI_GREEN : combustible > 20 ? ANSI_YELLOW : ANSI_RED);
    lineaStat("Tiempo en Orbita:", std::to_string((int)tiempoTranscurrido) + " segundos", ANSI_CYAN);
    lineaStat("Combo Maximo:", "x" + std::to_string(combo), combo >= 5 ? ANSI_GREEN : ANSI_RESET);
    lineaStat("Escudos Guardados:", std::to_string(cantidadEscudos) + " en cabina", ANSI_CYAN);
    lineaStat("Turbos Guardados:", std::to_string(cantidadTurbos) + " en cabina", ANSI_YELLOW);
    
    std::cout << ANSI_CYAN << "╠══════════════════════════════════════════════════════════════════════════════════╣\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "║ " << ANSI_YELLOW << "★" << ANSI_RESET << " TIP: ¡El modo gráfico 2D de Aero Evader Pro ofrece reflejos instantáneos a 60FPS!  " << ANSI_CYAN << "║\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "╠══════════════════════════════════════════════════════════════════════════════════╣\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "║                                                                                ║\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "║                 \x1B[90mPresiona cualquier tecla para volver al menu...\x1B[0m                " << ANSI_CYAN << "║\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "╚══════════════════════════════════════════════════════════════════════════════════╝\n" << ANSI_RESET;

    // Esperar a que el jugador pulse una tecla antes de restaurar
    while (!_kbhit()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    _getch();
}

int Juego2D::obtenerPuntaje() const {
    return puntaje;
}

void Juego2D::inicializarEstrellas() {
    estrellas.clear();
    std::default_random_engine generador((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distrX(0.0f, (float)anchoPantalla);
    std::uniform_real_distribution<float> distrY(0.0f, (float)altoPantalla);
    std::uniform_int_distribution<int> distrCapa(1, 3);
    
    for (int i = 0; i < 120; ++i) {
        Estrella2D estrella;
        estrella.posicion = { distrX(generador), distrY(generador) };
        estrella.capa = distrCapa(generador);
        
        // Asignar velocidades según capa
        if (estrella.capa == 3) estrella.velocidad = 220.0f;     // Capa cercana rápida
        else if (estrella.capa == 2) estrella.velocidad = 100.0f; // Capa media
        else estrella.velocidad = 40.0f;                         // Capa lejana lenta
        
        estrellas.push_back(estrella);
    }
}

void Juego2D::actualizarEstrellas(float dt) {
    static std::default_random_engine generador((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distrX(0.0f, (float)anchoPantalla);
    
    // Si el turbo está activo, el fondo de estrellas se mueve mucho más rápido (hiperespacio)
    float multiplicadorVelocidad = turboActivo ? 2.5f : 1.0f;
    
    for (auto &estrella : estrellas) {
        estrella.posicion.y += estrella.velocidad * multiplicadorVelocidad * dt;
        if (estrella.posicion.y >= altoPantalla) {
            estrella.posicion.y = 0;
            estrella.posicion.x = distrX(generador);
        }
    }
}

void Juego2D::dibujarEstrellas() const {
    for (const auto &estrella : estrellas) {
        Color colorEstrella;
        float radio = 1.0f;
        
        if (estrella.capa == 3) {
            colorEstrella = ColorAlpha(WHITE, 0.85f);
            radio = 2.0f;
        } else if (estrella.capa == 2) {
            colorEstrella = ColorAlpha(LIGHTGRAY, 0.6f);
            radio = 1.5f;
        } else {
            colorEstrella = ColorAlpha(DARKGRAY, 0.4f);
            radio = 1.0f;
        }
        
        DrawCircleV(estrella.posicion, radio, colorEstrella);
    }
}

void Juego2D::procesarEntrada() {
    float dt = GetFrameTime();
    if (dt > 0.1f) dt = 0.1f;
    
    float velocidadActual = jugadorVelocidad;
    // Si el turbo está activo, la nave se mueve a gran velocidad
    if (turboActivo) {
        velocidadActual *= 1.45f;
    }
    
    const float radioLimites = 25.0f; // Margen para las alas de la nave
    
    // Movimiento horizontal suave
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        jugadorPos.x = std::max(radioLimites, jugadorPos.x - velocidadActual * dt);
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        jugadorPos.x = std::min((float)anchoPantalla - radioLimites, jugadorPos.x + velocidadActual * dt);
    }
    
    // Salir del juego con ESC
    if (IsKeyPressed(KEY_ESCAPE)) {
        juegoActivo = false;
    }
    
    // Activar escudo manual
    if (IsKeyPressed(KEY_S)) {
        if (cantidadEscudos > 0 && !escudoActivo) {
            escudoActivo = true;
            duracionEscudo = 12.0f; // 12 segundos de duración
            cantidadEscudos--;
            mensajeNotificacion = "ESCUDO DE ENERGIA ACTIVADO - 100% CAPACIDAD";
            timerNotificacion = 3.0f;
        }
    }
    
    // Activar turbo manual
    if (IsKeyPressed(KEY_T)) {
        if (cantidadTurbos > 0 && !turboActivo) {
            turboActivo = true;
            duracionTurbo = 10.0f; // 10 segundos de duración
            cantidadTurbos--;
            mensajeNotificacion = "PROPULSORES HIPER-TURBO ACTIVOS!";
            timerNotificacion = 3.0f;
        }
    }
}

void Juego2D::generarObstaculos() {
    static std::default_random_engine generador((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    static std::uniform_real_distribution<float> distrChance(0.0f, 100.0f);
    static std::uniform_real_distribution<float> distrVel(100.0f, 220.0f);
    static std::uniform_real_distribution<float> distrRotSpeed(-3.0f, 3.0f);
    
    // Frecuencia y límite de obstáculos basado en el nivel
    int maxObstaculos = std::min(24, 7 + nivel * 2);
    if ((int)obstaculos.size() > maxObstaculos) {
        return;
    }
    
    // Frecuencia de generación (probabilidad por frame en 60 FPS)
    float umbralGeneracion = 1.6f + (float)nivel * 0.35f;
    if (distrChance(generador) < umbralGeneracion) {
        Obstaculo2D nuevo;
        nuevo.posicion.y = -50.0f; // Spawn por encima del borde superior
        
        // La velocidad base aumenta con el nivel
        float velBase = 120.0f + (float)nivel * 16.0f;
        nuevo.velocidad = velBase + distrVel(generador) * 0.4f;
        
        float chance = distrChance(generador);
        TipoObstaculo2D tipo;
        float escala = 1.0f;
        
        if (chance < 3.5f) {
            tipo = TipoObstaculo2D::PowerUpEscudo;
            escala = 1.0f;
        } else if (chance < 7.0f) {
            tipo = TipoObstaculo2D::PowerUpTurbo;
            escala = 1.0f;
        } else if (chance < 25.0f) {
            tipo = TipoObstaculo2D::Cometa;
            escala = 1.0f;
        } else if (chance < 50.0f) {
            tipo = TipoObstaculo2D::Drone;
            escala = 1.1f;
        } else {
            tipo = TipoObstaculo2D::Meteoro;
            // Variar el tamaño del meteoro
            std::uniform_real_distribution<float> distrEsc(0.8f, 1.6f);
            escala = distrEsc(generador);
        }
        
        // Spawn dentro de la pantalla cuidando que no se corte en los bordes
        float minX = 40.0f;
        float maxX = (float)anchoPantalla - 40.0f;
        std::uniform_real_distribution<float> distrX(minX, maxX);
        
        nuevo.posicion.x = distrX(generador);
        nuevo.tipo = tipo;
        nuevo.rotacion = 0.0f;
        nuevo.velocidadRotacion = distrRotSpeed(generador);
        nuevo.escala = escala;
        
        obstaculos.push_back(nuevo);
    }
}

void Juego2D::actualizarEscena(float dt) {
    actualizarEstrellas(dt);
    
    // Crear chispas de motor de manera continua
    crearChispasMotor(jugadorPos, turboActivo);
    
    // Mover obstáculos
    for (auto &obs : obstaculos) {
        obs.posicion.y += obs.velocidad * dt;
        obs.rotacion += obs.velocidadRotacion * 50.0f * dt;
    }
    
    // Puntos ganados por evadir obstáculos (los que caen al fondo)
    for (const auto &obs : obstaculos) {
        if (obs.posicion.y > (float)altoPantalla + 30.0f) {
            if (obs.tipo == TipoObstaculo2D::Meteoro || obs.tipo == TipoObstaculo2D::Drone) {
                puntaje += 15;
            } else if (obs.tipo == TipoObstaculo2D::Cometa) {
                puntaje += 12;
            }
        }
    }
    
    // Limpiar obstáculos fuera de pantalla
    obstaculos.erase(std::remove_if(obstaculos.begin(), obstaculos.end(), [&](const Obstaculo2D &obs) {
        return obs.posicion.y > (float)altoPantalla + 35.0f;
    }), obstaculos.end());
    
    // Verificación de colisiones
    for (auto it = obstaculos.begin(); it != obstaculos.end(); ) {
        // Área aproximada de colisión del jugador (nave espacial)
        // Radio circular o Rectángulo: Ancho = 45, Alto = 30
        Rectangle recNave = { jugadorPos.x - 22.0f, jugadorPos.y - 20.0f, 44.0f, 32.0f };
        
        // Área del obstáculo según tipo y escala
        Rectangle recObs;
        float obsRadius = 15.0f * it->escala;
        recObs = { it->posicion.x - obsRadius, it->posicion.y - obsRadius, obsRadius * 2.0f, obsRadius * 2.0f };
        
        bool colision = CheckCollisionRecs(recNave, recObs);
        
        if (colision) {
            Vector2 colPos = it->posicion;
            
            switch (it->tipo) {
                case TipoObstaculo2D::Meteoro:
                case TipoObstaculo2D::Drone:
                    if (escudoActivo) {
                        crearExplosion(colPos, SKYBLUE, 25);
                        escudoActivo = false;
                        duracionEscudo = 0.0f;
                        puntaje += 10;
                        mensajeNotificacion = "ESCUDO ABSORBIO EL IMPACTO! +10 pts";
                        timerNotificacion = 2.5f;
                    } else {
                        // Impacto grave
                        crearExplosion(colPos, RED, 40);
                        // Sacudir la pantalla o flash
                        vidas--;
                        combo = 0;
                        mensajeNotificacion = "✕ ALERTA: IMPACTO CRITICO! -1 VIDA ✕";
                        timerNotificacion = 3.5f;
                    }
                    break;
                case TipoObstaculo2D::Cometa:
                    crearExplosion(colPos, GOLD, 20);
                    puntaje += 30;
                    combo++;
                    combustible = std::min(130, combustible + 15);
                    mensajeNotificacion = "✦ NUCLEO ENERGETICO: +30 pts +15% fuel ✦";
                    timerNotificacion = 2.5f;
                    break;
                case TipoObstaculo2D::PowerUpEscudo:
                    crearExplosion(colPos, BLUE, 22);
                    cantidadEscudos = std::min(5, cantidadEscudos + 1);
                    puntaje += 25;
                    mensajeNotificacion = "▲ ESCUDO GUARDADO EN INVENTARIO (Teclado S) ▲";
                    timerNotificacion = 3.5f;
                    break;
                case TipoObstaculo2D::PowerUpTurbo:
                    crearExplosion(colPos, ORANGE, 22);
                    cantidadTurbos = std::min(5, cantidadTurbos + 1);
                    puntaje += 25;
                    mensajeNotificacion = "▲ TURBO GUARDADO EN INVENTARIO (Teclado T) ▲";
                    timerNotificacion = 3.5f;
                    break;
            }
            it = obstaculos.erase(it);
        } else {
            ++it;
        }
    }
    
    // Actualizar duraciones de Escudo/Turbo
    if (escudoActivo) {
        duracionEscudo -= dt;
        if (duracionEscudo <= 0.0f) {
            escudoActivo = false;
            mensajeNotificacion = "ESCUDO AGOTADO!";
            timerNotificacion = 2.0f;
        }
    }
    if (turboActivo) {
        duracionTurbo -= dt;
        if (duracionTurbo <= 0.0f) {
            turboActivo = false;
            mensajeNotificacion = "SISTEMA TURBO REFRIGERADO.";
            timerNotificacion = 2.0f;
        }
    }
    
    // Gasto de combustible progresivo
    float tasaGasto = (1.5f + (float)nivel * 0.25f);
    if (turboActivo) {
        tasaGasto *= 2.0f; // Turbo consume el doble
    }
    
    float fuelPrevio = (float)combustible;
    tiempoTranscurrido += dt;
    
    // Drenar combustible en base al delta de tiempo
    static float acumCombustible = 0.0f;
    acumCombustible += tasaGasto * dt;
    if (acumCombustible >= 1.0f) {
        int enteroGasto = (int)acumCombustible;
        combustible = std::max(0, combustible - enteroGasto);
        acumCombustible -= (float)enteroGasto;
    }
    
    if (combustible <= 0) {
        vidas--;
        combustible = 25; // Relleno de emergencia
        combo = 0;
        mensajeNotificacion = "✕ COMBUSTIBLE COMPLETAMENTE AGOTADO! -1 VIDA ✕";
        timerNotificacion = 4.0f;
    }
    
    // Combo bonus
    static float timerComboBonus = 0.0f;
    if (combo >= 5) {
        timerComboBonus += dt;
        if (timerComboBonus >= 0.8f) { // Ganar puntos periódicos por combo
            puntaje += 2 * (combo / 5);
            timerComboBonus = 0.0f;
        }
    }
    
    // Notificación de advertencia de combustible
    if (combustible <= 20 && fuelPrevio > 20 && timerNotificacion <= 0.0f) {
        mensajeNotificacion = "⚠ ADVERTENCIA: COMBUSTIBLE BAJO! BUSCA COMETAS ⚠";
        timerNotificacion = 3.5f;
    }
    
    // Actualizar partículas
    for (auto &p : particulas) {
        p.posicion.x += p.velocidad.x * dt;
        p.posicion.y += p.velocidad.y * dt;
        p.vidaRestante--;
        p.opacidad = (float)p.vidaRestante / (float)p.vidaMax;
    }
    
    particulas.erase(std::remove_if(particulas.begin(), particulas.end(), [](const Particula2D &p) {
        return p.vidaRestante <= 0 || p.posicion.x < -10.0f || p.posicion.x > 810.0f || p.posicion.y < -10.0f || p.posicion.y > 610.0f;
    }), particulas.end());
    
    // Decrementar timer de notificaciones
    if (timerNotificacion > 0.0f) {
        timerNotificacion -= dt;
        if (timerNotificacion <= 0.0f) {
            mensajeNotificacion = "";
        }
    }
    
    // Actualizar nivel periódicamente
    actualizarNivel();
    
    // Puntos ganados por tiempo en órbita
    static float acumTiempoPuntos = 0.0f;
    acumTiempoPuntos += dt;
    if (acumTiempoPuntos >= 0.8f) {
        puntaje += 2;
        acumTiempoPuntos = 0.0f;
    }
    
    if (vidas <= 0) {
        juegoActivo = false;
    }
}

void Juego2D::actualizarNivel() {
    int nuevoNivel = puntaje / 250 + 1; // Subir nivel cada 250 puntos
    if (nuevoNivel != nivel) {
        nivel = nuevoNivel;
        mensajeNotificacion = "★ ¡NIVEL COMPLETO! AHORA NIVEL " + std::to_string(nivel) + " ★";
        timerNotificacion = 4.0f;
        
        // Bonificación de combustible por subir nivel
        if (nivel % 2 == 0) {
            combustible = std::min(130, combustible + 20);
        }
    }
}

void Juego2D::dibujarFondo() const {
    // Fondo negro y un muy sutil degradado de nebulosa ciberpunk en el fondo
    DrawCircleGradient(Vector2{(float)anchoPantalla / 2.0f, (float)altoPantalla / 2.0f}, 450.0f, ColorAlpha(DARKBLUE, 0.04f), BLACK);
    DrawCircleGradient(Vector2{(float)anchoPantalla / 4.0f, (float)altoPantalla / 3.0f}, 250.0f, ColorAlpha(MAGENTA, 0.03f), BLACK);
}

void Juego2D::dibujarJugador() const {
    Vector2 p = jugadorPos;
    
    // Si escudo activo, dibujar aura de fuerza circular translúcida
    if (escudoActivo) {
        float latido = 1.0f + 0.06f * sinf((float)GetTime() * 10.0f);
        DrawCircleLinesV(p, 36.0f * latido, ColorAlpha(SKYBLUE, 0.9f));
        DrawCircleV(p, 36.0f * latido, ColorAlpha(SKYBLUE, 0.12f));
    }
    
    // Dibujo de la nave vectorizada Cyberpunk
    // 1. Alas laterales (Triángulos)
    Vector2 alaIzqP1 = { p.x - 24.0f, p.y + 12.0f };
    Vector2 alaIzqP2 = { p.x - 10.0f, p.y - 5.0f };
    Vector2 alaIzqP3 = { p.x - 6.0f, p.y + 12.0f };
    DrawTriangle(alaIzqP1, alaIzqP2, alaIzqP3, RED); // Punta izquierda ala
    DrawTriangleLines(alaIzqP1, alaIzqP2, alaIzqP3, MAROON);
    
    Vector2 alaDerP1 = { p.x + 6.0f, p.y + 12.0f };
    Vector2 alaDerP2 = { p.x + 10.0f, p.y - 5.0f };
    Vector2 alaDerP3 = { p.x + 24.0f, p.y + 12.0f };
    DrawTriangle(alaDerP1, alaDerP2, alaDerP3, RED); // Punta derecha ala
    DrawTriangleLines(alaDerP1, alaDerP2, alaDerP3, MAROON);
    
    // Estructuras metálicas de alas
    DrawRectangleRec(Rectangle{p.x - 22.0f, p.y + 4.0f, 16.0f, 8.0f}, GRAY);
    DrawRectangleRec(Rectangle{p.x + 6.0f, p.y + 4.0f, 16.0f, 8.0f}, GRAY);
    DrawRectangleLines(p.x - 22, p.y + 4, 16, 8, DARKGRAY);
    DrawRectangleLines(p.x + 6, p.y + 4, 16, 8, DARKGRAY);
    
    // 2. Cuerpo Central (Triángulo estilizado principal)
    Vector2 cuerpoP1 = { p.x, p.y - 28.0f };
    Vector2 cuerpoP2 = { p.x - 10.0f, p.y + 15.0f };
    Vector2 cuerpoP3 = { p.x + 10.0f, p.y + 15.0f };
    DrawTriangle(cuerpoP2, cuerpoP1, cuerpoP3, DARKBLUE);
    DrawTriangleLines(cuerpoP2, cuerpoP1, cuerpoP3, BLUE);
    
    // 3. Cabina de vidrio holográfica
    Vector2 cabinaP1 = { p.x, p.y - 18.0f };
    Vector2 cabinaP2 = { p.x - 5.0f, p.y };
    Vector2 cabinaP3 = { p.x + 5.0f, p.y };
    DrawTriangle(cabinaP2, cabinaP1, cabinaP3, SKYBLUE);
    DrawTriangleLines(cabinaP2, cabinaP1, cabinaP3, WHITE);
    
    // 4. Motores traseros y flama turbo decorativa
    DrawRectangle(p.x - 6, p.y + 15, 4, 4, LIGHTGRAY);
    DrawRectangle(p.x + 2, p.y + 15, 4, 4, LIGHTGRAY);
    
    // Si turbo activo, añadir chispas o destellos extra en las alas
    if (turboActivo) {
        DrawTriangle(Vector2{p.x - 18, p.y + 14}, Vector2{p.x - 14, p.y + 26}, Vector2{p.x - 10, p.y + 14}, SKYBLUE);
        DrawTriangle(Vector2{p.x + 10, p.y + 14}, Vector2{p.x + 14, p.y + 26}, Vector2{p.x + 18, p.y + 14}, SKYBLUE);
    }
}

void Juego2D::dibujarObstaculo(const Obstaculo2D &obs) const {
    Vector2 p = obs.posicion;
    float esc = obs.escala;
    
    switch (obs.tipo) {
        case TipoObstaculo2D::Meteoro: {
            // Asteroide irregular marrón dibujado vectorialmente mediante un polígono circular con relieve
            float radio = 18.0f * esc;
            DrawCircleV(p, radio, Color{105, 55, 30, 255});
            DrawCircleLinesV(p, radio, Color{70, 35, 15, 255});
            
            // Cráteres de relieve dentro del meteoro
            DrawCircle(p.x - 5 * esc, p.y - 4 * esc, 4 * esc, Color{80, 40, 20, 255});
            DrawCircle(p.x + 6 * esc, p.y + 5 * esc, 3.5f * esc, Color{80, 40, 20, 255});
            DrawCircle(p.x - 4 * esc, p.y + 6 * esc, 2.5f * esc, Color{80, 40, 20, 255});
            break;
        }
        case TipoObstaculo2D::Drone: {
            // Caza espacial enemigo verde brillante con alas y núcleo de energía rojo
            float s = 14.0f * esc;
            
            // Cuerpo del drone (Triángulo apuntando hacia abajo)
            Vector2 c1 = { p.x, p.y + s };
            Vector2 c2 = { p.x - s, p.y - s * 0.5f };
            Vector2 c3 = { p.x + s, p.y - s * 0.5f };
            DrawTriangle(c2, c1, c3, DARKGREEN);
            DrawTriangleLines(c2, c1, c3, GREEN);
            
            // Alas de Drone (Líneas / Triángulos neón)
            DrawTriangle(Vector2{p.x - s, p.y - s * 0.5f}, Vector2{p.x - s * 1.7f, p.y + s * 0.6f}, Vector2{p.x - s * 0.7f, p.y}, ORANGE);
            DrawTriangle(Vector2{p.x + s * 0.7f, p.y}, Vector2{p.x + s * 1.7f, p.y + s * 0.6f}, Vector2{p.x + s, p.y - s * 0.5f}, ORANGE);
            
            // Núcleo brillante central (Latiendo)
            float latido = 1.0f + 0.15f * sinf((float)GetTime() * 12.0f);
            DrawCircleV(p, 4.0f * esc * latido, RED);
            DrawCircleLinesV(p, 5.0f * esc * latido, WHITE);
            break;
        }
        case TipoObstaculo2D::Cometa: {
            // Cometa cósmico con cola estelar hermosa
            // Dibujar cola larga con opacidad degradada detrás del cometa
            for (int i = 0; i < 7; ++i) {
                float desplY = -(float)i * 6.5f;
                float radioEstela = (6.0f - (float)i * 0.8f) * esc;
                if (radioEstela < 1.0f) radioEstela = 1.0f;
                
                Color colEstela = ColorAlpha(SKYBLUE, 0.7f - (float)i * 0.1f);
                DrawCircle(p.x, p.y + desplY, radioEstela, colEstela);
            }
            // Cabeza del cometa
            DrawCircleV(p, 8.5f * esc, WHITE);
            DrawCircleLinesV(p, 10.0f * esc, GOLD);
            break;
        }
        case TipoObstaculo2D::PowerUpEscudo: {
            // Contenedor hexagonal azul cian neón con icono de escudo
            float tam = 12.0f;
            DrawPoly(p, 6, tam, (float)GetTime() * 45.0f, SKYBLUE);
            DrawPolyLines(p, 6, tam + 2.0f, (float)GetTime() * 45.0f, BLUE);
            DrawCircleV(p, 5.0f, ColorAlpha(BLACK, 0.4f));
            
            // Letra S en el interior
            DrawText("S", (int)p.x - 3, (int)p.y - 5, 11, WHITE);
            break;
        }
        case TipoObstaculo2D::PowerUpTurbo: {
            // Contenedor romboide naranja brillante con letra T
            float tam = 12.0f;
            DrawPoly(p, 4, tam, (float)GetTime() * -60.0f, ORANGE);
            DrawPolyLines(p, 4, tam + 2.0f, (float)GetTime() * -60.0f, GOLD);
            DrawCircleV(p, 5.0f, ColorAlpha(BLACK, 0.4f));
            
            // Letra T en el interior
            DrawText("T", (int)p.x - 3, (int)p.y - 5, 11, WHITE);
            break;
        }
    }
}

void Juego2D::dibujarParticulas() const {
    for (const auto &p : particulas) {
        Color col = ColorAlpha(p.color, p.opacidad);
        DrawCircleV(p.posicion, p.radio, col);
    }
}

void Juego2D::dibujarHUD() const {
    // Definir color del marco del HUD holográfico según estado
    Color colorMarco = SKYBLUE;
    if (vidas <= 1 || combustible <= 20) {
        // Alerta roja parpadeante
        if ((int)(tiempoTranscurrido * 5.0f) % 2 == 0) {
            colorMarco = RED;
        } else {
            colorMarco = Color{150, 0, 0, 255};
        }
    } else if (turboActivo) {
        colorMarco = GOLD;
    } else if (escudoActivo) {
        colorMarco = BLUE;
    }
    
    // 1. Líneas del marco cibernético (Cyberpunk HUD)
    DrawRectangleRec(Rectangle{4.0f, 4.0f, (float)anchoPantalla - 8.0f, 40.0f}, ColorAlpha(BLACK, 0.7f));
    DrawRectangleRec(Rectangle{4.0f, (float)altoPantalla - 44.0f, (float)anchoPantalla - 8.0f, 40.0f}, ColorAlpha(BLACK, 0.7f));
    
    // Bordes de líneas del HUD
    DrawLineEx(Vector2{5, 5}, Vector2{(float)anchoPantalla - 5, 5}, 1.5f, colorMarco);
    DrawLineEx(Vector2{5, 44}, Vector2{(float)anchoPantalla - 5, 44}, 1.5f, colorMarco);
    DrawLineEx(Vector2{5, (float)altoPantalla - 44}, Vector2{(float)anchoPantalla - 5, (float)altoPantalla - 44}, 1.5f, colorMarco);
    DrawLineEx(Vector2{5, (float)altoPantalla - 5}, Vector2{(float)anchoPantalla - 5, (float)altoPantalla - 5}, 1.5f, colorMarco);
    
    // Esquinas anguladas decorativas
    DrawLineEx(Vector2{5, 5}, Vector2{5, 20}, 2.5f, colorMarco);
    DrawLineEx(Vector2{(float)anchoPantalla - 5, 5}, Vector2{(float)anchoPantalla - 5, 20}, 2.5f, colorMarco);
    DrawLineEx(Vector2{5, (float)altoPantalla - 5}, Vector2{5, (float)altoPantalla - 20}, 2.5f, colorMarco);
    DrawLineEx(Vector2{(float)anchoPantalla - 5, (float)altoPantalla - 5}, Vector2{(float)anchoPantalla - 5, (float)altoPantalla - 20}, 2.5f, colorMarco);
    
    // 2. Textos informativos de cabina (Superior)
    DrawText("AERO EVADER PRO 2D", 20, 16, 16, WHITE);
    
    std::string pilotoInfo = "PILOTO: " + jugadorNombre + "   NIVEL: " + std::to_string(nivel);
    DrawText(pilotoInfo.c_str(), 240, 18, 12, SKYBLUE);
    
    std::string ptsInfo = "PUNTAJE: " + std::to_string(puntaje);
    DrawText(ptsInfo.c_str(), 640, 16, 16, GOLD);
    
    // 3. Estado de Nave (Inferior)
    // Combustible y barra gráfica
    DrawText("COMBUSTIBLE:", 20, altoPantalla - 32, 12, WHITE);
    
    Color colorFuel = GREEN;
    if (combustible <= 20) colorFuel = RED;
    else if (combustible <= 50) colorFuel = ORANGE;
    
    // Contenedor de la barra de combustible
    DrawRectangleLines(125, altoPantalla - 32, 130, 12, GRAY);
    int fuelWidth = (int)((float)combustible * 1.26f);
    if (fuelWidth < 0) fuelWidth = 0;
    if (fuelWidth > 126) fuelWidth = 126;
    DrawRectangle(127, altoPantalla - 30, fuelWidth, 8, colorFuel);
    
    std::string fuelTexto = std::to_string(combustible) + "%";
    DrawText(fuelTexto.c_str(), 265, altoPantalla - 32, 12, colorFuel);
    
    // Vidas como corazones vectoriales o texto
    DrawText("VIDAS:", 320, altoPantalla - 32, 12, WHITE);
    for (int i = 0; i < vidas; ++i) {
        // Dibujar pequeños corazones vectorizados usando triángulos de Raylib
        float hx = 370.0f + (float)i * 20.0f;
        float hy = (float)altoPantalla - 26.0f;
        
        DrawCircle(hx - 4, hy - 4, 4, RED);
        DrawCircle(hx + 4, hy - 4, 4, RED);
        DrawTriangle(Vector2{hx - 8, hy - 3}, Vector2{hx, hy + 6}, Vector2{hx + 8, hy - 3}, RED);
    }
    
    // Inventario de PowerUps
    std::string escudosInvent = "ESCUDOS [S]: " + std::to_string(cantidadEscudos) + (escudoActivo ? " (ACTIVO)" : " (LISTO)");
    Color colEscudoInv = escudoActivo ? SKYBLUE : GRAY;
    DrawText(escudosInvent.c_str(), 450, altoPantalla - 32, 12, colEscudoInv);
    
    std::string turbosInvent = "TURBOS [T]: " + std::to_string(cantidadTurbos) + (turboActivo ? " (ACTIVO)" : " (LISTO)");
    Color colTurboInv = turboActivo ? GOLD : GRAY;
    DrawText(turbosInvent.c_str(), 630, altoPantalla - 32, 12, colTurboInv);
    
    // 4. Mostrar notificaciones y avisos en el centro superior de la pantalla
    if (!mensajeNotificacion.empty() && timerNotificacion > 0.0f) {
        int textW = MeasureText(mensajeNotificacion.c_str(), 14);
        int notifX = (anchoPantalla - textW) / 2;
        
        // Fondo del banner translúcido para notificaciones
        DrawRectangleRec(Rectangle{(float)notifX - 15.0f, 54.0f, (float)textW + 30.0f, 26.0f}, ColorAlpha(BLACK, 0.8f));
        DrawRectangleLines((float)notifX - 15.0f, 54.0f, (float)textW + 30.0f, 26.0f, colorMarco);
        
        DrawText(mensajeNotificacion.c_str(), notifX, 60, 14, colorMarco);
    }
    
    // 5. Combo activo
    if (combo >= 5) {
        std::string comboMsg = "MULTIPLIER COMBO x" + std::to_string(combo);
        DrawText(comboMsg.c_str(), anchoPantalla / 2 - MeasureText(comboMsg.c_str(), 12) / 2, altoPantalla - 64, 12, GREEN);
    }
}

void Juego2D::crearExplosion(Vector2 pos, Color colorBase, int cantidad) {
    std::default_random_engine generador((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distrAngle(0.0f, 2.0f * PI);
    std::uniform_real_distribution<float> distrSpeed(60.0f, 220.0f);
    std::uniform_real_distribution<float> distrRadio(1.5f, 4.0f);
    std::uniform_int_distribution<int> distrVida(20, 50);
    
    for (int i = 0; i < cantidad; ++i) {
        Particula2D p;
        p.posicion = pos;
        
        float ang = distrAngle(generador);
        float spd = distrSpeed(generador);
        p.velocidad = { cosf(ang) * spd, sinf(ang) * spd };
        
        p.color = colorBase;
        p.radio = distrRadio(generador);
        p.opacidad = 1.0f;
        p.vidaMax = distrVida(generador);
        p.vidaRestante = p.vidaMax;
        
        particulas.push_back(p);
    }
}

void Juego2D::crearChispasMotor(Vector2 pos, bool conTurbo) {
    std::default_random_engine generador((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distrVelX(-30.0f, 30.0f);
    std::uniform_real_distribution<float> distrVelY(180.0f, 280.0f);
    std::uniform_real_distribution<float> distrRadio(1.0f, 2.5f);
    std::uniform_int_distribution<int> distrVida(12, 28);
    
    int cantidad = conTurbo ? 5 : 2;
    Color colorChispa = conTurbo ? SKYBLUE : GOLD;
    if (conTurbo && (int)(tiempoTranscurrido * 10.0f) % 2 == 0) {
        colorChispa = ORANGE; // Efecto llama bicolor con turbo
    }
    
    for (int i = 0; i < cantidad; ++i) {
        Particula2D p;
        
        // Spawn justo detrás de los dos propulsores de la nave
        float motorOffset = (i % 2 == 0) ? -4.0f : 4.0f;
        p.posicion = { pos.x + motorOffset, pos.y + 15.0f };
        
        p.velocidad = { distrVelX(generador), distrVelY(generador) };
        p.color = colorChispa;
        p.radio = distrRadio(generador);
        p.opacidad = 1.0f;
        p.vidaMax = distrVida(generador);
        p.vidaRestante = p.vidaMax;
        
        particulas.push_back(p);
    }
}
