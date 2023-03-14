#include "mdlSvmRun.h"
#include "mdlFileSys.h"
#include "mdlShellExec.h"

LPCTSTR mc_szScaleExeFile = TEXT("svm-scale.exe");
LPCTSTR mc_szTrainExeFile = TEXT("svm-train.exe");
LPCTSTR mc_szTestExeFile = TEXT("svm-predict.exe");

//�����ļ�sFile�Ƿ���ڣ�
//��������򷵻�true��
//�������ڣ��򷵻�false�����������ϢszPromptIfNotExist
bool TestFileExist(tstring& sFile, TCHAR* szPromptIfNotExist)
{
	if (FMFileExist(sFile.c_str()) == 1)
		return true;
	else
	{
		tstring sPrompt = sFile + TEXT("\r\n") + szPromptIfNotExist;
		MsgBox(sPrompt, TEXT("�ļ�������"), mb_OK, mb_IconExclamation);
		return false;
	}
}

//��֤LibSVM.exe�Ƿ����
//��������򷵻�true��
//�������ڣ��򷵻�false�����������ϢszPromptIfNotExist
bool VerifyLibSVMFiles()
{
	//pApp->Path():���Ӧ�ó���ǰ·��
	//FMAddBackSlash(): ȷ�������һ�� '\\'��
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	if (!TestFileExist(sAppPath + mc_szScaleExeFile, TEXT("svm-scale.exeδ��������ǰ���������ļ�����")))
		return false;
	if (!TestFileExist(sAppPath + mc_szTrainExeFile, TEXT("svm-train.exeδ��������ǰ���������ļ�����")))
		return false;
	if (!TestFileExist(sAppPath + mc_szTestExeFile, TEXT("svm-predict.exeδ��������ǰ���������ļ�����")))
		return false;

	return true;
}

//�����������ļ����Զ�ִ��
// szPath�������ļ�·��
// szBatFileName�������ļ�����(����.bat)
// sCmdLine�������ļ�ִ������
//�ɹ��������ɹ������򷵻�true����֮false
bool RunBat(TCHAR* szPath, TCHAR* szBatFileName, tstring& sCmdLine)
{
	//�����������ļ�(ȫ·��\\�ļ���)
	tstring sBatFile;
	sBatFile = FMAddBackSlash(szPath);
	sBatFile = sBatFile + szBatFileName;
	//���������ļ���д��������  EF_OpStyle_Output��˼�� ������д
	HANDLE hFileBat = EFOpen(sBatFile.c_str(), EF_OpStyle_Output);
	if (hFileBat == INVALID_HANDLE_VALUE)
		return false;

	EFPrint(hFileBat, sCmdLine.c_str());//д��������
	EFClose(hFileBat);//�ر��������ļ�
	//���ƿ���̨��ʾ״̬����ʾ����̨
	SERunWaitTerminate(sBatFile.c_str(), true, SW_ShowNormal);
	//ɾ���������ļ�
	//���лᷢ�����ڻ���վ�ҵ�ɾ����.bat�ļ�����͵ڶ�����������Ϊtrue�йأ�
	//����ȥ����վ���.bat�ļ�������ɶ��Ҳ����ֱ���ٴ˴���Ӷϵ���.bat��δ��ɾ��ʱ�鿴
	FMDeleteFiles(sBatFile.c_str(), true, true, false);

	return true;
}

