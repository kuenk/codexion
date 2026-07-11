# Codexion — explicación paso a paso

Este documento sigue el flujo real de ejecución del programa, en el orden en que
ocurren las cosas.

---

## 1. Arranque: `main.c`

```
main()
 ├─ ft_parsing()      → valida los 8 argumentos
 ├─ ft_mem_alloc()    → reserva memoria para coders[] y dongles[]
 ├─ ft_init()         → inicializa mutex y estado inicial de todo
 ├─ ft_start_simulation() → crea los hilos, espera a que terminen
 └─ ft_clean_all()    → destruye mutex, libera memoria
```

Si `ft_parsing` o `ft_mem_alloc` fallan, el programa sale inmediatamente
(`return (1)`) sin llegar a crear ningún hilo. Nada que limpiar en ese caso
porque nada se llegó a reservar (o solo lo que el propio `ft_mem_alloc` ya
liberó por su cuenta si el segundo `malloc` fallaba).

---

## 2. Parseo: `parsing.c`

`ft_parsing` hace tres comprobaciones en cascada, cortocircuitando en la
primera que falle:

1. `argc != 9` → error "Expected 8 arguments".
2. `ft_check_all_args` → recorre `argv[1..7]` con `ft_parse_uint` (dígito a
   dígito, rechaza vacío, signo `-`, letras, y detecta overflow *antes* de
   multiplicar por 10), y `argv[8]` con `ft_check_scheduler` (debe ser
   exactamente `"FIFO"` o `"EDF"`).
3. `ft_validate_and_save` → vuelve a parsear cada argumento (ya sabemos que
   son válidos) y los guarda en la struct `t_program`. Aquí también se aplica
   el rango `2 <= total_coders < 200`.

Nota: los argumentos se parsean dos veces (una para validar, otra para
guardar) — es redundante en trabajo pero mantiene las funciones cortas y
separadas.

---

## 3. Memoria e inicialización: `utils.c` + `init.c`

`ft_mem_alloc` reserva dos arrays del tamaño `total_coders`:
- `pgm->coders`  (array de `t_coder`)
- `pgm->dongles` (array de `t_dongle`)

`ft_init` después:
1. Inicializa `write_mutex` y `status_mutex` (los dos únicos mutex reales del
   programa — no hay mutex por dongle).
2. `pgm->simulation_end = 0`.
3. `ft_init_dongles` → pone `available_at = 0` en todos los dongles (0 =
   "disponible desde siempre", ya que el reloj de la simulación también
   arranca en torno a 0).
4. `ft_init_coders` → para cada coder `i` (0-indexado, `id = i+1`):
   - `left_dongle = &dongles[i]`
   - `right_dongle = &dongles[(i+1) % total_coders]`
   - `compile_count = 0`, `request_time = 0`, `is_waiting = 0`,
     `is_compiling = 0`, `last_compile_start = 0`.

**Topología del anillo**: el dongle `k` es el `left_dongle` del coder `k` y a
la vez el `right_dongle` del coder `k-1` (índices 0-based, con wraparound).
Por eso cada dongle solo puede ser disputado por exactamente 2 personas: sus
dos vecinos inmediatos.

---

## 4. Arranque de hilos: `ft_start_simulation` (`start_simulation.c`)

```
pgm->start_time = ft_get_time()          ← instante 0 de la simulación
para cada coder:
    coder.last_compile_start = start_time
    pthread_create(coder_routine, &coder)
pthread_create(supervisor, pgm)
... pthread_join a todos los coders, luego al supervisor ...
```

Importante: `last_compile_start` de todo el mundo arranca igual al inicio de
la simulación — esto es lo que hace que la primera "carrera" por dongles
(instante ~0) sea un empate técnico entre todos, sin que FIFO/EDF tengan
todavía ninguna diferencia real que aplicar.

A partir de aquí corren **N+1 hilos en paralelo**: N hilos `ft_coder_routine`
y 1 hilo `ft_supervisor`. Todo lo que sigue hay que leerlo pensando en que
puede intercalarse de cualquier forma entre esos hilos — el orden real lo
decide el sistema operativo, dentro de las reglas que imponen los mutex.

---

## 5. El ciclo de vida de un coder: `ft_coder_routine` (`routines.c`)

```c
while (!ft_check_end(pgm) && coder->compile_count < pgm->num_compiles)
{
    if (!ft_compile(coder))
        break ;
    ft_phase(coder, "is debugging", pgm->time_to_debug);
    ft_phase(coder, "is refactoring", pgm->time_to_refactor);
}
```

Cada vuelta del bucle es un ciclo completo: **compilar → depurar →
refactorizar**, y solo termina cuando `simulation_end` se activa (por
burnout de alguien, o por éxito global) o cuando este coder alcanza
`num_compiles`.

