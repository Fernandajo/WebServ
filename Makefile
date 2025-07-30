# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/29 23:32:19 by mdomnik           #+#    #+#              #
#    Updated: 2025/07/29 23:36:03 by mdomnik          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


# ===================== Webserv Makefile ======================

# === Text Formatting ===
RESET   = \033[0m
GREEN   = \033[1;32m
MAGENTA = \033[1;35m
CYAN    = \033[1;36m

# === Symbols ===
CHECK_MARK = $(MAGENTA)✔$(RESET)
CAT_EMOJI  = 🐈

# === Compiler and Flags ===
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# === Directories ===
SRC_DIR            := src
HELPER_DIR         := $(SRC_DIR)/helpers
CLASS_DIR          := $(SRC_DIR)/class_components
CLASS_HELPER_DIR   := $(CLASS_DIR)/helpers
INC_DIR            := inc
OBJ_DIR            := obj
TEST_DIR           := tests

# === Main Executable ===
TARGET = webserv

# === Main Sources ===
MAIN_SRCS := \
	$(CLASS_DIR)/HTTPRequest.cpp \
	$(CLASS_HELPER_DIR)/HTTPRequest_helper.cpp \
	$(CLASS_DIR)/Server.cpp \
	$(CLASS_DIR)/MultiServerManager.cpp \
	$(CLASS_DIR)/HTTPResponse.cpp \
	$(CLASS_DIR)/ConfigParser.cpp \
	$(CLASS_HELPER_DIR)/ConfigParser_helper.cpp \
	$(HELPER_DIR)/mime.cpp \
	$(HELPER_DIR)/ServerConfig.cpp \
	$(SRC_DIR)/main.cpp

MAIN_OBJS := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(MAIN_SRCS))

# === Test Sources and Targets ===
TEST_PARSER_SRC    := $(TEST_DIR)/HTTP_Parser_test.cpp
TEST_PARSER_OBJ    := $(OBJ_DIR)/$(TEST_PARSER_SRC:.cpp=.o)
TEST_PARSER_BIN    := test_parser

TEST_GET_SRC       := $(TEST_DIR)/HTTP_GET_test.cpp
TEST_GET_OBJ       := $(OBJ_DIR)/$(TEST_GET_SRC:.cpp=.o)
TEST_GET_BIN       := test_get

TEST_RESPONSE_SRC  := $(TEST_DIR)/HTTP_Response_test.cpp
TEST_RESPONSE_OBJ  := $(OBJ_DIR)/$(TEST_RESPONSE_SRC:.cpp=.o)
TEST_RESPONSE_BIN  := test_response

TEST_AUTOINDEX_SRC := $(TEST_DIR)/HTTP_Autoindex_test.cpp
TEST_AUTOINDEX_OBJ := $(OBJ_DIR)/$(TEST_AUTOINDEX_SRC:.cpp=.o)
TEST_AUTOINDEX_BIN := test_autoindex

# === Include Headers ===
INCLUDES := -I$(INC_DIR)

# =============================================================
# ======================== Build Targets =======================
# =============================================================

all: $(TARGET)

# === Main Binary Build ===
$(TARGET): $(MAIN_OBJS) | $(OBJ_DIR)
	@echo "$(MAGENTA)Linking $(TARGET)...$(RESET)"
	@$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "$(MAGENTA)Linked successfully$(RESET) $(CHECK_MARK)"
	@echo "$(GREEN)Main binary built!$(RESET) $(CAT_EMOJI)"

# === Object Compilation Rule (recursive safe) ===
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "$(CYAN)Compiling $<$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# === Create top-level obj directory ===
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# === Tests ===
test_parser:     $(TEST_PARSER_BIN)
test_get:        $(TEST_GET_BIN)
test_response:   $(TEST_RESPONSE_BIN)
test_autoindex:  $(TEST_AUTOINDEX_BIN)

# Build test_parser binary
$(TEST_PARSER_BIN): $(TEST_PARSER_OBJ) $(filter-out $(OBJ_DIR)/$(SRC_DIR)/main.o, $(MAIN_OBJS)) | $(OBJ_DIR)
	@echo "$(MAGENTA)Linking test_parser binary...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
	@echo "$(GREEN)test_parser built!$(RESET) $(CHECK_MARK)"

# Build test_get binary
$(TEST_GET_BIN): $(TEST_GET_OBJ) $(filter-out $(OBJ_DIR)/$(SRC_DIR)/main.o, $(MAIN_OBJS)) | $(OBJ_DIR)
	@echo "$(MAGENTA)Linking test_get binary...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
	@echo "$(GREEN)test_get built!$(RESET) $(CHECK_MARK)"

# Build test_response binary
$(TEST_RESPONSE_BIN): $(TEST_RESPONSE_OBJ) $(filter-out $(OBJ_DIR)/$(SRC_DIR)/main.o, $(MAIN_OBJS)) | $(OBJ_DIR)
	@echo "$(MAGENTA)Linking test_response binary...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
	@echo "$(GREEN)test_response built!$(RESET) $(CHECK_MARK)"

# Build test_autoindex binary
$(TEST_AUTOINDEX_BIN): $(TEST_AUTOINDEX_OBJ) $(filter-out $(OBJ_DIR)/$(SRC_DIR)/main.o, $(MAIN_OBJS)) | $(OBJ_DIR)
	@echo "$(MAGENTA)Linking test_autoindex binary...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
	@echo "$(GREEN)test_autoindex built!$(RESET) $(CHECK_MARK)"

# === Cleanup ===
clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(MAGENTA)Object files removed!$(RESET)"

fclean: clean
	@rm -f $(TARGET) \
		$(TEST_PARSER_BIN) \
		$(TEST_GET_BIN) \
		$(TEST_RESPONSE_BIN) \
		$(TEST_AUTOINDEX_BIN)
	@echo "$(MAGENTA)Binaries removed!$(RESET)"

re: fclean all

.PHONY: all clean fclean re test_parser test_get test_response test_autoindex
