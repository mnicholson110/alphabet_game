TARGET = bin/alphabet_game
SRC = $(wildcard src/*.c)

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin 
	gcc -o $(TARGET) $(SRC) -lSDL3 -lSDL3_image -lSDL3_ttf

clean:
	rm -rf bin/

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
