# battleshipterminal
A multiplayer battleship server done on terminal.

## Notes
- Edit port # in `server.c` if needed 
- Need separate terminal windows for hosting server and joining server as a player

## Game Overview
Board Size: 10×10 grid with coordinates ranging from (0,0) to (9,9).

Ships: Each player controls a single ship of length 5, placed either horizontally (1×5) or vertically (5×1).

### Joining:

Players register with a unique name and ship location.
Overlapping ships are allowed.
Once registered, the server announces the new player to everyone.

### Bombing:

Any registered player can bomb any cell on the grid at any time.
If a bomb hits part of a ship, the damage is recorded and broadcast to all players.
A bomb may damage multiple ships if they overlap.
If no ships occupy the target cell, the bombing is declared a miss.

### Losing:

When all 5 cells of a player’s ship are damaged, the player is eliminated.
The server broadcasts the loss and disconnects the player.

### Leaving: 
Disconnecting voluntarily is treated the same as losing.

## Message Protocol
'(x, y)': center coordinate of the ship
d: '-' for horizontal (1×5), '|' for vertical (5×1)

## Registration
`REG <name> <x> <y> <d>\n`
name: up to 20 characters (letters, digits, or -)

### Registration responses:
`INVALID\n` — invalid syntax or ship placement out of bounds
`TAKEN\n` — name already in use
`WELCOME\n` — registration successful

### Broadcast:
`JOIN <name>\n`
Shown to all players when new client joins server

## Bombing
### Client request:
`BOMB <x> <y>\n`

### Server responses:
Hit:
`HIT <attacker> <x> <y> <victim>\n`

### Miss:
`MISS <attacker> <x> <y>\n`

## Player Elimination
`GG <name>\n`

Broadcast when a player loses or disconnects.

## Error Handling

Invalid messages: Server responds with `INVALID\n.`
Overlong messages: Any message exceeding 100 bytes without a newline results in disconnection.
Write errors / SIGPIPE: Client is disconnected.
Premature disconnection: If registered, the loss is broadcast; otherwise, ignored
  
## How to play:
- Download all files
- On terminal, `sh cd (foldername of downloaded files)`

  To host a server: 
  - Compile all files: `sh gcc -O2 *.c -o (execname)`
  - Execute executable to run server: `sh ./(execname) (port#inserver.c)`
  
  To join a server:
  - On terminal, `sh cd (foldername of downloaded files)`
  - To join server `nc localhost (port#inserver.c)`

To exit game as a player or quit as host, do **CTRL+C**
