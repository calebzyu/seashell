# Seashell Makefile
CFLAGS := -Wall -g
CC := gcc $(CFLAGS)

############################################################
# Default target and cleaning target to remove compiled programs/objects
all : seashell

clean :
	rm -f seashell *.o

############################################################
# help message to show build targets
help :
	@echo 'Typical usage is:'
	@echo '  > make                          # build all programs'
	@echo '  > make clean                    # remove all compiled items'

############################################################
# targets
seashell : sea_main.o sea_funcs.o sea_builtins.o
	$(CC) -o $@ $^
	
%.o : %.c
	$(CC) -c $<




