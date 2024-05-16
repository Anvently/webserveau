NAME		=	webserv

INCLUDES	=	includes/

CPP_FLAGS	=	-Wall -Wextra -Werror -std=c++98 -g3 -I$(INCLUDES)
CPP			=	c++

SRCS_FOLDER	=	srcs/
OBJS_FOLDER	=	.objs/

SRCS_FILES	=	main.cpp ILogger.cpp IParseConfig.cpp ListenServer.cpp

SRCS	=	$(addprefix $(SRCS_FOLDER), $(SRCS_FILES))
OBJS	=	$(addprefix $(OBJS_FOLDER), $(SRCS_FILES:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPP_FLAGS) $(OBJS) -o $(NAME)

$(OBJS_FOLDER)%.o: $(SRCS_FOLDER)%.cpp Makefile
	mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) -c -o $@ $<

clean:
	rm -rfd $(OBJS_FOLDER)

fclean: clean
	rm -f $(NAME)

re: fclean all
