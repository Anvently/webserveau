NAME		=	webserv

TEST		= 	test

INCLUDES	=	includes/

CPP_FLAGS	=	-Wall -Wextra -Werror -std=c++98 -g3 -I$(INCLUDES)
CPP			=	c++

SRCS_FOLDER	=	srcs/
OBJS_FOLDER	=	.objs/


OBJS_TFOLDER	= .objs_test/

SRCS_FILES	=	main.cpp ILogger.cpp IParseConfig.cpp
SRCS_FTEST	= 	mainLouis.cpp Header.cpp

SRCS	=	$(addprefix $(SRCS_FOLDER), $(SRCS_FILES))
OBJS	=	$(addprefix $(OBJS_FOLDER), $(SRCS_FILES:.cpp=.o))

SRCS_TEST	=	$(addprefix $(SRCS_FOLDER), $(SRCS_FTEST))
OBJS_TEST	=	$(addprefix $(OBJS_TFOLDER), $(SRCS_FTEST:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPP_FLAGS) $(OBJS) -o $(NAME)

$(OBJS_FOLDER)%.o: $(SRCS_FOLDER)%.cpp Makefile
	mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) -c -o $@ $<

$(OBJS_TFOLDER)%.o: $(SRCS_FOLDER)%.cpp Makefile
	mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) -c -o $@ $<

$(TEST): $(OBJS_TEST)
	$(CPP) $(CPP_FLAGS) $(OBJS_TEST) -o $(TEST)

clean:
	rm -rfd $(OBJS_FOLDER)
	rm -rfd $(OBJS_TFOLDER)

fclean: clean
	rm -f $(NAME)

re: fclean all
