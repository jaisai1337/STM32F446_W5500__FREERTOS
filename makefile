# Toolchain
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld

SIZE = arm-none-eabi-size 
OBJCOPY = arm-none-eabi-objcopy

PROJECT = FREERTOS_PLUS_TCP

# MCU Options
CPU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# Directories
SRC_DIR = src
SYS_DIR = src/system
FREERTOS_DIR = FreeRTOS/FreeRTOS_Kernel
FREERTOS_PLUS_TCP_DIR = FreeRTOS/FreeRTOSPlusTCP/source
ETHERNET_DIR = Ethernet

INC_DIR = inc
BUILD_DIR = build

# Sources and objects
SRCS = $(wildcard $(SRC_DIR)/*.c)
SYS_SRCS = $(wildcard $(SYS_DIR)/*.c) $(wildcard $(SYS_DIR)/*.s)
ETHERNET_SRC = $(wildcard $(ETHERNET_DIR)/*.c)
DHCP_SRC = $(wildcard $(ETHERNET_DIR)/DHCP/*.c)
DNS_SRC = $(wildcard $(ETHERNET_DIR)/DNS/*.c)
W5500_SRC = $(wildcard $(ETHERNET_DIR)/W5500/*.c)
FREERTOS_SRCS = $(wildcard $(FREERTOS_DIR)/*.c)
FREERTOS_PORT_SRCS = $(wildcard $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/*.c)
FREERTOS_HEAP_SRCS = $(wildcard $(FREERTOS_DIR)/portable/MemMang/*.c)
FREERTOS_PLUS_TCP_SRCS = $(wildcard $(FREERTOS_PLUS_TCP_DIR)/*.c)

OBJS = \
		$(patsubst $(SYS_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SYS_SRCS))) \
		$(patsubst $(SYS_DIR)/%.s,$(BUILD_DIR)/%.o,$(filter %.s,$(SYS_SRCS))) \
		$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRCS))) \
		$(FREERTOS_OBJS) \
		$(ETHERNET_OBJS) \
		$(FREERTOS_PLUS_TCP_OBJS)


#$(FREERTOS_OBJS) \
#$(ETHERNET_OBJS) \
#$(FREERTOS_PLUS_TCP_OBJS)
	   
	   
	   

FREERTOS_OBJS = \
	$(BUILD_DIR)/list.o \
	$(BUILD_DIR)/queue.o \
	$(BUILD_DIR)/tasks.o \
	$(BUILD_DIR)/port.o \
	$(BUILD_DIR)/heap_4.o

FREERTOS_PLUS_TCP_OBJS = \
	$(patsubst $(FREERTOS_PLUS_TCP_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(FREERTOS_PLUS_TCP_SRCS))) \
	$(BUILD_DIR)/BufferAllocation_2.o

ETHERNET_OBJS = \
	$(patsubst $(ETHERNET_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(ETHERNET_SRC))) \
	$(patsubst $(ETHERNET_DIR)/DHCP/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(DHCP_SRC))) \
	$(patsubst $(ETHERNET_DIR)/DNS/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(DNS_SRC))) \
	$(patsubst $(ETHERNET_DIR)/W5500/%.c,$(BUILD_DIR)/%.o,$(filter %.c,$(W5500_SRC)))


ELF  = $(BUILD_DIR)/$(PROJECT).elf
BIN  = $(BUILD_DIR)/$(PROJECT).bin
HEX  = $(BUILD_DIR)/$(PROJECT).hex
JLINK = $(BUILD_DIR)/$(PROJECT).jlink

# Flags
INCLUDES = -I$(INC_DIR) \
	-I$(INC_DIR)/Core/Include \
	-I$(FREERTOS_DIR)/include \
	-I$(ETHERNET_DIR) \
	-I$(ETHERNET_DIR)/DHCP \
	-I$(ETHERNET_DIR)/DNS \
	-I$(ETHERNET_DIR)/W5500 \
	-I$(FREERTOS_DIR)/portable/GCC/ARM_CM4F \
	-I$(FREERTOS_PLUS_TCP_DIR)/include \
	-I$(FREERTOS_PLUS_TCP_DIR)/portable/Compiler/GCC

CFLAGS = $(CPU) -g -Wall -w -Os -ffunction-sections -fdata-sections $(INCLUDES) -DSTM32F446xx
LDFLAGS = -TSTM32F446RETX_FLASH.ld -Wl,--gc-sections


# Default target
all: $(BUILD_DIR) $(ELF) $(BIN) $(HEX) $(JLINK)
	@echo "Build complete: $(PROJECT)"

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SYS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/portable/MemMang/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FREERTOS_PLUS_TCP_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FREERTOS_PLUS_TCP_DIR)/portable/BufferManagement/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ETHERNET_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ETHERNET_DIR)/DHCP/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ETHERNET_DIR)/DNS/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ETHERNET_DIR)/W5500/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble S files
$(BUILD_DIR)/%.o: $(SYS_DIR)/%.s
	$(CC) $(CFLAGS) -c $< -o $@


# Link objects to ELF
$(ELF): $(OBJS)
	$(CC) $(CPU) $(OBJS) -o $@ $(LDFLAGS) -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map
	+@make -s print_size

# Convert ELF to BIN
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Convert ELF to HEX
$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(JLINK): $(HEX) hex_to_jlink_w4.py
	@echo "Generating J-Link w4 script..."
	python3 hex_to_jlink_w4.py $(HEX) $(JLINK)
	@echo "Done: $(JLINK)"

# Flash using JLink
flash: $(BIN)
	JLinkExe -device STM32F446RE -if SWD -speed 4000 -autoconnect 1 -CommandFile jflash.jlink

manual_flash:
	JLinkExe -device STM32F446RE -if SWD -speed 4000 -CommandFile build/FREERTOS_PLUS_TCP.jlink
# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all flash clean


print_size:
	@echo "Memory usage:"
	@$(SIZE) -B $(ELF)
	@$(SIZE) -B -A $(ELF) | awk '\
		/\.text/ { flash += $$2 } \
		/\.data/ { flash += $$2; ram += $$2 } \
		/\.bss/  { ram += $$2 } \
		END { \
			flash_total = 524288; \
			ram_total = 131072; \
			ram_perc = ram / ram_total * 100; \
			flash_perc = flash / flash_total * 100; \
			ram_bar = int(ram_perc / 10); \
			flash_bar = int(flash_perc / 10); \
			printf "RAM:   [%s%s] %4.1f%% (used %d bytes from %d bytes)\n", \
				repeat("#", ram_bar), repeat(" ", 10 - ram_bar), \
				ram_perc, ram, ram_total; \
			printf "Flash: [%s%s] %4.1f%% (used %d bytes from %d bytes)\n", \
				repeat("#", flash_bar), repeat(" ", 10 - flash_bar), \
				flash_perc, flash, flash_total; \
		} \
		function repeat(s, n,    r) { \
			r = ""; \
			for (i = 0; i < n; i++) r = r s; \
			return r; \
		}'

