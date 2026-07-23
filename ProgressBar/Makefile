SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
BIN=progressbar

$(BIN):$(OBJ)
	gcc -o $@ $^
%.o:%.c
	gcc -c $<
.PHONY:
clean:
	rm -f $(OBJ) $(BIN)
