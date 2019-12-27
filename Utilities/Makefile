include ../Makefile.in

OBJS =  \
       GeneralUtilities.o \
       FileReader.o       \
	   JsonReader.o       \
       ConfigManager.o    \
	   DefaultConfig.o    \
	   JsonConfig.o    

archive: $(OBJS)
	ar rv $(UTILlib) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *.a

