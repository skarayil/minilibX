#include "../minilibx/mlx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 800
#define ASTEROID_COUNT 8
#define SHIP_SIZE 25
#define ASTEROID_SIZE 35
#define MAX_SPEED 2.5
#define MAX_LASERS 50
#define LASER_SPEED 12
#define GAME_TIME 60
#define PARTICLE_COUNT 200
#define STAR_COUNT 80
#define COMET_COUNT 3
#define KEY_SPACE 49
#define KEY_L_SHIFT 257

float   g_bg_scroll = 0;

typedef struct s_particle
{
    float   x;
    float   y;
    float   vx;
    float   vy;
    int     life;
    int     color;
}   t_particle;

typedef struct s_star
{
    float   x;
    float   y;
    float   vx;
    float   vy;
    int     brightness;
    int     trail_length;
}   t_star;

typedef struct s_comet
{
    float   x;
    float   y;
    float   vx;
    float   vy;
    float   angle;
    int     size;
    int     active;
}   t_comet;

typedef struct s_obj
{
    float   x;
    float   y;
    float   vx;
    float   vy;
    float   angle;
    int     active;
}   t_obj;

typedef struct s_app
{
    void        *mlx;
    void        *win;
    void        *img;
    char        *data;
    int         bpp;
    int         sl;
    int         endian;
    void        *ship_img;
    char        *ship_data;
    int         ship_w;
    int         ship_h;
    int         ship_bpp;
    int         ship_sl;
    int         ship_endian;
    t_obj       ship;
    t_obj       asteroids[ASTEROID_COUNT];
    t_obj       lasers[MAX_LASERS];
    t_particle  particles[PARTICLE_COUNT];
    t_star      stars[STAR_COUNT];
    t_comet     comets[COMET_COUNT];
    int         score;
    time_t      start_time;
    time_t      real_start_time;
    int         time_survived;
    int         game_over;
    int         frame;
    float       ship_glow;
    int         comet_death;
    int         shield;
}   t_app;

void    draw_centered_text(t_app *a, int y, int color, char *text)
{
    int len;
    int x;

    len = strlen(text);
    x = (WIDTH / 2) - ((len * 7) / 2);
    mlx_string_put(a->mlx, a->win, x, y, color, text);
}

void    put_pixel(t_app *a, int x, int y, int color)
{
    int     offset;
    char    *dst;

    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return ;
    offset = y * a->sl + x * (a->bpp / 8);
    dst = a->data + offset;
    memcpy(dst, &color, sizeof(int));
}

unsigned int    get_pixel_color(t_app *a, int x, int y)
{
    char            *dst;
    unsigned int    color;

    if (x < 0 || x >= a->ship_w || y < 0 || y >= a->ship_h)
        return (0xFF000000);
    dst = a->ship_data + (y * a->ship_sl + x * (a->ship_bpp / 8));
    memcpy(&color, dst, sizeof(unsigned int));
    return (color);
}

void    draw_background_gradient(t_app *a)
{
    int     x_coord;
    int     y_coord;
    int     color;
    float   vars[4];

    g_bg_scroll += 2.5;
    if (g_bg_scroll > HEIGHT)
        g_bg_scroll = 0;
    y_coord = 0;
    while (y_coord < HEIGHT)
    {
        vars[0] = ((float)y_coord + g_bg_scroll) / (float)HEIGHT;
        vars[3] = 1.0f - fabsf(2.0f * (vars[0] - floorf(vars[0])) - 1.0f);
        vars[1] = vars[3] * vars[3] * (3.0f - 2.0f * vars[3]);
        x_coord = 0;
        while (x_coord < WIDTH)
        {
            vars[2] = 0.5f + 0.5f * sinf(0.045f * (float)x_coord + 0.013f
                    * (float)y_coord + 0.01f * a->frame);
            vars[0] = vars[1] + (vars[2] - 0.5f) * 0.05f;
            if (vars[0] < 0.0f)
                vars[0] = 0.0f;
            if (vars[0] > 1.0f)
                vars[0] = 1.0f;
            color = ((int)(10 * vars[0]) << 16) | ((int)(15 * vars[0]) << 8)
                | (int)(43 * vars[0]);
            put_pixel(a, x_coord, y_coord, color);
            x_coord++;
        }
        y_coord++;
    }
}

