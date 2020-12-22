#ifndef NDMATERIALFACTORY_H
#define NDMATERIALFACTORY_H

#include "LayerMaterial.h"
#include "NDMaterial.h"

class NDMaterialFactory
{
public:
	NDMaterialFactory();
	~NDMaterialFactory();

	NDMaterial* create_elastic_material(LayerMaterial layer_mat, int tag = 1);
	NDMaterial* create_J2Cyclic_material(LayerMaterial layer_mat, int tag = 1);
};

#endif // !defien NDMATERIALFACTORY_H

