#include "juego2d.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

Juego2D::Juego2D(int ancho, int alto, SistemaUsuarios* sistema)
    : anchoPantalla(ancho), altoPantalla(alto), puntaje(0), vidas(3), nivel(1),
      jugadorVelocidad(380.0f), juegoActivo(false), velocidadJuegoBase(100.0f),
      combustible(100), combo(0), tiempoTranscurrido(0.0f), escudoActivo(false),
      duracionEscudo(0.0f), turboActivo(false), duracionTurbo(0.0f),
      cantidadEscudos(0), cantidadTurbos(0), mensajeNotificacion(""), timerNotificacion(0.0f),
      estadoActual(EstadoJuego2D::LoginRegister), opcionMenu(0), opcionCustomizer(0),
      seleccionNave(0), colorNave(WHITE), seleccionArma(0), weaponCooldownTimer(0.0f),
      jefeActivoIndex(-1), jefeVida(0), jefeVidaMax(100), jefeFase(1), jefeMovTimer(0.0f),
      nextBossIndex(0), bossSpawnCooldown(10.0f), texturesLoaded(false),
      sistemaUsuarios(sistema), inputUser(""), inputPass(""), loginActiveField(0),
      authMessage(""), authMessageTimer(0.0f) {
    
    jugadorPos = { (float)ancho / 2.0f, (float)alto - 90.0f };
}

void Juego2D::cargarConfiguracionLocal() {
    std::ifstream file("config.txt");
    if (file.is_open()) {
        int r, g, b;
        if (file >> seleccionNave >> r >> g >> b >> seleccionArma) {
            colorNave = Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
        }
        file.close();
    }
}

void Juego2D::guardarConfiguracionLocal() {
    std::ofstream file("config.txt");
    if (file.is_open()) {
        file << seleccionNave << " " 
             << (int)colorNave.r << " " 
             << (int)colorNave.g << " " 
             << (int)colorNave.b << " " 
             << seleccionArma << "\n";
        file.close();
    }
}

// Dibuja un panel con estilo de vidrio cian cyberpunk (glassmorphic)
static void DibujarPanelVidrio(int x, int y, int w, int h, Color colorBorde, float opacidad = 0.82f) {
    DrawRectangle(x, y, w, h, ColorAlpha(Color{ 8, 10, 22, 255 }, opacidad));
    DrawRectangleLines(x, y, w, h, colorBorde);
    // Borde interno sutil
    DrawRectangleLines(x + 1, y + 1, w - 2, h - 2, ColorAlpha(colorBorde, 0.25f));
}

void Juego2D::iniciar() {
    juegoActivo = true;
    estadoActual = EstadoJuego2D::LoginRegister;
    
    InitWindow(anchoPantalla, altoPantalla, "Aero Evader Pro - Premium Edition");
    SetTargetFPS(60);

    // Intentar cargar texturas del juego
    shipTextures[0] = LoadTexture("assets/ship1_placeholder.png");
    shipTextures[1] = LoadTexture("assets/ship2_placeholder.png");
    shipTextures[2] = LoadTexture("assets/ship3_placeholder.png");
    bossTextures[0] = LoadTexture("assets/boss/boss0.png");
    bossTextures[1] = LoadTexture("assets/boss/boss1.png");
    texturesLoaded = true;

    // Configuración inicial de estrellas
    inicializarEstrellas();
    obstaculos.clear();
    particulas.clear();
    proyectiles.clear();
    jefeActivoIndex = -1;
    nextBossIndex = 0;
    bossSpawnCooldown = 12.0f; // Primer boss a los 12 segundos

    authMessage = "SISTEMAS ONLINE - INSERTE CREDENCIALES DE VUELO";
    authMessageTimer = 4.0f;

    while (!WindowShouldClose() && juegoActivo) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;

        // Soporte para F11 Pantalla Completa
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        // Decrementar temporizador de mensajes de autenticación si está activo
        if (authMessageTimer > 0.0f) {
            authMessageTimer -= dt;
            if (authMessageTimer <= 0.0f) authMessage = "";
        }

        switch (estadoActual) {
            case EstadoJuego2D::LoginRegister:
                procesarEntradaLoginRegister();
                
                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarLoginRegister();
                EndDrawing();
                break;

            case EstadoJuego2D::MenuInicio:
                procesarEntradaMenu();
                
                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarMenuInicio();
                EndDrawing();
                break;

            case EstadoJuego2D::Personalizacion:
                procesarEntradaPersonalizacion();

                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarPersonalizacion();
                EndDrawing();
                break;

            case EstadoJuego2D::RecordsGlobales:
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                    estadoActual = EstadoJuego2D::MenuInicio;
                }
                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarRecordsGlobales();
                EndDrawing();
                break;

            case EstadoJuego2D::EstadisticasPersonales:
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                    estadoActual = EstadoJuego2D::MenuInicio;
                }
                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarEstadisticasPersonales();
                EndDrawing();
                break;

            case EstadoJuego2D::HistorialPartidas:
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                    estadoActual = EstadoJuego2D::MenuInicio;
                }
                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarHistorialPartidas();
                EndDrawing();
                break;

            case EstadoJuego2D::InformacionJuego:
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                    estadoActual = EstadoJuego2D::MenuInicio;
                }
                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarInformacionJuego();
                EndDrawing();
                break;

            case EstadoJuego2D::Creditos:
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                    estadoActual = EstadoJuego2D::MenuInicio;
                }
                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                dibujarCreditos();
                EndDrawing();
                break;

            case EstadoJuego2D::Jugando:
                procesarEntrada();
                generarObstaculos();
                actualizarEscena(dt);
                
                BeginDrawing();
                ClearBackground(BLACK);
                
                dibujarFondo();
                dibujarEstrellas();
                dibujarParticulas();
                
                // Obstáculos normales
                for (const auto &obs : obstaculos) {
                    dibujarObstaculo(obs);
                }
                
                // Proyectiles
                for (const auto &proj : proyectiles) {
                    if (proj.activo) {
                        DrawRectangle((int)proj.posicion.x - 2, (int)proj.posicion.y - 8, 4, 16, RED);
                        DrawRectangle((int)proj.posicion.x - 1, (int)proj.posicion.y - 6, 2, 12, WHITE);
                    }
                }

                // Jefe
                if (jefeActivoIndex != -1) {
                    dibujarJefe();
                }
                
                dibujarJugador();
                dibujarHUD();
                
                EndDrawing();
                break;

            case EstadoJuego2D::GameOver:
                if (IsKeyPressed(KEY_SPACE)) {
                    // Reiniciar partida activa
                    puntaje = 0;
                    vidas = 3;
                    nivel = 1;
                    combustible = 100;
                    combo = 0;
                    tiempoTranscurrido = 0.0f;
                    escudoActivo = false;
                    turboActivo = false;
                    cantidadEscudos = 0;
                    cantidadTurbos = 0;
                    jefeActivoIndex = -1;
                    nextBossIndex = 0;
                    bossSpawnCooldown = 12.0f;
                    obstaculos.clear();
                    particulas.clear();
                    proyectiles.clear();
                    jugadorPos = { (float)anchoPantalla / 2.0f, (float)altoPantalla - 90.0f };
                    estadoActual = EstadoJuego2D::Jugando;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estadoActual = EstadoJuego2D::MenuInicio;
                }

                BeginDrawing();
                ClearBackground(BLACK);
                dibujarFondo();
                dibujarEstrellas();
                actualizarEstrellas(dt);
                
                DibujarPanelVidrio(150, 100, 500, 400, RED, 0.85f);
                
                DrawText("MISION CONCLUIDA", anchoPantalla / 2 - MeasureText("MISION CONCLUIDA", 32) / 2, 130, 32, RED);
                DrawText("EL SECTOR HA SIDO EVALUADO", anchoPantalla / 2 - MeasureText("EL SECTOR HA SIDO EVALUADO", 12) / 2, 175, 12, GRAY);
                
                // Mostrar estadísticas finales
                DrawText(TextFormat("PILOTO:  %s", jugadorNombre.c_str()), 200, 220, 16, WHITE);
                DrawText(TextFormat("PUNTAJE OBTENIDO:  %d PTS", puntaje), 200, 255, 16, GOLD);
                DrawText(TextFormat("NIVEL DE AMENAZA:  NIVEL %d", nivel), 200, 290, 16, SKYBLUE);
                DrawText(TextFormat("TIEMPO EN SECTOR:  %d SEGUNDOS", (int)tiempoTranscurrido), 200, 325, 16, ORANGE);
                
                std::string rangoVal = "D (RECLUTA)";
                Color colorRango = RED;
                if (puntaje >= 2000) { rangoVal = "S (LEYENDA ESTELAR)"; colorRango = GOLD; }
                else if (puntaje >= 1200) { rangoVal = "A (AS DE VUELO)"; colorRango = SKYBLUE; }
                else if (puntaje >= 700) { rangoVal = "B (SUPERIOR)"; colorRango = GREEN; }
                else if (puntaje >= 300) { rangoVal = "C (ESTANDAR)"; colorRango = YELLOW; }
                
                DrawText(TextFormat("RANGO ASIGNADO:"), 200, 360, 16, WHITE);
                DrawText(rangoVal.c_str(), 360, 360, 16, colorRango);
                
                DrawText("Presiona ESPACIO para reintentar", anchoPantalla / 2 - MeasureText("Presiona ESPACIO para reintentar", 14) / 2, 430, 14, WHITE);
                DrawText("Presiona ESC para volver al menu principal", anchoPantalla / 2 - MeasureText("Presiona ESC para volver al menu principal", 12) / 2, 460, 12, GRAY);
                
                EndDrawing();
                break;
        }
    }

    // Descargar texturas al finalizar
    if (texturesLoaded) {
        UnloadTexture(shipTextures[0]);
        UnloadTexture(shipTextures[1]);
        UnloadTexture(shipTextures[2]);
        UnloadTexture(bossTextures[0]);
        UnloadTexture(bossTextures[1]);
    }
    CloseWindow();
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
        
        if (estrella.capa == 3) estrella.velocidad = 220.0f;
        else if (estrella.capa == 2) estrella.velocidad = 100.0f;
        else estrella.velocidad = 40.0f;
        
        estrellas.push_back(estrella);
    }
}