void    draw_star_trails(t_app *a, int i, int x, int y, int brightness)
{
    int t_trail;
    int tx;
    int ty;
    int tb;
    int tc;

    t_trail = 1;
    while (t_trail < a->stars[i].trail_length)
    {
        tx = x - (int)(a->stars[i].vx * t_trail * 2);
        ty = y - (int)(a->stars[i].vy * t_trail * 2);
        tb = brightness * (a->stars[i].trail_length - t_trail)
            / a->stars[i].trail_length;
        tc = (tb << 16) | (tb << 8) | tb;
        put_pixel(a, tx, ty, tc);
        t_trail++;
    }
}

void    draw_stars(t_app *a)
{
    int i;
    int x;
    int y;
    int brightness;
    int color;

    i = 0;
    while (i < STAR_COUNT)
    {
        x = (int)a->stars[i].x;
        y = (int)a->stars[i].y;
        brightness = a->stars[i].brightness + (int)(sin(a->frame * 0.05 + i)
                * 30);
        if (brightness > 255)
            brightness = 255;
        if (brightness < 0)
            brightness = 0;
        color = (brightness << 16) | (brightness << 8) | brightness;
        put_pixel(a, x, y, color);
        draw_star_trails(a, i, x, y, brightness);
        i++;
    }
}

void    clear_image(t_app *a)
{
    draw_background_gradient(a);
    draw_stars(a);
}

void    draw_filled_circle(t_app *a, int cx, int cy, int r, int color)
{
    int x;
    int y;

    y = -r;
    while (y <= r)
    {
        x = -r;
        while (x <= r)
        {
            if (x * x + y * y <= r * r)
                put_pixel(a, cx + x, cy + y, color);
            x++;
        }
        y++;
    }
}

void    draw_circle(t_app *a, int cx, int cy, int r, int color)
{
    float   dist;
    int     x;
    int     y;

    y = -r;
    while (y <= r)
    {
        x = -r;
        while (x <= r)
        {
            dist = sqrt(x * x + y * y);
            if (dist >= r - 2 && dist <= r)
                put_pixel(a, cx + x, cy + y, color);
            x++;
        }
        y++;
    }
}

void    init_comet(t_app *a, int i)
{
    int side;

    a->comets[i].active = 1;
    a->comets[i].size = 55;
    side = rand() % 4;
    if (side == 0)
    {
        a->comets[i].x = rand() % WIDTH;
        a->comets[i].y = -50;
        a->comets[i].vx = ((rand() % 100) / 50.0 - 1.0) * 2;
        a->comets[i].vy = 2 + (rand() % 30) / 10.0;
    }
    else if (side == 1)
    {
        a->comets[i].x = WIDTH + 50;
        a->comets[i].y = rand() % HEIGHT;
        a->comets[i].vx = -2 - (rand() % 30) / 10.0;
        a->comets[i].vy = ((rand() % 100) / 50.0 - 1.0) * 2;
    }
    else if (side == 2)
    {
        a->comets[i].x = rand() % WIDTH;
        a->comets[i].y = HEIGHT + 50;
        a->comets[i].vx = ((rand() % 100) / 50.0 - 1.0) * 2;
        a->comets[i].vy = -2 - (rand() % 30) / 10.0;
    }
    else
    {
        a->comets[i].x = -50;
        a->comets[i].y = rand() % HEIGHT;
        a->comets[i].vx = 2 + (rand() % 30) / 10.0;
        a->comets[i].vy = ((rand() % 100) / 50.0 - 1.0) * 2;
    }
    a->comets[i].angle = atan2(a->comets[i].vy, a->comets[i].vx);
}

