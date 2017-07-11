# TicTacToe in SDL2 (Piškvorky)

Demonstation of basic functional elements of Simple DirectMedia graphics library
via game of TicTacToe. In the mean time you are able to play multiplayer game.
Later I would like to deploy Minimax algorithm, which will anable AI
functionality.


#### Prereqisites

You will need **SDL2, SDL2-image** for your operating system: 
[SDL download website](https://www.libsdl.org/download-2.0.php)

On linux (debian) you can get it from package manager:
```bash
sudo apt install libsdl2-2.0-0 libsdl2-image-2.0-0
```


### Build and Run
```bash
gcc piskvorky.c -lSDL2 -lSDL2_image -o piskvorky
```

### Licence
GNU GPLv2.1
