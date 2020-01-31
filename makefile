include ./Makefile.in

# -------------------------

siteResponse: ./SiteResponse/Main.cpp $(FEMlib)
	make libs
	@$(CXX) $(CXXOPTFLAG) $(LINCLUDE) $(MINCLUDE) $(THIRDPARTY_INCLUDE) ./SiteResponse/Main.cpp $(UTILlib) $(SRTlib) $(FEMlib) $(THIRDPARTY_LIB) $(NUMLIBS) -o $(source)/bin/siteresponse
	
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
	(cd FEM; make archive)
	(cd SiteResponse; make archive)
	(cd Utilities; make archive)

clean:
	(cd ThirdParty/AMD; make clean)
	(cd ThirdParty/UMFPACK; make clean)
	(cd FEM; make clean)
	(cd SiteResponse; make clean)
	(cd Utilities; make clean)
	
tidy:
	$(RM) $(RMFLAGS) $(source)/bin/siteresponse
	$(RM) $(RMFLAGS) $(source)/lib/*.a
	make clean

install: siteResponse
	cp $(source)/bin/siteresponse $(HOME)/bin/.


