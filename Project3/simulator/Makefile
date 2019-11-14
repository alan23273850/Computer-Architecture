CC = g++
CFLAGS = -Wall -MD
EXE = CMP
OBJS = body.o cache.o function.o instruction.o main.o memory.o prepare.o pagetable.o tlb.o

all:$(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS)

%.o:%.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(EXE) *.o *.d *.bin *.rpt *.exe *.out

-include $(OBJS:.o=.d)
