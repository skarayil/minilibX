CC          = cc
CFLAGS      = -Wall -Wextra -Werror

MLX_PATH    = minilibx
MLX         = $(MLX_PATH)/libmlx.a
FRAMEWORKS  = -framework OpenGL -framework AppKit -lz

DIRS        = asteroids breakout catchme

SRCS        = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))

EXES        = $(addsuffix _game, $(notdir $(SRCS:.c=)))

all: $(MLX) $(EXES)

$(MLX):
	@make -C $(MLX_PATH)

%_game: */%.c
	$(CC) $(CFLAGS) $< $(MLX) $(FRAMEWORKS) -I$(MLX_PATH) -o $@

clean:
	rm -f $(foreach dir,$(DIRS),$(dir)/*.o)
	@make -C $(MLX_PATH) clean

fclean: clean
	rm -f $(EXES)

re: fclean all

.PHONY: all clean fclean re
