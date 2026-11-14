# Roadmap

## Shaugno continuation: current priorities

This repository is now Shaugno's independent continuation of ByJunter's work.
The goal is reliable two-player co-op on the installed build, then a repeatable
way to assess later builds. The installed game is now **1.0.0.3663**. Full co-op
has not been demonstrated. The 3576 capture is blocked because Steam updated
the installation.

Current execution plan (2026-11-14):
[Co-op prototype plan](COOP-3663-PROTOTYPE-PLAN.md). The immediate target is
two players seeing each other walk and turn. Pause repeated captures; finish
the independent state/transport/actor-path work and tests as one Grok package,
then provide one consolidated handoff. The available post-ready stand attempt
stored zero rows, so movement layouts remain unproved. Approved checkpoint
`3e950a7` includes the controller, timing diagnostics, post-ready admission,
and sealed attempt logs; its final independent suite passed 100/100.
The older [capture plan](GROK-3663-EXECUTION-PLAN.md) remains available for
specific experiments. The paragraphs below preserve historical milestones.

Completed in this continuation: the live-tested read-only position diagnostic,
the passive observer, the real capture backend, and the version-checked module
validated against dummy hosts. The external loading/controller tool is also
reviewed, including control after hooks are installed and strict PID parsing.
The last published game-free suite passed 95/95, including the file-only 3663
scanner and strict packet-entry verifier. The separate 3663 passive runner and
its additional offline tests are implemented in the current work. Independent
review fixes are applied, and that reviewed suite passed 99/99. The 3663
controller and its game-free test are now local on this branch. That suite
passed 100/100 and is waiting for Codex review.
The first controlled 3576 capture was reviewed, and the current movement/action
and actor-message IDs were independently verified on disk. Full payload layouts
and native-session compatibility still need evidence.

1. The 3576 capture is blocked. Steam updated the installation to 1.0.0.3663.
   The aborted session is docs/probe-3576/PHASE1-CAPTURE-20261113.md. The 3576
   gate still rejects that file. Do not broaden it or load its collector.
2. The offline 3663 packet-entry review is complete. Exact send, receive, and
   pump roles pass a strict file verifier; see
   docs/probe-3663/ENTRY-VERIFIER-3663.md. The isolated passive runner is
   implemented and independently game-free tested; see
   docs/probe-3663/LIVE-RUNNER-3663.md. The separate controller is implemented
   locally and pending Codex review; see
   docs/probe-3663/LIVE-CONTROLLER-3663.md and
   docs/probe-3663/CONTROLLER-HANDOFF-20261113.md. The candidates remain
   experimental and unadopted. The local RelWithDebInfo suite passed 100/100
   with the game closed. After that review, the next step is a separately
   authorized controlled passive live session. Do not install the ASI.
3. Spawn and move a second visible actor from those verified fields. Do not
   invent a spawn call or reuse 2976 packet sizes.
4. Validate loading, reconnects, combat, save handling, and two-PC play before
   describing the result as a gameplay release.

Current evidence and limitations: [PROGRESS.txt](PROGRESS.txt).

## Original upstream roadmap and research history

The milestones below preserve ByJunter's original notes and build-2976 results.
Their completed markers do not establish compatibility with 3576.

Todo se desarrolla y prueba con **un solo PC**. El segundo jugador lo hace
`cdmp_peer.exe` hasta el hito 5.

## Hito 0: esqueleto ✅ (2026-10-03)
- `cdmp.asi` carga en `CrimsonDesert.exe` 1.0.0.2976 vía Ultimate ASI Loader (`winmm.dll`).
- Log con la versión exacta del juego (PE timestamp `0x6AB28F00`, base `0x140000000`).
- Red UDP con handshake con reintentos, ping/RTT, timeout y reconexión. Probado con
  10% de pérdida y 20 ms de jitter.
- El mod solo se activa dentro de `CrimsonDesert.exe`. El loader también se inyecta en
  `crashpad_handler.exe`, y ahí el mod no hace nada.
- Todavía no toca la memoria del juego.

## Hito 1: lector externo (sin inyección) ✅ (2026-10-03)
- `cdmp_probe.exe` (solo lectura): firmas, cadenas, watch, dump, RTTI, vtables, findptr.
- Build identificada: **parche 2.03.02**. 42/48 firmas de la comunidad con match único.
- Verificado en vivo con partida cargada: posición (global y física), rotación,
  tabla de componentes del actor, HP y cámara con proyección.
- Hallazgo clave: arquitectura cliente/servidor interna con 1.116 mensajes `Troc*`.
- Detalle en `docs/research/hallazgos-en-vivo.md`. Falta: ids de stamina y espíritu.

## Hito 2: posición real dentro del mod ✅ (2026-10-03)
- `PlayerReader` lee posición, rotación, HP y stamina a 20 Hz, en solo lectura y sin
  hooks. Cada salto se valida por vtable y cada lectura usa `VirtualQuery` + SEH.
- Probado en el juego: la posición coincide con el global del juego a menos de
  0,1 m, la rotación sigue los giros y el HP se regenera en vivo. El ghost recibió
  más de 3.000 estados sin rechazos.
- Filtra la posición de carga (0, 1000, 0) y se desactiva si la build no es la 1.0.0.2976.
- Viaje rápido: un salto de unos 790 m sin perder la cadena y con el control frente
  al global en 0,05 m. Sesión completa: 4.677 estados recibidos por el ghost, 0 rechazos.

## Hito 3: overlay ✅ (2026-10-03)
- ImGui sobre DX12 con hooks **inline** (safetyhook) a Present, Present1, ResizeBuffers
  y ResizeBuffers1, más ExecuteCommandLists para obtener la cola del hilo de present.
- Probado con DLSS 4.5 y generación de frames: dibuja a 76 FPS reales sin errores.
- Panel (F8 lo muestra u oculta) con red, HP y stamina propios, y HP y distancia del
  otro jugador.
- Marcador sobre el otro jugador con nombre, barra de HP y distancia. La cruz de
  control cae sobre el propio personaje (verificado con captura de pantalla).
- Lecciones del intento fallido:
  - Parchear la vtable no funciona porque Streamline llama a Present directamente.
  - Crear el dispositivo D3D12 de prueba al arrancar dejó el juego en negro. Ahora se
    instala cuando existe `WorldSystem`, la ventana es visible, y tras 5 s más.
- Interruptor de emergencia: `[overlay] enabled=0` en `bin64\cdmp.ini`.
- `cdmp_dx12_test.exe` prueba el hook sin el juego.

## Hito 4: usar los mensajes internos cliente/servidor (replanteado el 2026-10-03)
El juego conserva la arquitectura del MMO original: 1.115 mensajes Troc en bytes sobre
un loopback en memoria, con una sesión de servidor por usuario. Ver
`docs/research/troc-sintesis.md`.
- **4a:** sniffer pasivo en `ClientNet::PostPacket` y `ClientNet::OnRecvPacket`, solo
  registro, para obtener un catálogo de tráfico real.
- **4b:** decodificar los payloads clave (`MoveActor`, `UpdateVisiblePlayer`,
  `RemoveActor`).
- **4a ✅** sniffer v2 con paquetes completos, 0 pérdidas, y kit de captura para colaboradores.
- **4b ✅** formatos decodificados: `MoveActorReq`, `ActorLoadDataAck` (usuario y cuerpo,
  posición a 154 bytes del final) y `RemoveActorAck`.
- **4c ✅** clon de Kliff inyectando dos `ActorLoadDataAck` con claves nuevas (F9).
  Aparece al lado del jugador, con colisión. F10 lo quita con fundido.
- **4d ✅** modo espejo: los `MoveActor`/`StartAction`/`RotateActor` Req del jugador se
  reinyectan como Ack para el clon (formatos según `mirror-ack-formats.md`), con retardo
  configurable.
  - Ajustes que lo hicieron funcionar: reajustar marcas de tiempo, desplazar las
    posiciones de `StartAction` y limpiar la clave de montura.
  - Resultado en vivo: mucho mejor, con teletransportes muy ocasionales.
  - Esperable: el clon es una marioneta (como un jugador remoto en un MMO), no interactúa
    con el mundo. Con desplazamiento lateral las animaciones de escalada no coinciden
    con la pared; con `mirror_offset_m=0` sigue el camino exacto.
  - Escalada: subidas a salientes con `UpdateWallUpDataAck` (ya no desaparece) y escalada libre
    con un hook en `PointClimb::Enter` (ya no se queda colgado). Queda un leve deslizamiento.
    Ambas cosas harán falta igual con un jugador real.