void    update_comets(t_app *a)
{
    int i;

    i = 0;
    while (i < COMET_COUNT)
    {
        if (a->comets[i].active)
        {
            a->comets[i].x += a->comets[i].vx;
            a->comets[i].y += a->comets[i].vy;
            if (a->comets[i].x < -100 || a->comets[i].x > WIDTH + 100
                || a->comets[i].y < -100 || a->comets[i].y > HEIGHT + 100)
                a->comets[i].active = 0;
        }
        else
        {
            if (rand() % 300 == 0)
                init_comet(a, i);
        }
        i++;
    }
}

void    draw_one_comet(t_app *a, int i)
{
    int     t;
    int     vars[3];
    float   f;
    float   tf;
    int     col;

    t = 0;
    while (t < 60)
    {
        f = (60.0f - t) / 60.0f;
        vars[0] = (int)a->comets[i].x - (int)(a->comets[i].vx * t * 3.5);
        vars[1] = (int)a->comets[i].y - (int)(a->comets[i].vy * t * 3.5);
        vars[2] = (int)(a->comets[i].size / 2.5 * f);
        if (vars[2] < 1)
            vars[2] = 1;
        tf = (float)t / 59.0f;
        col = ((int)(255 * f) << 16) | ((int)((255 - (int)(100 * tf)) * f) << 8)
            | (int)((255.0f * tf) * f);
        draw_filled_circle(a, vars[0], vars[1], vars[2], col);
        t++;
    }
    draw_filled_circle(a, (int)a->comets[i].x, (int)a->comets[i].y,
        a->comets[i].size / 2, 0xFEFB2B);
    draw_filled_circle(a, (int)a->comets[i].x, (int)a->comets[i].y,
        a->comets[i].size / 3, 0xFCFB93);
    draw_filled_circle(a, (int)a->comets[i].x, (int)a->comets[i].y,
        a->comets[i].size / 5, 0xFFFFFF);
}

void    draw_comets(t_app *a)
{
    int i;

    i = 0;
    while (i < COMET_COUNT)
    {
        if (a->comets[i].active)
            draw_one_comet(a, i);
        i++;
    }
}

void    draw_shield(t_app *a, int cx, int cy)
{
    int pulse;
    int r;
    int g;
    int b;
    int col;
    int inner;

    pulse = (int)(sin(a->frame * 0.15) * 25);
    r = 155 + pulse;
    g = 175 + pulse;
    b = 255;
    col = (r << 16) | (g << 8) | b;
    draw_circle(a, cx, cy, (int)(SHIP_SIZE * 1.7), col);
    draw_circle(a, cx, cy, (int)(SHIP_SIZE * 1.6), col);
    inner = 100 + pulse;
    col = (inner << 16) | (inner << 8) | 200;
    draw_circle(a, cx, cy, (int)(SHIP_SIZE * 1.5), col);
}

void    draw_ship_body(t_app *a, int cx, int cy)
{
    int             y;
    int             x;
    int             s_x;
    int             s_y;
    unsigned int    c;

    y = -((a->ship_w > a->ship_h ? a->ship_w : a->ship_h) * 1.5) / 2;
    while (y < ((a->ship_w > a->ship_h ? a->ship_w : a->ship_h) * 1.5) / 2)
    {
        x = -((a->ship_w > a->ship_h ? a->ship_w : a->ship_h) * 1.5) / 2;
        while (x < ((a->ship_w > a->ship_h ? a->ship_w : a->ship_h) * 1.5) / 2)
        {
            s_x = (int)(x + a->ship_w / 2.0);
            s_y = (int)(y + a->ship_h / 2.0);
            if (s_x >= 0 && s_x < a->ship_w && s_y >= 0 && s_y < a->ship_h)
            {
                c = get_pixel_color(a, s_x, s_y);
                if ((c & 0x00FFFFFF) != 0)
                    put_pixel(a, cx + x, cy + y, c);
            }
            x++;
        }
        y++;
    }
}