`ft_phase` es genérica: comprueba `simulation_end`, imprime el mensaje, y
duerme la duración correspondiente. Si `simulation_end` ya está activo, ni
imprime ni duerme — sale inmediatamente.

---

## 6. El corazón del programa: `ft_compile` (`acquire.c`)

```c
int ft_compile(t_coder *coder)
{
    if (ft_check_end(pgm))           return (0);
    if (!ft_acquire_dongles(coder))  return (0);
    ft_print(coder, "is compiling");
    ft_usleep(pgm->time_to_compile);
    // bajo status_mutex:
    coder->left_dongle->available_at  = ahora + cooldown;
    coder->right_dongle->available_at = ahora + cooldown;
    coder->compile_count++;
    coder->is_compiling = 0;
    return (1);
}
```

### 6.1. `ft_acquire_dongles` — la adquisición atómica

```c
pthread_mutex_lock(&status_mutex);
if (simulation_end) { unlock; return 0; }
ft_wait_turn(coder);              // bucle de espera, ver 6.2
if (simulation_end) { unlock; return 0; }  // pudo cambiar MIENTRAS esperaba
ft_grant_dongles(coder);          // marca como ocupado, imprime, unlock
return (1);
```

Puntos clave:
- **Todo pasa con `status_mutex` sujeto**, salvo el propio `usleep` de espera
  dentro de `ft_wait_turn` (que suelta el mutex mientras duerme, para no
  bloquear a los demás hilos innecesariamente).
- Hay **dos comprobaciones de `simulation_end`**: una antes de empezar a
  esperar, otra justo después — porque la simulación puede terminar
  *mientras* este coder estaba dormido esperando su turno.

### 6.2. `ft_wait_turn` — el bucle de polling

```c
coder->is_waiting = 1;
coder->request_time = ft_get_time();
while (!simulation_end && !ft_can_take_dongles(coder))
{
    unlock(status_mutex);
    ft_usleep(1);              // duerme 1ms
    lock(status_mutex);
}
coder->is_waiting = 0;
```

Cada 1ms, este coder vuelve a preguntar "¿puedo ya?" (`ft_can_take_dongles`).
Mientras espera, dos campos quedan visibles para el resto: `is_waiting = 1`
(le dice a los demás "estoy compitiendo activamente ahora mismo") y
`request_time` (marca cuándo empezó a pedir, para el desempate FIFO).

### 6.3. `ft_can_take_dongles` — la política fifo/edf (`dongle_priority.c`)

```c
if (now < left_dongle->available_at)  return 0;   // izquierdo aún no libre
if (now < right_dongle->available_at) return 0;   // derecho aún no libre
if (left != right)   // caso normal (no degenerado)
{
    if (ft_loses_to(pgm, coder, left_neighbor))  return 0;
    if (ft_loses_to(pgm, coder, right_neighbor)) return 0;
}
return 1;
```

`ft_loses_to(me, rival)`:
- Si `rival` no está esperando ahora mismo (`is_waiting == 0`) → no hay
  disputa real, `me` no pierde nada. Devuelve 0.
- Si sí está esperando → se comparan las claves de prioridad
  (`ft_priority_key`): en FIFO es `request_time` (menor = más antiguo = gana);
  en EDF es `last_compile_start + time_to_burnout` (menor = más cerca de
  quemarse = gana). Si el rival tiene la clave más baja, `me` pierde y sigue
  esperando.

**Por qué solo hace falta mirar a los dos vecinos**: como cada dongle solo
puede ser disputado por 2 personas (ver sección 3), no hace falta comparar
contra todos los `N` coders — basta preguntar a quien comparte cada uno de
tus dos dongles.

### 6.4. `ft_grant_dongles` — tomar posesión

```c
left_dongle->available_at  = LLONG_MAX;   // "ocupado indefinidamente"
right_dongle->available_at = LLONG_MAX;
last_compile_start = ahora;               // reinicia el reloj de burnout
is_compiling = 1;                         // exime del chequeo de burnout
unlock(status_mutex);
print("has taken a dongle");
if (dongles distintos) print("has taken a dongle");   // segunda vez
```

`available_at = LLONG_MAX` es el truco para representar "ocupado ahora
mismo" reutilizando el mismo campo que se usa para el cooldown — cualquier
comparación `now < available_at` va a dar verdadero durante muchísimo
tiempo (prácticamente para siempre, hasta que se libere explícitamente).

### 6.5. Liberación (de vuelta en `ft_compile`, tras el `usleep` de compilar)

