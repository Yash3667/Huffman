CC = cc
FLAGS = -Wall -Wextra -Wpedantic -g
EXEC = huffman
OBJECTS = huffman_element.o huffman_list.o huffman_tree.o bit_vector.o huffman.o

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(FLAGS) -o $(EXEC) $(OBJECTS)
huffman.o: huffman.c
	$(CC) $(FLAGS) -c huffman.c
huffman_element.o: huffman_element.c
	$(CC) $(FLAGS) -c huffman_element.c
huffman_list.o: huffman_list.c
	$(CC) $(FLAGS) -c huffman_list.c
huffman_tree.o: huffman_tree.c
	$(CC) $(FLAGS) -c huffman_tree.c
bit_vector.o: bit_vector.c
	$(CC) $(FLAGS) -c bit_vector.c

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJECTS)
