#include "SistemaUsuarios.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

SistemaUsuarios::SistemaUsuarios(const std::string &rutaUsuarios, const std::string &rutaRecords)
    : rutaUsuarios(rutaUsuarios), rutaRecords(rutaRecords), sesionActiva(false) {
    cargarUsuarios();
}

void SistemaUsuarios::cargarUsuarios() {
    usuarios.clear();
    std::ifstream archivo(rutaUsuarios);
    if (!archivo.is_open()) {
        return;
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;
        std::istringstream iss(linea);
        Usuario usuario;
        if (iss >> usuario.nombre >> usuario.contrasena >> usuario.mejorPuntaje >> usuario.puntajeTotal >> usuario.juegosJugados) {
            usuarios.push_back(usuario);
        } else {
            iss.clear();
            iss.str(linea);
            if (iss >> usuario.nombre >> usuario.contrasena >> usuario.mejorPuntaje) {
                usuario.puntajeTotal = usuario.mejorPuntaje;
                usuario.juegosJugados = 0;
                usuarios.push_back(usuario);
            }
        }
    }
}

void SistemaUsuarios::guardarUsuarios() const {
    std::ofstream archivo(rutaUsuarios, std::ios::trunc);
    for (const auto &usuario : usuarios) {
        archivo << usuario.nombre << " " << usuario.contrasena << " " << usuario.mejorPuntaje
                << " " << usuario.puntajeTotal << " " << usuario.juegosJugados << "\n";
    }
}

bool SistemaUsuarios::registrarUsuario(const std::string &nombre, const std::string &contrasena) {
    if (nombre.empty() || contrasena.empty()) {
        return false;
    }

    // Evitar espacios o tabulaciones que corrompan el guardado de datos plano por espacios
    if (nombre.find_first_of(" \t\r\n") != std::string::npos || contrasena.find_first_of(" \t\r\n") != std::string::npos) {
        return false;
    }

    auto it = std::find_if(usuarios.begin(), usuarios.end(), [&](const Usuario &u) {
        return u.nombre == nombre;
    });

    if (it != usuarios.end()) {
        return false;
    }

    Usuario nuevo{nombre, contrasena, 0, 0, 0};
    usuarios.push_back(nuevo);
    guardarUsuarios();
    return true;
}

bool SistemaUsuarios::iniciarSesion(const std::string &nombre, const std::string &contrasena) {
    auto it = std::find_if(usuarios.begin(), usuarios.end(), [&](const Usuario &u) {
        return u.nombre == nombre && u.contrasena == contrasena;
    });

    if (it == usuarios.end()) {
        return false;
    }

    usuarioActual = *it;
    sesionActiva = true;
    return true;
}

Usuario SistemaUsuarios::obtenerUsuarioActual() const {
    return usuarioActual;
}

bool SistemaUsuarios::haySesionActiva() const {
    return sesionActiva;
}

std::string SistemaUsuarios::obtenerNombreActual() const {
    return sesionActiva ? usuarioActual.nombre : std::string();
}

void SistemaUsuarios::registrarRecord(const std::string &nombre, int puntaje) {
    if (nombre.empty()) return;

    std::ofstream archivo(rutaRecords, std::ios::app);
    if (!archivo.is_open()) return;
    archivo << nombre << " " << puntaje << "\n";

    std::ofstream historial("historial_" + nombre + ".txt", std::ios::app);
    if (historial.is_open()) {
        historial << puntaje << "\n";
    }
}

void SistemaUsuarios::mostrarRecords() const {
    std::ifstream archivo(rutaRecords);
    if (!archivo.is_open()) {
        std::cout << "No hay records guardados aun.\n";
        return;
    }

    std::vector<std::pair<int, std::string>> records;
    std::string nombre;
    int puntaje;
    while (archivo >> nombre >> puntaje) {
        records.emplace_back(puntaje, nombre);
    }

    std::sort(records.begin(), records.end(), [](const auto &a, const auto &b) {
        return a.first > b.first;
    });

    std::cout << "\n===== TABLA DE RECORDS GLOBAL =====\n";
    std::cout << std::left << std::setw(8) << "Puesto" << std::setw(16) << "Jugador" << std::setw(8) << "Puntaje" << "\n";
    std::cout << "------------------------------------\n";
    int limite = std::min<int>(10, records.size());
    for (int i = 0; i < limite; ++i) {
        std::cout << std::left << std::setw(8) << (i + 1)
                  << std::setw(16) << records[i].second
                  << std::setw(8) << records[i].first << "\n";
    }
    if (records.empty()) {
        std::cout << "Aun no hay puntuaciones registradas.\n";
    }
    std::cout << "===============================\n";
}

void SistemaUsuarios::mostrarEstadisticasUsuario() const {
    if (!sesionActiva) {
        std::cout << "Inicia sesion para ver tus estadisticas.\n";
        return;
    }

    std::cout << "\n===== ESTADISTICAS DE " << usuarioActual.nombre << " =====\n";
    std::cout << std::left << std::setw(22) << "Mejor puntaje:" << usuarioActual.mejorPuntaje << "\n";
    std::cout << std::left << std::setw(22) << "Juegos jugados:" << usuarioActual.juegosJugados << "\n";
    std::cout << std::left << std::setw(22) << "Puntaje total:" << usuarioActual.puntajeTotal << "\n";
    double promedio = usuarioActual.juegosJugados > 0 ? static_cast<double>(usuarioActual.puntajeTotal) / usuarioActual.juegosJugados : 0.0;
    std::cout << std::left << std::setw(22) << "Promedio por juego:" << std::fixed << std::setprecision(1) << promedio << "\n";
    std::cout << "===============================\n";
}

void SistemaUsuarios::mostrarHistorialUsuario() const {
    if (!sesionActiva) {
        std::cout << "Inicia sesion para ver tu historial.\n";
        return;
    }

    std::ifstream historial("historial_" + usuarioActual.nombre + ".txt");
    if (!historial.is_open()) {
        std::cout << "Aun no hay historial de partidas.\n";
        return;
    }

    std::cout << "\n===== HISTORIAL DE PARTIDAS =====\n";
    std::cout << std::left << std::setw(8) << "#" << std::setw(12) << "Puntaje" << "\n";
    std::cout << "------------------------\n";
    int puntaje;
    int index = 1;
    while (historial >> puntaje) {
        std::cout << std::left << std::setw(8) << index << std::setw(12) << puntaje << "\n";
        index++;
    }
    if (index == 1) {
        std::cout << "No se han jugado partidas todavia.\n";
    }
    std::cout << "========================\n";
}

void SistemaUsuarios::actualizarPuntajeActual(int puntaje) {
    if (!sesionActiva) return;
    for (auto &usuario : usuarios) {
        if (usuario.nombre == usuarioActual.nombre && usuario.contrasena == usuarioActual.contrasena) {
            usuario.juegosJugados += 1;
            usuario.puntajeTotal += puntaje;
            if (puntaje > usuario.mejorPuntaje) {
                usuario.mejorPuntaje = puntaje;
            }
            usuarioActual = usuario;
            guardarUsuarios();
            break;
        }
    }
}