- **4e ✅** cooperativo por red (2026-10-03). Probado en un PC con `cdmp_peer --mode relay`,
  que devuelve tus propios datos: se ve correctamente.
  - `local_capture` (emisor): Reqs del jugador, movimientos sintetizados, ruta del saliente y
    plantillas de carga.
  - `remote_puppet` (receptor): el otro jugador como actor remoto, con todos los arreglos del
    espejo. El espejo de F9/F10 usa el mismo código.
  - Protocolo v3: `GameEvent`, canal fiable fragmentado (`RelData`/`RelAck`) y `SpawnInfo`
    (~31 KB).
  - El receptor estima la diferencia de reloj de juego entre los dos (mínimo en 4 s, suavizado).
  - `cdmp_peer --mode relay`: reenvía entre dos jugadores o hace eco con uno. Avisa del nombre
    del otro jugador.
  - Kit para un amigo: `scripts\package_coop.ps1` → `dist\cdmp-coop-<fecha>.zip`.
  - Pruebas en vivo del 2026-10-03 (protocolo v4):
    - ✅ **equipo**: funciona desde que la copia tiene su propio número de personaje. El
      número compartido mezclaba las pantallas de equipo, porque la caché de equipo del
      cliente es única y va por número de personaje.
    - ✅ el arma ya cambiaba sola, con las acciones: el relevo de docking sobra.
    - ❌ **caballo**: con dueño, el cliente no crea la copia; sin dueño, el juego se cierra.
      Forzar `RideOff::Enter` dejaba la copia atascada. Queda desactivado (`horse=0`).
  - Detalle de lo que se añadió:
    - equipo: hook en el escritor del servidor de `OtherEquipItemAck` y quitar equipo desde
      `UnequipItemAck`; al aparecer se aplican los cambios desde la carga
      (`apariencia-estado-coop.md`);
    - arma en mano/espalda: hook en el escritor de `ChangeDockingSocketByFrameEventAck`
      (`combate-coop.md` §3);
    - caballo, fase 1: se carga una copia del caballo del emisor, se mueve con el `Path.world`
      del jinete y desde memoria cuando nadie lo monta, y hooks en `RideOn`/`RideOff::Enter`
      (`caballo-coop.md`). Interruptor: `[coop] horse=0`;
    - orientación al aparecer, y se ignora la carga por defecto en (0, 1000, 0);
    - el marcador del overlay va sobre el personaje dibujado.
  - Falta: combate compartido (enemigos, daño; plan en `combate-coop.md` §5).

## Hito 5: Steam (en curso)
- ✅ Transporte Steam sobre `ISteamNetworkingMessages` de la `steam_api64.dll` del juego, sin
  callbacks (`docs/research/steam-transporte.md`). Probado en vivo hasta "relay listo".
- ✅ Anfitrión e invitado sin `cdmp_peer` (`[net] role=host|guest`), también por UDP.
- Falta: primera prueba con una persona real.

## Después
Sincronizar enemigos y combate (con IDs estables entre PCs, no punteros), animaciones,
misiones y viaje rápido.

## Opción D: pendientes tras la sesión 16 (2026-10-11)
Hecho y validado en dos PC: NPC (horarios, reenlace, retención en la carga, orden padre-hijo),
viajeros y viaje rápido del invitado dentro del campo del anfitrión.
En orden de prioridad acordado con el usuario:
1. **Monturas: hecho** (sesión 23, 2026-10-11, funciona en los dos sentidos). Vínculo nativo en el
   observador (vf53), desmontar con el arreglo B, y el jinete remoto sentado en el hueso de la silla con vf54
   (tipo de enganche 9 → 4), que quita a la vez el desplazamiento y el parpadeo al montar
   (`pendientes/p8-impl-monturas.md` §10–§15). Queda: entrar tarde con alguien ya montado.
   - **Antes de seguir:** el aviso "Failed to save data" en bucle a los ~16 min (el guardián rechaza el
     autoguardado y el juego reintenta cada 10 s). Hay que sustituir el bloqueo por un guardado desviado.
2. **Hora y clima:** el invitado ve la hora y el clima del momento en que el anfitrión cargó la
   partida, no los actuales, y al teletransportarse parece que el reloj se reinicia. Prueba de la
   sesión 16: en el anfitrión anochece y llueve; en el invitado aún hay sol.
3. **Tenderos:** escena gemela S2 (`pendientes/p8-impl-tenderos-s2.md`).
4. **Mundo alrededor del invitado:** los NPC desaparecen a unos 250 m del anfitrión.
5. Combate con retardo; escenas personales del anfitrión que llegan al invitado (ESC/ver objeto);
   guardado del progreso propio del invitado.
6. Menores: viajeros pedidos por los dos clientes (densidad doble); objetos del escenario que van
   y vienen (2928/2221); huérfano residual del orden de familia.
