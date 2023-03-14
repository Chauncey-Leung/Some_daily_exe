#include "resource.h"
#include "BForm.h"

#include "mdlOpenSaveDlg.h"
#include "mdlSvmRun.h"
#include "mdlShellExec.h"
#include "mdlFileSys.h"

CBForm form1(ID_form1);

void cmdBrowDataFile_Click()
{
	// ���ѡ�������ļ�
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt"));
	TCHAR* file = OsdOpenDlg(form1.hWnd());
	if (*file)
	{
		form1.Control(ID_cboDataFile).AddItem(file);
		form1.Control(ID_cboDataFile).TextSet(file);
	}
}
void cmdBrowScaleParamFileTrain_Click()
{
	// ����ѵ��ʱ�����Ų����Ĳ���Ҫ���浽���ļ�
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt"));
	TCHAR* file = OsdSaveDlg(form1.hWnd());
	if (*file)
	{
		form1.Control(ID_cboScaleParamFileTrain).AddItem(file);
		form1.Control(ID_cboScaleParamFileTrain).TextSet(file);
	}
}
void cmdBrowModelFileTest_Click()
{
	//����ʱ�����ѡ��Ҫ�򿪵�ģ���ļ�
	OsdSetFilter(TEXT("ģ���ļ�(*.model)|*.model|�ı��ļ�(*.txt)|*.txt"));
	TCHAR* file = OsdOpenDlg(form1.hWnd());//���� OsdSaveDlg
	if (*file)
	{
		form1.Control(ID_cboModelFileTest).AddItem(file);
		form1.Control(ID_cboModelFileTest).TextSet(file);
	}
}

void cmdBrowResultFileTest_Click()
{			
	//���ò���ʱ�����ս��Ҫ���浽���ļ�
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt"));
	TCHAR* file = OsdSaveDlg(form1.hWnd());//���� OsdOpenDlgif(*file)
	{
		form1.Control(ID_cboResultFileTest).AddItem(file);
		form1.Control(ID_cboResultFileTest).TextSet(file);
	}
}
void cmdBrowScaleParamFileTest_Click()
{
	//����ʱ�����������ݣ����ѡ��Ҫ�򿪵����Ų����ļ�
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt"));
	TCHAR* file = OsdOpenDlg(form1.hWnd());// ���� 0sdSaveDlgif(*file)
	{
		form1.Control(ID_cboScaleParamFileTest).AddItem(file);
		form1.Control(ID_cboScaleParamFileTest).TextSet(file);
	}
}

//��ѵ��ʱ�Ĺ�һ������Ҫ���浽���ļ�����δ��д�����Զ�����һ�� 
void AutoFillScaleParamFileTrain()
{
	LPTSTR szFileData = form1.Control(ID_cboDataFile).Text();
	LPTSTR szFileParamTrain = form1.Control(ID_cboScaleParamFileTrain).Text();
	if (*szFileParamTrain == 0 && *szFileData != 0)
	{
		//��szFileData �ļ�������չ��֮ǰ��.param��������չ������.txt
		//��������Ҫ����Ĳ����ļ����ļ��������õ�ID_cboScaleParamFileTrain
		LPTSTR szPath, szExp;
		LPTSTR szFile = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
		tstring sParamFile = szPath;
		sParamFile = sParamFile + TEXT("\\") + szFile + TEXT(".param.txt");
		form1.Control(ID_cboScaleParamFileTrain).TextSet(sParamFile);
	}
}

void chkScaleTrain_Click()
{
	//�繴ѡ�������������ؼ�ʹ��;������ã����)����
	bool fChecked = form1.Control(ID_chkScaleTrain).ValueChecked();
	form1.Control(ID_lblScaleParamTrain).EnabledSet(fChecked);
	form1.Control(ID_cboScaleParamFileTrain).EnabledSet(fChecked);
	form1.Control(ID_cmdBrowScaleParamFileTrain).EnabledSet(fChecked);
	//�繴ѡΪ��Ч������δ��д�ļ���,�Զ�����һ���ļ���
	if (fChecked)
		AutoFillScaleParamFileTrain();
}

void chkScaleTest_Click()
{
	//�繴ѡ�������������ؼ�ʹ��;������ã����)����
	bool fChecked = form1.Control(ID_chkScaleTest).ValueChecked();
	form1.Control(ID_lblScaleParamTest).EnabledSet(fChecked);
	form1.Control(ID_cboScaleParamFileTest).EnabledSet(fChecked);
	form1.Control(ID_cmdBrowScaleParamFileTest).EnabledSet(fChecked);
}

