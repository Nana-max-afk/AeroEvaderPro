#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "SistemaUsuarios.h"

enum class TipoObstaculo2D {
    Meteoro,
    Drone,
    Cometa,
    PowerUpEscudo,
    PowerUpTurbo
};

struct Obstaculo2D {
    Vector2 posicion;
    float velocidad;
    TipoObstaculo2D tipo;
    float rotacion;
    float velocidadRotacion;
    float escala;
};

struct Estrella2D {
    Vector2 posicion;
    int capa; // Capa de profundidad de 1 a 3
    float velocidad;
};

struct Particula2D {
    Vector2 posicion;
    Vector2 velocidad;
    Color color;
    float radio;
    float opacidad; // Control de desvanecimiento
    int vidaMax;
    int vidaRestante;
};

enum class EstadoJuego2D {
    LoginRegister,          // Pantalla de autenticación
    MenuInicio,             // Menú principal interactivo
    Personalizacion,        // Personalización de la nave
    RecordsGlobales,        // Tabla de records
    EstadisticasPersonales, // Estadísticas del piloto
    HistorialPartidas,      // Historial personal
    InformacionJuego,       // Instrucciones
    Creditos,               // Créditos
    Jugando,                // Partida activa
    GameOver                // Fin de partida
};

struct Proyectil2D {
    Vector2 posicion;
    Vector2 velocidad;
    int damage;
    bool activo;
};

class Juego2D {
public:
    Juego2D(int ancho, int alto, SistemaUsuarios* sistema);
    void iniciar();
    int obtenerPuntaje() const;

private:
    int anchoPantalla;
    int altoPantalla;
    int puntaje;
    int vidas;
    int nivel;
    Vector2 jugadorPos;
    float jugadorVelocidad;
    bool juegoActivo;

    EstadoJuego2D estadoActual;
    int opcionMenu;
    int opcionCustomizer;

    // Ship design and weapons customizer
    int seleccionNave;
    Color colorNave;
    int seleccionArma;

    // Projectiles
    std::vector<Proyectil2D> proyectiles;
    float weaponCooldownTimer;

    // Bosses
    int jefeActivoIndex; // -1 if none, 0-9
    int jefeVida;
    int jefeVidaMax;
    int jefeFase;
    Vector2 jefePos;
    float jefeMovTimer;
    
    // Boss spawning cooldown rules
    int nextBossIndex;          // Siguiente jefe a invocar (0-9)
    float bossSpawnCooldown;    // Tiempo restante para el spawn en segundos
    float screenShakeIntensity; // Intensidad del efecto de sacudida de pantalla

    std::vector<Obstaculo2D> obstaculos;
    std::vector<Estrella2D> estrellas;
    std::vector<Particula2D> particulas;
    
    std::string jugadorNombre;
    float velocidadJuegoBase;
    int combustible;
    int combo;
    float tiempoTranscurrido; // En segundos
    
    bool escudoActivo;
    float duracionEscudo; // En segundos
    bool turboActivo;
    float duracionTurbo; // En segundos
    
    int cantidadEscudos;
    int cantidadTurbos;
    
    std::string mensajeNotificacion;
    float timerNotificacion; // En segundos

    // Textures support
    Texture2D shipTextures[3];
    Texture2D bossTextures[2];
    bool texturesLoaded;
    
    // User Management Integration
    SistemaUsuarios* sistemaUsuarios;
    
    // Login and Registration UI states
    std::string inputUser;
    std::string inputPass;
    int loginActiveField; // 0 = Username, 1 = Password, 2 = Login Button, 3 = Register Button, 4 = Exit Button
    std::string authMessage;
    float authMessageTimer;
    
    // Métodos auxiliares de inicialización y actualización
    void inicializarEstrellas();
    void actualizarEstrellas(float dt);
    void dibujarEstrellas() const;
    
    void actualizarEscena(float dt);
    void procesarEntrada();
    void procesarEntradaLoginRegister();
    void procesarEntradaMenu();
    void procesarEntradaPersonalizacion();
    void generarObstaculos();
    void actualizarNivel();
    void actualizarJefe(float dt);

    void cargarConfiguracionLocal();
    void guardarConfiguracionLocal();
    
    // Métodos de dibujo Raylib 2D
    void dibujarFondo() const;
    void dibujarJugador() const;
    void dibujarObstaculo(const Obstaculo2D &obs) const;
    void dibujarParticulas() const;
    void dibujarHUD() const;
    void dibujarLoginRegister() const;
    void dibujarMenuInicio() const;
    void dibujarPersonalizacion() const;
    void dibujarRecordsGlobales() const;
    void dibujarEstadisticasPersonales() const;
    void dibujarHistorialPartidas() const;
    void dibujarInformacionJuego() const;
    void dibujarCreditos() const;
    void dibujarJefe() const;
    
    // Efectos de partículas
    void crearExplosion(Vector2 pos, Color colorBase, int cantidad);
    void crearChispasMotor(Vector2 pos, bool conTurbo);
};
