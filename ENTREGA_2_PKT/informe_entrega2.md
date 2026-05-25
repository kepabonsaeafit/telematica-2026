# Informe Entrega 2 – Packet Tracer

## Interconexión de sistemas autónomos y acceso a servicios

### Telemática – Universidad EAFIT | Semestre 2026-1

**Integrantes:** (Kevin Pabón, Santiago Mafla, Geronimo Montes, Pablo Cardona)  
**Fecha:** 24 de mayo de 2026

---

## 1. Implementación de la topología (10 pts)

### 1.1 Descripción general

El grupo está conformado por **k = 4 estudiantes**, por lo tanto se implementaron **n = k + 1 = 5 sistemas autónomos**: AS1, AS2, AS3 y AS4 como redes de clientes, y AS99 como proveedor de servicios.

Cada AS cliente es structuralmente idéntico e incluye:

- 1 switch multicapa (Cisco 3560-24PS) con VLANs 10, 20 y 100
- 4 routers internos: R1, R2, R3, R4 (modelo Cisco 1941)
- 1 PC por VLAN (PC1 en VLAN 10, PC2 en VLAN 20)
- DHCP configurado directamente en el switch multicapa

AS99 incluye:

- 5 routers (R5 a R9, modelo Cisco 1941)
- 1 switch de capa 2 (2960-24TT) como backbone interno
- Servidor DNS (IP: 200.0.0.10)
- Servidor WEB (IP: 200.0.0.20)

### 1.2 Direccionamiento IP

#### VLANs internas (idéntico en todos los AS clientes)

| VLAN               | Red             | Gateway (SVI) |
| ------------------ | --------------- | ------------- |
| VLAN 10            | 10.0.0.0/24     | 10.0.0.1      |
| VLAN 20            | 172.16.0.0/24   | 172.16.0.1    |
| VLAN 100 (gestión) | 192.168.x0.0/24 | 192.168.x0.1  |

#### Links internos entre routers por AS cliente (/30)

| Enlace                          | AS1              | AS2              | AS3              | AS4              |
| ------------------------------- | ---------------- | ---------------- | ---------------- | ---------------- |
| SW ↔ R1 (GE0/0–Fa0/4)           | 192.168.10.0/30  | 192.168.20.0/30  | 192.168.30.0/30  | 192.168.40.0/30  |
| SW ↔ R3 (GE0/0–Fa0/5)           | 192.168.10.4/30  | 192.168.20.4/30  | 192.168.30.4/30  | 192.168.40.4/30  |
| R1 ↔ R2 (GE0/1–GE0/0)           | 192.168.10.8/30  | 192.168.20.8/30  | 192.168.30.8/30  | 192.168.40.8/30  |
| R3 ↔ R4 (GE0/1–GE0/0)           | 192.168.10.12/30 | 192.168.20.12/30 | 192.168.30.12/30 | 192.168.40.12/30 |
| R2 ↔ R4 (GE0/1–GE0/0 adicional) | 192.168.10.16/30 | 192.168.20.16/30 | 192.168.30.16/30 | 192.168.40.16/30 |

#### Links WAN (R4 de cada AS ↔ AS99) — /30

| AS       | Subred WAN    | IP R4 (cliente) | IP AS99    |
| -------- | ------------- | --------------- | ---------- |
| AS1 ↔ R5 | 10.10.10.0/30 | 10.10.10.1      | 10.10.10.2 |
| AS2 ↔ R7 | 10.20.20.0/30 | 10.20.20.1      | 10.20.20.2 |
| AS3 ↔ R8 | 10.30.30.0/30 | 10.30.30.1      | 10.30.30.2 |
| AS4 ↔ R9 | 10.40.40.0/30 | 10.40.40.1      | 10.40.40.2 |

#### Red interna AS99

| Dispositivo | IP            | Red          |
| ----------- | ------------- | ------------ |
| R5 (GE0/0)  | 200.0.0.1/24  | 200.0.0.0/24 |
| R6 (GE0/0)  | 200.0.0.2/24  | 200.0.0.0/24 |
| R7 (GE0/0)  | 200.0.0.3/24  | 200.0.0.0/24 |
| R8 (GE0/0)  | 200.0.0.4/24  | 200.0.0.0/24 |
| R9 (GE0/0)  | 200.0.0.5/24  | 200.0.0.0/24 |
| DNS-AS99    | 200.0.0.10/24 | 200.0.0.0/24 |
| WEB-AS99    | 200.0.0.20/24 | 200.0.0.0/24 |

Todos los dispositivos de AS99 se conectan a un switch de capa 2 (SW-AS99) que actúa como backbone, eliminando la necesidad de múltiples interfaces en R6.

---

## 2. Conectividad interna de cada AS (20 pts)

### 2.1 Protocolo de enrutamiento interno (IGP)

Para cumplir el requisito de usar más de un IGP, se asignaron protocolos distintos:

