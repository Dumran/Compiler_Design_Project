NAME = interpreter
CC = gcc -o $(NAME)

SRCS =  ft_utils.c interpreter.c main.c

$(NAME): $(SRCS)
	@$(CC) $(SRCS)

all: $(NAME)

clean: 
	$(RM) $(NAME)

re: clean all

.PHONY: all clean re