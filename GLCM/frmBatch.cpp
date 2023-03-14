#include "resource.h"
#include "BForm.h"
#include "mdlOpenSaveDlg.h"	//used to open/save dialog
#include "mdlFileSys.h"		//used to realize the operations for file
#include "mdlPathDlg.h"
#include "mdlShellExec.h"
#include "GLCM.h"	
#include "mdlSvmRun.h"
#include "BADO.h"

CBForm frmBatch(ID_frmBatch);
void EventsMapfrmBatch();
bool m_fCancelSign = false;

//����ID_cboPosi ID_cboNega
void UpdateClassList()
{
	CBAdoRecordset rs;
	if (!rs.Open(TEXT("SELECT DISTINCT ClassLabel FROM Samples")))
		return;
	frmBatch.Control(ID_cboPosi).ListClear();
	frmBatch.Control(ID_cboNega).ListClear();
	while (!rs.EOFRs())
	{
		frmBatch.Control(ID_cboPosi).AddItem(rs.GetField(TEXT("ClassLabel")));
		frmBatch.Control(ID_cboNega).AddItem(rs.GetField(TEXT("ClassLabel")));
		rs.MoveNext();
	}
	rs.Close();
}

void cmdBrowImgPath_Click()
{
	TCHAR* path = BrowPath(frmBatch.hWnd(), TEXT("��ѡ��һ��ͼ���ļ����ڵ��ļ���"), 
							true, frmBatch.Control(ID_cboImgPath).Text(), false, true);

	if (*path)
	{
		frmBatch.Control(ID_cboImgPath).TextSet(path);
		//�Զ����ID_txtClassLabel
		TCHAR** s;
		int n = Split(path, s, TEXT("\\"));
		frmBatch.Control(ID_txtClassLabel).TextSet(s[n-1]);
	}
		
}

void cmdBrowDataFile_Click()
{
	//���ѡ��data.txtʱҪ���浽��λ�ú��ļ���
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt"));
	TCHAR* file = OsdSaveDlg(frmBatch.hWnd(),0,TEXT("��ָ��Ҫ����������ļ�")); //���� OsdOpenDlgif(*file)
	if (*file)
		frmBatch.Control(ID_cboDataFile).TextSet(file);
}