| AS   | Protocolo IGP |
| ---- | ------------- |
| AS1  | RIP v2        |
| AS2  | OSPF Area 0   |
| AS3  | RIP v2        |
| AS4  | OSPF Area 0   |
| AS99 | OSPF Area 0   |

**Justificación:** RIP v2 es adecuado para redes pequeñas con topología simple como la de cada AS cliente. OSPF se eligió para AS2, AS4 y AS99 por su mayor escalabilidad y convergencia más rápida, y porque el enunciado exige que coexistan ambos protocolos.

### 2.2 Configuración DHCP

El enunciado solicita DHCP con relay. En la implementación, el DHCP se configuró directamente en el switch multicapa de cada AS cliente usando el IOS DHCP server integrado. Los pools configurados son:

| Pool        | Red           | Gateway    | DNS        |
| ----------- | ------------- | ---------- | ---------- |
| VLAN10_POOL | 10.0.0.0/24   | 10.0.0.1   | 200.0.0.10 |
| VLAN20_POOL | 172.16.0.0/24 | 172.16.0.1 | 200.0.0.10 |

**Nota:** El enunciado menciona dhcp-relay como referencia de consulta. En Packet Tracer, la funcionalidad `ip helper-address` requiere que el servidor DHCP sea un dispositivo externo al switch; dado que el switch multicapa ejecuta IOS completo, se optó por configurar los pools directamente en él, lo que es equivalente funcionalmente y soportado por la plataforma.

### 2.3 Problema encontrado en AS4: Router-ID duplicado en OSPF

Durante la configuración de AS4 se detectó que R1-AS4 y R2-AS4 tenían el mismo Router-ID (`192.168.40.9`), lo que impedía que OSPF avanzara del estado EXSTART a FULL. La solución fue forzar Router-IDs únicos:

```
! R1-AS4
router ospf 1
 router-id 1.1.1.1

! R2-AS4
router ospf 1
 router-id 2.2.2.2
```

Después de ejecutar `clear ip ospf process`, OSPF convergió completamente.

**Evidencia — `show ip ospf neighbor` en R2-AS4:**

```
Neighbor ID     Pri   State           Dead Time   Address         Interface
192.168.40.14     1   FULL/DR         00:00:36    192.168.40.14   GigabitEthernet0/1
1.1.1.1           1   FULL/BDR        00:00:36    192.168.40.9    GigabitEthernet0/0
```

---

## 3. Interconexión entre AS clientes y AS99 (40 pts)

### 3.1 Problema de overlapping de direcciones

Todos los AS clientes usan las mismas redes internas (10.0.0.0/24 y 172.16.0.0/24). Si se intentara enrutar directamente, AS99 no podría distinguir a qué cliente pertenece cada paquete.

**Solución: NAT/PAT (overload) en el border router R4 de cada AS cliente.**

Cada R4 traduce todas las direcciones internas de su AS a su propia IP pública WAN usando PAT. De este modo, AS99 identifica inequívocamente al cliente por la IP de la interfaz WAN de su R4.

### 3.2 Configuración NAT/PAT (ejemplo AS1)

```
! R4-AS1
access-list 1 permit 10.0.0.0 0.0.255.255
access-list 1 permit 172.16.0.0 0.0.0.255
access-list 1 permit 192.168.10.0 0.0.0.255

ip nat inside source list 1 interface GigabitEthernet0/1 overload

interface GigabitEthernet0/0
 ip nat inside

interface GigabitEthernet0/1
 ip nat outside

ip route 0.0.0.0 0.0.0.0 10.10.10.2
```

### 3.3 Enrutamiento en AS99

AS99 usa OSPF internamente. Cada router de AS99 (R5–R9) conoce la red 200.0.0.0/24 via OSPF. Las rutas hacia los clientes se propagan vía OSPF con `default-information originate` en los border routers de cada AS cliente, o mediante rutas estáticas en los routers de AS99 hacia cada subred WAN (/30).

### 3.4 Evidencia NAT — `show ip nat translations`

**R4-AS1:**

```
Pro  Inside global     Inside local       Outside local      Outside global
tcp 10.10.10.1:1025    10.0.0.10:1025     200.0.0.20:80      200.0.0.20:80
tcp 10.10.10.1:1026    10.0.0.10:1026     200.0.0.20:80      200.0.0.20:80
```

**R4-AS4:**

```
Pro  Inside global     Inside local       Outside local      Outside global
udp 10.40.40.1:1047    10.0.0.10:1047     200.0.0.10:53      200.0.0.10:53
tcp 10.40.40.1:1025    10.0.0.10:1025     200.0.0.20:80      200.0.0.20:80
```

Las traducciones confirman que el tráfico de los PCs internos (IP local 10.0.0.10) sale correctamente hacia AS99 con la IP WAN de cada R4.

### 3.5 ¿Puede establecerse conectividad entre AS clientes?

**No es posible directamente.** Existen tres razones:

