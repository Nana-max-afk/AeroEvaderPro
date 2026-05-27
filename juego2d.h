#pragma once
#include <string>
#include <vector>
#include "raylib.h"

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

class Juego2D {
public:
    Juego2D(int ancho, int alto);
    void iniciar(const std::string &nombreJugador);
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
    
    // Métodos auxiliares de inicialización y actualización
    void inicializarEstrellas();
    void actualizarEstrellas(float dt);
    void dibujarEstrellas() const;
    
    void actualizarEscena(float dt);
    void procesarEntrada();
    void generarObstaculos();
    void actualizarNivel();
    
    // Métodos de dibujo Raylib 2D
    void dibujarFondo() const;
    void dibujarJugador() const;
    void dibujarObstaculo(const Obstaculo2D &obs) const;
    void dibujarParticulas() const;
    void dibujarHUD() const;
    
    // Efectos de partículas
    void crearExplosion(Vector2 pos, Color colorBase, int cantidad);
    void crearChispasMotor(Vector2 pos, bool conTurbo);
};