void cmdCalcBatch_Click()
{
	//һ������·������ð�ť����������GLCM����ֵ����ʼ��������󣬰�ť��Ϊ��ȡ����������ֹ����
	/*��ťΪ��ȡ�����Ƕ�Ӧ�Ĺ���*/
	if (_tcscmp(frmBatch.Control(ID_cmdCalcBatch).Text(), TEXT("ȡ��")) == 0)
	{
		if (MsgBox(TEXT("������δ��ɣ�ȷ��Ҫ��ֹ��"), TEXT("��ֹ����"), mb_YesNoCancel, mb_IconQuestion) != idYes)
			return;
		m_fCancelSign = true;//ȫ�ֱ������Ƿ���;ȡ���ı�־
		return;
	}

	/*��ťΪ����������GLCM����ֵ���Ƕ�Ӧ�Ĺ��ܣ�
	����ID_cboImgPath·���µ�����ͼ���ļ�����ID_txtClassLabelΪ��ǩ��������������access��Features����
	*/
	TCHAR* szPath = frmBatch.Control(ID_cboImgPath).Text();//�����Ҫ�������ͼ���ļ����ڵ�·��
	TCHAR* szLabel = frmBatch.Control(ID_txtClassLabel).Text();//��ô���ͼ���ļ�������ǩ
	if (*szPath == TEXT('\0'))
	{
		MsgBox(TEXT("��ѡ��ͼ���ļ������ļ��У�"), TEXT("δѡ���ļ���"), mb_OK, mb_IconExclamation);
		return;
	}
	if (*szLabel == TEXT('\0'))
	{
		MsgBox(TEXT("��ѡ������ͼ���ļ�������ǩ��"), TEXT("δ��������ǩ"), mb_OK, mb_IconExclamation);
		return;

	}

	//�����ݿ����Ѿ����˸�����ǩ����Ҫ��ɾ�����ݿ��е���������
	CBAdoRecordset rs, rsSample;
	tstring sSQL, sPrompt;
	sSQL = TEXT("SELECT * FROM Features, Samples WHERE Features.SampleID=Samples.SampleID AND ClassLabel='");
	sSQL += szLabel;
	sSQL += TEXT("'");
	if (!rs.Open(sSQL, ADOConn))
		return;
	if (!rs.EOFRs())
	{
		//ɾ�����е��ظ���ǩ
		sPrompt = TEXT("���ݿ����Ѵ�������ǩ [");
		sPrompt += szLabel;
		sPrompt += TEXT("]��\r\n");
		sPrompt += TEXT("�ٴμ���ͬ������ǩ��ͼ���ļ����������ᵼ�����ݿ������е����ڸ��������ݱ�ɾ�����Ƿ������");
		if (MsgBox(sPrompt, TEXT("ɾ�����е��ظ���ǩ"), mb_YesNo, mb_IconQuestion) == idNo)
			return;
		sSQL = TEXT("DELETE FROM Features WHERE SampleID IN (SELECT SampleID FROM Samples WHERE ClassLabel='");
		sSQL += szLabel;
		sSQL += TEXT("')");
		ADOConn.Execute(sSQL.c_str());
		sSQL = TEXT("DELETE FROM Samples WHERE ClassLabel='");
		sSQL += szLabel;
		sSQL += TEXT("'");
		ADOConn.Execute(sSQL.c_str());
	}
	rs.Close();

	//���Samples��������SampleID���ڴ˻�����+1����µ�ID
	int idSample = 0;
	if (!rs.Open(TEXT("SELECT MAX(SampleID) FROM Samples"), ADOConn))
		return;
	if (rs.EOFRs())
		return;
	idSample = (int)Val(rs.GetField((long)0));
	rs.Close();

	//����Ҳ���������淽��
	if (!rs.Open(TEXT("SELECT * FROM Features"), ADOConn))
		return;
	if (!rsSample.Open(TEXT("SELECT * FROM Samples"), ADOConn))
		return;

	m_fCancelSign = false;
	frmBatch.Control(ID_cmdCalcBatch).TextSet(TEXT("ȡ��"));

	/*��������ͼ���ļ���GLCM����ֵ*/
	//����Ҫ��ȡ·�������е�ͼ���ļ�
	TCHAR** files = NULL, ** subFolders = NULL;
	int cntFiles = 0, cntSnbFolders = 0;
	CGLCM glcm;
	FMListFilesAPI(szPath, files, subFolders, &cntFiles, &cntSnbFolders);
	//׼�������� 
	frmBatch.Control(ID_proBatch).MinSet(0);
	frmBatch.Control(ID_proBatch).MaxSet(cntFiles);
	frmBatch.Control(ID_proBatch).ValueSet(0);
	frmBatch.Control(ID_proBatch).VisibleSet(true);
	//���㲢�洢
	for (int i = 1;i <= cntFiles;i++)//ע������0��֮����
	{
		frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT("���ڼ���"));
		frmBatch.Control(ID_lblStatusBatch).TextAdd(FMTrimFileName(files[i]));
		frmBatch.Control(ID_lblStatusBatch).TextAdd(TEXT("..."));
		frmBatch.Control(ID_proBatch).ValueSet(i);
		DoEvents();
		if (m_fCancelSign)
			break;
		if (!glcm.SetBitMapFile(files[i]))
		{
			MsgBox(glcm.ErrDesp(), FMTrimFileName(files[i]), mb_OK, mb_IconExclamation);
			break;
		}
		else//�ɹ���
		{
			if (rsSample.AddNew())
			{
				idSample++;
				rsSample.SetField(TEXT("SampleID"), idSample);
				rsSample.SetField(TEXT("ClassLabel"), szLabel);
				rsSample.SetField(TEXT("ImageFile"), files[i]);
				if (!rsSample.Update())
				{
					MsgBox(rsSample.ErrorLastStr(), TEXT("��Sample����������¼ʱ����ʧ��"), mb_OK, mb_IconExclamation);
					break;
				}
			}
			else
			{
				MsgBox(rsSample.ErrorLastStr(), TEXT("��Sample����������¼ʧ��"), mb_OK, mb_IconExclamation);
				break;
			}
			for (int j = 1;j <= glcm.GetGLCMFeatCount();j++)
			{
				if (rs.AddNew())
				{
					rs.SetField(TEXT("SampleID"), idSample);
					rs.SetField(TEXT("FeatureID"), j);
					rs.SetField(TEXT("FeatureValue"), glcm.GetGLCMFeat((EGLCMFeatureType)j));
					if (!rs.Update())
					{
						MsgBox(rs.ErrorLastStr(), TEXT("��Features����������¼ʱ����ʧ��"), mb_OK, mb_IconExclamation);
						break;
					}
				}
				else
				{
					MsgBox(rs.ErrorLastStr(), TEXT("��Features����������¼ʧ��"), mb_OK, mb_IconExclamation);
					break;
				}
			}
		}
	}

	rsSample.Close();
	rs.Close();
	frmBatch.Control(ID_proBatch).VisibleSet(false);
	UpdateClassList();//����ID_cboPosi ID_cboNega
	frmBatch.Control(ID_cmdCalcBatch).TextSet(TEXT("��������GLCM����ֵ"));
	if (m_fCancelSign)
	{
		frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT("�û���ֹ"));
	}
	else
	{
		frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT("��ɡ�"));
	}
}


