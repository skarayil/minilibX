#include "../minilibx/mlx.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 600
#define HEIGHT 600
#define SIZE 30
#define GAME_DURATION 30
#define MAX_PARTICLES 200
#define MAX_STARS 100

typedef struct s_particle
{
	float		x;
	float		y;
	float		vx;
	float		vy;
	int			life;
	int			color;
}				t_particle;

typedef struct s_star
{
	int			x;
	int			y;
	int			brightness;
	int			twinkle_speed;
}				t_star;

typedef struct s_app
{
	void		*mlx;
	void		*win;
	void		*img;
	char		*data;
	int			bpp;
	int			sl;
	int			endian;
	float		x;
	float		y;
	float		vx;
	float		vy;
	int			speed;
	int			color;
	int			score;
	time_t		start_time;
	int			game_over;
	t_particle	particles[MAX_PARTICLES];
	int			particle_count;
	t_star		stars[MAX_STARS];
	int			combo;
	time_t		last_catch_time;
	float		glow_pulse;
	int			trail_positions[20][2];
	int			trail_index;
}				t_app;

void	put_pixel(t_app *a, int x, int y, int color)
{
	int	offset;

	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
		return ;
	offset = y * a->sl + x * (a->bpp / 8);
	*(uint32_t *)(a->data + offset) = color;
}

int	blend_colors(int c1, int c2, float ratio)
{
	int	r1;
	int	g1;
	int	b1;
	int	r2;
	int	g2;
	int	b2;
	int	r;
	int	g;
	int	b;

	r1 = (c1 >> 16) & 0xFF;
	g1 = (c1 >> 8) & 0xFF;
	b1 = c1 & 0xFF;
	r2 = (c2 >> 16) & 0xFF;
	g2 = (c2 >> 8) & 0xFF;
	b2 = c2 & 0xFF;
	r = r1 + (r2 - r1) * ratio;
	g = g1 + (g2 - g1) * ratio;
	b = b1 + (b2 - b1) * ratio;
	return ((r << 16) | (g << 8) | b);
}

int	get_rainbow_color(float t)
{
	int	r;
	int	g;
	int	b;

	r = (sin(t) * 127 + 128);
	g = (sin(t + 2.094) * 127 + 128);
	b = (sin(t + 4.188) * 127 + 128);
	return ((r << 16) | (g << 8) | b);
}

void	init_stars(t_app *a)
{
	for (int i = 0; i < MAX_STARS; i++)
	{
		a->stars[i].x = rand() % WIDTH;
		a->stars[i].y = rand() % HEIGHT;
		a->stars[i].brightness = rand() % 150 + 50;
		a->stars[i].twinkle_speed = rand() % 3 + 1;
	}
}

void	draw_stars(t_app *a)
{
	static int	frame = 0;
	int			brightness;
	int			color;

	frame++;
	for (int i = 0; i < MAX_STARS; i++)
	{
		brightness = a->stars[i].brightness + sin((frame + i * 10) * 0.05
				* a->stars[i].twinkle_speed) * 50;
		if (brightness < 0)
			brightness = 0;
		if (brightness > 255)
			brightness = 255;
		color = (brightness << 16) | (brightness << 8) | brightness;
		put_pixel(a, a->stars[i].x, a->stars[i].y, color);
		if (brightness > 200)
		{
			put_pixel(a, a->stars[i].x + 1, a->stars[i].y, color >> 1);
			put_pixel(a, a->stars[i].x - 1, a->stars[i].y, color >> 1);
			put_pixel(a, a->stars[i].x, a->stars[i].y + 1, color >> 1);
			put_pixel(a, a->stars[i].x, a->stars[i].y - 1, color >> 1);
		}
	}
}

void	clear_image(t_app *a)
{
	memset(a->data, 0, HEIGHT * a->sl);
	draw_stars(a);
}

void	add_particle(t_app *a, float x, float y, int color)
{
	int		idx;
	float	angle;
	float	speed;

	if (a->particle_count >= MAX_PARTICLES)
		a->particle_count = 0;
	for (int i = 0; i < 15; i++)
	{
		idx = (a->particle_count + i) % MAX_PARTICLES;
		angle = (rand() % 360) * M_PI / 180.0;
		speed = (rand() % 100 + 50) / 50.0;
		a->particles[idx].x = x;
		a->particles[idx].y = y;
		a->particles[idx].vx = cos(angle) * speed;
		a->particles[idx].vy = sin(angle) * speed;
		a->particles[idx].life = 30;
		a->particles[idx].color = color;
	}
	a->particle_count += 15;
}

void	update_particles(t_app *a)
{
	float	life_ratio;
	int		color;
	int		r;
	int		g;
	int		b;
	int		faded_color;
	int		size;

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (a->particles[i].life > 0)
		{
			a->particles[i].x += a->particles[i].vx;
			a->particles[i].y += a->particles[i].vy;
			a->particles[i].vy += 0.2;
			a->particles[i].life--;
			life_ratio = a->particles[i].life / 30.0;
			color = a->particles[i].color;
			r = ((color >> 16) & 0xFF) * life_ratio;
			g = ((color >> 8) & 0xFF) * life_ratio;
			b = (color & 0xFF) * life_ratio;
			faded_color = (r << 16) | (g << 8) | b;
			size = (int)(life_ratio * 3) + 1;
			for (int dy = -size; dy <= size; dy++)
				for (int dx = -size; dx <= size; dx++)
					if (dx * dx + dy * dy <= size * size)
						put_pixel(a, (int)a->particles[i].x + dx,
							(int)a->particles[i].y + dy, faded_color);
		}
	}
}

