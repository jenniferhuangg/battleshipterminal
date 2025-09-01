# battleshipterminal
A multiplayer battleship server done on terminal.

*Notes*
- Edit port # in server.c if needed 
- Need separate terminal windows for hosting server and joining server as a player
  
How to play:
- Download all files
- On terminal, ```sh cd (foldername of downloaded files)

  To host a server: 
  - Compile all files: ```sh gcc -O2 *.c -o (execname)
  - Execute executable to run server: ```sh ./(execname) (port#inserver.c)
  
  To join a server:
  - On terminal, ```sh cd (foldername of downloaded files)
  - To join server ```nc localhost (port#inserver.c)

To exit game as a player or quit as host, do CTRL+C