void frmBatch_Load()
{
	//����ID_lblVS
	frmBatch.Control(ID_lblVS).FontNameSet(TEXT("����"));
	frmBatch.Control(ID_lblVS).FontSizeSet(30);
	frmBatch.Control(ID_lblVS).ForeColorSet(RGB(255, 0, 0));
	frmBatch.Control(ID_lblVS).FontItalicSet(true);

	//��ID_cboKernels�����ѡ��
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("���Ժ�(Linear��t=0)"));
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("����ʽ��(Polynomial��t=1)"));
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("��˹�������(Radial Basis Function��t=2)"));
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("�����������(Sigmoid,t=3)"));
	frmBatch.Control(ID_cboKernels).ListIndexSet(3);// Ĭ��ѡ���˹�������

	//lbl����ʾStatic
	frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT(" "));
	frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT(" "));

	//��ID_cboFeatSelect�����ѡ��
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("����(ASM)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("�Աȶ�(Constrast)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("�����(Correlation)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("����(Variance)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("ͬ�ʶ�(Homogeneity)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("�;�ֵ(SumAverage)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("�ͷ���(SumVar)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("����(SumEntropy)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("��(Entropy)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("���(DiffVar)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("����(DiffEntropy)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("��������(Dissimilarity)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("��ֵ(Mean)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("���׾ۼ���(ClusterShade)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("�Ľ׾ۼ���(ClusterProm)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("������(MaxProb)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("��С����(MinProb)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("ǿ��(Strength)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("MassX"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("MassY"));
	frmBatch.Control(ID_cboFeatSelect).ListIndexSet(1);
	
	frmBatch.Control(ID_txtFeatSelect).TextSet(TEXT("Contrast SumAverage MassX MassY "));

	//��ID_cboFold�����ѡ��
	for (int i = 2; i <= 10; i++)
		frmBatch.Control(ID_cboFold).AddItem(Str(i));
	frmBatch.Control(ID_cboFold).TextSet(TEXT("5"));// Ĭ��ѡ��:5-�۽�����֤

	frmBatch.Control(ID_proBatch).VisibleSet(false);//����ID_proBatch
	VerifyLibSVMFiles();//��֤LibSVM.exe�Ƿ����
	UpdateClassList();//����ID_cboPosi ID_cboNega
}


//���ļ�����SVM�����ļ���ʽд��һ����������ֵ��������������)����������access->txt
//idCboControl�ؼ�ID
//iPosiNega==1 д��������; д�븺����iPosiNega==-1
bool OutputDataFile(HANDLE hFile, unsigned int idCboControl, int iPosiNega)
{
	CBAdoRecordset rsSam, rs, rsUseFeatures;
	tstring sSQL;
	bool blRet = false;
	long lRet = 0;
	int iFeatureIndex = 0;//д���ļ�ʱ���������
	//������������� SampleID => rsSam
	if (iPosiNega > 0)
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("�����������..."));
	}
	else
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("�����������..."));
	}
	sSQL = TEXT("SELECT * FROM Samples WHERE ClassLabel='");
	sSQL += frmBatch.Control(idCboControl).Text();
	sSQL += TEXT("'ORDER BY SampleID");
	
	if (!rsSam.Open(sSQL, ADOConn))
		return false;

	//��ʾ��ʾ��Ϣ������д��UseFeature table
	frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("����д��UseFeature Table"));
	//����ID_txtFeatSelectȷ��UseFeature��д�����ݿ�
	TCHAR** sUseFeatures;
	int cntTotalUseFeat = Split(frmBatch.Control(ID_txtFeatSelect).Text(), sUseFeatures, TEXT(" "));
	if (!rsUseFeatures.Open(TEXT("SELECT * FROM UseFeatures"), ADOConn))
		return false;
	sSQL = TEXT("DELETE FROM UseFeatures");
	ADOConn.Execute(sSQL.c_str());
	int useFeatureID = 1;
	for (int i = 1;i < cntTotalUseFeat;i++)
	{
		if (rsUseFeatures.AddNew())
		{
			
			if (!_tcscmp(sUseFeatures[i], TEXT("ASM")))
				useFeatureID = 1;
			if (!_tcscmp(sUseFeatures[i], TEXT("Contrast")))
				useFeatureID = 2;
			if (!_tcscmp(sUseFeatures[i], TEXT("Correlation")))
				useFeatureID = 3;
			if (!_tcscmp(sUseFeatures[i], TEXT("Variance")))
				useFeatureID = 4;
			if (!_tcscmp(sUseFeatures[i], TEXT("Homogeneity")))
				useFeatureID = 5;
			if (!_tcscmp(sUseFeatures[i], TEXT("SumAverage")))
				useFeatureID = 6;
			if (!_tcscmp(sUseFeatures[i], TEXT("SumVar")))
				useFeatureID = 7;
			if (!_tcscmp(sUseFeatures[i], TEXT("SumEntropy")))
				useFeatureID = 8;
			if (!_tcscmp(sUseFeatures[i], TEXT("Entropy")))
				useFeatureID = 9;
			if (!_tcscmp(sUseFeatures[i], TEXT("DiffVar")))
				useFeatureID = 10;
			if (!_tcscmp(sUseFeatures[i], TEXT("DiffEntropy")))
				useFeatureID = 11;
			if (!_tcscmp(sUseFeatures[i], TEXT("Dissimilarity")))
				useFeatureID = 12;
			if (!_tcscmp(sUseFeatures[i], TEXT("Mean")))
				useFeatureID = 13;
			if (!_tcscmp(sUseFeatures[i], TEXT("ClusterShade")))
				useFeatureID = 14;
			if (!_tcscmp(sUseFeatures[i], TEXT("ClusterProm")))
				useFeatureID = 15;
			if (!_tcscmp(sUseFeatures[i], TEXT("MaxProb")))
				useFeatureID = 16;
			if (!_tcscmp(sUseFeatures[i], TEXT("MinProb")))
				useFeatureID = 17;
			if (!_tcscmp(sUseFeatures[i], TEXT("Strength")))
				useFeatureID = 18;
			if (!_tcscmp(sUseFeatures[i], TEXT("MassX")))
				useFeatureID = 19;
			if (!_tcscmp(sUseFeatures[i], TEXT("MassY")))
				useFeatureID = 20;
			rsUseFeatures.SetField(TEXT("FeatureID"), useFeatureID);
			if (!rsUseFeatures.Update())
			{
				MsgBox(rsUseFeatures.ErrorLastStr(), TEXT("��Sample����������¼ʱ����ʧ��"), mb_OK, mb_IconExclamation);
				break;
			}
		}
		else
		{
			MsgBox(rsUseFeatures.ErrorLastStr(), TEXT("��UseFeatures����������¼ʧ��"), mb_OK, mb_IconExclamation);
			break;
		}
	}
	rsUseFeatures.Close();
	

	//��ÿ��ClassLabel������䲿��Features(ѡ�ļ���FeaturesҪ����UseFeatures��)
	while (!rsSam.EOFRs())
	{

		//��ʾ��ʾ��Ϣ��������ʾ����ID+�����ļ�����ȥ��·������)
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("д����������ֵ"));
		frmBatch.Control(ID_lblStatusSVM).TextAdd(rsSam.GetField(TEXT("SampleID")));
		frmBatch.Control(ID_lblStatusSVM).TextAdd(TEXT(" - "));
		frmBatch.Control(ID_lblStatusSVM).TextAdd(FMTrimFileName(rsSam.GetField(TEXT("ImageFile"))));


		// EF_LineSeed_None��ʾд������ݺ󣬲��Զ�����
		if (EFPrint(hFile, Str(iPosiNega), EF_LineSeed_None) < 0)//д��ǩ:������Ϊ1��������Ϊ-1
			return false;// д��ʧ��
		if (EFPrint(hFile, TEXT(" "), EF_LineSeed_None) < 0)//д�ո�
			return false;//д��ʧ��
		//���Ҫд���ļ�������ֵ
		sSQL = TEXT("SELECT * FROM Features, UseFeatures WHERE ");
		sSQL += TEXT("Features.FeatureID = UseFeatures.FeatureID AND ");
		sSQL += TEXT("SampleID=");
		sSQL += rsSam.GetField(TEXT("SampleID"));
		sSQL += TEXT(" ORDER BY Features.FeatureID");
		if (!rs.Open(sSQL, ADOConn))
			return false;

		//ѭ����������ֵ,��SVM�����������ļ���ʽд���ļ�
		iFeatureIndex = 0;
		while (!rs.EOFRs())
		{
			iFeatureIndex++;
			//format-iFeatureIndex:FeatureValue[blank space]
			if (EFPrint(hFile, Str(iFeatureIndex), EF_LineSeed_None) < 0)//дFeatureID
				return false;// д��ʧ��
			if (EFPrint(hFile, TEXT(":"), EF_LineSeed_None) < 0)//д:
				return false;// д��ʧ��
			if (EFPrint(hFile, rs.GetField(TEXT("FeatureValue")), EF_LineSeed_None) < 0)//дFeatureValue
				return false;// д��ʧ��
			if (EFPrint(hFile, TEXT(" "), EF_LineSeed_None) < 0)//д�ո�
				return false;// д��ʧ��
			rs.MoveNext();
		}
		if (EFPrint(hFile, TEXT(""), EF_LineSeed_CrLf) < 0)//����
			return false;// д��ʧ��
		rsSam.MoveNext();
	}
	rsSam.Close();
	if (iPosiNega > 0)
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("��������д����ɡ�"));
	}
	else
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("��������д����ɡ�"));
	}
	return true;
}