void	draw_glow(t_app *a, int x, int y, int color, int radius)
{
	float	dist;
	float	intensity;
	int		r;
	int		g;
	int		b;
	int		glow_color;

	for (int dy = -radius; dy <= radius; dy++)
	{
		for (int dx = -radius; dx <= radius; dx++)
		{
			dist = sqrt(dx * dx + dy * dy);
			if (dist <= radius)
			{
				intensity = 1.0 - (dist / radius);
				intensity *= intensity;
				r = ((color >> 16) & 0xFF) * intensity;
				g = ((color >> 8) & 0xFF) * intensity;
				b = (color & 0xFF) * intensity;
				glow_color = (r << 16) | (g << 8) | b;
				put_pixel(a, x + dx, y + dy, glow_color);
			}
		}
	}
}

void	draw_square(t_app *a)
{
	static float	pulse_time = 0;
	int				glow_size;
	int				idx;
	float			alpha;
	int				trail_color;
	int				trail_size;
	float			dist;
	float			edge_factor;
	int				highlight;

	pulse_time += 0.1;
	glow_size = SIZE / 2 + 10 + sin(pulse_time) * 5;
	draw_glow(a, (int)a->x, (int)a->y, a->color, glow_size);
	a->trail_positions[a->trail_index][0] = (int)a->x;
	a->trail_positions[a->trail_index][1] = (int)a->y;
	a->trail_index = (a->trail_index + 1) % 20;
	for (int i = 0; i < 20; i++)
	{
		idx = (a->trail_index + i) % 20;
		if (a->trail_positions[idx][0] != 0)
		{
			alpha = i / 20.0;
			trail_color = blend_colors(0x000000, a->color, alpha * 0.3);
			trail_size = SIZE / 4 * alpha;
			for (int dy = -trail_size; dy < trail_size; dy++)
				for (int dx = -trail_size; dx < trail_size; dx++)
					put_pixel(a, a->trail_positions[idx][0] + dx,
						a->trail_positions[idx][1] + dy, trail_color);
		}
	}
	for (int dy = -SIZE / 2; dy < SIZE / 2; dy++)
	{
		for (int dx = -SIZE / 2; dx < SIZE / 2; dx++)
		{
			dist = sqrt(dx * dx + dy * dy);
			if (dist < SIZE / 2)
			{
				edge_factor = 1.0 - (dist / (SIZE / 2));
				highlight = blend_colors(a->color, 0xFFFFFF, edge_factor * 0.3);
				put_pixel(a, (int)a->x + dx, (int)a->y + dy, highlight);
			}
		}
	}
}

int	close_window(void *param)
{
	(void)param;
	exit(0);
}

void	random_direction(t_app *a)
{
	a->vx = ((rand() % 200) - 100) / 100.0;
	a->vy = ((rand() % 200) - 100) / 100.0;
	if (fabs(a->vx) < 0.2)
		a->vx = 0.3;
	if (fabs(a->vy) < 0.2)
		a->vy = 0.3;
}

int	mouse_click(int button, int mx, int my, void *param)
{
	t_app	*a;
	time_t	now;

	a = (t_app *)param;
	if (a->game_over || button != 1)
		return (0);
	if (fabs(mx - a->x) < SIZE / 2 && fabs(my - a->y) < SIZE / 2)
	{
		now = time(NULL);
		if (now - a->last_catch_time <= 2)
			a->combo++;
		else
			a->combo = 1;
		a->last_catch_time = now;
		a->color = get_rainbow_color((float)rand() / RAND_MAX * 6.28);
		add_particle(a, a->x, a->y, a->color);
		if (a->speed < 20)
			a->speed += 1;
		a->score += a->combo;
		printf("💥 Yakalandı! Skor: %d  Hız: %d  Combo: x%d\n", a->score,
			a->speed, a->combo);
	}
	return (0);
}

void	draw_outlined_text(t_app *a, int x, int y, int color, char *str)
{
	mlx_string_put(a->mlx, a->win, x - 1, y, 0x000000, str);
	mlx_string_put(a->mlx, a->win, x + 1, y, 0x000000, str);
	mlx_string_put(a->mlx, a->win, x, y - 1, 0x000000, str);
	mlx_string_put(a->mlx, a->win, x, y + 1, 0x000000, str);
	mlx_string_put(a->mlx, a->win, x, y, color, str);
}

