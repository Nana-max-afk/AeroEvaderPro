#pragma once
#include <string>
#include <vector>

struct Usuario {
    std::string nombre;
    std::string contrasena;
    int mejorPuntaje;
    int puntajeTotal;
    int juegosJugados;
};

class SistemaUsuarios {
public:
    SistemaUsuarios(const std::string &rutaUsuarios, const std::string &rutaRecords);
    bool registrarUsuario(const std::string &nombre, const std::string &contrasena);
    bool iniciarSesion(const std::string &nombre, const std::string &contrasena);
    void cargarUsuarios();
    void guardarUsuarios() const;
    void registrarRecord(const std::string &nombre, int puntaje);
    void mostrarRecords() const;
    void mostrarEstadisticasUsuario() const;
    void mostrarHistorialUsuario() const;
    Usuario obtenerUsuarioActual() const;
    void actualizarPuntajeActual(int puntaje);
    bool haySesionActiva() const;
    std::string obtenerNombreActual() const;

private:
    std::string rutaUsuarios;
    std::string rutaRecords;
    std::vector<Usuario> usuarios;
    Usuario usuarioActual;
    bool sesionActiva;
};