```c
end_cooldown = ahora + pgm->cooldown;
left_dongle->available_at  = end_cooldown;
right_dongle->available_at = end_cooldown;
compile_count++;
is_compiling = 0;
```

A partir de aquí, cualquier coder (este mismo en su siguiente ronda, o su
vecino) que pregunte `ft_can_take_dongles` va a ver `now < available_at`
como verdadero hasta que pasen esos `cooldown` ms — es la implementación
completa del "no disponible tras liberarse" del enunciado.

---

## 7. El hilo supervisor: `ft_supervisor` (`start_simulation.c`)

```c
while (!ft_check_end(pgm))
{
    para cada coder i:
        if (ft_check_death(pgm, i))   → imprime "burned out", marca
                                         simulation_end=1, return
    if (ft_all_coders_finished(pgm))
        → marca simulation_end = 1, break
    ft_usleep(1);        // vuelve a comprobar en 1ms
}
```

### 7.1. `ft_check_death` — el chequeo de burnout

```c
lock(status_mutex);
if (compile_count >= num_compiles)     { unlock; return 0; }  // ya terminó
if (is_compiling)                      { unlock; return 0; }  // exento
if ((ahora - last_compile_start) > time_to_burnout)
{
    lock(write_mutex);
    simulation_end = 1;
    printf("... has burned out.");
    unlock(write_mutex);
    unlock(status_mutex);
    return 1;
}
unlock(status_mutex);
return 0;
```

Se comprueba a **todos** los coders, uno a uno, cada 1ms. En cuanto uno
cumple la condición de burnout, se marca el fin global y el supervisor
retorna inmediatamente — el resto de coders, al ver `simulation_end == 1` en
su siguiente comprobación (dentro de `ft_check_end`, `ft_wait_turn`, o al
principio de su bucle en `ft_coder_routine`), dejan de hacer nada más.

### 7.2. `ft_all_coders_finished` — condición de éxito

Recorre `compile_count` de todos, bajo `status_mutex`. Si todos alcanzaron
`num_compiles`, se marca `simulation_end = 1` y el supervisor termina.

---

## 8. Los tres finales posibles

1. **Éxito**: todos los coders alcanzan `num_compiles`. Lo detecta el
   supervisor en `ft_all_coders_finished`. Ningún mensaje especial además de
   los normales de cada fase.
2. **Burnout**: alguien no empieza a compilar a tiempo. Lo detecta el
   supervisor en `ft_check_death`. Aparece exactamente una línea
   `has burned out.`, y nada más después.
3. **Error de parseo**: el programa ni siquiera llega a crear hilos. Sale con
   `return (1)` desde `main`, sin imprimir nada de la simulación.

En los casos 1 y 2, tras `ft_start_simulation` vuelve el control a `main`,
que llama a `ft_clean_all` (destruye los dos mutex, libera `coders` y
`dongles`) y devuelve `0` — actualmente `main` no distingue entre haber
terminado por éxito o por burnout a nivel de código de salida (lo comentamos
hace unas rondas: tu enunciado no lo exige, así que se quedó así a
propósito).

---

## 9. Invariantes de sincronización, para verificar por tu cuenta

Si quieres comprobar por ti mismo que no hay huecos, estas son las reglas
que el código intenta mantener siempre:

- **Todo acceso a `available_at`, `is_waiting`, `is_compiling`,
  `compile_count`, `last_compile_start`, `request_time` y `simulation_end`
  ocurre con `status_mutex` sujeto.** Es el único mutex que protege el
  estado compartido de coders y dongles.
- **`write_mutex` solo protege la salida por `printf`** (en `ft_print` y en
  el mensaje de burnout dentro de `ft_check_death`) — nunca protege datos,
  solo evita que dos mensajes se entremezclen en la terminal.
- **Nunca se sujetan los dos dongles con locks independientes** — no existen
  locks por dongle. Toda la "atomicidad de tomar dos a la vez" viene de que
  la comprobación y la reserva ocurren dentro del mismo tramo de
  `status_mutex`, sin soltar el mutex entre medias.
- **`ft_print` puede imprimir después de soltar `status_mutex`** (ves esto en
  `ft_grant_dongles`) — por eso, como vimos en las pruebas, dos mensajes que
  lógicamente ocurren "a la vez" pueden aparecer con 1ms de diferencia en el
  log: el estado ya estaba fijado dentro del mutex, solo el `printf` se
  retrasó por scheduling del hilo.

Con esto deberías poder trazar cualquier escenario a mano: en cada instante,
pregúntate qué coders están en `is_waiting`, cuáles en `is_compiling`, cuál
es el `available_at` de cada dongle, y aplica `ft_can_take_dongles` /
`ft_check_death` como si fueras tú el supervisor.