#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define sqr(x) ((x) * (x))
#define BARWIDTH  10

typedef enum {NO, ENDGAME, PLAYAGAIN} Endchoice;
typedef enum {NONE, NOUGHTS, CROSSES} OnTurn;

typedef struct {
    SDL_Window *win;
    SDL_Renderer *render;
    const SDL_Point pos;
    const size_t w, h;
} Window;

Window okno = {.pos.x = SDL_WINDOWPOS_CENTERED, 
               .pos.y = SDL_WINDOWPOS_CENTERED, 
               .w = 640, .h = 480};
SDL_Texture *ex, *ou;
SDL_Surface *ico;

int graph_init(void) 
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL sa nemohlo inicializovat: %s\n", SDL_GetError());
        return -1;
    }

    okno.win = SDL_CreateWindow("Piškvorky", okno.pos.x, okno.pos.y, 
                                okno.w, okno.h, 0);
    if (okno.win == NULL) {
        printf("Okno sa nemohlo vytvorit: %s\n", SDL_GetError());
        return -1;
    }

    okno.render = SDL_CreateRenderer(okno.win, -1, SDL_RENDERER_ACCELERATED);
    if (okno.render == NULL) {
        printf("Vykreslovac sa nemohol vytvorit: %s\n", SDL_GetError());
        return -1;
    }

    SDL_RenderSetLogicalSize(okno.render, okno.w, okno.h);
    SDL_SetRenderDrawColor(okno.render, 210, 210, 210, 255);
    SDL_RenderClear(okno.render);
    SDL_RenderPresent(okno.render);
    return 0;
}

void graph_close(void)
{
    SDL_DestroyRenderer(okno.render);
    SDL_DestroyWindow(okno.win);
    SDL_Quit();
}

void load_textures(void)
{
    ico = SDL_LoadBMP("icon.bmp");
    ex = IMG_LoadTexture(okno.render, "ex.png");
    ou = IMG_LoadTexture(okno.render, "ou.png");
    SDL_SetWindowIcon(okno.win, ico);
}

void grid_design(SDL_Rect dst[]) 
{
    /* Vertikálne */
    dst[0].x = okno.w / 3; 
    dst[0].y = BARWIDTH;
    dst[0].w = BARWIDTH;
    dst[0].h = okno.h - (BARWIDTH * 2);

    dst[1].x = (okno.w / 3) * 2;
    dst[1].y = BARWIDTH;
    dst[1].w = BARWIDTH;
    dst[1].h = okno.h - (BARWIDTH * 2);

    /* Horizontálne */
    dst[2].x = BARWIDTH;
    dst[2].y = okno.h / 3;
    dst[2].w = okno.w - (BARWIDTH * 2);
    dst[2].h = BARWIDTH;

    dst[3].x = BARWIDTH;
    dst[3].y = (okno.h / 3) * 2;
    dst[3].w = okno.w - (BARWIDTH * 2);
    dst[3].h = BARWIDTH;
}

int win_message(const char *message)
{
    int buttonid;

    puts(message);
    const SDL_MessageBoxButtonData buttons[] = {
                { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Skonci" },
                { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Hraj znova" },
    };

    const SDL_MessageBoxColorScheme colorScheme = {{
                  { 255,   0,   0 },
                  {   0, 255,   0 },
                  { 255, 255,   0 },
                  {   0,   0, 255 },
                  { 255,   0, 255 }}
    };

    const SDL_MessageBoxData messageboxdata = {
                SDL_MESSAGEBOX_INFORMATION, 
                okno.win, 
                "KONIEC HRY", 
                message, 
                SDL_arraysize(buttons), 
                buttons, 
                &colorScheme 
    };

    SDL_ShowMessageBox(&messageboxdata, &buttonid);
    return buttonid;
} 

int is_win(OnTurn game[3][3]) 
{
    int i, j;
    OnTurn player;
    int isnottie = 0;

    /* Všetky riadky */
    for (i = 0; i < 3; i++) {
        for (player = NOUGHTS; player <= CROSSES; player++) {
            if (game[i][0] == player && game[i][1] == player && game[i][2] == player)
                goto win;
        }
    }

    /* Všetky stĺpce */
    for (i = 0; i < 3; i++) {
        for (player = NOUGHTS; player <= CROSSES; player++) {
            if (game[0][i] == player && game[1][i] == player && game[2][i] == player)
                goto win;
        }
    }

    /* Všetky diagonály */
    for (player = NOUGHTS; player <= CROSSES; player++) {
        if (game[0][0] == player && game[1][1] == player && game[2][2] == player)
            goto win;
    }

    for (player = NOUGHTS; player <= CROSSES; player++) {
        if (game[0][2] == player && game[1][1] == player && game[2][0] == player)
            goto win;
    }

    /* Je Remíza? */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (game[i][j] == NONE)
                isnottie = 1;
        }
    }

    /* Máme remízu */ 
    if(!isnottie) {
        return win_message("Hra skoncila remizou");
    }
    return NO;
win:
    if (player == NOUGHTS) 
        return win_message("Hru vyhrali: Kruzky [O]");
    else if (player == CROSSES)
        return win_message("Hru vyhrali: Kriziky [X]");
}

