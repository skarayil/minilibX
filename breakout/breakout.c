#include "../minilibx/mlx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define BRICK_ROWS 6
#define BRICK_COLS 10
#define BRICK_W 80
#define BRICK_H 25
#define PADDLE_W 100
#define PADDLE_H 15
#define BALL_R 8
#define MAX_PARTICLES 200

#define KEY_ESC 53
#define KEY_LEFT 123
#define KEY_RIGHT 124
#define KEY_SPACE 49

typedef struct s_particle
{
	float		x;
	float		y;
	float		vx;
	float		vy;
	int			life;
	int			color;
	int			active;
}				t_particle;

typedef struct s_app
{
	void		*mlx;
	void		*win;
	void		*img;
	char		*addr;
	int			bpp;
	int			line_len;
	int			endian;
	int			running;
	int			bricks[BRICK_ROWS][BRICK_COLS];
	float		paddle_x;
	int			move_left;
	int			move_right;
	float		ball_x;
	float		ball_y;
	float		ball_vx;
	float		ball_vy;
	int			game_over;
	t_particle	particles[MAX_PARTICLES];
}				t_app;

void	my_mlx_pixel_put(t_app *data, int x, int y, int color)
{
	char	*dst;

	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
		return ;
	dst = data->addr + (y * data->line_len + x * (data->bpp / 8));
	*(unsigned int *)dst = color;
}

void	draw_centered_text(t_app *a, int y, int color, char *text)
{
	int	text_width;
	int	x;

	text_width = strlen(text) * 10;
	x = (WIDTH - text_width) / 2;
	mlx_string_put(a->mlx, a->win, x, y, color, text);
}

void	draw_rect(t_app *a, int x, int y, int w, int h, int color)
{
	int	i;
	int	j;

	i = 0;
	while (i < h)
	{
		j = 0;
		while (j < w)
		{
			my_mlx_pixel_put(a, x + j, y + i, color);
			j++;
		}
		i++;
	}
}

void	draw_circle(t_app *a, int cx, int cy, int r, int color)
{
	int	y;
	int	x;

	y = -r;
	while (y <= r)
	{
		x = -r;
		while (x <= r)
		{
			if (x * x + y * y <= r * r)
				my_mlx_pixel_put(a, cx + x, cy + y, color);
			x++;
		}
		y++;
	}
}

void	spawn_particles(t_app *a, float x, float y, int color)
{
	int		count;
	int		i;
	float	angle;
	float	speed;

	count = 0;
	i = 0;
	while (i < MAX_PARTICLES && count < 10)
	{
		if (!a->particles[i].active)
		{
			a->particles[i].active = 1;
			a->particles[i].x = x;
			a->particles[i].y = y;
			angle = ((float)(rand() % 360) / 180.0f) * M_PI;
			speed = ((float)(rand() % 100) / 100.0f) * 3.0f;
			a->particles[i].vx = cosf(angle) * speed;
			a->particles[i].vy = sinf(angle) * speed;
			a->particles[i].life = 15 + rand() % 15;
			a->particles[i].color = color;
			count++;
		}
		i++;
	}
}

void	update_particles(t_app *a)
{
	int	i;

	i = 0;
	while (i < MAX_PARTICLES)
	{
		if (a->particles[i].active)
		{
			a->particles[i].x += a->particles[i].vx;
			a->particles[i].y += a->particles[i].vy;
			a->particles[i].vy += 0.2f;
			a->particles[i].life--;
			if (a->particles[i].life <= 0)
				a->particles[i].active = 0;
			else
				my_mlx_pixel_put(a, (int)a->particles[i].x,
					(int)a->particles[i].y, a->particles[i].color);
		}
		i++;
	}
}

void	init_bricks(t_app *a)
{
	int	colors[6];
	int	r;
	int	c;

	colors[0] = 0xFF4444;
	colors[1] = 0xFF8844;
	colors[2] = 0xFFFF44;
	colors[3] = 0x44FF44;
	colors[4] = 0x4488FF;
	colors[5] = 0x8844FF;
	r = 0;
	while (r < BRICK_ROWS)
	{
		c = 0;
		while (c < BRICK_COLS)
		{
			a->bricks[r][c] = colors[r % 6];
			c++;
		}
		r++;
	}
}

void	reset_game(t_app *a)
{
	int	i;

	a->game_over = 0;
	a->ball_x = WIDTH / 2.0f;
	a->ball_y = HEIGHT / 2.0f + 50;
	a->ball_vx = 2.5f;
	if (rand() % 2)
		a->ball_vx = -2.5f;
	a->ball_vy = -3.5f;
	a->paddle_x = WIDTH / 2.0f - PADDLE_W / 2.0f;
	a->move_left = 0;
	a->move_right = 0;
	init_bricks(a);
	i = 0;
	while (i < MAX_PARTICLES)
	{
		a->particles[i].active = 0;
		i++;
	}
}

