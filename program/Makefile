TARGET=program.bin

# Files
SOURCE_DIR:=src

SOURCE_FILES:=$(wildcard $(SOURCE_DIR)/*.s $(SOURCE_DIR)/*/*.s $(SOURCE_DIR)/*.S $(SOURCE_DIR)/*/*.S)

ENTRY_POINT:=start
INTERRUPT_HANDLER:=interrupt
MEMORY_MAX_ADDR:=0xFFFFFFFF

EMULATOR:=../emulator/emulator
# Assembler
ASM:=../assembler/assembler
ASMFLAGS:=-e $(ENTRY_POINT) -i $(INTERRUPT_HANDLER) -sz $(MEMORY_MAX_ADDR)


# Rules

$(TARGET): $(SOURCE_FILES)
	./$(ASM) -o $@ $^ $(ASMFLAGS)

run: $(TARGET)
	./$(EMULATOR) $(TARGET)

clean:
	$(RM)	$(TARGET)