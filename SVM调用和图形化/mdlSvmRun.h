//support run SVM program automately
#include "BWindows.h"

/** @brief Return true if sFile exists, otherwise return false and give error information in szPromptIfNotExist.
* @param sFile the file name with full path
* @param szPromptIfNotExist the error information
*/
bool TestFileExist(tstring& sFile, TCHAR* szPromptIfNotExist);

/** @brief Return true if svm-scale.exe, svm-train.exe and svm-predict.exe exist,
* otherwise return false and give error information in szPromptIfNotExist.
* @param sFile the file name with full path
* @param szPromptIfNotExist the error information
*/
bool VerifyLibSVMFiles();

/**  @brief Create a .bat file and execute it automatically.
* Return true if .bat file is created successfully, otherwise return false.
* @param szPath path of the .bat file
* @param szBatFileName name of the .bat file
* @param sCmdLine the command executed by the .bat file
*/
bool RunBat(TCHAR* szPath, 
			TCHAR* szBatFileName, 
			tstring& sCmdLine);

/**  @brief Normalize the data file 'szFileData'
* Return true if normalization is successful, otherwise return false.
* @param szFileData file that need to be normalized
* @param szFileDataScaleParam the normalized param file, normalization wouldn't be operated if szFileDataScaleParam==NULL
* @param fParamReadOrWrite 
* If fParamReadOrWrite==true, read the param file, 
* If fParamReadOrWrite==false, writer the param(overwrite the previous content)
* @note only when szFileDataScaleParam!=NULL fParamReadOrWrite makes sense
* @param sRetString the file name of the normalized file. If not operate normalization sRetString is just szFileData
*/
bool SVMScaleData(	TCHAR* szFileData, 
					TCHAR* szFileDataScaleParam, 
					bool fParamReadOrWrite, 
					tstring& sRetString);


/**  @brief Run SVM to train and waiting for the program to finish running
* Return true if train successfully, otherwise return false.
* @param szFileData full path name of the data file
* @param szFileModel name of model file and create it if szFileModel != NULL, do n-fold verify if szFileModel=NULL
* @param szFileDataScaleParam the normalized param file that will be saved. 
* if szFileDataScaleParam=NULL, normalization wouldn't be operated
* @parami KernelFunc kernel used in SVM train.
* @param iFold iFold-fold verify
* @param sz0therParams other param such as string of commands
* @param sRetString the file name of the result file or model file if return true, the error information if return false
*/
bool SVMTrain(	TCHAR* szFileData, 
				TCHAR* szFileModel, 
				TCHAR* szFileDataScaleParam, 
				int iKernelFunc, 
				int iFold, 
				TCHAR* sz0therParams, 
				tstring& sRetString);


/**  @brief Run SVM to test and waiting for the program to finish running
* Return true if train successfully, otherwise return false.
* @param szFileData name of feature data file
* @param szFileModel name of model file and create it if szFileModel != NULL, do n-fold verify if szFileModel=NULL
* @param szFileDataScaleParam the normalized param file that will be saved.
* if szFileDataScaleParam=NULL, normalization wouldn't be operated
* @parami szFileResult the result file to be generated after the test
* @param iParamB -b param
* @param sRetString the file name of the result file if return true, the error information if return false
*/
bool SVMPredict(TCHAR* szFileData, 
				TCHAR* szFileModel, 
				TCHAR* szFileDataScaleParam, 
				TCHAR* szFileResult, 
				int iParamB, 
				tstring& sRetString);
