#include "BBMPLoad.h" //used to load BMP pic files

#define cntTotalGLCMFeatures 20 //total num of GLCM Features that can be calculated
#define mcGLCMMatDim 256		//total col_num and row_num of Matrix

enum EGLCMFeatureType //GLCM Features
{
	ASM = 1,
	Contrast = 2,
	Correlation = 3,
	Variance = 4,
	Homogeneity = 5,
	SumAverage = 6,
	SumVar = 7,
	SumEntropy = 8,
	Entropy = 9,
	DiffVar = 10,
	DiffEntropy = 11,
	Dissimilarity = 12,
	Mean = 13,
	ClusterShade = 14,
	ClusterProm = 15,
	MaxProb = 16,
	MinProb = 17,
	Strength = 18,
	MassX = 19,
	MassY = 20,
};

class CGLCM
{
public:
	/** @biref The step between pixels. 
	* If traverse all pixels of bitmap horizontally, (i,i+1)->(i+ParamD,i+1+ParamD)->(i+2*ParamD,i+1+2*ParamD)->...
	* default=1, see #CGLCM()
	*/
	int ParamD;

	/** @biref Open a bitmap and calculate all its CLCM features which will be stored in mGLCMFeats
	* @param bmpFile The bitmap that will be open
	*/
	bool SetBitMapFile(TCHAR* bmpFile);

	/** @biref Return the param featType
	* @param featType The GLCM feature that you want to get
	*/
	double GetGLCMFeat(EGLCMFeatureType featType);

	/** @biref Return cntTotalGLCMFeatures,which means the total nums of GLCM features
	*/
	int GetGLCMFeatCount();//Return 

	/** @biref Return the  name of CLCM features 
	* @param featType The GLCM feature that you want to get
	*/
	TCHAR* GetGLCMFeatName(EGLCMFeatureType featType);

	/** @biref Return return the width of the bitmap
	* @note Make sense after calling the SetBitMapFile()
	*/
	int Width();// 

	/** @biref Return return the height of the bitmap
	* @note Make sense after calling the SetBitMapFile()
	*/
	int Height();

	/** @biref Display error information
	*/
	TCHAR* ErrDesp();//

	/** @biref Constructor
	*/
	CGLCM();


private:
	
	CBBMPLoad mBMPLoad;
	
	double mGLCMFeats[cntTotalGLCMFeatures + 1];
	
	/** @biref GLCM matrix
	*/
	int mGLCMMat[mcGLCMMatDim][mcGLCMMatDim]; 
	
	TCHAR mFeatNameBuff[255];

	/** @biref Calculate the GLCM matrix which will be stored in mGLCMMat
	*/
	bool CalcGlcmMats();
	
	/** @biref Use mGLCMMat to get Features
	*/
	bool CalcFeatures();
};