// ��szFileData��һ���������ļ���������sRetString��
// szFileDataScaleParamΪ��һ���ļ������szFileDataScaleParam=NULL���򲻹�һ��
// �����һ������fParamReadOrWrite=trueʱ���������ļ�������д���µĲ����ļ�������֮ǰ����
// �ɹ��������ɹ������򷵻�true����֮false
bool SVMScaleData(	TCHAR* szFileData,
					TCHAR* szFileDataScaleParam,
					bool fParamReadOrWrite,
					tstring& sRetString)
{
	//ͨʽ��·��+�ļ�������ʵ�ַ���
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	tstring sSVMExeScale = sAppPath + mc_szScaleExeFile;//svm-scale.exe ȫ·��+�ļ���

	TCHAR* szPath = NULL;//szDataFile�е�·������
	TCHAR* szExp = NULL;//szDataFile�е���չ������
	// ���ļ���ȫ������·�����е�·�����֡���չ�����ֶ���ȥ��ֻ������������
	TCHAR* szFileNameOnly = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
	tstring sFileDataScale;
	tstring sCmdLine;

	//���콫Ҫ���ɵĹ�һ����������ļ���ȫ·�����ļ���
	//��szFileDataScaleParam != NULLʱ����Ҫ��һ��
	if (szFileDataScaleParam)
	{
		//����svm-scale.exe 
		//�����һ����������ļ�������eg. ·��\\data.txt  -> ·��\\data.scale.txt
		sFileDataScale = szPath;
		sFileDataScale += TEXT("\\");
		sFileDataScale += szFileNameOnly;
		sFileDataScale += TEXT(".scale.");
		sFileDataScale += szExp;
		//�����һ����������ļ� �� ��һ�������ļ�������ͬ������� ·��\\data.scale.txt -> ·��\\data.scale.data.txt
		if (_tcscmp(sFileDataScale.c_str(), szFileDataScaleParam) == 0)
		{
			sFileDataScale = szPath;
			sFileDataScale += TEXT("\\");
			sFileDataScale += szFileNameOnly;
			sFileDataScale += TEXT(".scale.data.");
			sFileDataScale += szExp;
		}

		//���sFileDataScale�Ѿ�������ɾ�����������Ƿ�ɾ���ɹ��ļ��
		FMDeleteFiles(sFileDataScale.c_str(), true, true, false);
		if (FMFileExist(sFileDataScale.c_str()) == 1)
		{
			sRetString = TEXT("����ɾ��֮ǰ�����Ĺ�һ�����ݽ���ļ�: ") + sFileDataScale;
			return false;
		}

		//׼����һ���������浽�ļ�����
		if (!fParamReadOrWrite)//If fParamReadOrWrite==false, writer the param(overwrite the previous content)
		{
			FMDeleteFiles(szFileDataScaleParam, true, true, false);
			if (FMFileExist(szFileDataScaleParam) == 1)
			{
				sRetString = TEXT("����ɾ��֮ǰ�����Ĺ�һ�������ļ�:") + sFileDataScale;//����˵��
				return false;
			}
			//���������� svm-scale.exe  -s
			//��data.txt�ļ��е����ݹ�һ�������ɽ���ļ�data.scale.txt�����й�һ�����õĲ���������ļ�scaleparams.txt��
			sCmdLine = TEXT("\"") + sSVMExeScale + TEXT("\" -s ");
		}
		else//If fParamReadOrWrite == true, read the param file,
		{
			//���������� svm-scale.exe  -r
			//��data.txt�ļ��е����ݹ�һ�������ɽ���ļ�data.scale.txt�����й�һ�����õĲ���ʹ��֮ǰ���ɵ��ļ�scaleparams.txt
			sCmdLine = TEXT("\"") + sSVMExeScale + TEXT("\" -r ");
		}
		//����������һ����������: svm-scale.exe  -s  scaleparams.txt  data.txt > data.scale.txt
		//Ҫ��data.txt�ļ��е����ݹ�һ�������ɽ���ļ�data.scale.txt�����й�һ�����õĲ���������ļ�scaleparams.txt��
		sCmdLine = sCmdLine + TEXT("\"") + szFileDataScaleParam + TEXT("\" ");
		sCmdLine = sCmdLine + TEXT("\"") + szFileData + TEXT("\" ");
		sCmdLine = sCmdLine + TEXT("> \"") + sFileDataScale + TEXT("\" ");

		//ͨ���������ļ����д�������ȴ����н���
		if (!RunBat(szPath, TEXT("svmscale.bat"), sCmdLine))
		{
			sRetString = TEXT("���ݹ�һ��ʧ�ܣ��޷����ļ���");
			sRetString = sRetString + szPath + TEXT(" �д����������������ļ�:svmscale.bat��");//����˵��
			return false;
		}
		if (FMFileExist(sFileDataScale.c_str()) != 1)
		{
			sRetString = TEXT("���ݹ�һ��ʧ�ܣ�δ�����ɹ�һ����Ľ�������ļ���") + sFileDataScale;//����˵��
			return false;
		}
	}
	else // normalization wouldn't be operated if szFileDataScaleParam==NULL
	{
		//ֱ������sFileDataScaleΪԭ�����ļ�
		sFileDataScale = szFileData;
	}
	sRetString = sFileDataScale;
	return true;
}

