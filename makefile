include ./Makefile.in

# -------------------------

siteResponse: ./src/SiteResponse/Main.cpp $(FEMlib)
	make libs
	@$(CXX) $(CXXOPTFLAG) $(THIRDPARTY_INCLUDE) $(LINCLUDE) $(MINCLUDE) ./src/SiteResponse/Main.cpp $(UTILlib) $(SRTlib) $(FEMlib) $(THIRDPARTY_LIB) $(NUMLIBS) -o $(source)/bin/siteresponse
	
fem:
	make tidy
	make siteResponse

$(FEMlib):
	@ make libs

archive: $(OBJS)
	ar rv $(FEMlib) $(OBJS)

libs:
	(mkdir -p bin)
	(mkdir -p lib)
	(cd src/FEM; make archive)
	(cd src/SiteResponse; make archive)
	(cd src/Utilities; make archive)

clean:
	(cd src/FEM; make clean)
	(cd src/SiteResponse; make clean)
	(cd src/Utilities; make clean)
	
tidy:
	rm -f $(source)/bin/siteresponse
	rm -f $(source)/lib/*.a
	make clean

install: siteResponse
	cp $(source)/bin/siteresponse $(HOME)/bin/.