void Juego2D::actualizarEstrellas(float dt) {
    static std::default_random_engine generador((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distrX(0.0f, (float)anchoPantalla);
    
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

void Juego2D::procesarEntradaLoginRegister() {
    if (IsKeyPressed(KEY_UP)) {
        loginActiveField = (loginActiveField - 1 + 5) % 5;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        loginActiveField = (loginActiveField + 1) % 5;
    }
    if (IsKeyPressed(KEY_TAB)) {
        loginActiveField = (loginActiveField + 1) % 5;
    }

    // Capturar teclado para los campos de texto
    if (loginActiveField == 0 || loginActiveField == 1) {
        int key = GetCharPressed();
        while (key > 0) {
            // Permitir caracteres de texto estándar sin espacios (para evitar dañar archivos planos)
            if ((key >= 33) && (key <= 126)) {
                if (loginActiveField == 0 && inputUser.length() < 18) {
                    inputUser += (char)key;
                } else if (loginActiveField == 1 && inputPass.length() < 18) {
                    inputPass += (char)key;
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (loginActiveField == 0 && !inputUser.empty()) {
                inputUser.pop_back();
            } else if (loginActiveField == 1 && !inputPass.empty()) {
                inputPass.pop_back();
            }
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        // Enviar según el botón seleccionado
        if (loginActiveField == 0 || loginActiveField == 1 || loginActiveField == 2) {
            // Intento de Login
            if (inputUser.empty() || inputPass.empty()) {
                authMessage = "✕ ERROR: CAMPOS VACIOS";
                authMessageTimer = 3.0f;
            } else {
                if (sistemaUsuarios->iniciarSesion(inputUser, inputPass)) {
                    jugadorNombre = inputUser;
                    estadoActual = EstadoJuego2D::MenuInicio;
                    opcionMenu = 0;
                    inputUser = "";
                    inputPass = "";
                    authMessage = "✓ BIENVENIDO DE NUEVO, PILOTO";
                    authMessageTimer = 2.0f;
                    cargarConfiguracionLocal();
                } else {
                    authMessage = "✕ ERROR: USUARIO O ACCESO INCORRECTO";
                    authMessageTimer = 3.0f;
                }
            }
        } else if (loginActiveField == 3) {
            // Intento de Registro
            if (inputUser.empty() || inputPass.empty()) {
                authMessage = "✕ ERROR: CAMPOS VACIOS";
                authMessageTimer = 3.0f;
            } else {
                if (sistemaUsuarios->registrarUsuario(inputUser, inputPass)) {
                    authMessage = "✓ PILOTO REGISTRADO. INICIANDO SESION...";
                    authMessageTimer = 3.0f;
                    // Auto-iniciar sesión
                    if (sistemaUsuarios->iniciarSesion(inputUser, inputPass)) {
                        jugadorNombre = inputUser;
                        estadoActual = EstadoJuego2D::MenuInicio;
                        opcionMenu = 0;
                        inputUser = "";
                        inputPass = "";
                        cargarConfiguracionLocal();
                    }
                } else {
                    authMessage = "✕ ERROR: NOMBRE YA OCUPADO U DATOS INVALIDOS";
                    authMessageTimer = 3.0f;
                }
            }
        } else if (loginActiveField == 4) {
            // Salir
            juegoActivo = false;
        }
    }
}

void Juego2D::procesarEntradaMenu() {
    if (IsKeyPressed(KEY_UP)) {
        opcionMenu = (opcionMenu - 1 + 9) % 9;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        opcionMenu = (opcionMenu + 1) % 9;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        switch (opcionMenu) {
            case 0: // Jugar (Start Mission)
                puntaje = 0;
                vidas = 3;
                nivel = 1;
                combustible = 100;
                combo = 0;
                tiempoTranscurrido = 0.0f;
                escudoActivo = false;
                turboActivo = false;
                cantidadEscudos = 0;
                cantidadTurbos = 0;
                jefeActivoIndex = -1;
                nextBossIndex = 0;
                bossSpawnCooldown = 12.0f; // Boss en 12 segundos
                obstaculos.clear();
                particulas.clear();
                proyectiles.clear();
                jugadorPos = { (float)anchoPantalla / 2.0f, (float)altoPantalla - 90.0f };
                estadoActual = EstadoJuego2D::Jugando;
                break;
            case 1: // Personalizar nave
                estadoActual = EstadoJuego2D::Personalizacion;
                opcionCustomizer = 0;
                break;
            case 2: // Records globales
                estadoActual = EstadoJuego2D::RecordsGlobales;
                break;
            case 3: // Estadísticas personales
                estadoActual = EstadoJuego2D::EstadisticasPersonales;
                break;
            case 4: // Historial de partidas
                estadoActual = EstadoJuego2D::HistorialPartidas;
                break;
            case 5: // Información del juego
                estadoActual = EstadoJuego2D::InformacionJuego;
                break;
            case 6: // Créditos
                estadoActual = EstadoJuego2D::Creditos;
                break;
            case 7: // Cerrar sesión (Logout)
                // Reiniciar credenciales e ir a Login
                inputUser = "";
                inputPass = "";
                loginActiveField = 0;
                authMessage = "SESION CERRADA CORRECTAMENTE";
                authMessageTimer = 3.0f;
                estadoActual = EstadoJuego2D::LoginRegister;
                break;
            case 8: // Salir
                juegoActivo = false;
                break;
        }
    }
}

void Juego2D::procesarEntradaPersonalizacion() {
    if (IsKeyPressed(KEY_UP)) {
        opcionCustomizer = (opcionCustomizer - 1 + 5) % 5;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        opcionCustomizer = (opcionCustomizer + 1) % 5;
    }
    if (IsKeyPressed(KEY_LEFT)) {
        if (opcionCustomizer == 0) {
            seleccionNave = (seleccionNave - 1 + 4) % 4;
        } else if (opcionCustomizer == 1) {
            colorNave.r = (colorNave.r >= 15) ? (colorNave.r - 15) : 0;
        } else if (opcionCustomizer == 2) {
            colorNave.g = (colorNave.g >= 15) ? (colorNave.g - 15) : 0;
        } else if (opcionCustomizer == 3) {
            colorNave.b = (colorNave.b >= 15) ? (colorNave.b - 15) : 0;
        } else if (opcionCustomizer == 4) {
            seleccionArma = (seleccionArma - 1 + 4) % 4;
        }
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        if (opcionCustomizer == 0) {
            seleccionNave = (seleccionNave + 1) % 4;
        } else if (opcionCustomizer == 1) {
            colorNave.r = (colorNave.r <= 240) ? (colorNave.r + 15) : 255;
        } else if (opcionCustomizer == 2) {
            colorNave.g = (colorNave.g <= 240) ? (colorNave.g + 15) : 255;
        } else if (opcionCustomizer == 3) {
            colorNave.b = (colorNave.b <= 240) ? (colorNave.b + 15) : 255;
        } else if (opcionCustomizer == 4) {
            seleccionArma = (seleccionArma + 1) % 4;
        }
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        guardarConfiguracionLocal();
        estadoActual = EstadoJuego2D::MenuInicio;
    }
}

void Juego2D::procesarEntrada() {
    float dt = GetFrameTime();
    if (dt > 0.1f) dt = 0.1f;
    
    float velocidadActual = jugadorVelocidad;
    if (turboActivo) {
        velocidadActual *= 1.45f;
    }
    
    const float radioLimites = 25.0f;
    
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        jugadorPos.x = std::max(radioLimites, jugadorPos.x - velocidadActual * dt);
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        jugadorPos.x = std::min((float)anchoPantalla - radioLimites, jugadorPos.x + velocidadActual * dt);
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        jugadorPos.y = std::max(50.0f, jugadorPos.y - velocidadActual * dt);
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        jugadorPos.y = std::min((float)altoPantalla - 50.0f, jugadorPos.y + velocidadActual * dt);
    }
    
    // Cambiar de arma en tiempo real
    if (IsKeyPressed(KEY_F)) {
        seleccionArma = (seleccionArma + 1) % 4;
        mensajeNotificacion = "WEAPON SWITCHED: " + std::string(seleccionArma == 0 ? "LASER" : seleccionArma == 1 ? "TRIPLE SHOT" : seleccionArma == 2 ? "ENERGY WAVE" : "GUIDED MISSILE");
        timerNotificacion = 2.0f;
    }

    // Activar consumibles manualmente
    if (IsKeyPressed(KEY_S)) {
        if (cantidadEscudos > 0 && !escudoActivo) {
            cantidadEscudos--;
            escudoActivo = true;
            duracionEscudo = 8.0f; // 8 segundos de escudo
            mensajeNotificacion = "ESCUDO DEFLECTOR ACTIVADO!";
            timerNotificacion = 2.0f;
        }
    }

    if (IsKeyPressed(KEY_T)) {
        if (cantidadTurbos > 0 && !turboActivo) {
            cantidadTurbos--;
            turboActivo = true;
            duracionTurbo = 6.0f; // 6 segundos de supervelocidad
            mensajeNotificacion = "SOBREPROPULSION TURBO ACTIVADA!";
            timerNotificacion = 2.0f;
        }
    }

    // Disparar proyectiles
    weaponCooldownTimer -= dt;
    if (IsKeyDown(KEY_SPACE) && weaponCooldownTimer <= 0.0f) {
        float cooldowns[] = { 0.2f, 0.35f, 0.5f, 0.8f };
        weaponCooldownTimer = cooldowns[seleccionArma];

        if (seleccionArma == 0) { // Laser
            proyectiles.push_back({ { jugadorPos.x, jugadorPos.y - 20 }, { 0, -500 }, 10, true });
        } else if (seleccionArma == 1) { // Triple
            proyectiles.push_back({ { jugadorPos.x - 10, jugadorPos.y - 15 }, { -80, -450 }, 8, true });
            proyectiles.push_back({ { jugadorPos.x, jugadorPos.y - 20 }, { 0, -450 }, 8, true });
            proyectiles.push_back({ { jugadorPos.x + 10, jugadorPos.y - 15 }, { 80, -450 }, 8, true });
        } else if (seleccionArma == 2) { // Energy Wave
            proyectiles.push_back({ { jugadorPos.x, jugadorPos.y - 20 }, { 0, -350 }, 25, true });
        } else if (seleccionArma == 3) { // Guided Missile
            proyectiles.push_back({ { jugadorPos.x, jugadorPos.y - 20 }, { 0, -300 }, 40, true });
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        // Guardar record de partida en curso por si acaso y salir a menu
        if (sistemaUsuarios) {
            sistemaUsuarios->registrarRecord(jugadorNombre, puntaje);
            sistemaUsuarios->actualizarPuntajeActual(puntaje);
        }
        estadoActual = EstadoJuego2D::MenuInicio;
    }
}

void Juego2D::generarObstaculos() {
    static std::default_random_engine generador((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    static std::uniform_real_distribution<float> distrChance(0.0f, 100.0f);
    static std::uniform_real_distribution<float> distrVel(100.0f, 220.0f);
    static std::uniform_real_distribution<float> distrRotSpeed(-3.0f, 3.0f);
    static std::uniform_real_distribution<float> distrX(40.0f, 760.0f);

    // CONTROL DE SPAWN DEL JEFE PERSISTENTE
    if (jefeActivoIndex == -1) {
        if (bossSpawnCooldown <= 0.0f) {
            // Invocar el jefe actual
            jefeActivoIndex = nextBossIndex;
            jefeVidaMax = 100 + jefeActivoIndex * 120;
            jefeVida = jefeVidaMax;
            jefeFase = 1;
            jefePos = { (float)anchoPantalla / 2.0f, -100.0f };
            jefeMovTimer = 0.0f;
            
            std::string bossNames[] = {
                "Alpha Vanguard", "Beta Swarm Lord", "Gamma Ray Cruiser",
                "Delta Mech-Leviathan", "Epsilon Singularity", "Zeta Cosmic Serpent",
                "Eta Nebular Colossus", "Theta Chrono Warden", "Iota Void Reaper",
                "Omega Evader Destroyer"
            };
            mensajeNotificacion = "AMENAZA DETECTADA: " + bossNames[jefeActivoIndex];
            timerNotificacion = 4.0f;
        } else {
            bossSpawnCooldown -= GetFrameTime();
        }
    }

    // Si hay un jefe activo, no generamos obstáculos dañinos normales (Meteoros/Drones), 
    // pero si cometas de combustible y powerups a baja tasa para no ahogar al jugador sin combustible.
    if (jefeActivoIndex != -1) {
        int maxItemsBossFight = 3;
        if ((int)obstaculos.size() >= maxItemsBossFight) return;

        if (distrChance(generador) < 1.5f) { // Probabilidad muy baja
            Obstaculo2D nuevo;
            nuevo.posicion.y = -50.0f;
            nuevo.velocidad = 150.0f;
            nuevo.rotacion = 0.0f;
            nuevo.velocidadRotacion = distrRotSpeed(generador);
            nuevo.escala = 1.0f;
            nuevo.posicion.x = distrX(generador);

            float roll = distrChance(generador);
            if (roll < 45.0f) {
                nuevo.tipo = TipoObstaculo2D::Cometa; // COMETA (Fuel)
            } else if (roll < 75.0f) {
                nuevo.tipo = TipoObstaculo2D::PowerUpEscudo;
            } else {
                nuevo.tipo = TipoObstaculo2D::PowerUpTurbo;
            }
            obstaculos.push_back(nuevo);
        }
        return;
    }

    // Generación normal de obstáculos cuando no hay jefe
    int maxObstaculos = std::min(24, 7 + nivel * 2);
    if ((int)obstaculos.size() > maxObstaculos) {
        return;
    }
    
    float umbralGeneracion = 1.6f + (float)nivel * 0.35f;
    if (distrChance(generador) < umbralGeneracion) {
        Obstaculo2D nuevo;
        nuevo.posicion.y = -50.0f;
        
        float velBase = 120.0f + (float)nivel * 16.0f;
        nuevo.velocidad = velBase + distrVel(generador) * 0.4f;
        
        float chance = distrChance(generador);
        TipoObstaculo2D tipo;
        float escala = 1.0f;
        
        if (chance < 4.0f) {
            tipo = TipoObstaculo2D::PowerUpEscudo;
        } else if (chance < 8.0f) {
            tipo = TipoObstaculo2D::PowerUpTurbo;
        } else if (chance < 28.0f) {
            tipo = TipoObstaculo2D::Cometa;
        } else if (chance < 55.0f) {
            tipo = TipoObstaculo2D::Drone;
            escala = 1.1f;
        } else {
            tipo = TipoObstaculo2D::Meteoro;
            std::uniform_real_distribution<float> distrEsc(0.8f, 1.6f);
            escala = distrEsc(generador);
        }
        
        nuevo.posicion.x = distrX(generador);
        nuevo.tipo = tipo;
        nuevo.rotacion = 0.0f;
        nuevo.velocidadRotacion = distrRotSpeed(generador);
        nuevo.escala = escala;
        
        obstaculos.push_back(nuevo);
    }
}

void Juego2D::actualizarJefe(float dt) {
    if (jefeActivoIndex == -1) return;

    jefeMovTimer += dt;

    // Movimiento de entrada del jefe
    if (jefePos.y < 120.0f) {
        jefePos.y += 80.0f * dt;
    } else {
        // Patrones de ataque del jefe según su fase
        switch (jefeFase) {
            case 1:
                jefePos.x = (float)anchoPantalla / 2.0f + sinf(jefeMovTimer * 2.0f) * 200.0f;
                break;
            case 2:
                jefePos.x = (float)anchoPantalla / 2.0f + sinf(jefeMovTimer * 2.5f) * 240.0f;
                jefePos.y = 120.0f + cosf(jefeMovTimer * 4.0f) * 30.0f;
                break;
            case 3:
                jefePos.x = (float)anchoPantalla / 2.0f + sinf(jefeMovTimer * 3.5f) * 280.0f;
                jefePos.y = 120.0f + sinf(jefeMovTimer * 1.5f) * 70.0f;
                break;
        }

        // Disparo periódico del boss (creación de partículas/proyectiles visuales)
        if (rand() % 70 == 0) {
            crearExplosion(jefePos, RED, 8);
            
            // Añadir un obstáculo temporal tipo dron dirigido hacia abajo justo debajo del boss
            Obstaculo2D balaBoss;
            balaBoss.posicion = { jefePos.x, jefePos.y + 20.0f };
            balaBoss.velocidad = 280.0f;
            balaBoss.tipo = TipoObstaculo2D::Drone;
            balaBoss.rotacion = 180.0f;
            balaBoss.velocidadRotacion = 0.0f;
            balaBoss.escala = 0.8f;
            obstaculos.push_back(balaBoss);
        }
    }
}

void Juego2D::actualizarEscena(float dt) {
    tiempoTranscurrido += dt;
    actualizarEstrellas(dt);
    crearChispasMotor(jugadorPos, turboActivo);
    
    // Actualizar jefe
    actualizarJefe(dt);

    // Mover obstáculos
    for (auto &obs : obstaculos) {
        obs.posicion.y += obs.velocidad * dt;
        obs.rotacion += obs.velocidadRotacion * 50.0f * dt;
    }
    
    // Mover proyectiles
    for (auto &proj : proyectiles) {
        proj.posicion.x += proj.velocidad.x * dt;
        proj.posicion.y += proj.velocidad.y * dt;
        if (proj.posicion.y < -20.0f || proj.posicion.x < -20.0f || proj.posicion.x > anchoPantalla + 20.0f) {
            proj.activo = false;
        }
    }
    
    // Limpiar proyectiles muertos
    proyectiles.erase(std::remove_if(proyectiles.begin(), proyectiles.end(), [](const Proyectil2D &p) {
        return !p.activo;
    }), proyectiles.end());

    // Puntos por esquivar obstáculos normales
    for (const auto &obs : obstaculos) {
        if (obs.posicion.y > (float)altoPantalla + 30.0f) {
            if (obs.tipo == TipoObstaculo2D::Meteoro || obs.tipo == TipoObstaculo2D::Drone) {
                puntaje += 15;
            }
        }
    }
    
    obstaculos.erase(std::remove_if(obstaculos.begin(), obstaculos.end(), [&](const Obstaculo2D &obs) {
        return obs.posicion.y > (float)altoPantalla + 35.0f;
    }), obstaculos.end());
    
    // Comprobación de Colisiones: Jugador vs Obstáculos
    Rectangle recNave = { jugadorPos.x - 22.0f, jugadorPos.y - 20.0f, 44.0f, 32.0f };
    
    for (auto it = obstaculos.begin(); it != obstaculos.end(); ) {
        float obsRadius = 15.0f * it->escala;
        Rectangle recObs = { it->posicion.x - obsRadius, it->posicion.y - obsRadius, obsRadius * 2.0f, obsRadius * 2.0f };
        
        if (CheckCollisionRecs(recNave, recObs)) {
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
                        crearExplosion(colPos, RED, 40);
                        vidas--;
                        combo = 0;
                        mensajeNotificacion = "✕ IMPACTO DETECTADO! -1 VIDA ✕";
                        timerNotificacion = 3.5f;
                    }
                    break;
                case TipoObstaculo2D::Cometa:
                    crearExplosion(colPos, GOLD, 20);
                    puntaje += 30;
                    combo++;
                    combustible = std::min(130, combustible + 15);
                    break;
                case TipoObstaculo2D::PowerUpEscudo:
                    crearExplosion(colPos, BLUE, 22);
                    cantidadEscudos = std::min(5, cantidadEscudos + 1);
                    break;
                case TipoObstaculo2D::PowerUpTurbo:
                    crearExplosion(colPos, ORANGE, 22);
                    cantidadTurbos = std::min(5, cantidadTurbos + 1);
                    break;
            }
            it = obstaculos.erase(it);
        } else {
            ++it;
        }
    }

    // Colisión física Jugador vs Cuerpo del Boss
    if (jefeActivoIndex != -1) {
        Rectangle bossRec = { jefePos.x - 70.0f, jefePos.y - 45.0f, 140.0f, 90.0f };
        if (CheckCollisionRecs(recNave, bossRec)) {
            if (escudoActivo) {
                crearExplosion(jugadorPos, SKYBLUE, 30);
                escudoActivo = false;
                duracionEscudo = 0.0f;
            } else {
                crearExplosion(jugadorPos, RED, 40);
                vidas--;
            }
            // Repeler jugador hacia abajo para evitar daño múltiple por contacto inmediato
            jugadorPos.y = std::min((float)altoPantalla - 90.0f, jugadorPos.y + 120.0f);
            mensajeNotificacion = "✕ COLISION DIRECTA CON JEFE! ✕";
            timerNotificacion = 3.0f;
        }
    }
    
    // Proyectiles del Jugador vs Jefe / Obstáculos
    for (auto &proj : proyectiles) {
        if (!proj.activo) continue;
        
        // Impacto contra el jefe activo
        if (jefeActivoIndex != -1) {
            Rectangle bossRec = { jefePos.x - 70.0f, jefePos.y - 45.0f, 140.0f, 90.0f };
            Vector2 pCenter = proj.posicion;
            if (CheckCollisionPointRec(pCenter, bossRec)) {
                jefeVida -= proj.damage;
                proj.activo = false;
                crearExplosion(pCenter, ORANGE, 6);
                
                // Actualizar fases del boss según salud
                float healthPct = (float)jefeVida / jefeVidaMax;
                if (healthPct < 0.35f && jefeFase < 3) {
                    jefeFase = 3;
                    mensajeNotificacion = "JEFE: FASE FINAL ACTIVA!";
                    timerNotificacion = 2.0f;
                } else if (healthPct < 0.7f && jefeFase < 2) {
                    jefeFase = 2;
                    mensajeNotificacion = "JEFE: FASE 2 INICIADA!";
                    timerNotificacion = 2.0f;
                }

                // Si el jefe es derrotado
                if (jefeVida <= 0) {
                    crearExplosion(jefePos, GOLD, 85);
                    jefeActivoIndex = -1; // Desactivar jefe actual
                    puntaje += 500;
                    nextBossIndex = (nextBossIndex + 1) % 10; // Cliclar entre los 10 jefes
                    bossSpawnCooldown = 15.0f; // 15 segundos hasta el próximo
                    mensajeNotificacion = "JEFE DERROTADO! +500 PTS. RECESO (15s)";
                    timerNotificacion = 4.0f;
                }
                continue;
            }
        }

        // Impacto contra meteoros / drones normales
        for (auto it = obstaculos.begin(); it != obstaculos.end(); ++it) {
            if (it->tipo == TipoObstaculo2D::Meteoro || it->tipo == TipoObstaculo2D::Drone) {
                float r = 15.0f * it->escala;
                Rectangle obsRec = { it->posicion.x - r, it->posicion.y - r, r * 2.0f, r * 2.0f };
                if (CheckCollisionPointRec(proj.posicion, obsRec)) {
                    proj.activo = false;
                    crearExplosion(it->posicion, RED, 18);
                    it = obstaculos.erase(it);
                    puntaje += 50;
                    break;
                }
            }
        }
    }

    // Duración de potenciadores activos
    if (escudoActivo) {
        duracionEscudo -= dt;
        if (duracionEscudo <= 0.0f) escudoActivo = false;
    }
    if (turboActivo) {
        duracionTurbo -= dt;
        if (duracionTurbo <= 0.0f) turboActivo = false;
    }
    
    // Gasto de combustible reactivo
    float tasaGasto = (1.5f + (float)nivel * 0.25f);
    if (turboActivo) tasaGasto *= 2.0f;
    
    static float acumCombustible = 0.0f;
    acumCombustible += tasaGasto * dt;
    if (acumCombustible >= 1.0f) {
        combustible = std::max(0, combustible - (int)acumCombustible);
        acumCombustible = 0.0f;
    }
    
    if (combustible <= 0) {
        vidas--;
        combustible = 35;
        mensajeNotificacion = "✕ COMBUSIBLE AGOTADO! -1 VIDA ✕";
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
        return p.vidaRestante <= 0;
    }), particulas.end());
    
    if (timerNotificacion > 0.0f) {
        timerNotificacion -= dt;
        if (timerNotificacion <= 0.0f) mensajeNotificacion = "";
    }
    
    actualizarNivel();
    
    // Al morir (sin vidas) se guarda automáticamente el record y estadísticas localmente
    if (vidas <= 0) {
        if (sistemaUsuarios) {
            sistemaUsuarios->registrarRecord(jugadorNombre, puntaje);
            sistemaUsuarios->actualizarPuntajeActual(puntaje);
        }
        estadoActual = EstadoJuego2D::GameOver;
    }
}

void Juego2D::actualizarNivel() {
    int nuevoNivel = puntaje / 250 + 1;
    if (nuevoNivel != nivel) {
        nivel = nuevoNivel;
        mensajeNotificacion = "★ SECTOR DE AMENAZA NIVEL " + std::to_string(nivel) + " ★";
        timerNotificacion = 4.0f;
    }
}

void Juego2D::dibujarFondo() const {
    DrawCircleGradient(Vector2{ (float)anchoPantalla / 2.0f, (float)altoPantalla / 2.0f }, 450.0f, ColorAlpha(DARKBLUE, 0.05f), BLACK);
}

void Juego2D::dibujarJugador() const {
    Vector2 p = jugadorPos;
    
    if (escudoActivo) {
        DrawCircleLinesV(p, 36.0f, ColorAlpha(SKYBLUE, 0.9f));
        DrawCircleV(p, 36.0f, ColorAlpha(SKYBLUE, 0.12f));
    }
    
    bool dibujadoConTextura = false;
    // Si la textura de nave se cargó correctamente, la dibujamos en lugar del vector
    if (texturesLoaded && seleccionNave >= 0 && seleccionNave < 3) {
        Texture2D tex = shipTextures[seleccionNave];
        if (tex.id > 0) {
            DrawTexturePro(tex, 
                Rectangle{ 0.0f, 0.0f, (float)tex.width, (float)tex.height },
                Rectangle{ p.x, p.y, 64.0f, 64.0f },
                Vector2{ 32.0f, 32.0f }, // centro como origen
                0.0f,
                colorNave);
            dibujadoConTextura = true;
        }
    }

    if (!dibujadoConTextura) {
        // Renderizado vectorial como fallback
        switch (seleccionNave) {
            case 0: // Interceptor: Classic triangle + wings
                {
                    Vector2 alaIzq = { p.x - 24, p.y + 12 };
                    Vector2 nose = { p.x, p.y - 28 };
                    Vector2 alaDer = { p.x + 24, p.y + 12 };
                    DrawTriangle(alaIzq, nose, alaDer, colorNave);
                    DrawTriangleLines(alaIzq, nose, alaDer, WHITE);
                }
                break;
            case 1: // Bomber: Rectangular delta wing
                {
                    DrawRectangle((int)p.x - 30, (int)p.y + 5, 60, 15, colorNave);
                    DrawRectangle((int)p.x - 10, (int)p.y - 25, 20, 30, colorNave);
                    DrawRectangleLines((int)p.x - 30, (int)p.y - 25, 60, 45, WHITE);
                }
                break;
            case 2: // Sleek Fighter: Swept wings + double tail
                {
                    DrawTriangle(Vector2{p.x, p.y - 32}, Vector2{p.x - 8, p.y + 8}, Vector2{p.x + 8, p.y + 8}, colorNave);
                    DrawLineV(Vector2{p.x - 24, p.y + 12}, Vector2{p.x - 8, p.y}, colorNave);
                    DrawLineV(Vector2{p.x + 24, p.y + 12}, Vector2{p.x + 8, p.y}, colorNave);
                }
                break;
            case 3: // Saucer/UFO: Circular body
                {
                    DrawCircleV(p, 22, colorNave);
                    DrawCircleLinesV(p, 22, WHITE);
                    DrawEllipse((int)p.x, (int)p.y + 5, 28, 8, DARKGRAY);
                }
                break;
        }
    }
}

void Juego2D::dibujarObstaculo(const Obstaculo2D &obs) const {
    Vector2 p = obs.posicion;
    float esc = obs.escala;
    
    switch (obs.tipo) {
        case TipoObstaculo2D::Meteoro: {
            DrawCircleV(p, 18.0f * esc, Color{105, 55, 30, 255});
            DrawCircleLinesV(p, 18.0f * esc, Color{70, 35, 15, 255});
            break;
        }
        case TipoObstaculo2D::Drone: {
            float s = 14.0f * esc;
            DrawTriangle(Vector2{p.x - s, p.y - s}, Vector2{p.x, p.y + s}, Vector2{p.x + s, p.y - s}, GREEN);
            DrawTriangleLines(Vector2{p.x - s, p.y - s}, Vector2{p.x, p.y + s}, Vector2{p.x + s, p.y - s}, WHITE);
            break;
        }
        case TipoObstaculo2D::Cometa: {
            DrawCircleV(p, 8.5f * esc, GOLD);
            break;
        }
        case TipoObstaculo2D::PowerUpEscudo: {
            DrawPoly(p, 6, 12, 0, SKYBLUE);
            DrawText("S", (int)p.x - 3, (int)p.y - 5, 11, BLACK);
            break;
        }
        case TipoObstaculo2D::PowerUpTurbo: {
            DrawPoly(p, 4, 12, 0, ORANGE);
            DrawText("T", (int)p.x - 3, (int)p.y - 5, 11, BLACK);
            break;
        }
    }
}

void Juego2D::dibujarJefe() const {
    if (jefeActivoIndex == -1) return;

    Vector2 p = jefePos;
    bool dibujadoConTextura = false;

    // Si la textura de jefe se cargó, la renderizamos de forma adaptativa
    if (texturesLoaded) {
        Texture2D tex = bossTextures[jefeActivoIndex % 2];
        if (tex.id > 0) {
            DrawTexturePro(tex,
                Rectangle{ 0.0f, 0.0f, (float)tex.width, (float)tex.height },
                Rectangle{ p.x, p.y, 140.0f, 90.0f },
                Vector2{ 70.0f, 45.0f }, // centro como origen
                0.0f,
                WHITE);
            dibujadoConTextura = true;
        }
    }

    if (!dibujadoConTextura) {
        // Vector fallback
        DrawRectangle((int)p.x - 60, (int)p.y - 35, 120, 70, MAROON);
        DrawRectangleLines((int)p.x - 60, (int)p.y - 35, 120, 70, RED);

        // Glowing core
        DrawCircleV(p, 14.0f, RED);
        DrawCircleLinesV(p, 14.0f, WHITE);
    }

    // Barra de salud del jefe
    DrawRectangle((int)p.x - 60, (int)p.y - 52, 120, 8, BLACK);
    float pct = (float)jefeVida / jefeVidaMax;
    DrawRectangle((int)p.x - 58, (int)p.y - 51, (int)(116.0f * pct), 6, RED);
}

void Juego2D::dibujarParticulas() const {
    for (const auto &p : particulas) {
        Color col = ColorAlpha(p.color, p.opacidad);
        DrawCircleV(p.posicion, p.radio, col);
    }
}

void Juego2D::dibujarLoginRegister() const {
    // Título cyberpunk
    DrawText("A E R O   E V A D E R   P R O", anchoPantalla / 2 - MeasureText("A E R O   E V A D E R   P R O", 26) / 2, 70, 26, SKYBLUE);
    DrawText("SISTEMA DE CONTROL COGNITIVO DEL PILOTO", anchoPantalla / 2 - MeasureText("SISTEMA DE CONTROL COGNITIVO DEL PILOTO", 10) / 2, 108, 10, GRAY);

    // Contenedor principal
    DibujarPanelVidrio(200, 140, 400, 370, SKYBLUE, 0.85f);

    // Campo de Usuario
    DrawText("IDENTIFICACION DEL PILOTO (USER)", 240, 175, 11, loginActiveField == 0 ? GOLD : WHITE);
    DrawRectangle(240, 195, 320, 36, ColorAlpha(BLACK, 0.5f));
    DrawRectangleLines(240, 195, 320, 36, loginActiveField == 0 ? GOLD : GRAY);
    
    std::string userDisplay = inputUser;
    if (loginActiveField == 0 && (int)(GetTime() * 2) % 2 == 0) userDisplay += "|";
    DrawText(userDisplay.c_str(), 255, 205, 16, WHITE);

    // Campo de Contraseña
    DrawText("CLAVE DE ACCESO CRIPTOGRAFICO (PASS)", 240, 255, 11, loginActiveField == 1 ? GOLD : WHITE);
    DrawRectangle(240, 275, 320, 36, ColorAlpha(BLACK, 0.5f));
    DrawRectangleLines(240, 275, 320, 36, loginActiveField == 1 ? GOLD : GRAY);
    
    std::string passDisplay(inputPass.length(), '*');
    if (loginActiveField == 1 && (int)(GetTime() * 2) % 2 == 0) passDisplay += "|";
    DrawText(passDisplay.c_str(), 255, 285, 16, WHITE);

    // Botones
    // Botón Iniciar Sesión
    DrawRectangle(240, 345, 320, 36, loginActiveField == 2 ? GOLD : ColorAlpha(SKYBLUE, 0.2f));
    DrawRectangleLines(240, 345, 320, 36, loginActiveField == 2 ? WHITE : SKYBLUE);
    DrawText("INICIAR SESION", 240 + 160 - MeasureText("INICIAR SESION", 14) / 2, 355, 14, loginActiveField == 2 ? BLACK : WHITE);

    // Botón Registrarse
    DrawRectangle(240, 395, 320, 36, loginActiveField == 3 ? GOLD : ColorAlpha(GREEN, 0.2f));
    DrawRectangleLines(240, 395, 320, 36, loginActiveField == 3 ? WHITE : GREEN);
    DrawText("REGISTRAR NUEVO PILOTO", 240 + 160 - MeasureText("REGISTRAR NUEVO PILOTO", 14) / 2, 405, 14, loginActiveField == 3 ? BLACK : WHITE);

    // Botón Salir
    DrawRectangle(240, 445, 320, 36, loginActiveField == 4 ? GOLD : ColorAlpha(RED, 0.2f));
    DrawRectangleLines(240, 445, 320, 36, loginActiveField == 4 ? WHITE : RED);
    DrawText("APAGAR SISTEMAS", 240 + 160 - MeasureText("APAGAR SISTEMAS", 14) / 2, 455, 14, loginActiveField == 4 ? BLACK : WHITE);

    // Mensaje de autenticación / Notificación
    if (!authMessage.empty()) {
        Color msgCol = SKYBLUE;
        if (authMessage[0] == 'X' || authMessage[0] == 0xe2 || authMessage.find("ERROR") != std::string::npos || authMessage.find("✕") != std::string::npos) {
            msgCol = RED;
        } else if (authMessage.find("✓") != std::string::npos) {
            msgCol = GREEN;
        }
        DrawText(authMessage.c_str(), anchoPantalla / 2 - MeasureText(authMessage.c_str(), 11) / 2, 525, 11, msgCol);
    }
}

void Juego2D::dibujarMenuInicio() const {
    DrawText("AERO EVADER PRO", 60, 60, 36, SKYBLUE);
    DrawText("PLANO DE VUELO & CONTROL DE SISTEMAS", 60, 105, 12, GRAY);
    
    // Panel de bienvenida
    std::string welcomeStr = "PILOTO AUTENTICADO:  " + jugadorNombre;
    DrawText(welcomeStr.c_str(), 60, 135, 14, GREEN);

    // Menú izquierdo
    DibujarPanelVidrio(60, 170, 320, 360, SKYBLUE, 0.82f);

    std::string options[] = {
        "1. INICIAR MISION DE VUELO",
        "2. CONFIGURAR NAVE Y ARMAS",
        "3. RECORDS GLOBALES",
        "4. DIAGNOSTICOS DEL PILOTO",
        "5. HISTORIAL DE PARTIDAS",
        "6. MANUAL DEL SISTEMA",
        "7. BITACORA DE CREDITOS",
        "8. DESCONECTAR PILOTO",
        "9. APAGAR SISTEMAS"
    };

    for (int i = 0; i < 9; ++i) {
        Color col = (opcionMenu == i) ? GOLD : WHITE;
        std::string txt = (opcionMenu == i) ? "> " + options[i] : "  " + options[i];
        DrawText(txt.c_str(), 80, 185 + i * 38, 14, col);
    }

    // Panel derecho: Vista previa interactiva de la nave
    DibujarPanelVidrio(410, 170, 330, 360, SKYBLUE, 0.82f);
    DrawText("ESTRUCTURA DE NAVE ASIGNADA", 430, 190, 12, GRAY);

    // Dibujar la vista previa de la nave rotando levemente en el centro del panel
    Vector2 previewPos = { 575.0f, 340.0f };
    float anguloAnimado = sinf(GetTime() * 1.5f) * 20.0f; // Oscilación visual de la nave

    // Si las texturas están cargadas, renderizamos la imagen de la nave seleccionada
    bool dibujadoConTextura = false;
    if (texturesLoaded && seleccionNave >= 0 && seleccionNave < 3) {
        Texture2D tex = shipTextures[seleccionNave];
        if (tex.id > 0) {
            DrawTexturePro(tex, 
                Rectangle{ 0.0f, 0.0f, (float)tex.width, (float)tex.height },
                Rectangle{ previewPos.x, previewPos.y, 110.0f, 110.0f },
                Vector2{ 55.0f, 55.0f }, // centro como origen
                anguloAnimado,
                colorNave);
            dibujadoConTextura = true;
        }
    }

    if (!dibujadoConTextura) {
        // Vectorial fallback
        DrawCircleV(previewPos, 32.0f, ColorAlpha(colorNave, 0.2f));
        DrawCircleLinesV(previewPos, 32.0f, ColorAlpha(WHITE, 0.4f));
        switch (seleccionNave) {
            case 0:
                DrawTriangle(Vector2{previewPos.x - 30, previewPos.y + 15}, Vector2{previewPos.x, previewPos.y - 35}, Vector2{previewPos.x + 30, previewPos.y + 15}, colorNave);
                break;
            case 1:
                DrawRectangle((int)previewPos.x - 35, (int)previewPos.y + 5, 70, 20, colorNave);
                DrawRectangle((int)previewPos.x - 12, (int)previewPos.y - 30, 24, 35, colorNave);
                break;
            case 2:
                DrawTriangle(Vector2{previewPos.x, previewPos.y - 40}, Vector2{previewPos.x - 12, previewPos.y + 10}, Vector2{previewPos.x + 12, previewPos.y + 10}, colorNave);
                break;
            case 3:
                DrawCircleV(previewPos, 28, colorNave);
                break;
        }
    }

    // Información del arma actual abajo del panel derecho
    std::string weaponNames[] = { "LASER SECUENCIAL", "DISPARO TRIPLE", "ONDA DE ENERGIA", "MISIL TELEDIRIGIDO" };
    DrawText("SISTEMA DE ARMAS ACTIVO:", 430, 480, 11, GRAY);
    DrawText(weaponNames[seleccionArma].c_str(), 430, 500, 15, GOLD);
}

void Juego2D::dibujarPersonalizacion() const {
    DrawText("PERSONALIZACION DE NAVE", anchoPantalla / 2 - MeasureText("PERSONALIZACION DE NAVE", 28) / 2, 60, 28, GREEN);
    DrawText("CALIBRACION DE COLOR Y SISTEMAS DE ATAQUE", anchoPantalla / 2 - MeasureText("CALIBRACION DE COLOR Y SISTEMAS DE ATAQUE", 11) / 2, 95, 11, GRAY);

    // Panel izquierdo
    DibujarPanelVidrio(100, 130, 350, 390, GREEN, 0.85f);

    std::string fields[] = {
        TextFormat("MODELO DE NAVE:   INTERCEPTOR %d/4", seleccionNave + 1),
        TextFormat("TONO ROJO:   %d", colorNave.r),
        TextFormat("TONO VERDE:  %d", colorNave.g),
        TextFormat("TONO AZUL:   %d", colorNave.b),
        TextFormat("ARMA INICIAL:  %s", seleccionArma == 0 ? "LASER" : seleccionArma == 1 ? "TRIPLE SHOT" : seleccionArma == 2 ? "ENERGY WAVE" : "GUIDED MISSILE")
    };

    for (int i = 0; i < 5; ++i) {
        Color col = (opcionCustomizer == i) ? GOLD : WHITE;
        std::string txt = (opcionCustomizer == i) ? "> " + fields[i] : "  " + fields[i];
        DrawText(txt.c_str(), 125, 175 + i * 55, 15, col);
    }

    // Panel derecho: Vista previa detallada
    DibujarPanelVidrio(470, 130, 230, 390, GREEN, 0.85f);
    DrawText("CALIBRADO DE CHASIS", 470 + 115 - MeasureText("CALIBRADO DE CHASIS", 11) / 2, 155, 11, GRAY);

    Vector2 previewPos = { 585.0f, 310.0f };
    
    // Dibujar nave detallada
    bool dibujadoConTextura = false;
    if (texturesLoaded && seleccionNave >= 0 && seleccionNave < 3) {
        Texture2D tex = shipTextures[seleccionNave];
        if (tex.id > 0) {
            DrawTexturePro(tex, 
                Rectangle{ 0.0f, 0.0f, (float)tex.width, (float)tex.height },
                Rectangle{ previewPos.x, previewPos.y, 120.0f, 120.0f },
                Vector2{ 60.0f, 60.0f },
                0.0f,
                colorNave);
            dibujadoConTextura = true;
        }
    }

    if (!dibujadoConTextura) {
        // Fallback vector
        DrawCircleV(previewPos, 40.0f, ColorAlpha(colorNave, 0.2f));
        DrawCircleLinesV(previewPos, 40.0f, ColorAlpha(WHITE, 0.4f));
        switch (seleccionNave) {
            case 0:
                DrawTriangle(Vector2{previewPos.x - 36, previewPos.y + 18}, Vector2{previewPos.x, previewPos.y - 45}, Vector2{previewPos.x + 36, previewPos.y + 18}, colorNave);
                break;
            case 1:
                DrawRectangle((int)previewPos.x - 42, (int)previewPos.y + 5, 84, 25, colorNave);
                DrawRectangle((int)previewPos.x - 16, (int)previewPos.y - 38, 32, 45, colorNave);
                break;
            case 2:
                DrawTriangle(Vector2{previewPos.x, previewPos.y - 48}, Vector2{previewPos.x - 14, previewPos.y + 12}, Vector2{previewPos.x + 14, previewPos.y + 12}, colorNave);
                break;
            case 3:
                DrawCircleV(previewPos, 32, colorNave);
                break;
        }
    }

    DrawText("Use ARRIB/ABAJ para navegar", anchoPantalla / 2 - MeasureText("Use ARRIB/ABAJ para navegar", 11) / 2, 535, 11, GRAY);
    DrawText("Use IZQ/DER para ajustar, ENTER para guardar", anchoPantalla / 2 - MeasureText("Use IZQ/DER para ajustar, ENTER para guardar", 11) / 2, 555, 11, GRAY);
}

void Juego2D::dibujarRecordsGlobales() const {
    DrawText("TABLA DE RECORDS GLOBALES", anchoPantalla / 2 - MeasureText("TABLA DE RECORDS GLOBALES", 28) / 2, 50, 28, GOLD);
    DrawText("PUNTUACIONES MAS ALTAS REGISTRADAS EN LA RED", anchoPantalla / 2 - MeasureText("PUNTUACIONES MAS ALTAS REGISTRADAS EN LA RED", 11) / 2, 85, 11, GRAY);

    DibujarPanelVidrio(150, 110, 500, 410, GOLD, 0.85f);

    // Leer records localmente
    struct Record {
        std::string name;
        int score;
    };
    std::vector<Record> records;
    std::ifstream archivo("records.txt");
    if (archivo.is_open()) {
        std::string n;
        int s;
        while (archivo >> n >> s) {
            records.push_back({ n, s });
        }
        archivo.close();
    }

    std::sort(records.begin(), records.end(), [](const Record &a, const Record &b) {
        return a.score > b.score;
    });

    DrawText("RANGO", 200, 140, 14, GOLD);
    DrawText("PILOTO (CALLSIGN)", 280, 140, 14, GOLD);
    DrawText("PUNTAJE", 520, 140, 14, GOLD);

    DrawLine(180, 165, 620, 165, ColorAlpha(GOLD, 0.5f));

    int entries = std::min<int>(9, records.size());
    for (int i = 0; i < entries; ++i) {
        Color rowCol = (records[i].name == jugadorNombre) ? GREEN : WHITE;
        DrawText(TextFormat("#%d", i + 1), 210, 185 + i * 34, 14, rowCol);
        DrawText(records[i].name.c_str(), 280, 185 + i * 34, 14, rowCol);
        DrawText(TextFormat("%d PTS", records[i].score), 520, 185 + i * 34, 14, rowCol);
    }

    if (records.empty()) {
        DrawText("NO HAY REGISTROS DE PARTIDAS AUN", anchoPantalla / 2 - MeasureText("NO HAY REGISTROS DE PARTIDAS AUN", 14) / 2, 280, 14, GRAY);
    }

    DrawText("Presiona ESC o ENTER para volver al menu", anchoPantalla / 2 - MeasureText("Presiona ESC o ENTER para volver al menu", 12) / 2, 535, 12, GRAY);
}

void Juego2D::dibujarEstadisticasPersonales() const {
    DrawText("DIAGNOSTICO Y ESTADISTICAS", anchoPantalla / 2 - MeasureText("DIAGNOSTICO Y ESTADISTICAS", 28) / 2, 60, 28, SKYBLUE);
    DrawText("REGISTROS OPERATIVOS DEL PILOTO ACTUAL", anchoPantalla / 2 - MeasureText("REGISTROS OPERATIVOS DEL PILOTO ACTUAL", 11) / 2, 95, 11, GRAY);

    DibujarPanelVidrio(180, 130, 440, 360, SKYBLUE, 0.85f);

    Usuario u = sistemaUsuarios->obtenerUsuarioActual();

    DrawText(TextFormat("CALLSIGN DE PILOTO:   %s", u.nombre.c_str()), 220, 175, 16, WHITE);
    DrawText(TextFormat("PARTIDAS COMPLETADAS: %d VUELOS", u.juegosJugados), 220, 225, 16, WHITE);
    DrawText(TextFormat("MEJOR PUNTUACION:     %d PTS", u.mejorPuntaje), 220, 275, 16, GOLD);
    DrawText(TextFormat("PUNTAJE CUMULATIVO:   %d PTS", u.puntajeTotal), 220, 325, 16, SKYBLUE);

    double promedio = u.juegosJugados > 0 ? (double)u.puntajeTotal / u.juegosJugados : 0.0;
    DrawText(TextFormat("PROMEDIO POR VUELO:   %.1f PTS", promedio), 220, 375, 16, GREEN);

    DrawText("Presiona ESC o ENTER para volver al menu", anchoPantalla / 2 - MeasureText("Presiona ESC o ENTER para volver al menu", 12) / 2, 520, 12, GRAY);
}

void Juego2D::dibujarHistorialPartidas() const {
    DrawText("HISTORIAL DE MISIONES", anchoPantalla / 2 - MeasureText("HISTORIAL DE MISIONES", 28) / 2, 50, 28, SKYBLUE);
    DrawText("ULTIMAS SIMULACIONES EJECUTADAS POR ESTA CABINA", anchoPantalla / 2 - MeasureText("ULTIMAS SIMULACIONES EJECUTADAS POR ESTA CABINA", 11) / 2, 85, 11, GRAY);

    DibujarPanelVidrio(200, 110, 400, 410, SKYBLUE, 0.85f);

    std::vector<int> scores;
    std::ifstream file("historial_" + jugadorNombre + ".txt");
    if (file.is_open()) {
        int sc;
        while (file >> sc) {
            scores.push_back(sc);
        }
        file.close();
    }

    std::reverse(scores.begin(), scores.end()); // Más recientes primero

    DrawText("INDICE", 250, 140, 14, SKYBLUE);
    DrawText("PUNTUACION OBTENIDA", 400, 140, 14, SKYBLUE);
    DrawLine(230, 165, 570, 165, ColorAlpha(SKYBLUE, 0.5f));

    int size = std::min<int>(10, scores.size());
    for (int i = 0; i < size; ++i) {
        DrawText(TextFormat("MISSION #%d", size - i), 250, 185 + i * 32, 14, WHITE);
        DrawText(TextFormat("%d PTS", scores[i]), 400, 185 + i * 32, 14, GOLD);
    }

    if (scores.empty()) {
        DrawText("NO HAY VUELOS REGISTRADOS PARA ESTE PILOTO", anchoPantalla / 2 - MeasureText("NO HAY VUELOS REGISTRADOS PARA ESTE PILOTO", 12) / 2, 280, 12, GRAY);
    }

    DrawText("Presiona ESC o ENTER para volver al menu", anchoPantalla / 2 - MeasureText("Presiona ESC o ENTER para volver al menu", 12) / 2, 535, 12, GRAY);
}

void Juego2D::dibujarInformacionJuego() const {
    DrawText("MANUAL DE SOBREVIVENCIA DEL PILOTO", anchoPantalla / 2 - MeasureText("MANUAL DE SOBREVIVENCIA DEL PILOTO", 28) / 2, 50, 28, GOLD);
    DrawText("GUIAS OPERATIVAS Y LEYENDA DEL HUD", anchoPantalla / 2 - MeasureText("GUIAS OPERATIVAS Y LEYENDA DEL HUD", 11) / 2, 85, 11, GRAY);

    DibujarPanelVidrio(80, 115, 640, 400, GOLD, 0.85f);

    int textY = 140;
    DrawText("CONTROLES BASICOS Y MOVIMIENTO:", 110, textY, 13, GOLD);
    DrawText("- Flechas Direccionales o [W][A][S][D] : Mueve tu nave espacial por la pantalla.", 110, textY + 22, 12, WHITE);
    DrawText("- Barra Espaciadora [SPACE]             : Dispara tu arma frontal.", 110, textY + 40, 12, WHITE);
    DrawText("- Tecla [F]                            : Cicla tu arma en tiempo real durante el vuelo.", 110, textY + 58, 12, WHITE);
    DrawText("- Tecla [S]                            : Activa escudo de energia temporal (si tienes inventario).", 110, textY + 76, 12, WHITE);
    DrawText("- Tecla [T]                            : Activa propulsor turbo temporal (si tienes inventario).", 110, textY + 94, 12, WHITE);

    DrawText("POTENCIADORES Y AMENAZAS EN SECTOR (LEYENDA):", 110, textY + 124, 13, GOLD);
    
    // Iconos ficticios para la leyenda
    DrawCircle(120, textY + 158, 8, Color{105, 55, 30, 255});
    DrawText("METEORO [M] : Objeto inerte pesado. Evita el impacto o perderas una vida.", 140, textY + 152, 12, WHITE);

    DrawTriangle(Vector2{112.0f, (float)(textY + 192)}, Vector2{120.0f, (float)(textY + 178)}, Vector2{128.0f, (float)(textY + 192)}, GREEN);
    DrawText("DRON ENEMIGO [D] : Nave hostil. Dispara proyectiles y te embiste.", 140, textY + 182, 12, WHITE);

    DrawCircle(120, textY + 214, 6, GOLD);
    DrawText("COMETA DE COMBUSIBLE [C] : Recogelo para restaurar energia/gasolina y ganar 30 pts.", 140, textY + 208, 12, WHITE);

    DrawPoly(Vector2{120.0f, (float)(textY + 242)}, 6, 8.0f, 0.0f, SKYBLUE);
    DrawText("ITEM ESCUDO [S] : Se almacena en cabina. Presiona [S] para volverte inmune temporalmente.", 140, textY + 236, 12, WHITE);

    DrawPoly(Vector2{120.0f, (float)(textY + 270)}, 4, 8.0f, 0.0f, ORANGE);
    DrawText("ITEM TURBO [T] : Se almacena. Presiona [T] para acelerar y moverte superrapido.", 140, textY + 264, 12, WHITE);

    DrawText("CRONOGRAMA DE COMBATE CONTRA JEFES:", 110, textY + 295, 13, GOLD);
    DrawText("- Un jefe gigante invadira el sector. No se retirara hasta que reduzcas su salud a cero.", 110, textY + 315, 12, WHITE);
    DrawText("- Tras destruirlo, tendras 15s de cooldown para recolectar combustible antes del proximo.", 110, textY + 333, 12, WHITE);

    DrawText("Presiona ESC o ENTER para volver al menu", anchoPantalla / 2 - MeasureText("Presiona ESC o ENTER para volver al menu", 12) / 2, 530, 12, GRAY);
}

void Juego2D::dibujarCreditos() const {
    DrawText("BITACORA DE CREDITOS", anchoPantalla / 2 - MeasureText("BITACORA DE CREDITOS", 28) / 2, 80, 28, SKYBLUE);
    DrawText("AERO EVADER PRO - INGENIERIA DE DESARROLLO", anchoPantalla / 2 - MeasureText("AERO EVADER PRO - INGENIERIA DE DESARROLLO", 11) / 2, 115, 11, GRAY);

    DibujarPanelVidrio(120, 150, 560, 310, SKYBLUE, 0.85f);

    int startY = 190;
    DrawText("DESARROLLO OPERATIVO:", 160, startY, 13, SKYBLUE);
    DrawText("Ingeniero de Sistemas de Vuelo - Inteligencia Artificial Antigravity", 160, startY + 22, 12, WHITE);
    DrawText("Proyecto Academico Integrador - Programacion I", 160, startY + 40, 12, WHITE);

    DrawText("TECNOLOGIAS DE SOPORTE:", 160, startY + 80, 13, SKYBLUE);
    DrawText("Compilador: Microsoft Visual C++ (MSVC) / C++17 Standard", 160, startY + 102, 12, WHITE);
    DrawText("Biblioteca de Renderizado: Raylib 6.0 (Vector Graphics Architecture)", 160, startY + 120, 12, WHITE);
    DrawText("Sub-Engine: Windowed Subsystem (Direct EXE Native Access)", 160, startY + 138, 12, WHITE);

    DrawText("AGRADECIMIENTOS:", 160, startY + 180, 13, SKYBLUE);
    DrawText("Gracias por pilotar en Aero Evader Pro. El sector galactico esta a salvo contigo.", 160, startY + 202, 12, GOLD);

    DrawText("Presiona ESC o ENTER para volver al menu", anchoPantalla / 2 - MeasureText("Presiona ESC o ENTER para volver al menu", 12) / 2, 500, 12, GRAY);
}

void Juego2D::dibujarHUD() const {
    Color hudCol = SKYBLUE;
    
    // Borde superior
    DrawLineEx({ 10, 40 }, { (float)anchoPantalla - 10, 40 }, 2.0f, hudCol);

    DrawText(TextFormat("VIDAS: %d", vidas), 20, 15, 16, RED);
    DrawText(TextFormat("PUNTAJE: %d", puntaje), anchoPantalla - 160, 15, 16, GOLD);
    DrawText(TextFormat("NIVEL: %d", nivel), anchoPantalla / 2 - 40, 15, 16, GREEN);

    // Combustible
    DrawText("COMB:", 20, altoPantalla - 30, 12, WHITE);
    DrawRectangleLines(70, altoPantalla - 30, 100, 12, GRAY);
    
    Color fuelColor = GREEN;
    if (combustible < 20) fuelColor = RED;
    else if (combustible < 50) fuelColor = YELLOW;
    DrawRectangle(72, altoPantalla - 28, (int)(96.0f * (std::min(100, combustible) / 100.0f)), 8, fuelColor);

    // Indicador numérico del combustible
    DrawText(TextFormat("%d%%", combustible), 180, altoPantalla - 30, 11, fuelColor);

    // Arma activa
    std::string weaponName = (seleccionArma == 0) ? "LASER" : (seleccionArma == 1) ? "TRIPLE SHOT" : (seleccionArma == 2) ? "ENERGY WAVE" : "GUIDED MISSILE";
    DrawText(TextFormat("ARMA [F]: %s", weaponName.c_str()), 220, altoPantalla - 30, 12, GOLD);

    // Inventario
    DrawText(TextFormat("ESCUDOS [S]: %d", cantidadEscudos), 460, altoPantalla - 30, 12, BLUE);
    DrawText(TextFormat("TURBO [T]: %d", cantidadTurbos), 620, altoPantalla - 30, 12, ORANGE);

    // Dibujar banner del jefe si está activo
    if (jefeActivoIndex != -1) {
        std::string bossNames[] = {
            "Alpha Vanguard", "Beta Swarm Lord", "Gamma Ray Cruiser",
            "Delta Mech-Leviathan", "Epsilon Singularity", "Zeta Cosmic Serpent",
            "Eta Nebular Colossus", "Theta Chrono Warden", "Iota Void Reaper",
            "Omega Evader Destroyer"
        };
        std::string bName = bossNames[jefeActivoIndex];
        DrawText(bName.c_str(), anchoPantalla / 2 - MeasureText(bName.c_str(), 14) / 2, 55, 14, RED);
    } else {
        // Si no hay jefe y el cooldown está corriendo, mostrar el tiempo restante en pantalla
        if (bossSpawnCooldown > 0.0f) {
            std::string cooldownStr = TextFormat("ALERTA DE JEFE EN: %.1fs", bossSpawnCooldown);
            DrawText(cooldownStr.c_str(), anchoPantalla / 2 - MeasureText(cooldownStr.c_str(), 12) / 2, 55, 12, GRAY);
        }
    }

    // Banner de notificación flotante temporal
    if (!mensajeNotificacion.empty() && timerNotificacion > 0.0f) {
        DrawRectangleRec({ (float)anchoPantalla/2 - 200, 85, 400, 30 }, ColorAlpha(BLACK, 0.75f));
        DrawRectangleLines((int)anchoPantalla/2 - 200, 85, 400, 30, GOLD);
        DrawText(mensajeNotificacion.c_str(), anchoPantalla / 2 - MeasureText(mensajeNotificacion.c_str(), 12) / 2, 94, 12, WHITE);
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
    
    for (int i = 0; i < cantidad; ++i) {
        Particula2D p;
        p.posicion = { pos.x, pos.y + 16 };
        p.velocidad = { distrVelX(generador), distrVelY(generador) };
        p.color = colorChispa;
        p.radio = distrRadio(generador);
        p.opacidad = 1.0f;
        p.vidaMax = distrVida(generador);
        p.vidaRestante = p.vidaMax;
        particulas.push_back(p);
    }
}