//����SVM ѵ�����ȴ��������
//szFileDataΪ���������ļ����ļ���
//szFileModelΪҪ��ȡ��model�ļ����ļ���
//szFileModel != NULL��������model�ļ�����ʱ����n-�۽�����֤����֮����n-�����۵���֤
//szFileDataScaleParamΪҪ����Ĺ�һ�������ļ�
//szFileDataScaleParam==NULL ʱ����һ�� 
//iKernelFuncΪҪʹ�õĺ˺�����=0��1,2,3)
//iFoldΪҪ���۽�����֤.(����szFileModel==NULL ʱ��Ч)
//szOtherParamsΪ�����������������ַ���)
//�ɹ�����true������sRetString ���ؽ���ļ�����ʧ�ܷ���false�����ɲ���sRetString����ʧ��ԭ��˵��
bool SVMTrain(	TCHAR* szFileData,
				TCHAR* szFileModel,
				TCHAR* szFileDataScaleParam,
				int iKernelFunc,
				int iFold,
				TCHAR* szOtherParams, 
				tstring& sRetString)
{
	// ·��\\svm-train.exe
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	tstring sSVMExeTrain = sAppPath + mc_szTrainExeFile;

	if (szFileData && szFileModel && _tcscmp(szFileData, szFileModel) == 0)
	{
		sRetString = TEXT("���������ļ���Ҫ���ɵ�model�ļ�������ͬһ�ļ���");
		return false;
	}
	if (szFileData && szFileDataScaleParam && _tcscmp(szFileData, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT("���������ļ��͹�һ�������ļ�������ͬһ�ļ���");
		return false;
	}
	if (szFileModel && szFileDataScaleParam && _tcscmp(szFileModel, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT("Ҫ���ɵ�model�ļ����͹�һ�������ļ�������ͬһ�ļ���");
		return false;
	}

	//��ȡszFileData���ļ���
	TCHAR* szPath = NULL;// szDataFile �е�·������
	TCHAR* szExp = NULL; // szDataFile �е���չ������
	TCHAR* szFileNameOnly = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
	tstring sFileDataScale, sCmdLine;
	//��һ�����ݣ���ɹ���sFileDataScale�б����˹�һ����Ľ�������ļ���
	if (!SVMScaleData(szFileData, szFileDataScaleParam, false, sFileDataScale))
	{
		sRetString = sFileDataScale;//ʧ��ʱ sFileDataScaIe ��Ϊ������Ϣ�����ݸ�sRetString
		return false;
	}

	//����SVM ѵ���������ģ��
	//�Զ������˽���ļ�data.scale.txt(t=2).training_result�����м�����˸���ͳ��ָ��
	//�����У�svm-train.exe -t 2 -v 5 data.scale.txt 
	//����ģ���ļ�data.scale.model.txt
	//�����У�svm-train.exe -t 2      data.scale.txt data.scale.model.txt
	sCmdLine = TEXT("\"") + sSVMExeTrain + TEXT("\" ");
	sCmdLine = sCmdLine + TEXT(" -t ") + Str(iKernelFunc);
	if (szFileModel == NULL)//��������model�ļ�����ִ��n-�۽�����֤
	{
		sCmdLine = sCmdLine + TEXT(" -v ") + Str(iFold);
	}
	sCmdLine = sCmdLine + TEXT(" ") + szOtherParams + TEXT(" ");//����չ��
	sCmdLine = sCmdLine + TEXT(" \"") + sFileDataScale + TEXT("\" ");
	if (szFileModel != NULL)//��������model�ļ�����ִ��n-�۽�����֤
	{
		sCmdLine = sCmdLine + TEXT(" \"") + szFileModel + TEXT("\"");
	}

	//ͨ���������ļ��ķ�ʽ�����д������У����ȴ����н���
	//�������ļ���ΪͬһĿ¼�µ�svmtrain.bat(�ļ�������Ϊ������Ҳ��)
	if (!RunBat(szPath, TEXT("svmtrain.bat"), sCmdLine))
	{
		sRetString = TEXT("SVMѵ��ʧ�ܣ��޷����ļ���");
		sRetString = sRetString + szPath + TEXT(" �д����������������ļ�: svmtrain.bat��");
		return false;
	}

	//��ʾ���
	tstring sFileResult;
	if (szFileModel == NULL)//������model�ļ�
	{
		//���ѵ������ļ��Ƿ�������szFileResult
		//eg.data.scale.txt(t=2).training_esult
		sFileResult = sFileDataScale + TEXT("(t=") + Str(iKernelFunc) + TEXT(").training_result");
		if (FMFileExist(sFileResult.c_str()) != 1)//������ļ��Ƿ�������
		{
			sRetString = TEXT("SVMѵ��ʧ��!δ������ѵ������ļ�:") + sFileResult;
			return false;
		}
		sRetString = sFileResult;//�ɹ����ɣ�sRetString�������ļ���
	}
	else//����model�ļ�
	{
		//���model�ļ��Ƿ�������
		if (FMFileExist(szFileModel) != 1)
		{ 
			sRetString = TEXT("model�ļ�����ʧ��!δ������model�ļ�: ");
			sRetString = sRetString + szFileModel;
			return false;
		}
		//�ɹ�: model�ļ������ɣ�sRetString����model�ļ���
		sRetString = szFileModel ;
	}
	return true;
}

//����SVM ���Բ��ȴ��������
//szFileDataΪ���������ļ����ļ���
//szFileModelΪҪ��ȡ��model�ļ����ļ���
//szFileDataScaleParamΪҪ��ȡ�Ĺ�һ�������ļ���
//����ѵ��ʱ�Ĺ�һ�������ļ���ͬ; szFileDataScaleParam == NULLʱ����һ��l 
//szFileResultΪ���Ժ�Ҫ���ɵĽ���ļ�
//iParamBTΪ -b ������ֵѡ��˺���
//�ɹ�����true������sRetString���ؽ���ļ���
//ʧ�ܷ���false�����ɲ���sRetString����ʧ��ԭ��˵��
bool SVMPredict(TCHAR* szFileData,
				TCHAR* szFileModel,
				TCHAR* szFileDataScaleParam,
				TCHAR* szFileResult,
				int iParamB,
				tstring& sRetString)
{
	// ·��\\svm-predict.exe
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	tstring sSVMExePredict = sAppPath + mc_szTestExeFile;

	//�����ļ�������ͬ
	if (szFileData && szFileModel && _tcscmp(szFileData, szFileModel) == 0)
	{
		sRetString = TEXT("���������ļ���model�ļ�������ͬһ�ļ���");
		return false;
	}
	if (szFileData && szFileDataScaleParam && _tcscmp(szFileData, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT("���������ļ��͹�һ�������ļ�������ͬһ�ļ���");
		return false;
	}
	if (szFileData && szFileResult && _tcscmp(szFileData, szFileResult) == 0)
	{
		sRetString = TEXT("���������ļ��ͽ���ļ�������ͬһ�ļ���");
		return false;
	}
	if (szFileModel && szFileDataScaleParam && _tcscmp(szFileModel, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT(" model�ļ��͹�һ�������ļ�������ͬһ�ļ���");
		return false;
	}
	if (szFileModel && szFileResult && _tcscmp(szFileModel, szFileResult) == 0)
	{
		sRetString = TEXT(" model�ļ��ͽ���ļ�������ͬһ�ļ���");
		return false;
	}
	if (szFileDataScaleParam && szFileResult && _tcscmp(szFileDataScaleParam, szFileResult) == 0)
	{
		sRetString = TEXT("��һ�������ļ��ͽ���ļ�������ͬһ�ļ���");
		return false;
	}

	//׼���ļ���
	TCHAR* szPath = NULL;// szDataFile �е�·������
	TCHAR* szExp = NULL; // szDataFile �е���չ������
	TCHAR* szFileNameOnly = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
	tstring sFileDataScale, sCmdLine;

	//��һ�����ݣ���ɹ���sFileDataScale�б����˹�һ����Ľ�������ļ���
	if (!SVMScaleData(szFileData, szFileDataScaleParam, true, sFileDataScale))
	{
		sRetString = sFileDataScale;//ʧ��ʱ sFileDataScaIe ��Ϊ������Ϣ
		return false;
	}
	//����SVM ���Գ������ģ��
	//�����У�svm-predict -b 1/0 test.scale.txt  data.scale.model.txt  out.txt
	sCmdLine = TEXT("\"") + sSVMExePredict + TEXT("\" ");
	if (iParamB > 0)
	{ 
		//���������м�-b ����ѡ��˺���
		sCmdLine = sCmdLine + TEXT(" -b ") + Str(iParamB);
	}
	sCmdLine = sCmdLine + TEXT(" \"") + sFileDataScale + TEXT("\" ");
	sCmdLine = sCmdLine + TEXT(" \"") + szFileModel + TEXT("\" ");
	sCmdLine = sCmdLine + TEXT(" \"") + szFileResult + TEXT("\"");
	//ͨ���������ļ��ķ�ʽ�����д������У����ȴ����н���
	//�������ļ���ΪͬһĿ¼�µ�svmpredict.bat(�ļ�������Ϊ������Ҳ��)
	if (!RunBat(szPath, TEXT("svmpredict.bat"), sCmdLine))
	{ 
		sRetString = TEXT("SVM����ʧ�ܣ��޷����ļ���");
		sRetString = sRetString + szPath + TEXT(" �д����������������ļ�:svmpredict. bat��");
		return false;
	}
	//��ʾ���
	//������ļ��Ƿ�������
	if (FMFileExist(szFileResult) != 1)
	{
		sRetString = TEXT("SVM����ʧ��!δ�����ɽ���ļ�:");
		sRetString = sRetString + szFileResult;
		return false;
	}
	//�ɹ�:����ļ������ɣ���sRetString�������ļ���
	sRetString = szFileResult;
	return true;
}
