# ===== Cross-platform Makefile (Windows / Unix) =====

# 编译器与选项
CC      = gcc
CFLAGS  = -I include -Wall -Wextra -O2
LDFLAGS = -lm

# 目录
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 源文件与目标文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- 平台判断（Windows_NT 为 Windows） ---
ifeq ($(OS),Windows_NT)
  EXEEXT    = .exe
  MKDIR_BIN = if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
  MKDIR_OBJ = if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
  CLEAN_CMD = -@(rmdir /S /Q "$(OBJ_DIR)" 2> NUL) & (rmdir /S /Q "$(BIN_DIR)" 2> NUL)
  RUN_PREFIX =
else
  EXEEXT    =
  MKDIR_BIN = mkdir -p "$(BIN_DIR)"
  MKDIR_OBJ = mkdir -p "$(OBJ_DIR)"
  CLEAN_CMD = rm -rf "$(OBJ_DIR)" "$(BIN_DIR)"
  RUN_PREFIX = ./
endif

# 可执行文件名
TARGET = $(BIN_DIR)/buzznav$(EXEEXT)

# 伪目标
.PHONY: all clean run

# 默认：生成最终可执行文件
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	@$(MKDIR_BIN)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

# 编译 .c -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	$(CLEAN_CMD)

# 运行（如需参数，可在命令行传，例如：make run ARGS="0 1"）
run: $(TARGET)
	$(RUN_PREFIX)$(TARGET) $(ARGS)
