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

#include <vector>
#include <iostream>
#include <sstream>

#include "FEModel.h"

#include "Vector.h"
#include "Matrix.h"

#include "Node.h"
#include "Element.h"
#include "NDMaterial.h"
#include "SP_Constraint.h"
#include "MP_Constraint.h"
#include "LinearSeries.h"
#include "PathSeries.h"
#include "PathTimeSeries.h"
#include "LoadPattern.h"
#include "NodalLoad.h"
#include "AnalysisModel.h"
#include "CTestNormDispIncr.h"
#include "StaticAnalysis.h"
#include "DirectIntegrationAnalysis.h"
#include "EquiSolnAlgo.h"
#include "StaticIntegrator.h"
#include "TransientIntegrator.h"
#include "ConstraintHandler.h"
#include "RCM.h"
#include "DOF_Numberer.h"
#include "BandGenLinSolver.h"
#include "LinearSOE.h"
#include "NodeIter.h"
#include "ElementIter.h"
#include "DataFileStream.h"
#include "Recorder.h"
#include "UniaxialMaterial.h"
#include "ElementStateParameter.h"

#include "SSPquad.h"
#include "SSPquadUP.h"
#include "SSPbrick.h"
#include "SSPbrickUP.h"
#include "Brick.h"
#include "J2CyclicBoundingSurface.h"
#include "ElasticIsotropicMaterial.h"
#include "ElasticIsotropicPlaneStrain2D.h"
#include "ElasticMaterial.h"
#include "PM4Sand.h"
#include "PM4Silt.h"
#include "NewtonRaphson.h"
#include "LoadControl.h"
#include "Newmark.h"
#include "HHT.h"
#include "PenaltyConstraintHandler.h"
#include "TransformationConstraintHandler.h"
#include "BandGenLinLapackSolver.h"
#include "BandGenLinSOE.h"
#include "UmfpackGenLinSolver.h"
#include "UmfpackGenLinSOE.h"
#include "GroundMotion.h"
#include "ImposedMotionSP.h"
#include "TimeSeriesIntegrator.h"
#include "MultiSupportPattern.h"
#include "UniformExcitation.h"
#include "VariableTimeStepDirectIntegrationAnalysis.h"
#include "NodeRecorder.h"
#include "ElementRecorder.h"
#include "ViscousMaterial.h"
#include "ZeroLength.h"
#include "SingleDomParamIter.h"

#include "Information.h"
#include "SRT_Globals.h"

// empty constructor
SiteResponseModel::SiteResponseModel() :
	theMotionX(0),
	theMotionZ(0),
	theOutputDir(".")
{

}

// main constructor
SiteResponseModel::SiteResponseModel(SiteLayering layering, OutcropMotion* motionX, OutcropMotion* motionY, OutcropMotion* motionZ) :
	SRM_layering(layering),
	theMotionX(motionX),
	theMotionY(motionY),
	theMotionZ(motionZ),
	theOutputDir(".")
{
	// check if the motion is specified and create a FE domain
	if (theMotionX->isInitialized() || theMotionY->isInitialized()|| theMotionZ->isInitialized())
		theDomain = new Domain();
	else
	{
		opserr << "No motion is specified." << endln;
		exit(-1);
	}
}

// class destructor - delete the FE domain id it's created
SiteResponseModel::~SiteResponseModel() {
	if (theDomain != NULL)
		delete theDomain;
	theDomain = NULL;
}

