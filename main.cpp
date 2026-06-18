#include "SistemaUsuarios.h"
#include "juego2d.h"

int main() {
    // Inicializar el sistema de usuarios y records localmente
    SistemaUsuarios sistema("usuarios.txt", "records.txt");
    
    // Instanciar el juego 2D con un tamaño de ventana de 800x600, pasándole el gestor de usuarios
    Juego2D juego2d(800, 600, &sistema);
    
    // Iniciar el juego (este se encargará de gestionar el login/registro en pantalla y los menús)
    juego2d.iniciar();
    
    return 0;
}
