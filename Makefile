RAYLIB_DIR   = external/raylib-5.5_linux_amd64
TARGET_DIR   = target
TARGET       = $(TARGET_DIR)/clicker

CFLAGS  = -I$(RAYLIB_DIR)/include
LDFLAGS = -L$(RAYLIB_DIR)/lib \
          -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
          -Wl,-rpath,'$$ORIGIN/../external/raylib-5.5_linux_amd64/lib'

.PHONY: all run clean

all: $(TARGET)

$(TARGET): main.c | $(TARGET_DIR)
	gcc main.c $(CFLAGS) -o $(TARGET) $(LDFLAGS)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(TARGET_DIR)
