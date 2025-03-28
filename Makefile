CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = minimal_db_sys
SRC = $(TARGET).c

all: $(TARGET)

$(TARGET): $(SRC)
		
	# For your reference:
	
	# $@ refers to the target name (minimal_db_system in this case)
	
	# $^ refers to all prerequisites (minimal_db_system.c in this case)
	
	# So the compilation line expands to:
	
	# gcc -Wall -Wextra -g -o minimal_db_system minimal_db_system.c
	
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean