// Casse briques - SDL2
//
// COMPILATION
// gcc src/main.c -o bin/prog -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer -mwindows

// EXECUTION
// bin\prog.exe

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 1100
#define WINDOW_HEIGHT 600

// #define FPS_LIMIT 0      // No limit
#define FPS_LIMIT 16     // floor(1000 / 60) == 16 -> 60 frames in 1000ms -> 60 FPS
// #define FPS_LIMIT 33     // floor(1000 / 30) == 33 -> 30 frames in 1000ms -> 30 FPS
// #define FPS_LIMIT 41     // 24 FPS
// #define FPS_LIMIT 200

double timing = 0.0;

#define MAX_GRAPH_LENGTH 340
SDL_Point graph[MAX_GRAPH_LENGTH];
unsigned int graph_length = 0;

// int y[18] = {100, 100, 100, -100, -100, -100, 100, 100, 100, -100, -100, -100, 100, 100, 100, -100, -100, -100 };

void showMsg(SDL_Renderer *renderer, char *txt, int x, int y, int size);

void SDL_RenderDrawEllipse(SDL_Renderer *renderer, int x, int y, int x_radius, int y_radius) {
    int x_plot, y_plot;
    int prev_x, prev_y;
    
    float f_x_radius = (float) x_radius;
    float f_y_radius = (float) y_radius;

    float step = (30.0 / f_x_radius);
    if(step > 0.1) step = 0.1;

    prev_x = (int) (SDL_cos(0) * f_x_radius);
    prev_y = (int) (SDL_sin(0) * f_y_radius);

    for(float theta=step; theta < M_PI/2.0 + step ; theta += step) {
        x_plot = (int) (SDL_cos(theta) * f_x_radius);
        y_plot = (int) (SDL_sin(theta) * f_y_radius);

        SDL_RenderDrawLine(renderer, x + prev_x, y + prev_y, x + x_plot, y + y_plot);
        SDL_RenderDrawLine(renderer, x + prev_x, y - prev_y, x + x_plot, y - y_plot);
        SDL_RenderDrawLine(renderer, x - prev_x, y - prev_y, x - x_plot, y - y_plot);
        SDL_RenderDrawLine(renderer, x - prev_x, y + prev_y, x - x_plot, y + y_plot);

        prev_x = x_plot;
        prev_y = y_plot;
    }
}

void SDL_RenderDrawCircle(SDL_Renderer *renderer, int x, int y, int radius) {
    SDL_RenderDrawEllipse(renderer, x, y, radius, radius);
}

void SDL_RenderFillCircle(SDL_Renderer *renderer, int x, int y, int radius, SDL_Color color) {
    int x_plot, y_plot;
    SDL_Color oldColor;

    SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for(int y_plot=0; y_plot<=radius; y_plot++)
        for(int x_plot=0; x_plot<=radius; x_plot++)
            if(x_plot*x_plot+y_plot*y_plot <= radius*radius) {
                SDL_Point points[] = { {x + x_plot, y - y_plot}, {x + x_plot, y + y_plot}, {x - x_plot, y + y_plot}, {x - x_plot, y - y_plot} };
                SDL_RenderDrawPoints(renderer, points, 4);
            }

    SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a);
}

