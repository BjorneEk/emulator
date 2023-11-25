TARGET=program.bin

# Files
SOURCE_DIR:=src

SOURCE_FILES:=$(wildcard $(SOURCE_DIR)/*.s $(SOURCE_DIR)/*/*.s $(SOURCE_DIR)/*.S $(SOURCE_DIR)/*/*.S)
SOURCE_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.s $(INSTRUCTIONS_DIR)/*/*.s)
SOURCE_FILES+=$(wildcard $(INSTRUCTIONS_DIR)/*.S $(INSTRUCTIONS_DIR)/*/*.S)
SOURCE_FILES+=$(wildcard $(COMMON_DIR)/*.s $(COMMON_DIR)/*/*.s $(COMMON_DIR)/*.S $(COMMON_DIR)/*/*.S)

ENTRY_POINT:=start
INTERRUPT_HANDLER:=interrupt
MEMORY_MAX_ADDR:=0xFFFFFFFF

EMULATOR:=./emulator
# Assembler
ASM:=./assembler
ASMFLAGS:=-e $(ENTRY_POINT) -i $(INTERRUPT_HANDLER) -sz $(MEMORY_MAX_ADDR)


# Rules

$(TARGET): $(SOURCE_FILES)
	$(ASM) -o $@ $^ $(ASMFLAGS)

run: $(TARGET)
	$(EMULATOR) $(TARGET)

clean:
	$(RM)	$(TARGET)