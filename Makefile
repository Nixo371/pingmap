SRCDIRS = src
INCDIRS = include

CC = gcc
FLAGS = -Wall -Wextra -Werror $(foreach D, $(INCDIRS), -I$(D))

SRCS = $(foreach D, $(SRCDIRS), $(wildcard $(D)/*.c))
OBJS = $(patsubst %.c, %.o, $(SRCS))

NAME = pingmap

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $^ -Llib -lpixelpng -lz -loping -o $(NAME)

%.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

.PHONY: all test clean fclean