void cbo_FilesDrop(int ptrArrFiles, int count,int x, int y) 
{
	//������ptrArrFilesǿ��ת��ΪTCHAR**���ͣ��̶���·)
	TCHAR** files = (TCHAR**)ptrArrFiles;
	//��files�ɱ�������TCHAR���ַ����Ķ�ά���飬Ҳ���ַ�����һά����
	//��files[1]�� files[2]��������files[count]��ø����ַ���
	//��Щ�ַ��������û��϶�����ļ�ʱ�����϶����ؼ����ĸ��ļ���
	//����������ֻ��һ���ļ� files[1]�����û��϶�����ļ�ʱ�����������ļ�
	//����ѡ����ļ�ȫ·����ӵ���Ͽ򣨶����Ͽ���¼������ñ�����)
	form1.Control(form1.IDRaisingEvent()).AddItem(files[1]);
	//����Ͽ�����ʾ��ѡ����ļ�ȫ·��
	form1.Control(form1.IDRaisingEvent()).TextSet(files[1]);
}
	
void form1_Load()
{
	//��ؼ�ID_cboKernels���˺����������ѡ��:���˿ؼ���Sort����һ����ΪFalse
	CBControl cboKernels(ID_cboKernels);
	cboKernels.AddItem(TEXT("���Ժ�(Linear��t=0)"));
	cboKernels.AddItem(TEXT("����ʽ��(Polynomial��t=1)"));
	cboKernels.AddItem(TEXT("��˹�������(Radial Basis Function��t=2)"));
	cboKernels.AddItem(TEXT("�����������(Sigmoid,t=3)"));
	cboKernels.ListIndexSet(3);// Ĭ��ѡ�е�$����Ŀ(RBF)
	//��ؼ�ID_cboFold(������֤)�����ѡ��:���˿ؼ���Sort����һ����ΪFalse
	CBControl cboFold(ID_cboFold);
	for (int i = 2; i <= 10; i++)
		cboFold.AddItem(Str(i));
	cboFold.TextSet(TEXT("5"));//Ĭ��ѡ��:5-�۽�����֤
	//������ʱ������ʾ�ؼ���ʹ��״̬����Ҫʱ�������º���)chkScaleTrain_Click() ;
	chkScaleTest_Click();
	chkScaleTrain_Click();
}


void Train_Click()
{
	//��������ļ��ļ���
	LPTSTR szFileData = form1.Control(ID_cboDataFile).Text();

	if (*szFileData == 0)
	{
		MsgBox(TEXT("��ѡ��ѵ���õ����������ļ���"), TEXT("δָ��ѵ���õ����������ļ�"), mb_OK, mb_IconExclamation);
		return;
	}
	//����������ݵĲ���Ҫ���浽���ļ���szFileScaleParam
	//����������ʱ����ΪNULL
	LPTSTR szFileScaleParam = NULL;
	if (form1.Control(ID_chkScaleTrain).ValueChecked())
	{
		szFileScaleParam = form1.Control(ID_cboScaleParamFileTrain).Text();
		if (*szFileScaleParam == 0)
		{
			MsgBox(TEXT("������ѵ��ʱ���Ų���Ҫ���浽���ļ���"), TEXT("δָ�����Ų����ļ�"), mb_OK, mb_IconExclamation);
			return;
		}
	}

	// model�ļ�(����model�ļ�ʱʹ��;���ѵ��������model�ļ�����ΪNULL)
	TCHAR* szFileModel = NULL;
	if (form1.IDRaisingEvent() == ID_cmdGenModel)
	{
		//����model�ļ�ʱ��ѡ�񱣴�model�ļ�
		OsdSetFilter(TEXT("ģ���ļ�(*.model)|*.model|�ı��ļ�(*.txt)|*.txt"));
		szFileModel = OsdSaveDlg(form1.hWnd());
		if (*szFileModel == 0)
			return;// �û���ѡ��model�ļ�ʱȡ��
	}

	tstring sReturn;
	bool ret = SVMTrain(szFileData, szFileModel, szFileScaleParam,
						form1.Control(ID_cboKernels).ListIndex() - 1,
						form1.Control(ID_cboFold).TextInt(),
						form1.Control(ID_txtParamsTrain).Text(), sReturn);
	if (ret)
	{
		if (form1.IDRaisingEvent() == ID_cmdGenModel)//����model�ļ�
		{
					
			sReturn = TEXT("model�ļ����ɳɹ�:\r\n") + sReturn;
			MsgBox(sReturn, TEXT("����model�ļ�"), mb_OK, mb_IconExclamation);
		}
		else//��ѵ��������model
		{
			//ͨ�����±���ʾ������ָ��Ľ���ļ����ļ���ΪsReturn)/ / SEShellRun������mdlShellExecͨ��ģ���еĺ���
			tstring sCmd;
			sCmd = TEXT("NotePad.exe \"") + sReturn + TEXT(" \"");
			SEShellRun(sCmd.c_str(), true);
		}
	}
	else//ʧ�ܣ���ʾ������Ϣ
	{
		MsgBox(sReturn, TEXT("ѵ��ʧ��"), mb_OK, mb_IconExclamation);
	}
	
}