void print_grid(OnTurn grid[3][3]) 
{
    int i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (grid[i][j] != NONE) {
                printf(" %c |", (grid[i][j] == NOUGHTS) ? 'O': 'X');
            } else {
                printf("   |");
            }
        }
        putchar('\n');
    }
    putchar('\n');
}

OnTurn board_add(OnTurn game[3][3], OnTurn turn, SDL_Point mouse)
{
    /* Vypočítaj pozíciu na mriežke (indexy) a zisti, či už
     * tam niekto je*/
    SDL_Point grid;
    grid.x = (int)(((mouse.x / (double)okno.w) * 100) / 33);  
    grid.y = (int)(((mouse.y / (double)okno.h) * 100) / 33);
 
    if (game[grid.y][grid.x] != NONE) 
        return turn;
  
    game[grid.y][grid.x] = turn;

    /* Dáme ďalšieho na ťahu */
    return ((turn == NOUGHTS) ? CROSSES : NOUGHTS);
}

SDL_Rect tile_rect(int row, int col) 
{
    SDL_Rect dst;

    dst.x = ((okno.w / 3.0) * col) + (2 * BARWIDTH);    
    dst.y = ((okno.h / 3.0) * row) + (2 * BARWIDTH);
    dst.w = (okno.w / 3) - (2 * BARWIDTH);
    dst.h = (okno.h / 3) - (2 * BARWIDTH);

    return dst;
}

void draw_boardstate(OnTurn board[3][3], const SDL_Rect grid[]) 
{ 
    int i, j;
    SDL_Rect tile;

    SDL_SetRenderDrawColor(okno.render, 210, 210, 210, 255); 
    SDL_RenderClear(okno.render);

    /* Herná mriežka */
    SDL_SetRenderDrawColor(okno.render, 150, 43, 24, 255);
    SDL_RenderFillRects(okno.render, grid, 4);

    /* Nakreslenie rozmiestnenie X a O */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (board[i][j] != NONE) {
                tile = tile_rect(i, j);
                if (board[i][j] == NOUGHTS) 
                    SDL_RenderCopy(okno.render, ou, NULL, &tile);
                else
                    SDL_RenderCopy(okno.render, ex, NULL, &tile);
            }
        }
    } 

    SDL_RenderPresent(okno.render);
}

void print_turn(OnTurn turn)
{
    if (turn == NOUGHTS) {
        SDL_SetWindowTitle(okno.win, "Piškvorky - ťah: 0");
        puts("> @Na tahu: O");
    } else { 
        SDL_SetWindowTitle(okno.win, "Piškvorky - ťah: X");
        puts("> @Na tahu: X");
    }
}

void event_loop(void) 
{
again: ;
    OnTurn gameboard[3][3] = {{NONE, NONE, NONE}, 
                              {NONE, NONE, NONE}, 
                              {NONE, NONE, NONE}};
    OnTurn turn = NOUGHTS; /* Hru začínajú O-čká*/
    SDL_Rect grid[4];
    SDL_Event ev;
    SDL_Point mouse;
    int win = NO;

    grid_design(grid);
    print_turn(turn);

    for (;;) {
        draw_boardstate(gameboard, grid);
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    return;
                case SDL_MOUSEBUTTONDOWN:

                    if (ev.button.button == SDL_BUTTON_LEFT) {
                        SDL_GetMouseState(&mouse.x, &mouse.y);
                        
                        turn = board_add(gameboard, turn, mouse); 
                        print_turn(turn);
                        print_grid(gameboard);
                        draw_boardstate(gameboard, grid);

                        win = is_win(gameboard);
                        if (win == PLAYAGAIN)
                            goto again;
                        else if (win == ENDGAME) 
                            return;
                        else 
                            continue;
                    }
            }
        }
        SDL_Delay(16);
    }
}

int main(void)
{
    if (graph_init() != 0) 
        return -1;

    load_textures();
    event_loop();
    graph_close();
}
