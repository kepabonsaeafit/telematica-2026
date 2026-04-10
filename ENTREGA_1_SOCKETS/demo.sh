#!/bin/bash

# Matar cualquier sesión previa de tmux
tmux kill-session -t telematica 2>/dev/null

# Crear una nueva sesión llamada 'telematica'
tmux new-session -d -s telematica

# Dividir la pantalla: Arriba e Izquierda
tmux split-window -h -t telematica:0.0
tmux split-window -v -t telematica:0.1
tmux split-window -v -t telematica:0.0

# 1. Panel arriba-izquierda: Servidor C
tmux send-keys -t telematica:0.0 "make run" C-m

# 2. Panel arriba-derecha: Servidor Identidad (Lobby)
tmux send-keys -t telematica:0.1 "python3 login_server.py" C-m

# 3. Panel abajo-izquierda: Cliente 1
tmux send-keys -t telematica:0.2 "sleep 2 && python3 client.py" C-m

# 4. Panel abajo-derecha: Cliente 2
tmux send-keys -t telematica:0.3 "sleep 3 && python3 client.py" C-m

# Seleccionar el primer panel y entrar a la sesión
tmux select-pane -t telematica:0.2
tmux attach-session -t telematica
