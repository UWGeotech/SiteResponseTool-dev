#include <string>
#include "NDMaterialFactory.h"

#include "NDMaterial.h"
#include "ElasticIsotropicThreeDimensional.h"
#include "J2CyclicBoundingSurface.h"

NDMaterialFactory::NDMaterialFactory()
{
}

NDMaterialFactory::~NDMaterialFactory()
{
}

NDMaterial*
NDMaterialFactory::create_elastic_material(LayerMaterial layer_mat)
{
    NDMaterial* theMat;
    int tag = 1;
    double E = std::stod( layer_mat.get_prperty("E") );
    double nu = std::stod( layer_mat.get_prperty("nu") );
    double rho = std::stod( layer_mat.get_prperty("rho") );

    theMat = new ElasticIsotropicMaterial(tag, E, nu, rho);

    return theMat;
}

NDMaterial* 
NDMaterialFactory::create_J2Cyclic_material(LayerMaterial layer_mat)
{
    NDMaterial* theMat;
    int tag = 1;

    double G = std::stod(layer_mat.get_prperty("G"));
    double K = std::stod(layer_mat.get_prperty("K"));
    double su = std::stod(layer_mat.get_prperty("su"));
    double rho = std::stod(layer_mat.get_prperty("rho"));
    double h = std::stod(layer_mat.get_prperty("h"));
    double m = std::stod(layer_mat.get_prperty("m"));
    double h0 = std::stod(layer_mat.get_prperty("h0"));
    double chi = std::stod(layer_mat.get_prperty("chi"));
    double beta = std::stod(layer_mat.get_prperty("beta"));

    theMat = new J2CyclicBoundingSurface(tag, G, K, su, rho, h, m, h0, chi, beta);
    return theMat;
}
