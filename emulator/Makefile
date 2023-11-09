TARGET=emulator

# Files
SOURCE_DIR:=src
INSTRUCTIONS_DIR:=../arch
COMMON_DIR:=../common

SOURCE_FILES:=$(wildcard $(SOURCE_DIR)/*.c $(SOURCE_DIR)/*/*.c)
SOURCE_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.c $(INSTRUCTIONS_DIR)/*/*.c)
SOURCE_FILES+=$(wildcard $(COMMON_DIR)/*.c $(COMMON_DIR)/*/*.c)

HEADER_FILES:=$(wildcard $(SOURCE_DIR)/*.h $(SOURCE_DIR)/*/*.h)
HEADER_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.h $(INSTRUCTIONS_DIR)/*/*.h)
HEADER_FILES+=$(wildcard $(COMMON_DIR)/*.h $(COMMON_DIR)/*/*.h)

ASSEMB_FILES:=$(wildcard $(SOURCE_DIR)/*.S $(SOURCE_DIR)/*/*.S)
ASSEMB_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.S $(INSTRUCTIONS_DIR)/*/*.S)
ASSEMB_FILES+=$(wildcard $(COMMON_DIR)/*.S $(COMMON_DIR)/*/*.S)

OBJ:=${SOURCE_FILES:.c=.o}
OBJ+=${ASSEMB_FILES:.S=.o}

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

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM)	$(OBJ)
	$(RM)	$(TARGET)