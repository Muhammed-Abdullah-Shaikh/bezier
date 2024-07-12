/* Learning SDL and Creating Bezier Curves 
 * Date: 11 July 2024
 * Author: Muhammed Abdullah
 * Ref: https://www.youtube.com/watch?v=2oKzBq43ShE&list=PLpM-Dvs8t0VY5sYK_mm1k9dZw5tFWgg4L
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SDL.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_FPS 60
#define DELTA_TIME_SEC (1.0f / SCREEN_FPS)
#define DELTA_TIME_MS ((Uint32)floorf(DELTA_TIME_SEC * 1000.0f))
#define MARKER_SIZE 15.0f

#define BACKGROUND_COLOR    0x353535FF
#define RED_COLOR          0xDA2C38FF
#define GREEN_COLOR          0x87C38FFF
#define BLUE_COLOR          0x748CABFF

#define HEX_COLOR(hex)                      \
    ((hex) >> (3 * 8)) & 0xFF,              \
    ((hex) >> (2 * 8)) & 0xFF,              \
    ((hex) >> (1 * 8)) & 0xFF,              \
    ((hex) >> (0 * 8)) & 0xFF

int check_sdl_code(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        exit(1);
    }
    return code;
}

void *check_sdl_ptr(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}


float lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

typedef struct Vec2 
{
    float x;
    float y;
} Vec2;

Vec2 vec2(float x, float y)
{
    return (Vec2) {x, y};
}

Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return vec2(a.x + b.x, a.y + b.y);
}

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return vec2(a.x - b.x, a.y - b.y);
}

Vec2 vec2_scale(Vec2 a, float s)
{
    return vec2(a.x * s, a.y * s);
}

Vec2 lerpv2(Vec2 a, Vec2 b, float t)
{
    return vec2_add(a, vec2_scale(vec2_sub(b, a), t));
}

typedef union Color
{
    uint32_t  hex_color;
    SDL_Color color;
} Color;

void render_line(SDL_Renderer *renderer, Vec2 begin, Vec2 end, Color color)
{

    check_sdl_code(
            SDL_SetRenderDrawColor(renderer, HEX_COLOR(color.hex_color)));

    check_sdl_code(
        SDL_RenderDrawLineF(
            renderer, 
            (int) floorf(begin.x), 
            (int) floorf(begin.y), 
            (int) floorf(end.x), 
            (int) floorf(end.y)
    ));

}

void fill_rect(SDL_Renderer *renderer, Vec2 pos, Vec2 size, Color color)
{
    check_sdl_code(
            SDL_SetRenderDrawColor(renderer, HEX_COLOR(color.hex_color))
    );

    const SDL_Rect rect = (SDL_Rect) {
        (int) floorf(pos.x), 
        (int) floorf(pos.y), 
        (int) floorf(size.x), 
        (int) floorf(size.y)
    };
    check_sdl_code(SDL_RenderFillRect(renderer, &rect));
}

/* Will draw a rectangle with the position as the center
 * @param renderer : SDL_Renderer pointer 
 * @param position : Vec2
 * @param color : Color
 */
void render_marker(SDL_Renderer *renderer, Vec2 position, Color color)
{
    const Vec2 size = vec2(MARKER_SIZE, MARKER_SIZE);
    fill_rect(
        renderer, 
        vec2_sub(position, vec2_scale(size, 0.5f)), 
        size, 
        color
    );

}

/**
 * Draws Bezier curve from 4 points a,b,c,d
 */
void render_bezier_markers(SDL_Renderer *renderer, 
        Vec2 a, Vec2 b, Vec2 c, Vec2 d,
        float s, Color color)
{
    for (float p = 0.0f+s; p <= 1.0f; p += s)
    {
        const Vec2 ab = lerpv2(a, b, p);
        const Vec2 bc = lerpv2(b, c, p);
        const Vec2 cd = lerpv2(c, d, p);
        const Vec2 abc = lerpv2(ab, bc, p);
        const Vec2 bcd = lerpv2(bc, cd, p);
        const Vec2 abcd = lerpv2(abc, bcd, p);

        render_marker(renderer, abcd, color);

    }
}

