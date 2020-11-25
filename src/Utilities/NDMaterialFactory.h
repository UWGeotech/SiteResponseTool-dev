#ifndef NDMATERIALFACTORY_H
#define NDMATERIALFACTORY_H

#include "LayerMaterial.h"
#include "NDMaterial.h"

class NDMaterialFactory
{
public:
	NDMaterialFactory();
	~NDMaterialFactory();

	NDMaterial* create_elastic_material(LayerMaterial layer_mat);
	NDMaterial* create_J2Cyclic_material(LayerMaterial layer_mat);
};

#endif // !defien NDMATERIALFACTORY_H

