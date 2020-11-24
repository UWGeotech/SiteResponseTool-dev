#ifndef SiteGeometry_H
#define SiteGeometry_H

#include <string> 
#include <vector>

#include "GeometryLayer.h"
#include "GeometryLayer_Library.h"
#include "GeometryBase.h"

#include "MaterialManager.h"
#include "LayerMaterial.h"

class SiteGeometryFile_Reader;

class SiteGeometry
{
public:
	SiteGeometry();
	SiteGeometry(std::string geometry_fn);
	SiteGeometry(std::string geometry_fn, std::string filetype);
	~SiteGeometry();

	double getTotThickness() { return sl_totalThickness; };
	double getEquivalentShearVel() { return sl_equivalentVS; };
	double getEquivalentCompVel() { return sl_equivalentVP; };
	int    getNumLayers() { return sl_numLayers; };
	double getGWTdepth() { return sl_GWT_depth; };

	void addNewLayer(GeometryLayer newLayer);
	void addNewLayerToLibrary(GeometryLayer newLayer);
	void setBaseLayer(GeometryBase baseLayer) { sl_base = baseLayer; };
	void set_GWT_depth(double GWT_depth) { sl_GWT_depth = GWT_depth; };
	GeometryLayer getLayerFromLibrary(std::string layer_tag) { return sl_layer_library.get_layer(layer_tag); };
	GeometryLayer getLayer(int index);
	GeometryBase getBase();
	double getNaturalPeriod();
	int read_geometry_file(std::string geometry_fn, std::string filetype = "__na__");

private:
	MaterialManager* theMatManager;
	SiteGeometryFile_Reader* geometry_file;

	std::vector<GeometryLayer> sl_layers;
	GeometryLayer_Library sl_layer_library;
	GeometryBase sl_base;

	double sl_natPeriod;
	double sl_totalThickness;
	double sl_equivalentVS;
	double sl_equivalentVP;
	double sl_GWT_depth;
	int    sl_numLayers;
};

#endif // !SiteGeometry_H