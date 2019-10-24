CC = gcc
CFLAGS = -MD
EXE = single_cycle
OBJS = function.o main.o

all:$(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(EXE) *.o *.d *.bin *.rpt *.exe *.out

-include $(OBJS:.o=.d)
