# RFC 001 - Protocolo de Juego Ciber-Defensa (Versión Final)

## 1. Visión General
El protocolo opera en la capa de aplicación sobre TCP. Sigue un modelo Cliente-Servidor donde el servidor mantiene el estado del plano, los recursos críticos y la vida (HP) de los activos.

## 2. Primitivas del Servicio (Comandos)
- **AUTH <nombre>**: Registra al jugador. El servidor asigna un **Rol** (ATACANTE o DEFENSOR) y entrega información inicial asimétrica.
- **MOVE <N|S|E|W> <cant>**: Desplaza al jugador un número determinado de unidades (Máximo 20 por turno) en un mapa de 100x100.
- **ATTACK**: (Solo Atacante) Reduce 10 HP de una base si el jugador está a menos de 10 unidades de distancia.
- **SHIELD**: (Solo Defensor) Recupera 5 HP (hasta 100) de una base si el jugador está a menos de 10 unidades de distancia.
- **QUIT**: Finaliza la conexión de forma segura.

## 3. Modelo de Información (Reglas de Negocio)
1. **Asimetría de Información**: Al autenticarse, los **Defensores** ven las coordenadas exactas de las bases. Los **Atacantes** deben encontrarlas usando el radar de proximidad.
2. **Radar de Proximidad**: A menos de 15 unidades de un recurso, el servidor envía notificaciones automáticas de presencia.
3. **Mecánica de Daño**: Las bases (`Base_Datos` y `Web_Server`) inician con 100 HP. Si llegan a 0, se consideran comprometidas.