//����LibSVM���������ļ�
//szFile �ļ���
bool GenSVMDataFile(TCHAR* szFile)
{
	if (*szFile == TEXT('\0'))
	{
		MsgBox(TEXT("��ָ�������ļ����ļ����ͱ���·��"), TEXT("δָ�������ļ�"), mb_OK, mb_IconExclamation);
		return false;
	}
	TCHAR* szClassLabel = frmBatch.Control(ID_cboPosi).Text();
	if (*szClassLabel == TEXT('\0'))
	{
		MsgBox(TEXT("��ѡ����������������ǩ"), TEXT("δѡ����������������ǩ"), mb_OK, mb_IconExclamation);
		return false;
	}
	szClassLabel = frmBatch.Control(ID_cboNega).Text();
	if (*szClassLabel == TEXT('\0'))
	{
		MsgBox(TEXT("��ѡ����������������ǩ"), TEXT("δѡ����������������ǩ"), mb_OK, mb_IconExclamation);
		return false;
	}

	HANDLE hFile = EFOpen(szFile, EF_OpStyle_Output);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	//����OutputDataFile()д��access������txt
	if (!OutputDataFile(hFile, ID_cboPosi, 1))
	{
		MsgBox(TEXT("д��������������ֵ���ļ�ʧ�ܣ�"), TEXT("д����������ʧ��"), mb_OK, mb_IconExclamation);
		return false;
	}
	if (!OutputDataFile(hFile, ID_cboNega, -1))
	{
		MsgBox(TEXT("д��������������ֵ���ļ�ʧ�ܣ�"), TEXT("д����������ʧ��"), mb_OK, mb_IconExclamation);
		return false;
	}
	EFClose(hFile);
	return true;
}