void    draw_ship(t_app *a)
{
    int cx;
    int cy;

    cx = (int)a->ship.x;
    cy = (int)a->ship.y;
    if (a->shield)
        draw_shield(a, cx, cy);
    if (a->ship_img)
        draw_ship_body(a, cx, cy);
}

void    add_particle(t_app *a, float x, float y, int color)
{
    int i;

    i = 0;
    while (i < PARTICLE_COUNT)
    {
        if (a->particles[i].life <= 0)
        {
            a->particles[i].x = x;
            a->particles[i].y = y;
            a->particles[i].vx = ((rand() % 200) / 100.0 - 1.0) * 5;
            a->particles[i].vy = ((rand() % 200) / 100.0 - 1.0) * 5;
            a->particles[i].life = 30 + rand() % 30;
            a->particles[i].color = color;
            break ;
        }
        i++;
    }
}

void    update_particles(t_app *a)
{
    int i;

    i = 0;
    while (i < PARTICLE_COUNT)
    {
        if (a->particles[i].life > 0)
        {
            a->particles[i].x += a->particles[i].vx;
            a->particles[i].y += a->particles[i].vy;
            a->particles[i].vy += 0.1;
            a->particles[i].life--;
        }
        i++;
    }
}

void    draw_particles(t_app *a)
{
    int i;
    int alpha;
    int color;
    int size;

    i = 0;
    while (i < PARTICLE_COUNT)
    {
        if (a->particles[i].life > 0)
        {
            alpha = (a->particles[i].life * 255) / 60;
            color = ((((a->particles[i].color >> 16) & 0xFF) * alpha / 255)
                    << 16) | ((((a->particles[i].color >> 8) & 0xFF)
                        * alpha / 255) << 8) | ((a->particles[i].color & 0xFF)
                    * alpha / 255);
            size = 3 + (a->particles[i].life / 15);
            draw_filled_circle(a, (int)a->particles[i].x,
                (int)a->particles[i].y, size, color);
        }
        i++;
    }
}

void    update_asteroids(t_app *a)
{
    int i;

    i = 0;
    while (i < ASTEROID_COUNT)
    {
        a->asteroids[i].x += a->asteroids[i].vx;
        a->asteroids[i].y += a->asteroids[i].vy;
        if (a->asteroids[i].x < 0)
            a->asteroids[i].x = WIDTH;
        if (a->asteroids[i].x > WIDTH)
            a->asteroids[i].x = 0;
        if (a->asteroids[i].y < 0)
            a->asteroids[i].y = HEIGHT;
        if (a->asteroids[i].y > HEIGHT)
            a->asteroids[i].y = 0;
        i++;
    }
}

void    draw_asteroids(t_app *a)
{
    int i;
    int r;
    int brightness;
    int color;

    i = 0;
    while (i < ASTEROID_COUNT)
    {
        draw_filled_circle(a, (int)a->asteroids[i].x + 4,
            (int)a->asteroids[i].y + 4, ASTEROID_SIZE / 2, 0x0A0A0A);
        r = ASTEROID_SIZE / 2;
        while (r > 0)
        {
            brightness = 30 + (r * 60 / (ASTEROID_SIZE / 2));
            color = (brightness << 16) | (brightness << 8) | brightness;
            draw_filled_circle(a, (int)a->asteroids[i].x,
                (int)a->asteroids[i].y, r, color);
            r -= 2;
        }
        draw_filled_circle(a, (int)a->asteroids[i].x - 7,
            (int)a->asteroids[i].y - 5, 5, 0x151515);
        draw_filled_circle(a, (int)a->asteroids[i].x + 6,
            (int)a->asteroids[i].y + 6, 4, 0x181818);
        i++;
    }
}