void cmdPredict_Click()
{
	//��������ļ��ļ���
	LPTSTR szFileData = form1.Control(ID_cboDataFile).Text();
	if (*szFileData == 0)
	{
		MsgBox(TEXT("��ѡ������õ����������ļ���"),
			TEXT("δָ�������ļ�"), mb_OK, mb_IconExclamation);
		return;
	}
	// ���model�ļ��ļ���
	TCHAR* szFileModel = form1.Control(ID_cboModelFileTest).Text();
	if (*szFileModel == 0)
	{
		MsgBox(TEXT("��ѡ������õ� model��ģ�ͣ��ļ������ļ���ѵ�������ɡ�"),
			TEXT("δָ��model�ļ�"), mb_OK, mb_IconExclamation);
		return;
	}

	//��ý���ļ��ļ���
	TCHAR* szFileResult = form1.Control(ID_cboResultFileTest).Text();
	if (*szFileResult == 0)
	{
		MsgBox(TEXT("��ָ�����Ժ�Ľ��Ҫ���浽���ļ���"),
			TEXT("δָ������ļ�"), mb_OK, mb_IconExclamation);
		return;
	}
	//����������ݵĲ���Ҫ���浽���ļ���=>szFileScaleParam
	//����������ʱ,��ΪNULL
	LPTSTR szFileScaleParam = NULL;
	if (form1.Control(ID_chkScaleTest).ValueChecked())
	{
		szFileScaleParam = form1.Control(ID_cboScaleParamFileTest).Text();
		if (*szFileScaleParam == 0)
		{
			MsgBox(TEXT("���������ʱ���Ų����ļ������ļ�Ҫ��ѵ��ʱ����ͬ����"),
				TEXT("δָ�����Ų����ļ�"), mb_OK, mb_IconExclamation);
			return;
		}
	}

	//����Ƿ�ҪԤ����ʹ���ֵ=> iPararB�����Ǵ�ֵΪ1)
	int iParamB = 0;
	if (form1.Control(ID_chkPredictB).ValueChecked())
		iParamB = 1;
	tstring sReturn;
	bool ret = SVMPredict(szFileData, szFileModel, szFileScaleParam, szFileResult, iParamB, sReturn);
	if (ret)
	{
		if (MsgBox(TEXT("Ԥ�����ļ����ɳɹ�!Ҫ���ڴ���? "),
			TEXT("Ԥ�����"), mb_OkCancel, mb_IconQuestion) == idOk)
		{
			//��Ԥ�����ļ�
			tstring sCmd;
			sCmd = TEXT("NotePad.exe \"") + sReturn + TEXT(" \"");
			SEShellRun(sCmd.c_str(), true);
		}
	}
	else //ʧ��
	{
		MsgBox(sReturn, TEXT("����ʧ��"), mb_OK, mb_IconExclamation);
	}
}

int main()
{
	if (!VerifyLibSVMFiles())
	{
		return 1;
	}

	form1.EventAdd(0, eForm_Load, form1_Load);
	form1.EventAdd(ID_cmdBrowDataFile, eCommandButton_Click, cmdBrowDataFile_Click);
	form1.EventAdd(ID_cmdBrowScaleParamFileTest, eCommandButton_Click, cmdBrowScaleParamFileTest_Click);
	form1.EventAdd(ID_cmdBrowModelFileTest, eCommandButton_Click, cmdBrowModelFileTest_Click);
	form1.EventAdd(ID_cmdBrowResultFileTest, eCommandButton_Click, cmdBrowResultFileTest_Click);
	form1.EventAdd(ID_cmdBrowScaleParamFileTrain, eCommandButton_Click, cmdBrowScaleParamFileTrain_Click);
	form1.EventAdd(ID_chkScaleTrain, eCommandButton_Click, chkScaleTrain_Click);
	form1.EventAdd(ID_chkScaleTest, eCommandButton_Click, chkScaleTest_Click);

	//֧��5����Ͽ��ļ��Ϸ�:��������ͬһ�¼�����: 
	form1.EventAdd(ID_cboDataFile, eFilesDrop, cbo_FilesDrop);
	form1.EventAdd(ID_cboScaleParamFileTrain, eFilesDrop, cbo_FilesDrop);
	form1.EventAdd(ID_cboModelFileTest, eFilesDrop, cbo_FilesDrop);
	form1.EventAdd(ID_cboResultFileTest,eFilesDrop,cbo_FilesDrop);
	form1.EventAdd(ID_cboScaleParamFileTest, eFilesDrop, cbo_FilesDrop);

	// ID_cmdTrain�� ID_cmdGenModel ������ť������Train_Click
	form1.EventAdd(ID_cmdTrain, eCommandButton_Click, Train_Click);
	form1.EventAdd(ID_cmdGenModel, eCommandButton_Click, Train_Click);
	form1.EventAdd(ID_cmdPredict, eCommandButton_Click, cmdPredict_Click);

	form1.IconSet(IDI_ICON1);

	form1.Show();

	return 0;
}