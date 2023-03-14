#include "resource.h"
#include "BForm.h"
#include "mdlOpenSaveDlg.h"	//used to open/save dialogues
#include "mdlFileSys.h"		//used to realize the operations for file
#include "BBMPLoad.h"		//used to load BMP pic files
#include "GLCM.h"			//a self-written header file used to calculate the GLCM
#include "BADO.h"

CBForm form1(ID_form1);//�ô����Ĵ���ȥ��ʼ����CBForm

extern CBForm frmBatch;
extern void EventsMapfrmBatch();

void cmdBrow_Click()
{
	LPTSTR szFile;
	OsdSetFilter(TEXT("λͼ�ļ�(*.bmp)|*.bmp"));
	szFile = OsdOpenDlg(form1.hWnd(), TEXT("��λͼ�ļ�"));
	if (*szFile)
	{
		form1.Control(ID_cboFile).AddItem(szFile);
		form1.Control(ID_cboFile).TextSet(szFile);

		form1.Control(ID_pic1).PictureSet(szFile);
		form1.Control(ID_pic1).Refresh();
	}
}

void cmdGLCM_Click()
{
	TCHAR* szfile;
	CGLCM glcm;
	szfile = form1.Control(ID_cboFile).Text();
	if (*szfile == 0)
	{
		MsgBox(TEXT("��ѡ�������GLCM������λͼ�ļ���"), TEXT("δѡ��λͼ�ļ�"), mb_OK, mb_IconExclamation);
		return;
	}
	//��ȡparamD
	glcm.ParamD = form1.Control(ID_txtD).TextInt();

	if (!glcm.SetBitMapFile(szfile))
	{
		MsgBox(glcm.ErrDesp(), TEXT("��λͼ�ļ�ʧ��"), mb_OK, mb_IconExclamation);
	}
	else
	{
		for (int i = 1;i <= glcm.GetGLCMFeatCount();i++)
		{
			form1.Control(ID_txtASM - 1 + i).TextSet(glcm.GetGLCMFeat((EGLCMFeatureType)i));
		}
	}
}

void chkStretch_Click()
{
	if (form1.Control(ID_chkStretch).ValueChecked())
	{
		form1.Control(ID_pic1).StretchSet(true);
	}
	else
	{
		form1.Control(ID_pic1).StretchSet(false);
	}
	form1.Control(ID_pic1).Refresh();
}

void cmdGLCMBat_Click()
{
	frmBatch.Show(0, form1.hWnd());
}

void cmdExit_Click()
{
	form1.UnLoad();
}

void form1_QueryUnload(int pCancel)
{
	if (MsgBox(TEXT("ȷʵҪ�˳���"), TEXT("ȷ���˳�"), mb_YesNo, mb_IconQuestion) == idNo)
	{
		*((int*)pCancel) = 1;//ȡ���˳�����
	}
}

int main()
{
	form1.EventAdd(0, eForm_QueryUnload, form1_QueryUnload);
	form1.EventAdd(ID_cmdExit, eCommandButton_Click, cmdExit_Click);

	form1.EventAdd(ID_cmdBrow, eCommandButton_Click, cmdBrow_Click);
	form1.EventAdd(ID_cmdBrow, eCommandButton_Click, cmdBrow_Click);
	form1.EventAdd(ID_cmdGLCM, eCommandButton_Click, cmdGLCM_Click);
	form1.EventAdd(ID_chkStretch, eCheck_Click, chkStretch_Click);
	form1.Control(ID_txtD).TextSet(1);//default paramD = 1

	form1.EventAdd(ID_cmdGLCMBat, eCommandButton_Click, cmdGLCMBat_Click);
	EventsMapfrmBatch();

	//link the database
	if (!ADOConn.Open(TEXT("glcm.accdb")))
	{
		MsgBox(ADOConn.ErrorLastStr(), TEXT("���ݿ�����ʧ��"), mb_OK, mb_IconExclamation);
	}

	form1.IconSet(IDI_ICON1);
	form1.Show();
	return 0;
}

