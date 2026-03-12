NAME = ft_ping
CC = cc
CFLAGS = -Wall -Wextra -Werror -g 
RM = rm -f


SRCS = main.c parser.c ping.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

run : re
	sudo ./$(NAME) -D -v -s 1400 -W 1 8.8.8.8

re: fclean all

.PHONY: all clean fclean re