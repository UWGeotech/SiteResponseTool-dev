include ../../Makefile.in

OBJS =  \
       GeneralUtilities.o \
       ConfigManager.o    \
       DefaultConfig.o    \
       JsonConfig.o       \
       JsonReader.o       

archive: $(OBJS)
	ar rv $(UTILlib) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *.a

