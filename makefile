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
	$(AR) $(ARFLAGS) $(FEMlib) $(OBJS)

libs:
	(mkdir -p bin)
	(mkdir -p lib)
	(cd ThirdParty/AMD; make)
	(cd ThirdParty/UMFPACK; make)
	(cd src/FEM; make archive)
	(cd src/SiteResponse; make archive)
	(cd src/Utilities; make archive)

clean:
	(cd ThirdParty/AMD; make clean)
	(cd ThirdParty/UMFPACK; make clean)
	(cd src/FEM; make clean)
	(cd src/SiteResponse; make clean)
	(cd src/Utilities; make clean)
	
tidy:
	$(RM) $(RMFLAGS) $(source)/bin/siteresponse
	$(RM) $(RMFLAGS) $(source)/lib/*.a
	make clean

install: siteResponse
	cp $(source)/bin/siteresponse $(HOME)/bin/.
