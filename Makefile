NAME = Webserv
SRC = src/main.cpp src/ServerOrg.cpp
OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))
COMPILER = c++
FLAGS = -std=c++98 -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJ)
	$(COMPILER) $(FLAGS) $^ -o $@

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(COMPILER) $(FLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Phony targets
.PHONY: all clean fclean re