#define PS_CAPACITY 256

Vec2 ps[PS_CAPACITY];
int ps_count = 0;
int ps_selected = -1;

/** 
 * Take a position and check if there is marker there
 * @param pos : Vec2
 * @return index of the marker 
 */
int ps_at(Vec2 pos)
{
    const Vec2 ps_size = vec2(MARKER_SIZE, MARKER_SIZE);
    for (int i = 0; i < ps_count; i++)
    {
        const Vec2 ps_begin = vec2_sub(ps[i], vec2_scale(ps_size, 0.5f));
        const Vec2 ps_end = vec2_add(ps_begin, ps_size);

        if (pos.x >= ps_begin.x && pos.x <= ps_end.x
            && pos.y >= ps_begin.y && pos.y <= ps_end.y)
        {
            return i;
        }
    }
    return -1;
}


int main(int argc, char *argv[])
{
    check_sdl_code(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window * const window = SDL_CreateWindow(
            "Bezier Curves",
            340, 150,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_RESIZABLE);

    SDL_Renderer * const renderer = 
        check_sdl_ptr(
                SDL_CreateRenderer(
                    window, -1, SDL_RENDERER_ACCELERATED));

    check_sdl_code(SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT));

    float t = 0.0f;
    int quit = 0;
    while(!quit)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                
                case SDL_MOUSEBUTTONDOWN:
                    switch (event.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            ;const Vec2 mouse_pos = vec2(event.button.x, event.button.y);
                            if (ps_count < 4)
                                ps[ps_count++] = mouse_pos;
                            else
                                ps_selected = ps_at(mouse_pos);
                            break;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    ;Vec2 mouse_pos = vec2(event.motion.x, event.motion.y);
                    if (ps_selected >= 0)
                    {
                        ps[ps_selected] = mouse_pos;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        ps_selected = -1;
                    }
                    break;
            }
        }

        check_sdl_code(SDL_SetRenderDrawColor(
                renderer,
                HEX_COLOR(BACKGROUND_COLOR)));

        check_sdl_code(SDL_RenderClear(renderer));

        
        for (int i = 0; ps_count > 0 && i < ps_count; i++)
        {
            render_marker(renderer, ps[i], (Color){RED_COLOR});
        }

        if (ps_count >= 4)
        {
            render_line(renderer, ps[0], ps[1], (Color){RED_COLOR});
            render_line(renderer, ps[2], ps[3], (Color){RED_COLOR});

            render_bezier_markers(renderer, ps[0], ps[1], ps[2], ps[3], 0.05f, (Color){GREEN_COLOR});
        }

        //const float p = (sinf(t) + 1) * 0.5f;

         //Phase 1
        //for (int i = 0; i < ps_count - 1; i++)
        //{
            //render_marker(renderer, ps[i], (Color){RED_COLOR});
        //}

         //Phase 2
        //for (int i = 0; ps_count > 0 && i < ps_count - 1; i++)
        //{
           //render_marker(renderer, lerpv2(ps[i], ps[i + 1], p), (Color){GREEN_COLOR});
        //}

         //Phase 3
        //for (int i = 0; ps_count > 1 && i < ps_count - 2; i++)
        //{
            //const Vec2 a = lerpv2(ps[i], ps[i + 1], p);
            //const Vec2 b = lerpv2(ps[i + 1], ps[i + 2], p);

            //render_marker(renderer, lerpv2(a, b, p), (Color){BLUE_COLOR});
        //}

        SDL_RenderPresent(renderer);

        SDL_Delay(DELTA_TIME_MS);
        t += DELTA_TIME_SEC;

    }



    SDL_Quit();

    return 0;
}