// run a total stress site response analysis
int
SiteResponseModel::runTotalStressModel3DLotung()
{
	Vector zeroVec(3);
	zeroVec.Zero();

	std::vector<int> layerNumElems;
	std::vector<int> layerNumNodes;
	std::vector<double> layerElemSize;

	// setup the geometry and mesh parameters
	int numLayers = SRM_layering.getNumLayers();
	int numElems = 0;
	int numNodes = 0;

	// loop over the layers and setup the mesh
	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		double thisLayerThick = SRM_layering.getLayer(layerCount).getThickness();
		double thisLayerVS = SRM_layering.getLayer(layerCount).getShearVelocity();
		double thisLayerMinWL = thisLayerVS / program_config->getFloatProperty("Meshing|MaxFrequency");
		int thisLayerNumEle = 1;

		// save these in a vector for later use
		layerNumElems.push_back(thisLayerNumEle);
		layerNumNodes.push_back(4 * (thisLayerNumEle + (layerCount == 0)));
		layerElemSize.push_back(thisLayerThick);

		// add up number of elements and nodes
		numElems += thisLayerNumEle;
		numNodes += 4 * (thisLayerNumEle + (layerCount == numLayers - 2));

		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "Layer " << SRM_layering.getLayer(layerCount).getName().c_str() << " : Num Elements = " << thisLayerNumEle
			   << "(" << thisLayerThick / thisLayerNumEle << "), "
			   << ", Num Nodes = " << 4 * (thisLayerNumEle + (layerCount == 0)) << endln;
	}

	// FE mesh - create the nodes
	Node* theNode;

	double yCoord = 0.0;
	int nCount = 0;
	for (int layerCount = numLayers - 2; layerCount > -1; --layerCount)
	{
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "layer : " << SRM_layering.getLayer(layerCount).getName().c_str() << " - Number of Elements = "
			<< layerNumElems[layerCount] << " - Number of Nodes = " << layerNumNodes[layerCount]
			<< " - Element Thickness = " << layerElemSize[layerCount] << endln;
		
		for (int nodeCount = 0; nodeCount < layerNumNodes[layerCount]; nodeCount += 4)
		{
			theNode = new Node(nCount + nodeCount + 1, 3, 0.0, yCoord, 0.0); theDomain->addNode(theNode);
			theNode = new Node(nCount + nodeCount + 2, 3, 0.0, yCoord, 1.0); theDomain->addNode(theNode);
			theNode = new Node(nCount + nodeCount + 3, 3, 1.0, yCoord, 1.0); theDomain->addNode(theNode);
			theNode = new Node(nCount + nodeCount + 4, 3, 1.0, yCoord, 0.0); theDomain->addNode(theNode);

			if (program_config->getBooleanProperty("General|PrintDebug"))
			{
				opserr << "Node " << nCount + nodeCount + 1 << " - 0.0" << ", " << yCoord << ", 0.0" << endln;
				opserr << "Node " << nCount + nodeCount + 2 << " - 0.0" << ", " << yCoord << ", 1.0" << endln;
				opserr << "Node " << nCount + nodeCount + 3 << " - 1.0" << ", " << yCoord << ", 1.0" << endln;
				opserr << "Node " << nCount + nodeCount + 4 << " - 1.0" << ", " << yCoord << ", 0.0" << endln;
			}

			yCoord += layerElemSize[layerCount];
		}
		nCount += layerNumNodes[layerCount];
	}

	// FE mesh - apply fixities
	SP_Constraint* theSP;
	ID theSPtoRemove(8); // these fixities should be removed later on if compliant base is used
	theSP = new SP_Constraint(1, 0, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(0) = theSP->getTag();
	theSP = new SP_Constraint(1, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(1, 2, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(1) = theSP->getTag();
	theSP = new SP_Constraint(2, 0, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(2) = theSP->getTag();
	theSP = new SP_Constraint(2, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(2, 2, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(3) = theSP->getTag();
	theSP = new SP_Constraint(3, 0, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(4) = theSP->getTag();
	theSP = new SP_Constraint(3, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(3, 2, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(5) = theSP->getTag();
	theSP = new SP_Constraint(4, 0, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(6) = theSP->getTag();
	theSP = new SP_Constraint(4, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(4, 2, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(7) = theSP->getTag();

	// FE mesh - apply equalDOF
	MP_Constraint* theMP;
	Matrix Ccr(3, 3); Ccr(0, 0) = 1.0; Ccr(1, 1) = 1.0; Ccr(2, 2) = 1.0;
	ID rcDOF(3); rcDOF(0) = 0; rcDOF(1) = 1; rcDOF(2) = 2;
	for (int nodeCount = 4; nodeCount < numNodes; nodeCount += 4)
	{
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 2, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 3, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 4, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
	}

	// FE mesh - create the materials
	NDMaterial* theMat;
	SoilLayer theLayer;
	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		// get properties for this layer 
		theLayer = (SRM_layering.getLayer(numLayers - layerCount - 2));
		//theMat = new J2CyclicBoundingSurface(numLayers - layerCount - 1, theLayer.getMatShearModulus(), theLayer.getMatBulkModulus(),
		//theLayer.getSu(), theLayer.getRho(), theLayer.getMat_h()*theLayer.getMatShearModulus(), theLayer.getMat_m(),
		//theLayer.getMat_h0()*theLayer.getMatShearModulus(), theLayer.getMat_chi(), 0.5);
		//theMat = new ElasticIsotropicMaterial(numLayers - layerCount - 1, 2.0 * theLayer.getMatShearModulus()*(1.0+theLayer.getMatPoissonRatio()), theLayer.getMatPoissonRatio(), theLayer.getRho());
		theMat = new ElasticIsotropicMaterial(numLayers - layerCount - 1, 3.845521e+08, 0.48, 1850.0);

		OPS_addNDMaterial(theMat);

		if (program_config->getBooleanProperty("General|PrintDebug"))
		{
			opserr << "Material " << theLayer.getName().c_str() << " tag = " << numLayers - layerCount - 1 << endln;
			opserr << "        nu = " << 0.48 << ", E = " << 3.845521e+08<< endln;
			//opserr << "        nu = " << theLayer.getMatPoissonRatio() << ", E = " << 2.0 * theLayer.getMatShearModulus()*(1.0 + theLayer.getMatPoissonRatio()) << endln;

			//opserr << "ID =" << numLayers - layerCount - 1 << ", Go = " << theLayer.getMatShearModulus() << ", K = " << theLayer.getMatBulkModulus()
			//	<< ", Su = " << theLayer.getSu()     << ", rho = " << theLayer.getRho()     << ", h = " << theLayer.getMat_h() << ", m = " << theLayer.getMat_m()
			//	<< ", ho = " << theLayer.getMat_h0() << ", chi = " << theLayer.getMat_chi() << endln;
		}
	}

	// FE mesh - create soil elements and add the material state parameter
	Element* theEle;
	int nElem = 0;

	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		theMat = OPS_getNDMaterial(numLayers - layerCount - 1);
		for (int elemCount = 0; elemCount < layerNumElems[numLayers - layerCount - 2]; ++elemCount)
		{
			int node1Tag = 4 * (nElem + elemCount);
			
			//theEle = new SSPbrick(nElem + elemCount + 1, node1Tag + 1, node1Tag + 2, node1Tag + 3, node1Tag + 4, node1Tag + 5, 
			//	node1Tag + 6, node1Tag + 7, node1Tag + 8, *theMat, 0.0, -program_config->getFloatProperty("Units|g") * theMat->getRho()*1.0, 0.0);
			//	theDomain->addElement(theEle);

				theEle = new SSPbrick(nElem + elemCount + 1, node1Tag + 1, node1Tag + 2, node1Tag + 3, node1Tag + 4, node1Tag + 5,
					node1Tag + 6, node1Tag + 7, node1Tag + 8, *theMat, 0.0, -9.81*1850.0, 0.0);
				theDomain->addElement(theEle);

			//theEle = new Brick(nElem + elemCount + 1, node1Tag + 1, node1Tag + 2, node1Tag + 3, node1Tag + 4, node1Tag + 5,
			//	node1Tag + 6, node1Tag + 7, node1Tag + 8, *theMat, 0.0, -program_config->getFloatProperty("Units|g") * theMat->getRho()*1.0, 0.0);
			//theDomain->addElement(theEle);
		
			if (program_config->getBooleanProperty("General|PrintDebug"))
				opserr << "Element " << nElem + elemCount + 1 << ": Nodes = " << node1Tag + 1 << " to " << node1Tag + 8 << "  - Mat tag = " << numLayers - layerCount - 1 << endln;
		}
		nElem += layerNumElems[numLayers - layerCount - 2];
	}

	if (program_config->getBooleanProperty("General|PrintDebug"))
		opserr << "Total number of elements = " << nElem << endln;

	
	// Dynamic Analysis
	// ----------------

	// FE mesh - apply the motion
	int numSteps = 0;
	std::vector<double> dt;
	
	if (theMotionY->isInitialized())
	{
		// using uniform excitation to apply vertical motion
		LoadPattern* theLP = new UniformExcitation(*(theMotionY->getGroundMotion()), 1, 12, 0.0, 9.81);
		theDomain->addLoadPattern(theLP);

		// update the number of steps as well as the dt vector
		int temp = theMotionY->getNumSteps();
		if (temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionY->getDTvector();
		}
	}
	

	// FE mesh - using a stress input with the dashpot
	if (theMotionX->isInitialized())
	{
		// check if rigid base
		if (program_config->getBooleanProperty("Analysis|RigidBase"))
		{
			LoadPattern* theLP = new UniformExcitation(*(theMotionX->getGroundMotion()), 0, 13, 0.0, 9.81);
			theDomain->addLoadPattern(theLP);
		} else {
			opserr << "Viscous X not implemented yet = " << endln;
		}
		// update the number of steps as well as the dt vector
		int temp = theMotionX->getNumSteps();
		if ( temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionX->getDTvector();
		}
	}
	
	if (theMotionZ->isInitialized()) 
	{
		// check if rigid base
		if (program_config->getBooleanProperty("Analysis|RigidBase"))
		{
			LoadPattern* theLP = new UniformExcitation(*(theMotionZ->getGroundMotion()), 2, 14, 0.0, 9.81);
			theDomain->addLoadPattern(theLP);
		} else {
			opserr << "Viscous Z not implemented yet = " << endln;
		}
		int temp = theMotionZ->getNumSteps();
		if (temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionZ->getDTvector();
		}
	}
	

	// Define Analysis
	// ---------------
	AnalysisModel* theModel = new AnalysisModel();
	CTestNormDispIncr* theTest = new CTestNormDispIncr(program_config->getFloatProperty("Analysis|Dynamic|ConvergenceTest|Tolerance"),
	                                                   program_config->getIntProperty("Analysis|Dynamic|ConvergenceTest|MaxNumIterations"),
		                                               program_config->getIntProperty("Analysis|Dynamic|ConvergenceTest|PrintTag"));

	EquiSolnAlgo* theSolnAlgo = new NewtonRaphson(*theTest);
	ConstraintHandler* theHandler = new TransformationConstraintHandler();
	RCM* theRCM = new RCM();
	DOF_Numberer* theNumberer = new DOF_Numberer(*theRCM);

	// Define Dynamic Solver
	LinearSOE* theSOE = 0;
	if (program_config->getStringProperty("Analysis|Dynamic|Solver") == "BandGeneral") {
		BandGenLinSolver* theSolver = new BandGenLinLapackSolver();
		theSOE = new BandGenLinSOE(*theSolver);
	}
	else if (program_config->getStringProperty("Analysis|Dynamic|Solver") == "UmfPack") {
		UmfpackGenLinSolver *theSolver = new UmfpackGenLinSolver();
		// theSOE = new UmfpackGenLinSOE(*theSolver, factLVALUE, factorOnce, printTime);      
		theSOE = new UmfpackGenLinSOE(*theSolver);
	}
	else {
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "unknown Dynamic Solver = " << endln;
	}

	//Define Dynamic Integrator
	TransientIntegrator* theTransientIntegrator = 0;
	if (program_config->getStringProperty("Analysis|Dynamic|Integrator") == "Newmark") {
		theTransientIntegrator = new Newmark(program_config->getFloatProperty("Analysis|Dynamic|Newmark_Gamma"), program_config->getFloatProperty("Analysis|Dynamic|Newmark_Beta"));
	}
	else if (program_config->getStringProperty("Analysis|Dynamic|Integrator") == "HHT") {
		theTransientIntegrator = new HHT(program_config->getFloatProperty("Analysis|Dynamic|HHT_Alpha"));
	}
	else {
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "unknown Dynamic Integrator = " << endln;
	}

	// Define Dynamic Analysis
	DirectIntegrationAnalysis* theTransientAnalysis;
	theTransientAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

	//VariableTimeStepDirectIntegrationAnalysis* theTransientAnalysis;
	//theTransientAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);


	// FE mesh - reset time in the domain
	theDomain->setCurrentTime(0.0);

	// FE mesh - create the output streams
	OPS_Stream* theOutputStream;
	Recorder* theRecorder;

	// record last node's results
	ID nodesToRecord(1);
	nodesToRecord(0) = numNodes;

	ID dofToRecord(3);
	dofToRecord(0) = 0;
	dofToRecord(1) = 1;
	dofToRecord(2) = 2;

	// surface recorder
	std::string outFile = theOutputDir + PATH_SEPARATOR +  "surface.acc";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.vel";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.disp";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);


	// recorder for bottom of layers
	nCount = 0;
	for (int layerCount = numLayers - 2; layerCount > -1; --layerCount)
	{		
		opserr << "layer_IO : " << SRM_layering.getLayer(layerCount).get_IO() << endln;
		nodesToRecord(0) = nCount + 1;
		if (SRM_layering.getLayer(layerCount).get_IO())
		{
			//nodesToRecord(0) = nCount + 1;

			opserr << "layer : " << SRM_layering.getLayer(layerCount).getName().c_str() << " - Number of Elements = "
				<< layerNumElems[layerCount] << " - Number of Nodes = " << layerNumNodes[layerCount]
				<< " - Element Thickness = " << layerElemSize[layerCount] << ", nodes being recorded: " << nodesToRecord << endln;

			outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".acc";
			theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
			theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, 0.0, true, NULL);
			theDomain->addRecorder(*theRecorder);

			outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".vel";
			theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
			theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, 0.0, true, NULL);
			theDomain->addRecorder(*theRecorder);

			outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".disp";
			theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
			theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, 0.0, true, NULL);
			theDomain->addRecorder(*theRecorder);

			//nCount += layerNumNodes[layerCount];
		}
		nCount += layerNumNodes[layerCount];
	}
	
	// FE mesh - perform analysis
	opserr << "Analysis started:" << endln;
	std::stringstream progressBar;
	for (int analysisCount = 0; analysisCount < numSteps; ++analysisCount) {
		//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		//double stepDT = dt[analysisCount];
		double stepDT = 0.02;

		//int converged = theTransientAnalysis->analyze(1, stepDT, stepDT / 2.0, stepDT * 2.0, 1);
		int converged = theTransientAnalysis->analyze(1, stepDT);
		if (!converged) {
			opserr << "Converged at time " << theDomain->getCurrentTime() << endln;

			if (analysisCount % (int)(numSteps / 20) == 0)
			{
				progressBar << "\r[";
				for (int ii = 0; ii < (int)(20 * analysisCount / numSteps); ii++)
					progressBar << ".";
				for (int ii = (int)(20 * analysisCount / numSteps); ii < 20; ii++)
					progressBar << "-";

				progressBar << "]  " << (int)(100 * analysisCount / numSteps) << "%";
				opsout << progressBar.str().c_str();
				opsout.flush();
			}
		}
		else {
			opserr << "Site response analysis did not converge." << endln;
			exit(-1);
		}
	}
	progressBar << "\r[";
	for (int ii = 0; ii < 20; ii++)
		progressBar << ".";

	progressBar << "] 100%";
	opsout << progressBar.str().c_str();
	opsout.flush();
	opsout << endln;

	return 0;
}

// run a total stress site response analysis
int
SiteResponseModel::runTotalStressModel3D()
{
	Vector zeroVec(3);
	zeroVec.Zero();

	std::vector<int> layerNumElems;
	std::vector<int> layerNumNodes;
	std::vector<double> layerElemSize;

	// setup the geometry and mesh parameters
	int numLayers = SRM_layering.getNumLayers();
	int numElems = 0;
	int numNodes = 0;

	// loop over the layers and setup the mesh
	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		double thisLayerThick = SRM_layering.getLayer(layerCount).getThickness();
		double thisLayerVS = SRM_layering.getLayer(layerCount).getShearVelocity();
		double thisLayerMinWL = thisLayerVS / program_config->getFloatProperty("Meshing|MaxFrequency");

		// calculate the thickness of elements in this layer
		int thisLayerNumEle = 1;
		if (!program_config->getBooleanProperty("Meshing|Manual")) {
			thisLayerThick = (thisLayerThick < thisLayerMinWL) ? thisLayerMinWL : thisLayerThick;
			// calculate number of elements in this layer
			thisLayerNumEle = program_config->getIntProperty("Meshing|NumNodesPerWaveLength") * static_cast<int>(thisLayerThick / thisLayerMinWL) - 1;
		}
		// save these in a vector for later use
		layerNumElems.push_back(thisLayerNumEle);
		layerNumNodes.push_back(4 * (thisLayerNumEle + (layerCount == 0)));
		layerElemSize.push_back(thisLayerThick / thisLayerNumEle);

		// add up number of elements and nodes
		numElems += thisLayerNumEle;
		numNodes += 4 * (thisLayerNumEle + (layerCount == 0));

		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "Layer " << SRM_layering.getLayer(layerCount).getName().c_str() << " : Num Elements = " << thisLayerNumEle
			<< "(" << thisLayerThick / thisLayerNumEle << "), "
			<< ", Num Nodes = " << 4 * (thisLayerNumEle + (layerCount == 0)) << endln;
	}

	// FE mesh - create the nodes
	Node* theNode;

	double yCoord = 0.0;
	int nCount = 0;
	for (int layerCount = numLayers - 2; layerCount > -1; --layerCount)
	{
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "layer : " << SRM_layering.getLayer(layerCount).getName().c_str() << " - Number of Elements = "
			<< layerNumElems[layerCount] << " - Number of Nodes = " << layerNumNodes[layerCount]
			<< " - Element Thickness = " << layerElemSize[layerCount] << endln;

		for (int nodeCount = 0; nodeCount < layerNumNodes[layerCount]; nodeCount += 4)
		{
			theNode = new Node(nCount + nodeCount + 1, 3, 0.0, yCoord, 0.0); theDomain->addNode(theNode);
			theNode = new Node(nCount + nodeCount + 2, 3, 0.0, yCoord, 1.0); theDomain->addNode(theNode);
			theNode = new Node(nCount + nodeCount + 3, 3, 1.0, yCoord, 1.0); theDomain->addNode(theNode);
			theNode = new Node(nCount + nodeCount + 4, 3, 1.0, yCoord, 0.0); theDomain->addNode(theNode);

			if (program_config->getBooleanProperty("General|PrintDebug"))
			{
				opserr << "Node " << nCount + nodeCount + 1 << " - 0.0" << ", " << yCoord << ", 0.0" << endln;
				opserr << "Node " << nCount + nodeCount + 2 << " - 0.0" << ", " << yCoord << ", 1.0" << endln;
				opserr << "Node " << nCount + nodeCount + 3 << " - 1.0" << ", " << yCoord << ", 1.0" << endln;
				opserr << "Node " << nCount + nodeCount + 4 << " - 1.0" << ", " << yCoord << ", 0.0" << endln;
			}

			yCoord += layerElemSize[layerCount];
		}
		nCount += layerNumNodes[layerCount];
	}

	// FE mesh - apply fixities
	SP_Constraint* theSP;
	ID theSPtoRemove(8); // these fixities should be removed later on if compliant base is used
	theSP = new SP_Constraint(1, 0, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(0) = theSP->getTag();
	theSP = new SP_Constraint(1, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(1, 2, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(1) = theSP->getTag();
	theSP = new SP_Constraint(2, 0, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(2) = theSP->getTag();
	theSP = new SP_Constraint(2, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(2, 2, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(3) = theSP->getTag();
	theSP = new SP_Constraint(3, 0, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(4) = theSP->getTag();
	theSP = new SP_Constraint(3, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(3, 2, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(5) = theSP->getTag();
	theSP = new SP_Constraint(4, 0, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(6) = theSP->getTag();
	theSP = new SP_Constraint(4, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(4, 2, 0.0, true); theDomain->addSP_Constraint(theSP); theSPtoRemove(7) = theSP->getTag();

	// FE mesh - apply equalDOF
	MP_Constraint* theMP;
	Matrix Ccr(3, 3); Ccr(0, 0) = 1.0; Ccr(1, 1) = 1.0; Ccr(2, 2) = 1.0;
	ID rcDOF(3); rcDOF(0) = 0; rcDOF(1) = 1; rcDOF(2) = 2;
	for (int nodeCount = 4; nodeCount < numNodes; nodeCount += 4)
	{
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 2, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 3, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 4, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
	}

	// FE mesh - create the materials
	NDMaterial* theMat;
	SoilLayer theLayer;
	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		// get properties for this layer 
		theLayer = (SRM_layering.getLayer(numLayers - layerCount - 2));

		theMat = new J2CyclicBoundingSurface(numLayers - layerCount - 1, theLayer.getMatShearModulus(), theLayer.getMatBulkModulus(),
		theLayer.getSu(), theLayer.getRho(), theLayer.getMat_h()*theLayer.getMatShearModulus(), theLayer.getMat_m(),
		theLayer.getMat_h0()*theLayer.getMatShearModulus(), theLayer.getMat_chi(), 0.5);
		//theMat = new ElasticIsotropicMaterial(numLayers - layerCount - 1, 2.0 * theLayer.getMatShearModulus()*(1.0 + theLayer.getMatPoissonRatio()), theLayer.getMatPoissonRatio(), theLayer.getRho());
		//theMat = new ElasticIsotropicMaterial(numLayers - layerCount - 1, 3.845521e+08, 0.48, 1850.0);
		
		OPS_addNDMaterial(theMat);

		if (program_config->getBooleanProperty("General|PrintDebug"))
		{
			opserr << "Material " << theLayer.getName().c_str() << " tag = " << numLayers - layerCount - 1 << endln;
			//opserr << "        nu = " << theLayer.getMatPoissonRatio() << ", E = " << 2.0 * theLayer.getMatShearModulus()*(1.0+theLayer.getMatPoissonRatio()) << endln;
			opserr << "ID =" << numLayers - layerCount - 1 << ", Go = " << theLayer.getMatShearModulus() << ", K = " << theLayer.getMatBulkModulus()
				<< ", Su = " << theLayer.getSu() << ", rho = " << theLayer.getRho() << ", h = " << theLayer.getMat_h() << ", m = " << theLayer.getMat_m()
				<< ", ho = " << theLayer.getMat_h0() << ", chi = " << theLayer.getMat_chi() << endln;
		}
	}

	// FE mesh - create soil elements and add the material state parameter
	Element* theEle;
	Parameter* theParameter;
	char** paramArgs = new char*[2];

	paramArgs[0] = new char[15];
	paramArgs[1] = new char[5];
	sprintf(paramArgs[0], "materialState");

	int nElem = 0;

	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		theMat = OPS_getNDMaterial(numLayers - layerCount - 1);
		for (int elemCount = 0; elemCount < layerNumElems[numLayers - layerCount - 2]; ++elemCount)
		{
			int node1Tag = 4 * (nElem + elemCount);

			
			theEle = new SSPbrick(nElem + elemCount + 1, node1Tag + 1, node1Tag + 2, node1Tag + 3, node1Tag + 4, node1Tag + 5, 
				node1Tag + 6, node1Tag + 7, node1Tag + 8, *theMat, 0.0, -program_config->getFloatProperty("Units|g") * theMat->getRho(), 0.0);

			theDomain->addElement(theEle);

			//theEle = new Brick(nElem + elemCount + 1, node1Tag + 1, node1Tag + 2, node1Tag + 3, node1Tag + 4, node1Tag + 5,
			//	node1Tag + 6, node1Tag + 7, node1Tag + 8, *theMat, 0.0, -program_config->getFloatProperty("Units|g") * theMat->getRho()*1.0, 0.0);
			//theDomain->addElement(theEle);

			theParameter = new Parameter(nElem + elemCount + 1, 0, 0, 0);
			sprintf(paramArgs[1], "%d", theMat->getTag());
			theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
			theDomain->addParameter(theParameter);

			if (program_config->getBooleanProperty("General|PrintDebug"))
				opserr << "Element " << nElem + elemCount + 1 << ": Nodes = " << node1Tag + 1 << " to " << node1Tag + 8 << "  - Mat tag = " << numLayers - layerCount - 1 << ", Gamma = " << -program_config->getFloatProperty("Units|g") * theMat->getRho() << endln;
		}
		nElem += layerNumElems[numLayers - layerCount - 2];
	}

	if (program_config->getBooleanProperty("General|PrintDebug"))
		opserr << "Total number of elements = " << nElem << endln;

	// Gravity Analysis
	// ----------------
	// FE mesh - update material stage
	ParameterIter& theParamIter = theDomain->getParameters();
	while ((theParameter = theParamIter()) != 0)
	{
		theParameter->update(0.0);
	}

	// FE mesh - create analysis objects - static analysis for gravity
	AnalysisModel* theModel = new AnalysisModel();
	CTestNormDispIncr* theTest = new CTestNormDispIncr(program_config->getFloatProperty("Analysis|Gravity|ConvergenceTest|Tolerance"),
		                                               program_config->getIntProperty("Analysis|Gravity|ConvergenceTest|MaxNumIterations"),
		                                               program_config->getIntProperty("Analysis|Gravity|ConvergenceTest|PrintTag"));

	EquiSolnAlgo* theSolnAlgo = new NewtonRaphson(*theTest);
	StaticIntegrator* theIntegrator = new LoadControl(0.05, 1, 0.05, 1.0);
	//TransientIntegrator* theIntegrator = new Newmark(0.5, 0.25);
	//ConstraintHandler* theHandler = new PenaltyConstraintHandler(1.0e14, 1.0e14);
	ConstraintHandler* theHandler = new TransformationConstraintHandler();
	RCM* theRCM = new RCM();
	DOF_Numberer* theNumberer = new DOF_Numberer(*theRCM);

	LinearSOE* theSOE = 0;
	if (program_config->getStringProperty("Analysis|Gravity|Solver") == "BandGeneral") {
		BandGenLinSolver* theSolver = new BandGenLinLapackSolver();
		theSOE = new BandGenLinSOE(*theSolver);
	}
	else if (program_config->getStringProperty("Analysis|Gravity|Solver") == "UmfPack") {
		UmfpackGenLinSolver *theSolver = new UmfpackGenLinSolver();
		// theSOE = new UmfpackGenLinSOE(*theSolver, factLVALUE, factorOnce, printTime);      
		theSOE = new UmfpackGenLinSOE(*theSolver);
	}
	else {
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "unknown Solver = " << endln;
	}
	//DirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	StaticAnalysis *theAnalysis;
	theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator);
	theAnalysis->setConvergenceTest(*theTest);

	if (program_config->getBooleanProperty("Analysis|Gravity|PerformGravity")) {
		for (int analysisCount = 0; analysisCount < 10; ++analysisCount) {
			//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
			int converged = theAnalysis->analyze(1);
			if (!converged) {
				opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
			}
		}
		
		// FE mesh - update material response to plastic
		theParamIter = theDomain->getParameters();
		while ((theParameter = theParamIter()) != 0)
		{
			theParameter->update(1.0);
		}
		
		for (int analysisCount = 0; analysisCount < 10; ++analysisCount) {
			//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
			int converged = theAnalysis->analyze(1);
			if (!converged) {
				opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
			}
		}
	}
	
	// theAnalysis->clearAll();
	// delete theAnalysis;
	// theAnalysis = 0;

	// Dynamic Analysis
	// ----------------
	
	double vis_C;
	if (! program_config->getBooleanProperty("Analysis|RigidBase"))
	{
		
		// FE mesh - add the compliant base - use the last layer properties
		vis_C = SRM_layering.getLayer(numLayers - 1).getShearVelocity() * SRM_layering.getLayer(numLayers - 1).getRho();
		UniaxialMaterial* theViscousMats[2];
		theViscousMats[0] = new ViscousMaterial(numLayers + 10, vis_C, 1.0); OPS_addUniaxialMaterial(theViscousMats[0]);
		theViscousMats[1] = new ViscousMaterial(numLayers + 20, vis_C, 1.0); OPS_addUniaxialMaterial(theViscousMats[1]);
		ID directions(2);
		directions(0) = 0; directions(1) = 2;
		
		// FE mesh - create dashpot nodes and apply proper fixities
		theNode = new Node(numNodes + 1, 3, 0.0, 0.0, 0.0, NULL); theDomain->addNode(theNode);
		theNode = new Node(numNodes + 2, 3, 0.0, 0.0, 0.0, NULL); theDomain->addNode(theNode);
		theSP = new SP_Constraint(numNodes + 1, 0, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(numNodes + 1, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(numNodes + 1, 2, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(numNodes + 2, 1, 0.0, true); theDomain->addSP_Constraint(theSP);

		// FE mesh - apply equalDOF to the node connected to the column
		Matrix constrainInXZ(2, 2); constrainInXZ(0, 0) = 1.0; constrainInXZ(1, 1) = 1.0;
		ID constDOF(2); constDOF(0) = 0; constDOF(1) = 2;
		theMP = new MP_Constraint(1, numNodes + 2, constrainInXZ, constDOF, constDOF); theDomain->addMP_Constraint(theMP);

		// FE mesh - remove fixities created for gravity
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(0)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(1)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(2)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(3)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(4)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(5)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(6)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(7)); delete theSP;

		// FE mesh - equalDOF the first 4 nodes
		theMP = new MP_Constraint(1, 2, constrainInXZ, constDOF, constDOF); theDomain->addMP_Constraint(theMP);
		theMP = new MP_Constraint(1, 3, constrainInXZ, constDOF, constDOF); theDomain->addMP_Constraint(theMP);
		theMP = new MP_Constraint(1, 4, constrainInXZ, constDOF, constDOF); theDomain->addMP_Constraint(theMP);

		// FE mesh - create the dashpot element
		Vector x(3); x(0) = 1.0; x(1) = 0.0; x(2) = 0.0;
		Vector y(3); y(1) = 1.0; y(0) = 0.0; y(2) = 0.0;
		theEle = new ZeroLength(numElems + 1, 3, numNodes + 1, numNodes + 2, x, y, 2, theViscousMats, directions);
		theDomain->addElement(theEle);
	}

	// FE mesh - apply the motion
	int numSteps = 0;
	std::vector<double> dt;

	// FE mesh - using multiple support
	//MultiSupportPattern* theLP = new MultiSupportPattern(1);
	//theLP->addMotion(*theMotionX->getGroundMotion(), 1);
	//theLP->addSP_Constraint(new ImposedMotionSP(1, 0, 1, 1));
	//theLP->addSP_Constraint(new ImposedMotionSP(2, 0, 1, 1));
	//theLP->addSP_Constraint(new ImposedMotionSP(3, 0, 1, 1));
	//theLP->addSP_Constraint(new ImposedMotionSP(4, 0, 1, 1));

	if (theMotionY->isInitialized())
	{
		// using uniform excitation to apply vertical motion
		LoadPattern* theLP = new UniformExcitation(*(theMotionY->getGroundMotion()), 1, 12, 0.0, program_config->getFloatProperty("Units|g"));
		theDomain->addLoadPattern(theLP);

		// update the number of steps as well as the dt vector
		int temp = theMotionY->getNumSteps();
		if (temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionY->getDTvector();
		}
	}

	// FE mesh - using a stress input with the dashpot
	if (theMotionX->isInitialized())
	{
		// check if rigid base
		if (program_config->getBooleanProperty("Analysis|RigidBase"))
		{
			LoadPattern* theLP = new UniformExcitation(*(theMotionX->getGroundMotion()), 0, 13, 0.0, program_config->getFloatProperty("Units|g"));
			theDomain->addLoadPattern(theLP);
		}
		else {
			LoadPattern* theLP = new LoadPattern(1, vis_C);
			theLP->setTimeSeries(theMotionX->getVelSeries());

			NodalLoad* theLoad;
			Vector load(3);
			load(0) = 1.0;
			load(1) = 0.0;
			load(2) = 0.0;

			theLoad = new NodalLoad(1, numNodes + 2, load, false); theLP->addNodalLoad(theLoad);
			theDomain->addLoadPattern(theLP);
		}
		// update the number of steps as well as the dt vector
		int temp = theMotionX->getNumSteps();
		if (temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionX->getDTvector();
		}
	}

	if (theMotionZ->isInitialized())
	{
		// check if rigid base
		if (program_config->getBooleanProperty("Analysis|RigidBase"))
		{
			LoadPattern* theLP = new UniformExcitation(*(theMotionZ->getGroundMotion()), 2, 14, 0.0, program_config->getFloatProperty("Units|g"));
			theDomain->addLoadPattern(theLP);
		}
		else {
			LoadPattern* theLP = new LoadPattern(2, vis_C);
			theLP->setTimeSeries(theMotionZ->getVelSeries());

			NodalLoad* theLoad;
			Vector load(3);
			load(0) = 0.0;
			load(1) = 0.0;
			load(2) = 1.0;

			theLoad = new NodalLoad(2, numNodes + 2, load, false); theLP->addNodalLoad(theLoad);
			theDomain->addLoadPattern(theLP);
		}
		int temp = theMotionZ->getNumSteps();
		if (temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionZ->getDTvector();
		}
	}

	// I have to change to a transient analysis
	// FE mesh - remove the static analysis and create new transient objects
	// theModel = new AnalysisModel();
	// theTest = new CTestNormDispIncr(program_config->getFloatProperty("Analysis|Dynamic|ConvergenceTest|Tolerance"), 
	// 	                                               program_config->getIntProperty("Analysis|Dynamic|ConvergenceTest|MaxNumIterations"), 
	// 	                                               program_config->getIntProperty("Analysis|Dynamic|ConvergenceTest|PrintTag"));
	// theSolnAlgo = new NewtonRaphson(*theTest);
	//TransientIntegrator* theIntegrator = new Newmark(0.5, 0.25);
	//ConstraintHandler* theHandler = new PenaltyConstraintHandler(1.0e14, 1.0e14);
	// theHandler = new TransformationConstraintHandler();
	// theRCM = new RCM();
	// theNumberer = new DOF_Numberer(*theRCM);

	//DirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);


	// Define Dynamic Solver
	if (program_config->getStringProperty("Analysis|Dynamic|Solver") == "BandGeneral") {
		BandGenLinSolver* theSolver = new BandGenLinLapackSolver();
		theSOE = new BandGenLinSOE(*theSolver);
	}
	else if (program_config->getStringProperty("Analysis|Dynamic|Solver") == "UmfPack") {
		UmfpackGenLinSolver *theSolver = new UmfpackGenLinSolver();
		// theSOE = new UmfpackGenLinSOE(*theSolver, factLVALUE, factorOnce, printTime);      
		theSOE = new UmfpackGenLinSOE(*theSolver);
	}
	else {
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "unknown Dynamic Solver = " << endln;
	}

	//Define Dynamic Integrator
	TransientIntegrator* theTransientIntegrator = 0;
	if (program_config->getStringProperty("Analysis|Dynamic|Integrator") == "Newmark") {
		theTransientIntegrator = new Newmark(program_config->getFloatProperty("Analysis|Dynamic|Newmark_Gamma"), program_config->getFloatProperty("Analysis|Dynamic|Newmark_Beta"));
	}
	else if (program_config->getStringProperty("Analysis|Dynamic|Integrator") == "HHT") {
		theTransientIntegrator = new HHT(program_config->getFloatProperty("Analysis|Dynamic|HHT_Alpha"));
	}
	else {
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "unknown Dynamic Integrator = " << endln;
	}

	theTest->setTolerance(program_config->getFloatProperty("Analysis|Dynamic|ConvergenceTest|Tolerance"));

	//DirectIntegrationAnalysis* theTransientAnalysis;
	//theTransientAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

	VariableTimeStepDirectIntegrationAnalysis* theTransientAnalysis;
	theTransientAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

	// FE mesh - reset time in the domain
	theDomain->setCurrentTime(0.0);

	// define Rayleigh damping
	double omega1 = 0.0;
	double omega2 = 0.0;
	if (program_config->getBooleanProperty("Analysis|Damping|ModalRayleigh"))
	{
		double natPeriod = SRM_layering.getNaturalPeriod();
		omega1 = 2.0 * PI * (2 * program_config->getFloatProperty("Analysis|Damping|Mode1") - 1) / natPeriod;
		omega2 = 2.0 * PI * (2 * program_config->getFloatProperty("Analysis|Damping|Mode2") - 1) / natPeriod;
	}
	else {
		omega1 = 2.0 * PI * program_config->getFloatProperty("Analysis|Damping|Frequency1");
		omega2 = 2.0 * PI * program_config->getFloatProperty("Analysis|Damping|Frequency2");
	}

	if (program_config->getBooleanProperty("Analysis|Damping|ElemByElem"))
	{
		opserr << "This part is not implemented yet." << endln;
		exit(-1);
	}
	else {
		double dampRatio = program_config->getFloatProperty("Analysis|Damping|Ratio");
		double a0 = dampRatio * (2.0 * omega1 * omega2) / (omega1 + omega2);
		double a1 = dampRatio * (2.0 / (omega1 + omega2));
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "a0 = " << a0 << "    a1 = " << a1 << endln;
		theDomain->setRayleighDampingFactors(a0, 0.0, a1, 0.0);
	}


	// FE mesh - create the output streams
	OPS_Stream* theOutputStream;
	Recorder* theRecorder;

	// record last node's results
	ID nodesToRecord(1);
	nodesToRecord(0) = numNodes;

	ID dofToRecord(3);
	dofToRecord(0) = 0;
	dofToRecord(1) = 1;
	dofToRecord(2) = 2;

	// surface recorder
	std::string outFile = theOutputDir + PATH_SEPARATOR + "surface.acc";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.vel";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.disp";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);


	// recorder for bottom of layers if IOStr = true
	nCount = 0;
	for (int layerCount = numLayers - 2; layerCount > -1; --layerCount)
	{
		opserr << "layer_IO : " << SRM_layering.getLayer(layerCount).get_IO() << endln;
		nodesToRecord(0) = nCount + 1;
		if (SRM_layering.getLayer(layerCount).get_IO())
		{
			//nodesToRecord(0) = nCount + 1;

			opserr << "layer : " << SRM_layering.getLayer(layerCount).getName().c_str() << " - Number of Elements = "
				<< layerNumElems[layerCount] << " - Number of Nodes = " << layerNumNodes[layerCount]
				<< " - Element Thickness = " << layerElemSize[layerCount] << ", nodes being recorded: " << nodesToRecord << endln;

			outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".acc";
			theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
			theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, 0.0, true, NULL);
			theDomain->addRecorder(*theRecorder);

			outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".vel";
			theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
			theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, 0.0, true, NULL);
			theDomain->addRecorder(*theRecorder);

			outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".disp";
			theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
			theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, 0.0, true, NULL);
			theDomain->addRecorder(*theRecorder);

			//nCount += layerNumNodes[layerCount];
		}
		nCount += layerNumNodes[layerCount];
	}

	// record element results
	// OPS_Stream* theOutputStream2;
	// ID elemsToRecord(5);
	// elemsToRecord(0) = 1;
	// elemsToRecord(1) = 2;
	// elemsToRecord(2) = 3;
	// elemsToRecord(3) = 4;
	// elemsToRecord(4) = 5;
	// const char* eleArgs = "stress";
	// 
	// theOutputStream2 = new DataFileStream("Output2.out", OVERWRITE, 2, 0, false, 6, false);
	// theRecorder = new ElementRecorder(&elemsToRecord, &eleArgs, 1, true, *theDomain, *theOutputStream2, 0.0, NULL);
	// theDomain->addRecorder(*theRecorder);

	// FE mesh - perform analysis
	opserr << "Analysis started:" << endln;
	std::stringstream progressBar;
	numSteps = 1777;
	for (int analysisCount = 0; analysisCount < numSteps; ++analysisCount) {
		//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		double stepDT = dt[analysisCount];

		//double stepDT = 0.02;

		int converged = theTransientAnalysis->analyze(1, stepDT, stepDT / 2.0, stepDT * 2.0, 1);
		//int converged = theTransientAnalysis->analyze(1, stepDT);
		if (!converged) {
			opserr << "Converged at time " << theDomain->getCurrentTime() << endln;

			if (analysisCount % (int)(numSteps / 20) == 0)
			{
				progressBar << "\r[";
				for (int ii = 0; ii < (int)(20 * analysisCount / numSteps); ii++)
					progressBar << ".";
				for (int ii = (int)(20 * analysisCount / numSteps); ii < 20; ii++)
					progressBar << "-";

				progressBar << "]  " << (int)(100 * analysisCount / numSteps) << "%";
				opsout << progressBar.str().c_str();
				opsout.flush();
			}
		}
		else {
			opserr << "Site response analysis did not converge." << endln;
			exit(-1);
		}
	}
	progressBar << "\r[";
	for (int ii = 0; ii < 20; ii++)
		progressBar << ".";

	progressBar << "] 100%";
	opsout << progressBar.str().c_str();
	opsout.flush();
	opsout << endln;


	opserr << *theDomain << endln;

	//if (program_config->getBooleanProperty("General|PrintDebug"))
	//{
	//	Information info;
	//	theEle = theDomain->getElement(1);
	//	theEle->getResponse(1, info);
	//	opserr << "Stress = " << info.getData();
	//	theEle->getResponse(2, info);
	//	opserr << "Strain = " << info.getData();
	//}

	//int count = 0;
	//NodeIter& theNodeIter = theDomain->getNodes();
	//Node * thisNode;
	//while ((thisNode = theNodeIter()) != 0)
	//{
	//	count++;
	//	opserr << "Node " << thisNode->getTag() << " = " << thisNode->getCrds() << endln;
	//}

	//int count = 0;
	//ElementIter& theEleIter = theDomain->getElements();
	//Element * thisEle;
	//while ((thisEle = theEleIter()) != 0)
	//{
	//	count++;
	//	opserr << "Element " << thisEle->getTag() << " = " << thisEle->getExternalNodes() << endln;
	//}

	return 0;
}

// run an effective stress site response analysis
int
SiteResponseModel::runEffectiveStressModel2D()
{
	Vector zeroVec(2);
	zeroVec.Zero();

	std::vector<int> layerNumElems;
	std::vector<int> layerNumNodes;
	std::vector<double> layerElemSize;

	// setup the geometry and mesh parameters
	int numLayers = SRM_layering.getNumLayers();
	int numElems = 0;
	int numNodes = 0;

	// loop over the layers and setup the mesh
	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		double thisLayerThick = SRM_layering.getLayer(layerCount).getThickness();
		double thisLayerVS = SRM_layering.getLayer(layerCount).getShearVelocity();
		double thisLayerMinWL = thisLayerVS / program_config->getFloatProperty("Meshing|MaxFrequency");

		// calculate the thickness of elements in this layer
		thisLayerThick = (thisLayerThick < thisLayerMinWL) ? thisLayerMinWL : thisLayerThick;

		// calculate number of elements in this layer
		int thisLayerNumEle = program_config->getIntProperty("Meshing|NumNodesPerWaveLength") * static_cast<int>(thisLayerThick / thisLayerMinWL) - 1;
		
		// save these in a vector for later use
		layerNumElems.push_back(thisLayerNumEle);
		layerNumNodes.push_back(2 * (thisLayerNumEle + (layerCount == 0)));
		layerElemSize.push_back(thisLayerThick / thisLayerNumEle);

		// add up number of elements and nodes
		numElems += thisLayerNumEle;
		numNodes += 2 * (thisLayerNumEle + (layerCount == numLayers - 2));

		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "Layer " << SRM_layering.getLayer(layerCount).getName().c_str() << " : Num Elements = " << thisLayerNumEle
			   << "(" << thisLayerThick / thisLayerNumEle << "), "
			   << ", Num Nodes = " << 2 * (thisLayerNumEle + (layerCount == 0)) << endln;
	}

	// FE mesh - create the nodes
	Node* theNode;

	double yCoord = 0.0;
	int nCount = 0;
	for (int layerCount = numLayers - 2; layerCount > -1; --layerCount)
	{
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "layer : " << SRM_layering.getLayer(layerCount).getName().c_str() << " - Number of Elements = "
			<< layerNumElems[layerCount] << " - Number of Nodes = " << layerNumNodes[layerCount]
			<< " - Element Thickness = " << layerElemSize[layerCount] << endln;
		
		for (int nodeCount = 0; nodeCount < layerNumNodes[layerCount]; nodeCount += 2)
		{
			theNode = new Node(nCount + nodeCount + 1, 3, 0.0, yCoord, 0.0); theDomain->addNode(theNode);
			theNode = new Node(nCount + nodeCount + 2, 3, 1.0, yCoord, 0.0); theDomain->addNode(theNode);

			if (program_config->getBooleanProperty("General|PrintDebug"))
			{
				opserr << "Node " << nCount + nodeCount + 1 << " - 0.0" << ", " << yCoord << endln;
				opserr << "Node " << nCount + nodeCount + 2 << " - 1.0" << ", " << yCoord << endln;
			}

			yCoord += layerElemSize[layerCount];
		}
		nCount += layerNumNodes[layerCount];
	}

	// FE mesh - apply fixities
	SP_Constraint* theSP;
	ID theSPtoRemove(2); // these fixities should be removed later on if compliant base is used
	theSP = new SP_Constraint(1, 0, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(0) = theSP->getTag();
	theSP = new SP_Constraint(1, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(2, 0, 0.0, true);  theDomain->addSP_Constraint(theSP); theSPtoRemove(1) = theSP->getTag();
	theSP = new SP_Constraint(2, 1, 0.0, true); theDomain->addSP_Constraint(theSP);

	for (int nodeCount = 1; nodeCount <= numNodes; nodeCount++)
	{
		theSP = new SP_Constraint(nodeCount, 2, 0.0, true); theDomain->addSP_Constraint(theSP);
	}

	// FE mesh - apply equalDOF
	MP_Constraint* theMP;
	Matrix Ccr(2, 2); Ccr(0, 0) = 1.0; Ccr(1, 1) = 1.0;
	ID rcDOF(2); rcDOF(0) = 0; rcDOF(1) = 1;
	for (int nodeCount = 2; nodeCount < numNodes; nodeCount += 2)
	{
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 2, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
	}

	// FE mesh - create the materials
	NDMaterial* theMat;
	SoilLayer theLayer;
	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		// get properties for this layer 
		theLayer = (SRM_layering.getLayer(numLayers - layerCount - 2));
		// theMat = new ElasticIsotropicMaterial(numLayers - layerCount - 1, 2.0 * theLayer.getMatShearModulus()*(1.0+theLayer.getMatPoissonRatio()), theLayer.getMatPoissonRatio(), theLayer.getRho());
		theMat = new PM4Sand(numLayers - layerCount - 1, 0.7, 200.0, 1.5, theLayer.getRho());
		OPS_addNDMaterial(theMat);

		if (program_config->getBooleanProperty("General|PrintDebug"))
		{
			opserr << "Material " << theLayer.getName().c_str() << " tag = " << numLayers - layerCount - 1 << endln;
			opserr << "        nu = " << theLayer.getMatPoissonRatio() << ", E = " << 2.0 * theLayer.getMatShearModulus()*(1.0+theLayer.getMatPoissonRatio()) << endln;
		}
	}

	// FE mesh - create soil elements and add the material state parameter
	Element* theEle;
	Parameter* theParameter;
	char** paramArgs = new char*[2];

	paramArgs[0] = new char[15];
	paramArgs[1] = new char[5];
	sprintf(paramArgs[0], "materialState");

	int nElem = 0;

	for (int layerCount = 0; layerCount < numLayers - 1; ++layerCount)
	{
		theMat = OPS_getNDMaterial(numLayers - layerCount - 1);
		for (int elemCount = 0; elemCount < layerNumElems[numLayers - layerCount - 2]; ++elemCount)
		{
			int node1Tag = 2 * (nElem + elemCount);
			
			theEle = new SSPquadUP(nElem + elemCount + 1, node1Tag + 1, node1Tag + 2, node1Tag + 4, node1Tag + 3, *theMat, 1.0, 2.1e6, 1.0, 1.0, 1.0, 0.7, 1.0e-5, 0.0, - program_config->getFloatProperty("Units|g") * theMat->getRho());
			theDomain->addElement(theEle);


			theParameter = new Parameter(nElem + elemCount + 1, 0, 0, 0);
			sprintf(paramArgs[1], "%d", theMat->getTag());
			theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
			theDomain->addParameter(theParameter);

			if (program_config->getBooleanProperty("General|PrintDebug"))
				opserr << "Element " << nElem + elemCount + 1 << ": Nodes = " << node1Tag + 1 << " to " << node1Tag + 8 << "  - Mat tag = " << numLayers - layerCount - 1 << endln;
		}
		nElem += layerNumElems[numLayers - layerCount - 2];
	}

	if (program_config->getBooleanProperty("General|PrintDebug"))
		opserr << "Total number of elements = " << nElem << endln;



	// FE mesh - update material stage
	ParameterIter& theParamIter = theDomain->getParameters();
	while ((theParameter = theParamIter()) != 0)
	{
		theParameter->update(0.0);
	}



	// FE mesh - create analysis objects - I use static analysis for gravity
	AnalysisModel* theModel = new AnalysisModel();
	CTestNormDispIncr* theTest = new CTestNormDispIncr(program_config->getFloatProperty("Analysis|Gravity|ConvergenceTest|Tolerance"), program_config->getIntProperty("Analysis|Gravity|ConvergenceTest|MaxNumIterations"), program_config->getIntProperty("Analysis|Gravity|ConvergenceTest|PrintTag"));
	EquiSolnAlgo* theSolnAlgo = new NewtonRaphson(*theTest);
	StaticIntegrator* theIntegrator    = new LoadControl(0.05, 1, 0.05, 1.0);
	//TransientIntegrator* theIntegrator = new Newmark(0.5, 0.25);
	//ConstraintHandler* theHandler = new PenaltyConstraintHandler(1.0e14, 1.0e14);
	ConstraintHandler* theHandler = new TransformationConstraintHandler();
	RCM* theRCM = new RCM();
	DOF_Numberer* theNumberer = new DOF_Numberer(*theRCM);
	BandGenLinSolver* theSolver = new BandGenLinLapackSolver();
	LinearSOE* theSOE = new BandGenLinSOE(*theSolver);



	//DirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	StaticAnalysis *theAnalysis;
	theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator);
	theAnalysis->setConvergenceTest(*theTest);

	for (int analysisCount = 0; analysisCount < 2; ++analysisCount) {
		//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		int converged = theAnalysis->analyze(1);
		if (!converged) {
			opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
		}
	}

	// FE mesh - update material response to plastic
	theParamIter = theDomain->getParameters();
	while ((theParameter = theParamIter()) != 0)
	{
		theParameter->update(1.0);
	}

	for (int analysisCount = 0; analysisCount < 2; ++analysisCount) {
		//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		int converged = theAnalysis->analyze(1);
		if (!converged) {
			opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
		}
	}

	// FE mesh - add the compliant base - use the last layer properties
	double vis_C = SRM_layering.getLayer(numLayers - 1).getShearVelocity() * SRM_layering.getLayer(numLayers - 1).getRho();
	UniaxialMaterial* theViscousMats[2];
	theViscousMats[0] = new ViscousMaterial(numLayers + 10, vis_C, 1.0); OPS_addUniaxialMaterial(theViscousMats[0]);
	ID directions(1);
	directions(0) = 0;

	if (! program_config->getBooleanProperty("Analysis|RigidBase"))
	{
		// FE mesh - create dashpot nodes and apply proper fixities
		theNode = new Node(numNodes + 1, 2, 0.0, 0.0, 0.0, NULL); theDomain->addNode(theNode);
		theNode = new Node(numNodes + 2, 2, 0.0, 0.0, 0.0, NULL); theDomain->addNode(theNode);
		theSP = new SP_Constraint(numNodes + 1, 0, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(numNodes + 1, 1, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(numNodes + 2, 1, 0.0, true); theDomain->addSP_Constraint(theSP);

		// FE mesh - apply equalDOF to the node connected to the column
		Matrix constrainInXZ(2, 2); constrainInXZ(0, 0) = 1.0; constrainInXZ(1, 1) = 1.0;
		ID constDOF(2); constDOF(0) = 0; constDOF(1) = 1;
		theMP = new MP_Constraint(1, numNodes + 2, constrainInXZ, constDOF, constDOF); theDomain->addMP_Constraint(theMP);

		// FE mesh - remove fixities created for gravity
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(0)); delete theSP;
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(1)); delete theSP;

		// FE mesh - equalDOF the first 2 nodes
		theMP = new MP_Constraint(1, 2, constrainInXZ, constDOF, constDOF); theDomain->addMP_Constraint(theMP);

		// FE mesh - create the dashpot element
		Vector x(3); x(0) = 1.0; x(1) = 0.0; x(2) = 0.0;
		Vector y(3); y(1) = 1.0; y(0) = 0.0; y(2) = 0.0;
		theEle = new ZeroLength(numElems + 1, 2, numNodes + 1, numNodes + 2, x, y, 1, theViscousMats, directions);

		theDomain->addElement(theEle);
	}

	// FE mesh - apply the motion
	int numSteps = 0;
	std::vector<double> dt;

	// FE mesh - using multiple support
	//MultiSupportPattern* theLP = new MultiSupportPattern(1);
	//theLP->addMotion(*theMotionX->getGroundMotion(), 1);
	//theLP->addSP_Constraint(new ImposedMotionSP(1, 0, 1, 1));
	//theLP->addSP_Constraint(new ImposedMotionSP(2, 0, 1, 1));
	//theLP->addSP_Constraint(new ImposedMotionSP(3, 0, 1, 1));
	//theLP->addSP_Constraint(new ImposedMotionSP(4, 0, 1, 1));

	if (theMotionY->isInitialized())
	{
		// using uniform excitation to apply vertical motion
		LoadPattern* theLP = new UniformExcitation(*(theMotionY->getGroundMotion()), 1, 12, 0.0, -program_config->getFloatProperty("Units|g"));
		theDomain->addLoadPattern(theLP);
	}

	// FE mesh - using a stress input with the dashpot
	if (theMotionX->isInitialized())
	{
		// check if rigid base
		if (program_config->getBooleanProperty("Analysis|RigidBase"))
		{
			LoadPattern* theLP = new UniformExcitation(*(theMotionX->getGroundMotion()), 0, 13, 0.0, -program_config->getFloatProperty("Units|g"));
			theDomain->addLoadPattern(theLP);
		} else {
			LoadPattern* theLP = new LoadPattern(1, vis_C);
			theLP->setTimeSeries(theMotionX->getVelSeries());

			NodalLoad* theLoad;
			Vector load(2);
			load(0) = 1.0;
			load(1) = 0.0;

			theLoad = new NodalLoad(1, numNodes + 2, load, false); theLP->addNodalLoad(theLoad);
			theDomain->addLoadPattern(theLP);
		}
		// update the number of steps as well as the dt vector
		int temp = theMotionX->getNumSteps();
		if ( temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionX->getDTvector();
		}
	}

	// I have to change to a transient analysis
	// FE mesh - remove the static analysis and create new transient objects
	delete theIntegrator;
	delete theAnalysis;

	TransientIntegrator* theTransientIntegrator = new Newmark(program_config->getFloatProperty("Analysis|Dynamic|Newmark_Gamma"), program_config->getFloatProperty("Analysis|Dynamic|Newmark_Beta"));
	theTest->setTolerance(program_config->getFloatProperty("Analysis|Dynamic|ConvergenceTest|Tolerance"));

	// DirectIntegrationAnalysis* theTransientAnalysis;
	// theTransientAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

	VariableTimeStepDirectIntegrationAnalysis* theTransientAnalysis;
	theTransientAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);
	// FE mesh - reset time in the domain
	theDomain->setCurrentTime(0.0);

	// define Rayleigh damping
	double omega1 = 0.0;
	double omega2 = 0.0;
	if (program_config->getBooleanProperty("Analysis|Damping|ModalRayleigh"))
	{
		double natPeriod = SRM_layering.getNaturalPeriod();
		omega1 = 2.0 * PI * (2*program_config->getFloatProperty("Analysis|Damping|Mode1") - 1) / natPeriod; 
		omega2 = 2.0 * PI * (2*program_config->getFloatProperty("Analysis|Damping|Mode2") - 1) / natPeriod; 
	} else {
		omega1 = 2.0 * PI * program_config->getFloatProperty("Analysis|Damping|Frequency1"); 
		omega2 = 2.0 * PI * program_config->getFloatProperty("Analysis|Damping|Frequency2"); 
	}


	if (program_config->getBooleanProperty("Analysis|Damping|ElemByElem"))
	{
		opserr << "This part is not implemented yet." << endln;
		exit(-1);
	} else {
		double dampRatio = program_config->getFloatProperty("Analysis|Damping|Ratio");
		double a0 = dampRatio * (2.0 * omega1 * omega2) / (omega1 + omega2) ;
		double a1 = dampRatio * (2.0/(omega1 + omega2));
		if (program_config->getBooleanProperty("General|PrintDebug"))
			opserr << "a0 = " << a0 << "    a1 = " << a1 << endln;
		theDomain->setRayleighDampingFactors(a0, 0.0, a1, 0.0);
	}
	// FE mesh - create the output streams
	OPS_Stream* theOutputStream;
	Recorder* theRecorder;

	// record last node's results
	ID nodesToRecord(1);
	nodesToRecord(0) = numNodes;

	ID dofToRecord(2);
	dofToRecord(0) = 0;
	dofToRecord(1) = 1;

	// surface recorder
	std::string outFile = theOutputDir + PATH_SEPARATOR +  "surface.acc";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.vel";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.disp";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, 0.0, true, NULL);
	theDomain->addRecorder(*theRecorder);


	// recorder for bottom of layers
	nCount = 0;
	for (int layerCount = numLayers - 2; layerCount > -1; --layerCount)
	{		
		nodesToRecord(0) = nCount + 1;

		
		opserr << "layer : " << SRM_layering.getLayer(layerCount).getName().c_str() << " - Number of Elements = "
		<< layerNumElems[layerCount] << " - Number of Nodes = " << layerNumNodes[layerCount]
		<< " - Element Thickness = " << layerElemSize[layerCount] << ", nodes being recorded: " << nodesToRecord << endln;

		outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".acc";
		theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
		theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, 0.0, true, NULL);
		theDomain->addRecorder(*theRecorder);

		outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".vel";
		theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
		theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, 0.0, true, NULL);
		theDomain->addRecorder(*theRecorder);

		outFile = theOutputDir + PATH_SEPARATOR + std::to_string(layerCount + 1) + "_" + SRM_layering.getLayer(layerCount).getName().c_str() + ".disp";
		theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
		theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, 0.0, true, NULL);
		theDomain->addRecorder(*theRecorder);
		

		nCount += layerNumNodes[layerCount];
	}

	// record element results
	// OPS_Stream* theOutputStream2;
	// ID elemsToRecord(5);
	// elemsToRecord(0) = 1;
	// elemsToRecord(1) = 2;
	// elemsToRecord(2) = 3;
	// elemsToRecord(3) = 4;
	// elemsToRecord(4) = 5;
	// const char* eleArgs = "stress";
	// 
	// theOutputStream2 = new DataFileStream("Output2.out", OVERWRITE, 2, 0, false, 6, false);
	// theRecorder = new ElementRecorder(&elemsToRecord, &eleArgs, 1, true, *theDomain, *theOutputStream2, 0.0, NULL);
	// theDomain->addRecorder(*theRecorder);

	// FE mesh - perform analysis
	opserr << "Analysis started:" << endln;
	std::stringstream progressBar;
	numSteps = 80000;
	for (int analysisCount = 0; analysisCount < numSteps; ++analysisCount) {
		//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		double stepDT = dt[analysisCount];
		stepDT = 0.0001;
		int converged = theTransientAnalysis->analyze(1, stepDT, stepDT / 2.0, stepDT * 2.0, 1);
		// int converged = theTransientAnalysis->analyze(1, stepDT);
		if (!converged) {
			opserr << "Converged at time " << theDomain->getCurrentTime() << endln;

			if (analysisCount % (int)(numSteps / 20) == 0)
			{
				progressBar << "\r[";
				for (int ii = 0; ii < (int)(20 * analysisCount / numSteps); ii++)
					progressBar << ".";
				for (int ii = (int)(20 * analysisCount / numSteps); ii < 20; ii++)
					progressBar << "-";

				progressBar << "]  " << (int)(100 * analysisCount / numSteps) << "%";
				opsout << progressBar.str().c_str();
				opsout.flush();
			}
		}
		else {
			opserr << "Site response analysis did not converge." << endln;
			exit(-1);
		}
	}
	progressBar << "\r[";
	for (int ii = 0; ii < 20; ii++)
		progressBar << ".";

	progressBar << "] 100%";
	opsout << progressBar.str().c_str();
	opsout.flush();
	opsout << endln;
	//if (program_config->getBooleanProperty("General|PrintDebug"))
	//{
	//	Information info;
	//	theEle = theDomain->getElement(1);
	//	theEle->getResponse(1, info);
	//	opserr << "Stress = " << info.getData();
	//	theEle->getResponse(2, info);
	//	opserr << "Strain = " << info.getData();
	//}

	//int count = 0;
	//NodeIter& theNodeIter = theDomain->getNodes();
	//Node * thisNode;
	//while ((thisNode = theNodeIter()) != 0)
	//{
	//	count++;
	//	opserr << "Node " << thisNode->getTag() << " = " << thisNode->getCrds() << endln;
	//}

	//int count = 0;
	//ElementIter& theEleIter = theDomain->getElements();
	//Element * thisEle;
	//while ((thisEle = theEleIter()) != 0)
	//{
	//	count++;
	//	opserr << "Element " << thisEle->getTag() << " = " << thisEle->getExternalNodes() << endln;
	//}

	return 0;
}


int
SiteResponseModel::runTestModel()
{
	Vector zeroVec(3);
	zeroVec.Zero();

	Node* theNode;

	theNode = new Node(1, 3, 0.0, 0.0, 0.0); theDomain->addNode(theNode);
	theNode = new Node(2, 3, 1.0, 0.0, 0.0); theDomain->addNode(theNode);
	theNode = new Node(3, 3, 1.0, 1.0, 0.0); theDomain->addNode(theNode);
	theNode = new Node(4, 3, 0.0, 1.0, 0.0); theDomain->addNode(theNode);
	theNode = new Node(5, 3, 0.0, 0.0, 1.0); theDomain->addNode(theNode);
	theNode = new Node(6, 3, 1.0, 0.0, 1.0); theDomain->addNode(theNode);
	theNode = new Node(7, 3, 1.0, 1.0, 1.0); theDomain->addNode(theNode);
	theNode = new Node(8, 3, 0.0, 1.0, 1.0); theDomain->addNode(theNode);

	SP_Constraint* theSP;
	for (int counter = 0; counter < 3; ++counter) {
		theSP = new SP_Constraint(1, counter, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(2, counter, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(3, counter, 0.0, true); theDomain->addSP_Constraint(theSP);
		theSP = new SP_Constraint(4, counter, 0.0, true); theDomain->addSP_Constraint(theSP);
	}

	// MP_Constraint* theMP;
	// Matrix Ccr(3, 3); Ccr(0, 0) = 1.0; Ccr(1, 1) = 1.0; Ccr(2, 2) = 1.0;
	// ID rcDOF(3); rcDOF(0) = 0; rcDOF(1) = 1; rcDOF(2) = 2;
	// theMP = new MP_Constraint(5, 6, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
	// theMP = new MP_Constraint(5, 7, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
	// theMP = new MP_Constraint(5, 8, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);

	MP_Constraint* theMP;
	Matrix Ccr(2, 2); Ccr(0, 0) = 1.0; Ccr(1, 1) = 1.0;
	ID rcDOF(2); rcDOF(0) = 0; rcDOF(1) = 2;
	theMP = new MP_Constraint(5, 6, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
	theMP = new MP_Constraint(5, 7, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);
	theMP = new MP_Constraint(5, 8, Ccr, rcDOF, rcDOF); theDomain->addMP_Constraint(theMP);

	NDMaterial* theMat;
	theMat = new J2CyclicBoundingSurface(1, 20000.0, 25000.0, 100.0, 0.0, 20000.0, 1.0, 0.0, 0.0, 0.5);
	OPS_addNDMaterial(theMat);

	Element* theEle;
	theMat = OPS_getNDMaterial(1);
	theEle = new SSPbrick(1, 1, 2, 3, 4, 5, 6, 7, 8, *theMat, 0.0, 0.0, 0.0); theDomain->addElement(theEle);
	//theEle = new Brick(1, 1, 2, 3, 4, 5, 6, 7, 8, *theMat, 0.0, 0.0, 0.0); theDomain->addElement(theEle);

	//LinearSeries* theTS_disp;
	//theTS_disp = new LinearSeries(1, 1.0);

	Vector theTime(3);
	theTime(0) = 0.0;
	theTime(1) = 1.0;
	theTime(2) = 100.0;

	Vector theValue_Disp(3);
	theValue_Disp(0) = 0.0;
	theValue_Disp(1) = 1.0;
	theValue_Disp(2) = 1.0;

	Vector theValue_Vel(3);
	theValue_Vel(0) = 1.0;
	theValue_Vel(1) = 1.0;
	theValue_Vel(2) = 1.0;

	Vector theValue_Acc(3);
	theValue_Acc(0) = 0.0;
	theValue_Acc(1) = 0.0;
	theValue_Acc(2) = 0.0;
	PathTimeSeries* theTS_disp = new PathTimeSeries(1, theValue_Disp, theTime, 1.0, true);
	//PathTimeSeries* theTS_disp = NULL;
	PathTimeSeries* theTS_vel = new PathTimeSeries(1, theValue_Vel, theTime, 1.0, true);
	//PathTimeSeries* theTS_vel  = NULL;
	PathTimeSeries* theTS_acc = new PathTimeSeries(1, theValue_Acc, theTime, 1.0, true);
	//PathTimeSeries* theTS_acc = NULL;

	//LoadPattern* theLP;
	//theLP = new LoadPattern(1);
	//theLP->setTimeSeries(theTS_disp);

	MultiSupportPattern* theLP = new MultiSupportPattern(1);
	//theLP->setTimeSeries(theTS_disp);

	//NodalLoad* theLoad;
	//Vector load(3);
	//load(0) = 1.0;
	//load(1) = 0.0;
	//load(2) = 0.0;
	//theLoad = new NodalLoad(1, 5, load, false); theLP->addNodalLoad(theLoad);
	//theLoad = new NodalLoad(2, 6, load, false); theLP->addNodalLoad(theLoad);
	//theLoad = new NodalLoad(3, 7, load, false); theLP->addNodalLoad(theLoad);
	//theLoad = new NodalLoad(4, 8, load, false); theLP->addNodalLoad(theLoad);
	GroundMotion* theMotion = new GroundMotion(theTS_disp, theTS_vel, theTS_acc);
	theLP->addMotion(*theMotion, 1);

	theLP->addSP_Constraint(new ImposedMotionSP(5, 0, 1, 1));
	theLP->addSP_Constraint(new ImposedMotionSP(5, 0, 1, 1));
	theLP->addSP_Constraint(new ImposedMotionSP(5, 0, 1, 1));
	theLP->addSP_Constraint(new ImposedMotionSP(5, 0, 1, 1));

	//theLP->addSP_Constraint(new SP_Constraint(5, 0, 1.0, false));
	//theLP->addSP_Constraint(new SP_Constraint(6, 0, 1.0, false));
	//theLP->addSP_Constraint(new SP_Constraint(7, 0, 1.0, false));
	//theLP->addSP_Constraint(new SP_Constraint(8, 0, 1.0, false));
	theDomain->addLoadPattern(theLP);

	AnalysisModel* theModel = new AnalysisModel();
	CTestNormDispIncr* theTest = new CTestNormDispIncr(1.0e-7, 30, 1);
	EquiSolnAlgo* theSolnAlgo = new NewtonRaphson(*theTest);
	//StaticIntegrator* theIntegrator    = new LoadControl(0.05, 1, 0.05, 1.0);
	TransientIntegrator* theIntegrator = new Newmark(0.5, 0.25);
	ConstraintHandler* theHandler = new PenaltyConstraintHandler(1.0e15, 1.0e15);
	RCM* theRCM = new RCM();
	DOF_Numberer* theNumberer = new DOF_Numberer(*theRCM);
	BandGenLinSolver* theSolver = new BandGenLinLapackSolver();
	LinearSOE* theSOE = new BandGenLinSOE(*theSolver);


	//DirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
	theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//StaticAnalysis *theAnalysis;
	//theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator);
	//theAnalysis->setConvergenceTest(*theTest);

	for (int analysisCount = 0; analysisCount < 15; ++analysisCount) {
		int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		if (!converged) {
			opserr << "Converged at time " << theDomain->getCurrentTime() << endln;

			opserr << "Disp = " << theDomain->getNode(5)->getDisp()(0);
			opserr << ", Vel = " << theDomain->getNode(5)->getTrialVel()(0);
			opserr << ", acc = " << theDomain->getNode(5)->getTrialAccel()(0) << endln;

			opserr << "From the ground motion: " << endln;
			opserr << "Disp = " << theMotion->getDisp(theDomain->getCurrentTime());
			opserr << ", Vel = " << theMotion->getVel(theDomain->getCurrentTime());
			opserr << ", acc = " << theMotion->getAccel(theDomain->getCurrentTime()) << endln;
		}
	}

	Information info;
	theEle->getResponse(1, info);
	opserr << "Stress = " << info.getData();
	theEle->getResponse(2, info);
	opserr << "Strain = " << info.getData();



	return 0;
}