void    update_lasers(t_app *a)
{
    int i;

    i = 0;
    while (i < MAX_LASERS)
    {
        if (a->lasers[i].active)
        {
            a->lasers[i].x += a->lasers[i].vx;
            a->lasers[i].y += a->lasers[i].vy;
            if (a->lasers[i].x < 0 || a->lasers[i].x > WIDTH
                || a->lasers[i].y < 0 || a->lasers[i].y > HEIGHT)
                a->lasers[i].active = 0;
        }
        i++;
    }
}

void    draw_lasers(t_app *a)
{
    int i;
    int t;
    int tx;
    int ty;

    i = 0;
    while (i < MAX_LASERS)
    {
        if (a->lasers[i].active)
        {
            draw_filled_circle(a, (int)a->lasers[i].x, (int)a->lasers[i].y,
                6, 0xFFAA00);
            t = 1;
            while (t < 5)
            {
                tx = (int)a->lasers[i].x - (int)(a->lasers[i].vx * t * 0.5);
                ty = (int)a->lasers[i].y - (int)(a->lasers[i].vy * t * 0.5);
                draw_filled_circle(a, tx, ty, 5 - t, 0xFFFF00);
                t++;
            }
        }
        i++;
    }
}

void    handle_collision(t_app *a, int i, int j)
{
    int k;

    a->lasers[i].active = 0;
    k = 0;
    while (k < 20)
    {
        add_particle(a, a->asteroids[j].x, a->asteroids[j].y, 0xFFAA00);
        k++;
    }
    a->asteroids[j].x = rand() % WIDTH;
    a->asteroids[j].y = rand() % HEIGHT;
    a->asteroids[j].vx = ((rand() % 200) / 100.0 - 1.0) * MAX_SPEED;
    a->asteroids[j].vy = ((rand() % 200) / 100.0 - 1.0) * MAX_SPEED;
    a->start_time += 3;
}

void    check_collisions(t_app *a)
{
    int     i;
    int     j;
    float   dist;

    i = 0;
    while (i < MAX_LASERS)
    {
        if (a->lasers[i].active)
        {
            j = 0;
            while (j < ASTEROID_COUNT)
            {
                dist = sqrt(pow(a->lasers[i].x - a->asteroids[j].x, 2)
                        + pow(a->lasers[i].y - a->asteroids[j].y, 2));
                if (dist < ASTEROID_SIZE / 2)
                    handle_collision(a, i, j);
                j++;
            }
        }
        i++;
    }
}

void    handle_comet_hit(t_app *a, int i)
{
    int k;

    if (!a->shield)
    {
        a->game_over = 1;
        a->comet_death = 1;
        a->time_survived = (int)difftime(time(NULL), a->real_start_time);
        k = 0;
        while (k < 100)
        {
            add_particle(a, a->ship.x, a->ship.y, 0xFFAA00);
            k++;
        }
    }
    else
    {
        a->comets[i].active = 0;
        a->start_time += 5;
        k = 0;
        while (k < 30)
        {
            add_particle(a, a->comets[i].x, a->comets[i].y, 0x00FFFF);
            k++;
        }
    }
}

void    check_comet_collision(t_app *a)
{
    int     i;
    float   dist;

    i = 0;
    while (i < COMET_COUNT)
    {
        if (a->comets[i].active)
        {
            dist = sqrt(pow(a->ship.x - a->comets[i].x, 2)
                    + pow(a->ship.y - a->comets[i].y, 2));
            if (dist < (SHIP_SIZE + a->comets[i].size / 2) && !a->game_over)
                handle_comet_hit(a, i);
        }
        i++;
    }
}