//��ID_cmdTrain��ID_cmdGenModel�ǵĻص��������书���У�����SVM�����ļ�������model�ļ�(��ѡ)
void Train_Click()
{
	TCHAR* szFileModel = NULL;
	if (frmBatch.IDRaisingEvent() == ID_cmdGenModel)
	{
		//���ѡ��modelҪ���浽��·�����ļ���
		OsdSetFilter(TEXT("ģ���ļ�(*.model)|*.model|�ı��ļ�(*.txt)|*.txt"));
		szFileModel = OsdSaveDlg(frmBatch.hWnd());
		if (*szFileModel == 0)
			return;
	}

	TCHAR* szFile = frmBatch.Control(ID_cboDataFile).Text();
	if (!GenSVMDataFile(szFile))
		return;

	//�Զ�ȷ�����Ų����ļ����ļ����м�sacale.param��·��+�ļ��� szFileScaleParam
	TCHAR* szPath = NULL;//szFileScaleParam�е�·������
	TCHAR* szExp = NULL;//szFileScaleParam�е���չ������
	// ���ļ���ȫ������·�����е�·�����֡���չ�����ֶ���ȥ��ֻ������������
	TCHAR* szFileNameOnly = FMTrimFileName(szFile, true, true, &szPath, &szExp);
	TCHAR* szFileScaleParam = StrAppend(szPath, TEXT("\\"), szFileNameOnly, TEXT(".scale.param."), szExp);
	//ɾ���Ѿ����ڵ��ļ�
	FMDeleteFiles(szFileScaleParam, true, true, false);
	if (FMFileExist(szFileScaleParam) == 1)
	{
		MsgBox(ID_cboFold, szFileScaleParam, mb_OK, mb_IconExclamation);
		return;
	}
	tstring sReturn;
	frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("SVM����ѵ��..."));
	bool ret = SVMTrain(szFile, szFileModel, szFileScaleParam,
		frmBatch.Control(ID_cboKernels).ListIndex() - 1,
		frmBatch.Control(ID_cboFold).TextInt(),
		frmBatch.Control(ID_txtParams).Text(), sReturn);
	if (!ret)
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("ѵ��ʧ�ܡ�"));
		MsgBox(sReturn, TEXT("ѵ��ʧ��"), mb_OK, mb_IconExclamation);
	}
	else
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("ѵ���ɹ���"));
		if (frmBatch.IDRaisingEvent() == ID_cmdGenModel)
		{
			sReturn = TEXT("model �ļ����ɳɹ�:\r\n") + sReturn;
			MsgBox(sReturn, TEXT("ѵ��ʧ��"), mb_OK, mb_IconExclamation);
		}
		else
		{
			tstring sCmd;
			sCmd = TEXT("NotePad.exe \"") + sReturn + TEXT(" \"");
			SEShellRun(sCmd.c_str(), true);
		}
	}
}

