include ../Makefile.in

OBJS =  \
       ConfigManager.o \
	   DefaultConfig.o

archive: $(OBJS)
	ar rv $(UTILlib) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *.a

