TARGET=assembler
UNIT_TEST=assembler-unit-test

# Files
SOURCE_DIR:=src
INSTRUCTIONS_DIR:=../arch
UNIT_TEST_DIR:=$(SOURCE_DIR)/unit-tests
COMMON_DIR:=../common

MAIN_SOURCE:=$(SOURCE_DIR)/main.c

SOURCE_FILES:=$(wildcard $(SOURCE_DIR)/*.c $(SOURCE_DIR)/*/*.c)
SOURCE_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.c $(INSTRUCTIONS_DIR)/*/*.c)
SOURCE_FILES+=$(wildcard $(COMMON_DIR)/*.c $(COMMON_DIR)/*/*.c)

MAIN_SOURCE_FILES:=$(filter-out $(UNIT_TEST_DIR)/%, $(SOURCE_FILES))
UNIT_TEST_SOURCE_FILES:=$(filter-out $(MAIN_SOURCE), $(SOURCE_FILES))

HEADER_FILES:=$(wildcard $(SOURCE_DIR)/*.h $(SOURCE_DIR)/*/*.h)
HEADER_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.h $(INSTRUCTIONS_DIR)/*/*.h)
HEADER_FILES+=$(wildcard $(COMMON_DIR)/*.h $(COMMON_DIR)/*/*.h)

MAIN_HEADER_FILES:=$(filter-out $(UNIT_TEST_DIR)/%, $(HEADER_FILES))
UNIT_TEST_HEADER_FILES:=$(filter-out $(MAIN_SOURCE), $(HEADER_FILES))

ASSEMB_FILES:=$(wildcard $(SOURCE_DIR)/*.S $(SOURCE_DIR)/*/*.S)
ASSEMB_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.S $(INSTRUCTIONS_DIR)/*/*.S)
ASSEMB_FILES+=$(wildcard $(COMMON_DIR)/*.S $(COMMON_DIR)/*/*.S)

MAIN_ASSEMB_FILES:=$(filter-out $(UNIT_TEST_DIR)/%, $(ASSEMB_FILES))
UNIT_TEST_ASSEMB_FILES:=$(filter-out $(MAIN_SOURCE), $(ASSEMB_FILES))

OBJ:=${MAIN_SOURCE_FILES:.c=.o}
OBJ+=${MAIN_ASSEMB_FILES:.S=.o}

UNIT_OBJ:=${UNIT_TEST_SOURCE_FILES:.c=.o}
UNIT_OBJ+=${UNIT_TEST_ASSEMB_FILES:.S=.o}

# Compiler
CC:=gcc
ARCH:=arm64 # make automatic
OPT:=-g
CFLAGS:= -I$(SOURCE_DIR) -I$(COMMON_DIR) -I$(INSTRUCTIONS_DIR) $(OPT)
LIBS:=

# Assembler
ASM:=gcc
ASMFLAGS:=#-arch $(ARCH) # make automatic

# Preprocessor
PP:=gcc
PPFLAGS:=-E

# Rules
%.o: %.S Makefile
	$(PP) $< -o tmp.tmp $(PPFLAGS)
	sed /^#/d tmp.tmp > preprocessed.tmp.s
	$(RM) tmp.tmp
	$(ASM) -c -o $@ preprocessed.tmp.s $(ASMFLAGS)
	$(RM) preprocessed.tmp.s

%.o: %.c $(HEADER_FILES) Makefile
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(UNIT_TEST): $(UNIT_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

run: $(TARGET)
	./$(TARGET) -i interrupt -e start -o prog.bin -sz 0xFFFFFFFF ../program/src/main.s

test: $(UNIT_TEST)
	./$(UNIT_TEST)

clean:
	$(RM)	$(OBJ)
	$(RM)	$(UNIT_OBJ)
	$(RM)	$(TARGET)
	$(RM)	$(UNIT_TEST)