void    reset_game(t_app *a)
{
    int i;

    a->ship.x = WIDTH / 2;
    a->ship.y = HEIGHT * 0.75;
    a->ship.angle = 0;
    i = 0;
    while (i < ASTEROID_COUNT)
    {
        a->asteroids[i].x = rand() % WIDTH;
        a->asteroids[i].y = rand() % HEIGHT;
        a->asteroids[i].vx = ((rand() % 200) / 100.0 - 1.0) * MAX_SPEED;
        a->asteroids[i].vy = ((rand() % 200) / 100.0 - 1.0) * MAX_SPEED;
        i++;
    }
    i = 0;
    while (i < MAX_LASERS)
        a->lasers[i++].active = 0;
    i = 0;
    while (i < COMET_COUNT)
        a->comets[i++].active = 0;
    i = 0;
    while (i < PARTICLE_COUNT)
        a->particles[i++].life = 0;
    a->score = 0;
    a->start_time = time(NULL);
    a->real_start_time = time(NULL);
    a->game_over = 0;
    a->comet_death = 0;
    a->shield = 0;
}

void    fire_laser(t_app *a)
{
    int i;

    i = 0;
    while (i < MAX_LASERS)
    {
        if (!a->lasers[i].active)
        {
            a->lasers[i].active = 1;
            a->lasers[i].x = a->ship.x;
            a->lasers[i].y = a->ship.y - SHIP_SIZE;
            a->lasers[i].vx = 0;
            a->lasers[i].vy = -LASER_SPEED;
            add_particle(a, a->lasers[i].x, a->lasers[i].y, 0xFFFF00);
            break ;
        }
        i++;
    }
}

int key_press(int key, void *param)
{
    t_app   *a;

    a = (t_app *)param;
    if (key == KEY_SPACE && a->game_over)
    {
        reset_game(a);
        return (0);
    }
    if (a->game_over)
        return (0);
    if (key == KEY_L_SHIFT)
        a->shield = 1;
    if (key == KEY_SPACE)
        fire_laser(a);
    return (0);
}

int key_release(int key, void *param)
{
    t_app   *a;

    a = (t_app *)param;
    if (key == KEY_L_SHIFT)
        a->shield = 0;
    return (0);
}

int close_window(void *param)
{
    (void)param;
    exit(0);
    return (0);
}

void    draw_hud(t_app *a, int remaining)
{
    char    buf[256];
    int     hud_x;
    int     hud_y_base;
    int     time_color;

    hud_x = WIDTH - 150;
    hud_y_base = HEIGHT - 80;
    sprintf(buf, "Skor: %d sn", a->score);
    mlx_string_put(a->mlx, a->win, hud_x, hud_y_base, 0x00FFFF, buf);
    sprintf(buf, "Kalan Sure: %d", remaining);
    if (remaining < 10)
        time_color = 0xFF3333;
    else
        time_color = 0xFFFF00;
    mlx_string_put(a->mlx, a->win, hud_x, hud_y_base + 20, time_color, buf);
}

void    draw_game_over_box(t_app *a)
{
    int y;
    int x;

    y = HEIGHT / 2 - 120;
    while (y < HEIGHT / 2 + 180)
    {
        x = WIDTH / 2 - 280;
        while (x < WIDTH / 2 + 280)
        {
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
                put_pixel(a, x, y, 0x0A0020);
            x++;
        }
        y++;
    }
}

void    draw_game_over_screen(t_app *a)
{
    char    buf[256];

    draw_game_over_box(a);
    mlx_put_image_to_window(a->mlx, a->win, a->img, 0, 0);
    if (a->comet_death)
        draw_centered_text(a, HEIGHT / 2 - 70, 0xFF4444,
            "KUYRUKLU YILDIZA CARPTIN!");
    else
        draw_centered_text(a, HEIGHT / 2 - 70, 0xFF4444, "OYUN BITTI!");
    sprintf(buf, "Hayatta Kalma Suresi: %d sn", a->score);
    draw_centered_text(a, HEIGHT / 2 - 30, 0xFFFF00, buf);
    draw_centered_text(a, HEIGHT / 2 + 50, 0xFFFFFF,
        "Yeniden Baslat: SPACE");
    draw_centered_text(a, HEIGHT / 2 + 120, 0xFFFFFF,
        "Sude Naz Karayildirim");
    draw_centered_text(a, HEIGHT / 2 + 140, 0xAAAAAA,
        "github.com/skarayil");
}