1. **Overlapping de direcciones:** AS1, AS2, AS3 y AS4 comparten las redes 10.0.0.0/24 y 172.16.0.0/24. No hay forma de enrutar paquetes entre ellos sin ambigüedad de destino.
2. **NAT oculta las IPs internas:** El NAT/PAT en R4 traduce las IPs internas a la IP WAN. Un paquete de AS1 que llegara a AS2 tendría como destino una dirección que AS2 no puede resolver de vuelta.
3. **Diseño de ISP:** El modelo hub-and-spoke del enunciado establece que los clientes acceden únicamente a AS99 (el proveedor), no entre sí. Para comunicación entre clientes sería necesario un acuerdo de peering entre AS, que está fuera del alcance de esta entrega.

### 3.6 Limitaciones de Packet Tracer con BGP

Packet Tracer tiene soporte muy limitado para iBGP. No permite configurar sesiones iBGP completas ni route reflectors. Por esta razón, la interconexión AS cliente ↔ AS99 se implementó con:

- Ruta estática por defecto en cada R4 cliente (`ip route 0.0.0.0 0.0.0.0 <IP_WAN_AS99>`)
- OSPF con `default-information originate` para propagar la ruta por defecto hacia los routers internos del AS cliente
- Rutas estáticas en los routers de AS99 hacia las subredes WAN de cada cliente

Esta aproximación es funcionalmente equivalente al resultado esperado en producción para este escenario.

---

## 4. Acceso a servicios en AS99 (30 pts)

### 4.1 Servidor DNS

- **IP:** 200.0.0.10/24
- **Registro A configurado:** `www.servicios.net → 200.0.0.20`
- El servidor DNS de Packet Tracer fue configurado con el servicio DNS habilitado (toggle ON).

### 4.2 Servidor WEB

- **IP:** 200.0.0.20/24
- Servidor HTTP de Packet Tracer con servicio activado.
- Accesible por IP directa y por nombre de dominio.

### 4.3 Evidencia de pruebas

#### Conectividad IP (ping desde PC1 de cada AS a DNS 200.0.0.10)

**PC1-AS1:**

```
C:\>ping 200.0.0.10
Reply from 200.0.0.10: bytes=32 time<1ms TTL=122  (×4, 0% loss)
```

**PC1-AS4:**

```
C:\>ping 200.0.0.10
Reply from 200.0.0.10: bytes=32 time<1ms TTL=122  (×4, 0% loss)
```

#### Resolución DNS

Desde browser de PC en cualquier AS: `http://www.servicios.net` resuelve correctamente a 200.0.0.20 y carga la página del servidor WEB.

#### Acceso HTTP

`http://www.servicios.net` carga exitosamente desde PC1 y PC2 de AS1, AS2, AS3 y AS4.

#### DHCP

Todos los PCs obtuvieron sus IPs vía DHCP:
| PC | IP obtenida | VLAN |
|----|-------------|------|
| PC1-AS1 | 10.0.0.10 | VLAN 10 |
| PC2-AS1 | 172.16.0.10 | VLAN 20 |
| PC1-AS2 | 10.0.0.10 | VLAN 10 |
| PC2-AS2 | 172.16.0.10 | VLAN 20 |
| PC1-AS3 | 10.0.0.10 | VLAN 10 |
| PC2-AS3 | 172.16.0.10 | VLAN 20 |
| PC1-AS4 | 10.0.0.10 | VLAN 10 |
| PC2-AS4 | 172.16.0.10 | VLAN 20 |

---

## 5. Decisiones de ingeniería justificadas

| Decisión                                                             | Justificación                                                                                                                                                                                               |
| -------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| NAT/PAT en R4 de cada AS cliente                                     | Única solución al overlapping de 10.0.0.0/24 y 172.16.0.0/24 entre los 4 AS clientes. Permite que AS99 identifique a cada cliente por IP WAN única.                                                         |
| Switch L2 (SW-AS99) como backbone en AS99                            | R6 solo tiene 2 interfaces GigabitEthernet. Con 4 clientes + DNS + WEB se necesitan 6+ conexiones. El switch L2 resuelve el problema sin enrutamiento adicional.                                            |
| DHCP en switch multicapa en lugar de servidor dedicado               | Packet Tracer soporta `ip dhcp pool` en IOS de switches 3560. Elimina la necesidad de un servidor DHCP separado y de configurar `ip helper-address` hacia un dispositivo externo.                           |
| Ruta estática por defecto en R4 + OSPF default-information originate | Packet Tracer no soporta iBGP completo. Esta combinación logra el mismo efecto funcional: los routers internos del AS aprenden la ruta de salida hacia AS99.                                                |
| Router-IDs forzados en OSPF (AS4)                                    | Packet Tracer asigna Router-ID automáticamente al IP más alto activo. Con topologías simétricas esto genera duplicados. Forzar IDs únicos (1.1.1.1, 2.2.2.2, etc.) previene el problema EXSTART permanente. |
| DNS y WEB en misma subred 200.0.0.0/24                               | El backbone de AS99 es un switch L2 (sin enrutamiento). Colocar todos los servidores en la misma /24 evita la necesidad de routing inter-VLAN en AS99.                                                      |
