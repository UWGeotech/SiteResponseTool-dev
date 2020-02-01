/* ********************************************************************* **
**                 Site Response Analysis Tool                           **
**   -----------------------------------------------------------------   **
**                                                                       **
**   Developed by: UW Computational Geomechanics Group                   **
**                 Alborz Ghofrani (alborzgh@uw.edu)                     **
**                 Pedro Arduino (parduino@uw.edu)                       **
**                 University of Washington                              **
**                                                                       **
**   Date: October 2018                                                  **
**                                                                       **
** ********************************************************************* */

#ifndef SOILLAYER_H
#define SOILLAYER_H

#include <string>

class SoilLayer {
public:
	SoilLayer();
	~SoilLayer();

	SoilLayer(std::string name, double thickness, double vs, double vp, double rho, double Su, double hG, double m, double h0, double chi, bool IO);


	std::string getName() { return sl_name;  };
	double      getThickness() { return sl_thickness; };
	double      getShearVelocity() { return sl_vs; };
	double      getCompVelocity() { return sl_vp; };
	double      getRho() { return sl_rho; };
	double      getSu() { return sl_su; };
	double      getMat_h() { return sl_hG; };
	double      getMat_m() { return sl_m; };
	double      getMat_h0() { return sl_h0; };
	double      getMat_chi() { return sl_chi; };
	double      get_IO() { return sl_IO; };

	double      getMatShearModulus();
	double      getMatBulkModulus();
	double      getMatPoissonRatio();
	double      getNaturalPeriod();

private:
	std::string sl_name;
	double sl_thickness;
	double sl_vs;
	double sl_vp;
	double sl_rho;
	double sl_su;
	double sl_hG;
	double sl_m;
	double sl_h0;
	double sl_chi;
	bool sl_IO;
};

#endif