void SDL_RenderFillEllipse(SDL_Renderer *renderer, int x, int y, int x_radius, int y_radius, SDL_Color color) {
    int hh = y_radius * y_radius;
    int ww = x_radius * x_radius;
    int hhww = hh * ww;
    int x0 = x_radius;
    int dx = 0;
    SDL_Color oldColor;

    SDL_GetRenderDrawColor(renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // do the horizontal diameter
    for (int x1 = -x_radius; x1 <= x_radius; x1++)
        SDL_RenderDrawPoint(renderer, x + x1, y);

    // now do both halves at the same time, away from the diameter
    for (int y1 = 1; y1 <= y_radius; y1++) {
        int x1 = x0 - (dx - 1);  // try slopes of dx - 1 or more
        for ( ; x1 > 0; x1--)
            if (x1*x1*hh + y1*y1*ww <= hhww)
                break;
        dx = x0 - x1;  // current approximation of the slope
        x0 = x1;

        for (int x1 = -x0; x1 <= x0; x1++) {
            SDL_RenderDrawPoint(renderer, x + x1, y - y1);
            SDL_RenderDrawPoint(renderer, x + x1, y + y1);
        }
    }

    SDL_SetRenderDrawColor(renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a);
}

void init_playground() {

    // dft(y);

    
}

void update_playground() {

}

int map(int val, int min_from, int max_from, int min_target, int max_target) {
    return((int) ((((float)val - min_from) / (max_from - min_from)) * (float) (max_target - min_target) + min_target));
}

void push_curv_point(SDL_Point * arr, double y, int x_offset, unsigned int length) {

    SDL_Point new_point;

    new_point.x = x_offset;
    new_point.y = (int) round(y);

    for(unsigned int i = length; i > 0; i--) {
        arr[i] = arr[i-1];
        arr[i].x++;
        }

    arr[0] = new_point;
}

int main(int argc, char **argv) {

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    srand( time(NULL) );

    if(SDL_Init( SDL_INIT_VIDEO ) != 0) {
    // if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("ERREUR : Initialisation SDL > %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if(TTF_Init() != 0) {
        SDL_Log("ERREUR : Initialisation TTF > %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Programme
    window = SDL_CreateWindow(  "Fourier Transform Epicycles",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH, WINDOW_HEIGHT,
                                0);
    if(window == NULL) {
        SDL_Log("ERREUR : Echec creation fenetre > %s\n", SDL_GetError());
        // TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    /*------------------------------------------*/

    // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(renderer == NULL) {
        SDL_Log("ERREUR : Echec creation renderer > %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    if(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) != 0) {
        SDL_Log("ERREUR : Echec selection du blend mode pour le renderer > %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Iinitialize the grid
    init_playground();

    unsigned int current_tick, last_tick, next_tick = 0;
    unsigned int dw1, dw2 = 0;

    int counter = 0;

    last_tick = SDL_GetTicks();

    SDL_bool program_launched = SDL_TRUE;
    SDL_Event event;
    SDL_bool flagShowFPS = SDL_FALSE;

    char buffer[32];
    buffer[0] = '\0';

    while(program_launched) {

        while(SDL_PollEvent(&event)) {
            switch(event.type) {

                // case SDL_WINDOWEVENT:

                //     // if(event.window.event == SDL_WINDOWEVENT_ENTER)
                //     //     SDL_ShowCursor(SDL_DISABLE);

                //     // if(event.window.event == SDL_WINDOWEVENT_LEAVE)
                //     //     SDL_ShowCursor(SDL_ENABLE);

                //     break;

                // case SDL_MOUSEMOTION:
                //     break;

                // case SDL_MOUSEBUTTONDOWN:
                //       break;

                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_f:
                            if(flagShowFPS == SDL_FALSE)
                                flagShowFPS = SDL_TRUE;
                            else
                                flagShowFPS = SDL_FALSE;
                            break;
                        default:
                            break;
                    }
                    break;

                case SDL_QUIT:
                    program_launched = SDL_FALSE;
                    break;

                default:
                    break;
            }
        }

        // Clear background
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Set draw color
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 100);

        #define PI 3.14159265
        #define x_offset 350
        #define y_offset 300

        double x = 0.0;
        double y = 0.0;

        int x_circle_offset = 0;
        int y_circle_offset = 0;

        for(double i = 0.0; i < 5.0; i++) {

            double n = i * 2.0 + 1.0;

            double radius = 150.0 * (4.0 / (n * PI));

            x = (double) radius * cos(n * timing);
            y = (double) radius * sin(n * timing);

            SDL_SetRenderDrawColor(renderer, 200, 50, 200, 100);
            SDL_RenderDrawCircle(renderer, x_circle_offset + x_offset, y_circle_offset + y_offset, radius);

            // SDL_Color point_color;
            // point_color.r = 255;
            // point_color.g = 255;
            // point_color.b = 255;
            // point_color.a = 100;
            // SDL_RenderFillCircle(renderer, x_circle_offset + x_offset + (int) x, y_circle_offset + y_offset + (int) y, 3, point_color);

            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderDrawLine(renderer, x_circle_offset + x_offset + 0, y_circle_offset + y_offset + 0, x_circle_offset + x_offset + (int) x, y_circle_offset + y_offset + (int) y);

            x_circle_offset += (int) x;
            y_circle_offset += (int) y;
        }

        x_circle_offset -= (int) x;
        y_circle_offset -= (int) y;

        push_curv_point(graph, y + (double) y_offset + (double) y_circle_offset, x_offset+400, graph_length++);

        if(graph_length > MAX_GRAPH_LENGTH-1)
            graph_length--;

        SDL_Color point_color;
        point_color.r = 255;
        point_color.g = 0;
        point_color.b = 0;
        point_color.a = 100;
        SDL_RenderFillCircle(renderer, x_circle_offset + x_offset + (int) x, y_circle_offset + y_offset + (int) y, 4, point_color);

        SDL_SetRenderDrawColor(renderer, 50, 50, 200, 100);
        SDL_RenderDrawLine(renderer, graph[0].x, graph[0].y, x_circle_offset + x_offset + (int) x, y_circle_offset + y_offset + (int) y);

        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 100);
        SDL_RenderDrawLines(renderer, graph, graph_length);

        // float x, y = 0.0;

        // for(int i=0; i < 5; i++) {
        //     float prevx = x;
        //     float prevy = y;

        //     float n = (float) (i * 2 + 1);
        //     float radius = 75.0 * (4.0 / (n * PI));
        //     x += radius * cos(n * timing);
        //     y += radius * sin(n * timing);

        //     SDL_RenderDrawEllipse(renderer, x_offset+(int)prevx, y_offset+(int)prevy, (int) radius, (int) radius);

        // }

        timing -= 0.012;

        if(flagShowFPS) {

            if(++counter > 10) {
                counter = 0;
                sprintf(buffer, "FPS : %d", 1000 / (dw2 - dw1)); 
            }
            
            showMsg(renderer, buffer, WINDOW_WIDTH - 150, 10, 24);
        }

        SDL_RenderPresent(renderer);

        next_tick = last_tick + FPS_LIMIT;
        current_tick = SDL_GetTicks();
        if(current_tick < next_tick)
            SDL_Delay(next_tick - current_tick);

        dw1 = dw2;
        last_tick = dw2 = SDL_GetTicks();

        update_playground();
    }

    /*------------------------------------------*/

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}

void showMsg(SDL_Renderer *renderer, char *txt, int x, int y, int size) {

    if(txt[0] == '\0')
        return;

    //this opens a font style and sets a size
    TTF_Font* Arial = TTF_OpenFont("../ressources/fonts/arial.ttf", size);
    if(Arial == NULL) {
        SDL_Log("ERREUR : Echec chargement fonte > %s\n", SDL_GetError());
        return;
    }

    // Text color
    SDL_Color White = {255, 255, 255};

    // as TTF_RenderText_Solid could only be used on
    // SDL_Surface then you have to create the surface first
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Arial, txt, White);
    if(surfaceMessage == NULL) {
        SDL_Log("ERREUR : Echec lors de la creation de la \"surface\" > %s\n", SDL_GetError());
    }
    
    TTF_CloseFont(Arial);
    Arial = NULL;

    // now you can convert it into a texture
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    if(Message == NULL) {
        SDL_Log("ERREUR : Echec lors de la creation de la texture > %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surfaceMessage);

    SDL_Rect Message_rect = {x, y, 0, 0}; //create a rect to store texture size
    // Reads the size of the texture
    if(SDL_QueryTexture(Message, NULL, NULL, &Message_rect.w, &Message_rect.h) < 0) {
        SDL_Log("ERREUR : Echec lors de la lecture de la taille de la texture > %s\n", SDL_GetError());
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    Message_rect.x -= 10;
    Message_rect.w += 20;
    SDL_RenderFillRect(renderer, &Message_rect);

    Message_rect.x += 10;
    Message_rect.w -= 20;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    if(SDL_RenderCopy(renderer, Message, NULL, &Message_rect) < 0) {
        SDL_Log("ERREUR : Echec lors du rendu de la texture > %s\n", SDL_GetError());
    }

    // Free texture
    SDL_DestroyTexture(Message);
}