void cmdFeatAdd_Click()
{
	int idxFeatToAdd = frmBatch.Control(ID_cboFeatSelect).ListIndex();
	CGLCM glcm_temp;
	TCHAR* szFeatToAdd = glcm_temp.GetGLCMFeatName(EGLCMFeatureType(idxFeatToAdd));
	TCHAR* szFeatAdded = frmBatch.Control(ID_txtFeatSelect).Text();
	//����Ƿ�������ֵ�ظ����
	TCHAR* pFind = _tcsstr(szFeatAdded, szFeatToAdd);
	if (pFind != NULL)
	{
		MsgBox(TEXT("ͬһ������ֵֻ�����һ�Σ�"), TEXT("�������ֵʧ��"), mb_OK, mb_IconExclamation);
		return;
	}
	frmBatch.Control(ID_txtFeatSelect).TextAdd(szFeatToAdd);
	frmBatch.Control(ID_txtFeatSelect).TextAdd(TEXT(" "));
}

void cmdFeatDel_Click()
{
	int idxFeatToDel = frmBatch.Control(ID_cboFeatSelect).ListIndex();
	CGLCM glcm_temp;
	TCHAR* szFeatToDel = glcm_temp.GetGLCMFeatName(EGLCMFeatureType(idxFeatToDel));//��ɾ���ִ�
	
	TCHAR* szFeatAdded = frmBatch.Control(ID_txtFeatSelect).Text();//ԭ�ַ���
	TCHAR* szFeatChanged = NULL;//ɾ���ִ�����ַ���
	//����Ƿ��ɾ������ֵ�Ƿ��Ѿ����
	TCHAR* pFind = _tcsstr(szFeatAdded, szFeatToDel);
	if (pFind == NULL)
	{
		MsgBox(TEXT("������ֵδ��ӣ����޷�ɾ��"), TEXT("ɾ������ֵʧ��"), mb_OK, mb_IconExclamation);
		return;
	}
	else {
		//ɾ���ַ����Ĳ�������Ϊͬһ����ֵ�����ظ���ӣ�����ֻҪɾ��һ������
		szFeatAdded[(int)(pFind - szFeatAdded)] = TEXT('\0');
		pFind = pFind + _tcslen(szFeatToDel) + 1;
		if (pFind != NULL)
			_tcscat(szFeatAdded, pFind);
	}
	frmBatch.Control(ID_txtFeatSelect).TextSet(szFeatAdded);
}

void cmdFeatDefault_Click()
{
	frmBatch.Control(ID_txtFeatSelect).TextSet(TEXT("Contrast SumAverage MassX MassY "));
}

void EventsMapfrmBatch()
{
	frmBatch.EventAdd(0, eForm_Load, frmBatch_Load);
	frmBatch.EventAdd(ID_cmdBrowImgPath, eCommandButton_Click, cmdBrowImgPath_Click);
	frmBatch.EventAdd(ID_cmdBrowDataFile, eCommandButton_Click, cmdBrowDataFile_Click);
	frmBatch.EventAdd(ID_cmdCalcBatch, eCommandButton_Click, cmdCalcBatch_Click);

	frmBatch.EventAdd(ID_cmdTrain, eCommandButton_Click, Train_Click);
	frmBatch.EventAdd(ID_cmdGenModel, eCommandButton_Click, Train_Click);

	frmBatch.EventAdd(ID_cmdFeatAdd, eCommandButton_Click, cmdFeatAdd_Click);
	frmBatch.EventAdd(ID_cmdFeatDel, eCommandButton_Click, cmdFeatDel_Click);
	frmBatch.EventAdd(ID_cmdFeatDefault, eCommandButton_Click, cmdFeatDefault_Click);
}