#pragma once
#include <string>
#include <vector>

enum class TipoObstaculo {
    Meteoro,
    Drone,
    Cometa,
    PowerUpEscudo,
    PowerUpTurbo
};

struct Obstaculo {
    int x;
    int y;
    int velocidad;
    TipoObstaculo tipo;
};

struct Pixel {
    std::string caracter = " ";
    std::string colorAnsi = "";
};

struct Estrella {
    int x;
    int y;
    int capa; // Capa de profundidad de 1 a 3
};

struct Particula {
    double x;
    double y;
    double vx;
    double vy;
    int vida;
    std::string caracter;
    std::string colorAnsi;
};

class Juego {
public:
    Juego(int ancho, int alto);
    void iniciar(const std::string &nombreJugador);
    int obtenerPuntaje() const;

private:
    int anchoPantalla;
    int altoPantalla;
    int puntaje;
    int vidas;
    int nivel;
    int jugadorX;
    int jugadorY;
    bool juegoActivo;
    std::vector<Obstaculo> obstaculos;
    std::vector<Estrella> estrellas;
    std::vector<Particula> particulas;
    std::string jugadorNombre;
    int velocidadGeneral;
    int combustible;
    int combo;
    int tiempoMilisegundos;
    bool escudoActivo;
    int duracionEscudo;
    bool turboActivo;
    int duracionTurbo;
    int cantidadEscudos;
    int cantidadTurbos;
    std::string mensajeNotificacion;
    int timerNotificacion;
    void mostrarPantallaInicio() const;
    void mostrarPantallaCarga() const;
    void inicializarEstrellas();
    void actualizarEstrellas();
    void dibujarEstrellas(std::vector<std::vector<Pixel>> &pantalla) const;
    void actualizarEscena();
    void actualizarTiempo(int deltaMs);
    void dibujarEscena() const;
    void dibujarJugador(std::vector<std::vector<Pixel>> &pantalla) const;
    void dibujarObstaculo(std::vector<std::vector<Pixel>> &pantalla, const Obstaculo &obs) const;
    void procesarEntrada();
    void generarObstaculos();
    void limpiarPantalla() const;
    void mostrarInformeFinal() const;
    void mostrarBarraEstado(std::string &salida) const;
    void actualizarParticulas();
    void dibujarParticulas(std::vector<std::vector<Pixel>> &pantalla) const;
    void crearExplosion(int x, int y, const std::string &colorBase, int cantidad);
    void actualizarNivel();
    char dibujarObjeto(const Obstaculo &obs) const;
    int obtenerAnchoTerminal() const;
    int obtenerAltoTerminal() const;
};
