NAME = interpreter.a
CC = gcc
CFLAGS = -Wall -Wextra -Werror
AR = ar -rc

SRCS =  ft_utils.c interpreter.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(AR) $(NAME) $(OBJS)

clean:
	$(RM) $(OBJS) $(BONUSOBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all