int update(void *param)
{
    t_app   *a;
    int     elapsed;
    int     remaining;

    a = (t_app *)param;
    elapsed = (int)difftime(time(NULL), a->start_time);
    remaining = GAME_TIME - elapsed;
    if (!a->game_over)
        a->score = (int)difftime(time(NULL), a->real_start_time);
    if (remaining <= 0 && !a->game_over)
    {
        a->game_over = 1;
        remaining = 0;
        a->time_survived = a->score;
    }
    a->frame++;
    clear_image(a);
    if (!a->game_over)
    {
        update_asteroids(a);
        update_lasers(a);
        update_comets(a);
        check_collisions(a);
        check_comet_collision(a);
        update_particles(a);
    }
    draw_comets(a);
    draw_particles(a);
    draw_asteroids(a);
    if (!a->comet_death)
        draw_ship(a);
    draw_lasers(a);
    if (!a->game_over)
    {
        mlx_put_image_to_window(a->mlx, a->win, a->img, 0, 0);
        draw_hud(a, remaining);
    }
    else
        draw_game_over_screen(a);
    return (0);
}

void    print_instructions(void)
{
    printf("\n--- SPACE ASTEROIDS ---\n\n");
    printf("  Amac: Mumkun oldugunca uzun sure hayatta kal!\n");
    printf("  Skor: Gecirdigin sureye gore artar.\n");
    printf("  Asteroidleri Vur: Surene +3 Saniye eklenir.\n");
    printf("  \n");
    printf("  Kontroller:\n");
    printf("  Space: Ates Et\n");
    printf("  Sol Shift: Kalkan\n");
    printf("----------------------------------\n\n");
}

void    init_stars(t_app *a)
{
    int i;

    i = 0;
    while (i < STAR_COUNT)
    {
        a->stars[i].x = rand() % WIDTH;
        a->stars[i].y = rand() % HEIGHT;
        a->stars[i].vx = ((rand() % 100) / 100.0 - 0.5) * 0.5;
        a->stars[i].vy = ((rand() % 100) / 100.0 - 0.5) * 0.5;
        a->stars[i].brightness = 100 + rand() % 155;
        a->stars[i].trail_length = 3 + rand() % 5;
        i++;
    }
}

int main(void)
{
    t_app   a;

    srand(time(NULL));
    print_instructions();
    a.mlx = mlx_init();
    if (!a.mlx)
        return (1);
    a.win = mlx_new_window(a.mlx, WIDTH, HEIGHT, "Asteroids");
    if (!a.win)
        return (1);
    a.img = mlx_new_image(a.mlx, WIDTH, HEIGHT);
    if (!a.img)
        return (1);
    a.data = mlx_get_data_addr(a.img, &a.bpp, &a.sl, &a.endian);
    a.ship_img = mlx_xpm_file_to_image(a.mlx, "asteroids/rocket.xpm", &a.ship_w,
            &a.ship_h);
    if (a.ship_img)
        a.ship_data = mlx_get_data_addr(a.ship_img, &a.ship_bpp, &a.ship_sl,
                &a.ship_endian);
    else
        printf("Uyari: rocket.xpm bulunamadi, gemi cizilemeyebilir.\n");
    init_stars(&a);
    reset_game(&a);
    mlx_hook(a.win, 17, 0, close_window, NULL);
    mlx_hook(a.win, 2, (1L << 0), key_press, &a);
    mlx_hook(a.win, 3, (1L << 1), key_release, &a);
    mlx_loop_hook(a.mlx, update, &a);
    mlx_loop(a.mlx);
    return (0);
}