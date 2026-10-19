CC = clang
LD = clang

BIN = dobby
# will rm -rf anything defined for OBJS_DIR and BIN_DIR
OBJS_DIR = .objs
BIN_DIR = bin

WARNINGS = -Wall -Wextra -Wpedantic -Wmisleading-indentation -Wunused -Wuninitialized -Wshadow -Wconversion -Werror
CCFLAGS = -DNDEBUG -O3 -march=native -std=c11 $(WARNINGS)
LDFLAGS = 

HDRS = dobby.h
SRCS = $(wildcard tests/*.c)
ASMS = $(wildcard tests/*.S)
OBJS = $(patsubst tests/%.c,$(OBJS_DIR)/%.o,$(SRCS)) $(patsubst tests/%.S,$(OBJS_DIR)/%.o,$(ASMS))

all: $(BIN_DIR)/$(BIN)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# c files
$(OBJS_DIR)/%.o: tests/%.c | $(OBJS_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

# assembly files
$(OBJS_DIR)/%.o: tests/%.S | $(OBJS_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

# linking elf
$(BIN_DIR)/$(BIN): $(OBJS) | $(BIN_DIR)
	$(LD) $^ $(LDFLAGS) -o $@

fmt:
	clang-format -i $(SRCS) $(HDRS)

lint:
	clang-format --dry-run --Werror $(SRCS) $(HDRS)
	clang-check --analyze --extra-arg=-Xanalyzer --extra-arg=-analyzer-output=text $(SRCS) -- $(CCFLAGS)

dump: $(OBJS)
	llvm-objdump -d --no-show-raw-insn $^

clean:
	rm -rf $(OBJS_DIR)
	@# glob ignores hidden files (.gitignore)
	rm -rf $(BIN_DIR)/*

.PHONY: all fmt lint dump clean
