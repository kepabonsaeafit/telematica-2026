#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_CLIENTS 10
#define MAP_SIZE 100

typedef struct { 
    int x, y, hp; 
    char nombre[20]; 
} Recurso;

Recurso servidores[2] = {{25, 30, 100, "Base_Datos"}, {75, 80, 100, "Web_Server"}};

int clientes_activos[MAX_CLIENTS];
int contador_jugadores = 0;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *archivo_log;

void registrar_log(const char *ip, int puerto, const char *evento) {
    pthread_mutex_lock(&log_mutex);
    time_t ahora = time(NULL); char *tiempo = ctime(&ahora); tiempo[strlen(tiempo) - 1] = '\0';
    fprintf(archivo_log, "[%s] [%s:%d] %s\n", tiempo, ip, puerto, evento);
    fflush(archivo_log);
    printf("[%s] [%s:%d] %s\n", tiempo, ip, puerto, evento);
    pthread_mutex_unlock(&log_mutex);
}

void enviar_a_todos(const char *mensaje) {
    for (int i = 0; i < MAX_CLIENTS; i++) if (clientes_activos[i] != 0) send(clientes_activos[i], mensaje, strlen(mensaje), 0);
}

void *manejar_cliente(void *arg) {
    int mi_socket = *(int *)arg; free(arg);
    struct sockaddr_in info; socklen_t tam = sizeof(info);
    getpeername(mi_socket, (struct sockaddr *)&info, &tam);
    char ip[INET_ADDRSTRLEN]; inet_ntop(AF_INET, &info.sin_addr, ip, INET_ADDRSTRLEN);
    int puerto = ntohs(info.sin_port);

    char buffer[1024]; char nombre[50] = "Anonimo";
    int mi_x = rand() % MAP_SIZE; int mi_y = rand() % MAP_SIZE;
    
    int id_secuencial = contador_jugadores;
    int es_atacante = (id_secuencial % 2 != 0); 
    char *mi_rol = es_atacante ? "ATACANTE" : "DEFENSOR";

    while (1) {
        memset(buffer, 0, 1024);
        if (recv(mi_socket, buffer, 1024, 0) <= 0) break;

        if (strncmp(buffer, "AUTH ", 5) == 0) {
            sscanf(buffer + 5, "%s", nombre);
            char res[1024];
            sprintf(res, "\n[SISTEMA] Bienvenido %s | ROL: %s | POS: (%d,%d)\n", nombre, mi_rol, mi_x, mi_y);
            if(!es_atacante) strcat(res, ">> MISION: Protege las bases en (25,30) y (75,80).\n");
            strcat(res, "Comandos: MOVE <D> <K>, ATTACK, SHIELD, QUIT\n--------------------------------------------\n");
            send(mi_socket, res, strlen(res), 0);
            registrar_log(ip, puerto, "Usuario autenticado");
        }
        else if (strncmp(buffer, "MOVE ", 5) == 0) {
            char dir; int cant = 1; sscanf(buffer + 5, "%c %d", &dir, &cant);
            if (cant > 20) cant = 20;
            if (dir == 'N') mi_y += cant; else if (dir == 'S') mi_y -= cant;
            else if (dir == 'E') mi_x += cant; else if (dir == 'W') mi_x -= cant;
            if(mi_x<0) mi_x=0; if(mi_x>100) mi_x=100; if(mi_y<0) mi_y=0; if(mi_y>100) mi_y=100;
            char pos[100]; sprintf(pos, "[MOV] Posicion: (%d,%d)\n", mi_x, mi_y);
            send(mi_socket, pos, strlen(pos), 0);
            for(int i=0; i<2; i++) {
                if(abs(mi_x - servidores[i].x) < 15 && abs(mi_y - servidores[i].y) < 15) {
                    char av[100]; sprintf(av, "[RADAR] %s detectado cerca HP: %d\n", servidores[i].nombre, servidores[i].hp);
                    send(mi_socket, av, strlen(av), 0);
                }
            }
        }
        else if (strncmp(buffer, "ATTACK", 6) == 0) {
            if (!es_atacante) {
                send(mi_socket, "[!] ERROR: Eres DEFENSOR. No puedes auto-atacarte.\n", 52, 0);
            } else {
                int impacto = 0;
                for(int i=0; i<2; i++) {
                    if(abs(mi_x - servidores[i].x) < 10 && abs(mi_y - servidores[i].y) < 10) {
                        servidores[i].hp -= 10; impacto = 1;
                        char al[150]; sprintf(al, "\n[ALERTA] %s ATACO %s! HP: %d\n", nombre, servidores[i].nombre, servidores[i].hp);
                        enviar_a_todos(al);
                    }
                }
                if(!impacto) send(mi_socket, "[!] ERROR: Fuera de rango para atacar.\n", 40, 0);
            }
        }
        else if (strncmp(buffer, "SHIELD", 6) == 0) {
            if (es_atacante) {
                send(mi_socket, "[!] ERROR: Eres ATACANTE. No tienes acceso a los escudos.\n", 59, 0);
            } else {
                int exito = 0;
                for(int i=0; i<2; i++) {
                    if(abs(mi_x - servidores[i].x) < 10 && abs(mi_y - servidores[i].y) < 10) {
                        servidores[i].hp += 5; if(servidores[i].hp > 100) servidores[i].hp = 100;
                        exito = 1;
                        char sh[150]; sprintf(sh, "\n[SEGURIDAD] %s REPARO %s! HP: %d\n", nombre, servidores[i].nombre, servidores[i].hp);
                        enviar_a_todos(sh);
                    }
                }
                if(!exito) send(mi_socket, "[!] ERROR: Debes estar cerca de la base para repararla.\n", 57, 0);
            }
        }
    }
    close(mi_socket);
    for (int i=0; i<MAX_CLIENTS; i++) if (clientes_activos[i] == mi_socket) clientes_activos[i] = 0;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) return 1;
    srand(time(NULL));
    archivo_log = fopen(argv[2], "a");
    if (!archivo_log) return 1;

    int sock_servidor = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock_servidor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sock_servidor, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) return 1;
    listen(sock_servidor, 5);

    printf("[*] Servidor Iniciado en puerto %s\n", argv[1]);

    while (1) {
        int *ns = malloc(sizeof(int));
        *ns = accept(sock_servidor, NULL, NULL);
        if (*ns < 0) { free(ns); continue; }
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientes_activos[i] == 0) {
                clientes_activos[i] = *ns;
                contador_jugadores++;
                pthread_t t_id;
                pthread_create(&t_id, NULL, manejar_cliente, ns);
                pthread_detach(t_id);
                break;
            }
        }
    }
    return 0;
}