void	check_wall_collision(t_app *a)
{
	float	offset;

	if (a->ball_x - BALL_R <= 0 || a->ball_x + BALL_R >= WIDTH)
	{
		if (a->ball_x - BALL_R <= 0)
			a->ball_x = BALL_R;
		else
			a->ball_x = WIDTH - BALL_R;
		a->ball_vx *= -1;
	}
	if (a->ball_y - BALL_R <= 0)
	{
		a->ball_y = BALL_R;
		a->ball_vy *= -1;
	}
	if (a->ball_y >= HEIGHT - 40 && a->ball_y <= HEIGHT - 40 + PADDLE_H
		&& a->ball_x >= a->paddle_x && a->ball_x <= a->paddle_x + PADDLE_W
		&& a->ball_vy > 0)
	{
		a->ball_vy = -fabsf(a->ball_vy);
		offset = (a->ball_x - (a->paddle_x + PADDLE_W / 2.0f)) / (PADDLE_W
				/ 2.0f);
		a->ball_vx = offset * 4.5f;
		spawn_particles(a, a->ball_x, HEIGHT - 40, 0xFFFFFF);
	}
}

int	check_brick_logic(t_app *a, int i, int j)
{
	float	bx;
	float	by;

	if (a->bricks[i][j] != 0)
	{
		bx = j * BRICK_W;
		by = i * BRICK_H;
		if (a->ball_x + BALL_R > bx && a->ball_x - BALL_R < bx + BRICK_W
			&& a->ball_y + BALL_R > by && a->ball_y - BALL_R < by + BRICK_H)
		{
			spawn_particles(a, a->ball_x, a->ball_y, a->bricks[i][j]);
			a->bricks[i][j] = 0;
			a->ball_vy *= -1;
			return (1);
		}
		return (2);
	}
	return (0);
}

void	check_collisions(t_app *a)
{
	int	active;
	int	i;
	int	j;
	int	res;

	check_wall_collision(a);
	active = 0;
	i = 0;
	while (i < BRICK_ROWS)
	{
		j = 0;
		while (j < BRICK_COLS)
		{
			res = check_brick_logic(a, i, j);
			if (res == 1)
				return ;
			if (res == 2)
				active++;
			j++;
		}
		i++;
	}
	if (active == 0)
		init_bricks(a);
	if (a->ball_y - BALL_R > HEIGHT)
		a->game_over = 1;
}

void	draw_game_objects(t_app *a)
{
	int	i;
	int	j;

	i = 0;
	while (i < BRICK_ROWS)
	{
		j = 0;
		while (j < BRICK_COLS)
		{
			if (a->bricks[i][j] != 0)
				draw_rect(a, j * BRICK_W + 2, i * BRICK_H + 2, BRICK_W - 4,
					BRICK_H - 4, a->bricks[i][j]);
			j++;
		}
		i++;
	}
	draw_rect(a, (int)a->paddle_x, HEIGHT - 40, PADDLE_W, PADDLE_H, 0x666666);
	draw_circle(a, (int)a->ball_x, (int)a->ball_y, BALL_R, 0xFFFFFF);
	update_particles(a);
}

int	update(t_app *a)
{
	memset(a->addr, 0, HEIGHT * a->line_len);
	if (!a->game_over)
	{
		if (a->move_left)
			a->paddle_x -= 8.0f;
		if (a->move_right)
			a->paddle_x += 8.0f;
		if (a->paddle_x < 0)
			a->paddle_x = 0;
		if (a->paddle_x + PADDLE_W > WIDTH)
			a->paddle_x = WIDTH - PADDLE_W;
		a->ball_x += a->ball_vx;
		a->ball_y += a->ball_vy;
		check_collisions(a);
	}
	draw_game_objects(a);
	mlx_put_image_to_window(a->mlx, a->win, a->img, 0, 0);
	if (a->game_over)
	{
		draw_centered_text(a, HEIGHT / 2 - 20, 0xFF3333, "O Y U N   B I T T I");
		draw_centered_text(a, HEIGHT / 2 + 20, 0xFFFF00, "SPACE ile Yeniden");
		draw_centered_text(a, HEIGHT - 50, 0x666666, "Sude Naz Karayildirim");
	}
	return (0);
}

int	key_down(int keycode, t_app *a)
{
	if (keycode == KEY_ESC)
		exit(0);
	if (keycode == KEY_LEFT)
		a->move_left = 1;
	if (keycode == KEY_RIGHT)
		a->move_right = 1;
	if (keycode == KEY_SPACE && a->game_over)
		reset_game(a);
	return (0);
}

int	key_up(int keycode, t_app *a)
{
	if (keycode == KEY_LEFT)
		a->move_left = 0;
	if (keycode == KEY_RIGHT)
		a->move_right = 0;
	return (0);
}

int	close_window(t_app *a)
{
	(void)a;
	exit(0);
	return (0);
}

int	main(void)
{
	t_app a;

	srand((unsigned)time(NULL));
	a.mlx = mlx_init();
	a.win = mlx_new_window(a.mlx, WIDTH, HEIGHT, "Breakout");
	a.img = mlx_new_image(a.mlx, WIDTH, HEIGHT);
	a.addr = mlx_get_data_addr(a.img, &a.bpp, &a.line_len, &a.endian);
	reset_game(&a);
	mlx_hook(a.win, 2, 1L << 0, key_down, &a);
	mlx_hook(a.win, 3, 1L << 1, key_up, &a);
	mlx_hook(a.win, 17, 0, close_window, &a);
	mlx_loop_hook(a.mlx, update, &a);
	mlx_loop(a.mlx);
	return (0);
}