void	draw_bar(t_app *a, int x, int y, int width, int height,
		float percentage, int color)
{
	int	px;
	int	py;

	for (int dy = 0; dy < height; dy++)
		for (int dx = 0; dx < width; dx++)
		{
			px = x + dx;
			py = y + dy;
			if (dy == 0 || dy == height - 1 || dx == 0 || dx == width - 1)
				put_pixel(a, px, py, 0xFFFFFF);
			else if (dx < width * percentage)
				put_pixel(a, px, py, color);
			else
				put_pixel(a, px, py, 0x333333);
		}
}

void	draw_hud(t_app *a)
{
	time_t	now;
	int		remaining;
	char	buf[64];

	now = time(NULL);
	remaining = GAME_DURATION - (int)(now - a->start_time);
	if (remaining < 0)
		remaining = 0;
	sprintf(buf, "SKOR: %d", a->score);
	draw_outlined_text(a, 30, 30, 0x02D0FE, buf);
	if (a->combo > 1)
	{
		sprintf(buf, "COMBO x%d", a->combo);
		draw_outlined_text(a, 30, 60, 0xF601FE, buf);
	}
	draw_bar(a, WIDTH - 220, 20, 200, 20, (float)remaining / GAME_DURATION,
		0x00FF00);
	sprintf(buf, "ZAMAN: %02d", remaining);
	draw_outlined_text(a, WIDTH - 100, 30, 0x00FF00, buf);
	draw_outlined_text(a, WIDTH / 2 - 180, HEIGHT - 25, 0xFFFFFF,
		"Sude Naz Karayildirim  | ");
	draw_outlined_text(a, WIDTH / 2 - 10, HEIGHT - 25, 0x888888,
		" github.com/skarayil");
	if (a->game_over)
	{
		for (int i = 0; i < 5; i++)
			draw_bar(a, WIDTH / 2 - 202 + i, HEIGHT / 2 - 52 + i, 404 - i * 2,
				104 - i * 2, 1.0, 0x550000);
		sprintf(buf, "OYUN BITTI!");
		draw_outlined_text(a, WIDTH / 2 - 50, HEIGHT / 2 - 20, 0xFF0000, buf);
		sprintf(buf, "FINAL SKORU: %d", a->score);
		draw_outlined_text(a, WIDTH / 2 - 60, HEIGHT / 2 + 10, 0xFFA500, buf);
		sprintf(buf, "SPACE ile Yeniden Baslat");
		draw_outlined_text(a, WIDTH / 2 - 90, HEIGHT / 2 + 40, 0xFFFF00, buf);
	}
}

int	key_press(int keycode, void *param)
{
	t_app	*a;

	a = (t_app *)param;
	if (keycode == 49)
	{
		a->score = 0;
		a->speed = 3;
		a->x = WIDTH / 2;
		a->y = HEIGHT / 2;
		a->game_over = 0;
		a->combo = 0;
		a->start_time = time(NULL);
		a->last_catch_time = 0;
		a->particle_count = 0;
		memset(a->trail_positions, 0, sizeof(a->trail_positions));
		a->trail_index = 0;
		random_direction(a);
		printf("🔄 Oyun yeniden basladi!\n");
	}
	return (0);
}

int	update(t_app *a)
{
	time_t	now;

	now = time(NULL);
	if (!a->game_over && (now - a->start_time) >= GAME_DURATION)
		a->game_over = 1;
	if (!a->game_over)
	{
		a->x += a->vx * a->speed;
		a->y += a->vy * a->speed;
		if (a->x < SIZE / 2 || a->x > WIDTH - SIZE / 2)
			a->vx = -a->vx;
		if (a->y < SIZE / 2 || a->y > HEIGHT - SIZE / 2)
			a->vy = -a->vy;
	}
	clear_image(a);
	update_particles(a);
	draw_square(a);
	mlx_put_image_to_window(a->mlx, a->win, a->img, 0, 0);
	draw_hud(a);
	return (0);
}

int	main(void)
{
	t_app	a;

	srand(time(NULL));
	memset(&a, 0, sizeof(t_app));
	a.mlx = mlx_init();
	a.win = mlx_new_window(a.mlx, WIDTH, HEIGHT, "catchme!");
	a.img = mlx_new_image(a.mlx, WIDTH, HEIGHT);
	a.data = mlx_get_data_addr(a.img, &a.bpp, &a.sl, &a.endian);
	a.x = WIDTH / 2;
	a.y = HEIGHT / 2;
	a.speed = 3;
	a.color = 0xD100FF;
	a.score = 0;
	a.game_over = 0;
	a.combo = 0;
	a.start_time = time(NULL);
	a.last_catch_time = 0;
	init_stars(&a);
	random_direction(&a);
	printf("\n=== catchme! ===\n\n");
	printf("Amac: Kareyi fare ile yakala ve uzerine tikla!\n\n");
	printf(" * Skor arttikca kare hizlanir.\n");
	printf(" * Ard arda yakalayarak COMBO yap!\n");
	printf(" * Space tusuna basarak oyunu yeniden baslatabilirsin.\n\n");
	mlx_hook(a.win, 17, 0, close_window, NULL);
	mlx_mouse_hook(a.win, mouse_click, &a);
	mlx_key_hook(a.win, key_press, &a);
	mlx_loop_hook(a.mlx, update, &a);
	mlx_loop(a.mlx);
	return (0);
}
