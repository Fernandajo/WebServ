# ===================== Webserv Makefile ======================

# === Text Formatting ===
RESET   = \033[0m
GREEN   = \033[1;32m
MAGENTA = \033[1;35m
CYAN    = \033[1;36m

# === Symbols ===
CHECK_MARK = $(MAGENTA)✔$(RESET)
ROCKET     = 🚀

# === Compiler and Flags ===
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# === Directories ===
SRC_DIR            := src
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
	$(CLASS_DIR)/ServerOrg.cpp \
	$(SRC_DIR)/main.cpp

MAIN_OBJS := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(MAIN_SRCS))

# === Test Executables ===
TESTS        := testHTTP
TESTHTTP_SRC := $(TEST_DIR)/HTTP_Parser_test.cpp
TESTHTTP_OBJ := $(OBJ_DIR)/$(TESTHTTP_SRC:.cpp=.o)
TESTHTTP_BIN := testHTTP

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
	@echo "$(GREEN)Main binary built!$(RESET) $(ROCKET)"

# === Object Compilation Rules (recursive path-safe) ===
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "$(CYAN)Compiling $<$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Create object subdirectory automatically
$(OBJ_DIR)/%:
	@mkdir -p $(dir $@)

# === Create top-level obj directory ===
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# === Test Builds ===
test: $(TESTHTTP_BIN)

$(TESTHTTP_BIN): $(TESTHTTP_OBJ) $(filter-out $(OBJ_DIR)/$(SRC_DIR)/main.o, $(MAIN_OBJS)) | $(OBJ_DIR)
	@echo "$(MAGENTA)Linking test binary: $@$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
	@echo "$(GREEN)Test binary $@ built!$(RESET) $(CHECK_MARK)"

# === Cleanup ===
clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(MAGENTA)Object files removed!$(RESET)"

fclean: clean
	@rm -f $(TARGET) $(TESTS)
	@echo "$(MAGENTA)Binaries removed!$(RESET)"

re: fclean all

.PHONY: all clean fclean re test
