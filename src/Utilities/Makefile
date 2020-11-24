include ../../Makefile.in

OBJS =  \
       GeneralUtilities.o \
       ConfigManager.o    \
       DefaultConfig.o    \
       JsonConfig.o       \
       JsonReader.o       \
       GeometryBase.o     \
       GeometryLayer.o    \
       GeometryLayer_Library.o \
       GeometryFile_Reader.o   \
       SiteGeometry.o          \
       SiteGeometryFile_Reader.o     \
       JsonSiteGeometryFile_Reader.o \
       LayerMaterial.o       \
       MaterialFile_Reader.o \
       JsonMaterialFile_Reader.o \
       MaterialManager.o
	   

archive: $(OBJS)
	ar rv $(UTILlib) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *.a

