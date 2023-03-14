#include "GLCM.h"
#include "math.h"

CGLCM::CGLCM()
{
	ParamD = 1;
	memset(mGLCMFeats, 0, sizeof(double) * (cntTotalGLCMFeatures + 1));
	memset(mGLCMMat, 0, sizeof(int) * (mcGLCMMatDim * mcGLCMMatDim));
}

bool CGLCM::SetBitMapFile(TCHAR* bmpFile)//Open a bmp and calculate all its GLCM features which will be stored in mGLCMFeats
{
	bool bl = mBMPLoad.LoadBitmapFile(bmpFile);
	if (!bl)
		return false;
	if (!CalcGlcmMats())
		return false;
	if (!CalcFeatures())
		return false;

	return true;
}

double CGLCM::GetGLCMFeat(EGLCMFeatureType featType)//Return the param featType
{
	/*here can add the featType check to enhance the program's robust*/
	return floor(mGLCMFeats[featType] * 1e6 +0.5) / 1.0e6;
	//return (int)(mGLCMFeats[featType] * 1e6 + 0.5) / 1e6;
}

TCHAR* CGLCM::GetGLCMFeatName(EGLCMFeatureType featType)
{
	switch (featType)
	{
	case ASM:			_tcscpy(mFeatNameBuff, TEXT("ASM"));			break;
	case Contrast:		_tcscpy(mFeatNameBuff, TEXT("Contrast"));		break;
	case Correlation:	_tcscpy(mFeatNameBuff, TEXT("Correlation"));	break;
	case Variance:		_tcscpy(mFeatNameBuff, TEXT("Variance"));		break;
	case Homogeneity:	_tcscpy(mFeatNameBuff, TEXT("Homogeneity"));	break;
	case SumAverage:	_tcscpy(mFeatNameBuff, TEXT("SumAverage"));		break;
	case SumVar:		_tcscpy(mFeatNameBuff, TEXT("SumVar"));			break;
	case SumEntropy:	_tcscpy(mFeatNameBuff, TEXT("SumEntropy"));		break;
	case Entropy:		_tcscpy(mFeatNameBuff, TEXT("Entropy"));		break;
	case DiffVar:		_tcscpy(mFeatNameBuff, TEXT("DiffVar"));		break;
	case DiffEntropy:	_tcscpy(mFeatNameBuff, TEXT("DiffEntropy"));	break;
	case Dissimilarity:	_tcscpy(mFeatNameBuff, TEXT("Dissimilarity"));	break;
	case Mean:			_tcscpy(mFeatNameBuff, TEXT("Mean"));			break;
	case ClusterShade:	_tcscpy(mFeatNameBuff, TEXT("ClusterShade"));	break;
	case ClusterProm:	_tcscpy(mFeatNameBuff, TEXT("ClusterProm"));	break;
	case MaxProb:		_tcscpy(mFeatNameBuff, TEXT("MaxProb"));		break;
	case MinProb:		_tcscpy(mFeatNameBuff, TEXT("MinProb"));		break;
	case Strength:		_tcscpy(mFeatNameBuff, TEXT("Strength"));		break;
	case MassX:			_tcscpy(mFeatNameBuff, TEXT("MassX"));			break;
	case MassY:			_tcscpy(mFeatNameBuff, TEXT("MassY"));			break;
	default:			_tcscpy(mFeatNameBuff, TEXT(""));
	}
	return mFeatNameBuff;
}

int CGLCM::Width()// Be effective after calling the SetBitMapFile()
{
	return mBMPLoad.Width();
}

int CGLCM::Height()// Be effective after calling the SetBitMapFile()
{
	return mBMPLoad.Height();
}

int CGLCM::GetGLCMFeatCount()//Return cntTotalGLCMFeatures
{
	return cntTotalGLCMFeatures;
}

TCHAR* CGLCM::ErrDesp()//Display error information
{
	return mBMPLoad.GetErrDesp();
}

bool CGLCM::CalcGlcmMats()
{
	unsigned char** pixels = mBMPLoad.GetBMPBytes();
	if (pixels == 0)
		return false;

	unsigned char pxl, pxlNear;
	int bmpWidth = mBMPLoad.Width();
	int bmpHeight = mBMPLoad.Height();

	memset(mGLCMMat, 0, sizeof(int) * (mcGLCMMatDim* mcGLCMMatDim));

	//Compare direction:0 rad
	for (int i = 0;i < bmpHeight;i++)//scanning each row 
	{
		for (int j = 0;j < bmpWidth - ParamD;j++)//scanning each col
		{
			pxl = pixels[i][j];
			pxlNear = pixels[i][j + ParamD];
			if (pxl < pxlNear)
				mGLCMMat[pxl][pxlNear]++;
			else if ((pxl > pxlNear))
				mGLCMMat[pxlNear][pxl]++;
			else
				mGLCMMat[pxl][pxlNear] += 2;
		}
	}
	//Compare direction:45 rad
	for (int i = 0;i < bmpHeight - ParamD;i++)//scanning each row 
	{
		for (int j = 0;j < bmpWidth - ParamD;j++)//scanning each col
		{
			pxl = pixels[i][j];
			pxlNear = pixels[i + ParamD][j + ParamD];
			if (pxl < pxlNear)
				mGLCMMat[pxl][pxlNear]++;
			else if ((pxl > pxlNear))
				mGLCMMat[pxlNear][pxl]++;
			else
				mGLCMMat[pxl][pxlNear] += 2;
		}
	}
	//Compare direction:135 rad
	for (int i = ParamD;i < bmpHeight;i++)//scanning each row 
	{
		for (int j = 0;j < bmpWidth-ParamD;j++)//scanning each col
		{
			pxl = pixels[i][j];
			pxlNear = pixels[i - ParamD][j + ParamD];
			if (pxl < pxlNear)
				mGLCMMat[pxl][pxlNear]++;
			else if ((pxl > pxlNear))
				mGLCMMat[pxlNear][pxl]++;
			else
				mGLCMMat[pxl][pxlNear] += 2;
		}
	}
	//Compare direction:180 rad
	for (int i = 0;i < bmpHeight - ParamD;i++)//scanning each row 
	{
		for (int j = 0;j < bmpWidth;j++)//scanning each col
		{
			pxl = pixels[i][j];
			pxlNear = pixels[i + ParamD][j];
			if (pxl < pxlNear)
				mGLCMMat[pxl][pxlNear]++;
			else if ((pxl > pxlNear))
				mGLCMMat[pxlNear][pxl]++;
			else
				mGLCMMat[pxl][pxlNear] += 2;
		}
	}
	return true;
}

bool CGLCM::CalcFeatures() //Use mGLCMMat to get Features
{
	int i, j, k;	//��ʱ������ѭ������
	double t, t2;	//��ʱ�������洢�����м����
	
	// matNorm[256][256]���м�������һ����� GLCM ���������±� 0-255��mMatNorm=mMat/sum
	// ֮����ʹ�ö�̬�ڴ���䣬ʱ�������ݹ����¶�ջ���
	double* matNormData, ** matNorm;
	matNormData = new double[mcGLCMMatDim * mcGLCMMatDim];	// ����̬��ά���������Ԫ�ض���Ϊ���Կռ�
	matNorm = new double* [mcGLCMMatDim];	// ָ�����鱣������׵�ַ
	for (i = 0; i < mcGLCMMatDim; i++)
		matNorm[i] = matNormData + mcGLCMMatDim * i;

	int elemMat;								// ʹ�� elemMat �������Ԫ�ص�ֵ������Ҫ����ʹ�� mMat[][] ��ά������±�,���Ч��
	double elemNorm;							// ʹ�� elemNorm �����׼����ľ���Ԫ�ص�ֵ������Ҫ����ͨ�������±�ʹ�� matNorm[][] ��ά����
	double pXYPlus[2 * (mcGLCMMatDim - 1) + 1];	// 0 to 510, p_x+y (k) ��ʾi+j=k ����ЩԪ��֮��
	double pXYMinus[mcGLCMMatDim];				// 0 to 255, p_x-y (k)����ʾ|i-j|=k ����ЩԪ��֮��
	double pXYMinusMean = 1.0 / mcGLCMMatDim;	// pXYMinus[] ��ֵ��ԶΪ 1/256���� pXYMinus[] ֮��Ϊ��׼����ľ��������Ԫ��֮��Ϊ1

	memset(mGLCMFeats, 0, sizeof(double) * (cntTotalGLCMFeatures + 1));
	memset(pXYPlus, 0, sizeof(double) * (2 * (mcGLCMMatDim - 1) + 1));
	memset(pXYMinus, 0, sizeof(double) * (mcGLCMMatDim));

	// ============ ͳ�ƾ����е�����Ԫ�غͣ��Լ������Сֵ ============
	int sum = 0, max = 0 - 2147483648, min = 2147483647;
	for (i = 0;i < mcGLCMMatDim;i++)
	{
		for (j = mcGLCMMatDim - 1;j >= i;j--)
		{
			elemMat = mGLCMMat[i][j];
			max = (elemMat > max ? elemMat : max);
			min = (elemMat < min ? elemMat : min);
			sum = sum + elemMat * 2;//����ǷǶԽ���Ԫ�أ���ô��2*elemMat
		}
		sum -= elemMat;//����ǶԽ���Ԫ�أ���ôֻҪ��elemMat���൱�ڼ�2*elemMat�ټ�elemMat
		//�ڲ�forѭ��ÿ�ν���ʱ����Ȼi=j, �������Խ���Ԫ��
		//��������д���ܿ���if�ж���䣬����˳�������
	}
	if (sum == 0) // �����ض������ܺ�Ϊ0���޷���������
		return false;	

	mGLCMFeats[MaxProb] = (double)max / sum;
	mGLCMFeats[MinProb] = (double)min / sum;

	// ����
	// ע�⣺����ѭ��ֻѭ���˾���������ǣ�û��ѭ������������ǣ�
	//       ��ĳЩ����������Ҫ������Ԫ��ʱ�������������λ��(i,j)��Ӧ
	//       ���������λ�õ�ֵ�����д��������Ƕ�Ӧλ�õ�ֵ�������Ǵ�ֵ����ȵģ�
	//       ���磬����Ҫ�ۼӼ�������е�����Ԫ�ص�ƽ����ʱ��
	for (i = 0; i < mcGLCMMatDim; i++)		// i��ʾGLCM������кţ��߶�
		for (j = i; j < mcGLCMMatDim; j++)	// j��ʾGLCM������кţ����
		{
			elemNorm = (double)mGLCMMat[i][j] / sum;
			matNorm[i][j] = elemNorm;
			if (i != j) 
				matNorm[j][i] = elemNorm;	// ��������һ�����Ϊ���������Ƕ�Ӧλ�õ���ֵͬ

			// ---- ���㼸������
			// ��� elemNorm=0 �򲻼��㣨�� mMat[i][j]==0 �ж� double �͵� elemNorm==0��
			if (mGLCMMat[i][j] == 0) continue;

			// ��3����������������ASM��or energy or homogeneity�����Ƕ��׾�Angular Second Moment ��
			t = elemNorm * elemNorm;
			mGLCMFeats[ASM] += t;
			if (i != j) mGLCMFeats[ASM] += t;		// �������Խ��ߣ��ټ�������Ԫ��ֵ

			// ��4��������������(Entropy)
			t = elemNorm * log(elemNorm);
			mGLCMFeats[Entropy] -= t;
			if (i != j) mGLCMFeats[Entropy] -= t;
			// ��5������������Contrast(�Աȶ�)
			t = elemNorm * pow((i - j), 2);
			mGLCMFeats[Contrast] += t;
			if (i != j) mGLCMFeats[Contrast] += t;
			// ��6������������Dissimilarity
			t = elemNorm * abs(i - j);
			mGLCMFeats[Dissimilarity] += t;
			if (i != j) mGLCMFeats[Dissimilarity] += t;
			// ��7����������������������(Homogeneity) 
			t = elemNorm * (1. / (1 + pow(i - j, 2)));
			mGLCMFeats[Homogeneity] += t;
			if (i != j) mGLCMFeats[Homogeneity] += t;


			// ��8��������������ֵ(Mean)��������� ����(Variance)��ҲҪ�ô˾�ֵ��
			t = i * elemNorm;		// i and j are symmetric���� j* Ҳ�ǿ��Ե�
			mGLCMFeats[Mean] += t;
			if (i != j) mGLCMFeats[Mean] += j * elemNorm;	// �������Խ��ߣ��ټ�������Ԫ��ֵ

			// ��8.5������ pXYPlus()��pXYMinus()
			pXYPlus[i + j] += elemNorm;
			if (i != j) pXYPlus[i + j] += elemNorm;	// �������Խ��ߣ��ټ�������Ԫ��ֵ
			pXYMinus[j - i] += elemNorm;				// i-j �ľ���ֵ��|i-j| ����ֵ�� = j-i, ��Ϊ j>=i
			if (i != j) pXYMinus[j - i] += elemNorm;	// �������Խ��ߣ��ټ�������Ԫ��ֵ

		}  // for (j=i; j<mcGLCMMatDim; j++)	// j��ʾGLCM������кţ����

	for (k = 0;k <= mcGLCMMatDim - 1;k++)
	{
		t = pXYMinus[k];
		// ��12����������������(Difference Entropy)
		mGLCMFeats[DiffEntropy] -= (t > 0 ? t * log(t) : 0);
		// ��13���������������(Difference Variance)
		mGLCMFeats[DiffVar] += (pXYMinus[k] - pXYMinusMean) * (pXYMinus[k] - pXYMinusMean);
		t = pXYPlus[k];
		// ��9�������������;�ֵ(Sum Average)
		mGLCMFeats[SumAverage] += k * t;
		// ��10����������������(Sum Entropy)������ �ͷ���(Sum Variance) ҲҪ�ú��صĽ����
		mGLCMFeats[SumEntropy] -= (t > 0 ? t * log(t) : 0);
	}

	mGLCMFeats[DiffVar] /= (mcGLCMMatDim - 1);
	
	for (k = mcGLCMMatDim;k <= 2 * mcGLCMMatDim - 2;k++)
	{
		t = pXYPlus[k];
		// �������㡾9���;�ֵ(Sum Average)
		mGLCMFeats[SumAverage] += k * t;
		// �������㡾10������(Sum Entropy)������ �ͷ���(Sum Variance) ҲҪ�ú��صĽ����
		mGLCMFeats[SumEntropy] -= (t > 0 ? t * log(t) : 0);
	}
	// ��11�������������ͷ���(Sum Variance)
	for (k = 0;k <= 2 * mcGLCMMatDim - 2;k++)
		mGLCMFeats[SumVar] += (pXYPlus[k] > 0 ? pXYPlus[k] : 0) *pow(k - mGLCMFeats[SumEntropy], 2);


	//�������������
	double matMean = mGLCMFeats[Mean];
	for (i = 0; i < mcGLCMMatDim; i++)		// i��ʾGLCM������кţ��߶�
	{
		for (j = 0; j < mcGLCMMatDim; j++)	// j��ʾGLCM������кţ����
		{
			elemNorm = matNorm[i][j];
			// ��14����������������(Sum of Squares, Variance)����Ҫ�õ�ƽ��ֵ mGLCMFeats[Mean]
			t = (i - matMean) * (i - matMean) * elemNorm;
			mGLCMFeats[Variance] += t;
			// ��15���������������׾ۼ���(Cluster Shade) 
			t = i + j - matMean - matMean;
			t2 = t * t * t * elemNorm;
			mGLCMFeats[ClusterShade] += t2;
			// ��16�������������Ľ׾ۼ���(Cluster Prominence)
			t2 *= t;
			mGLCMFeats[ClusterProm] += t2;
			// ��17�����������������(Correlation)
			t = i * j * elemNorm;
			mGLCMFeats[Correlation] += t;
		}
	}
	// ������:��17�������(Correlation)
	mGLCMFeats[Correlation] = (mGLCMFeats[Correlation] - matMean * matMean) / mGLCMFeats[Variance];
	
	unsigned char** pixels;
	int pxl;
	double xSum = 0, ySum = 0, pxlSum = 0;
	pixels = mBMPLoad.GetBMPBytes();
	int bmpWidth = mBMPLoad.Width();
	int bmpHeight = mBMPLoad.Height();
	for (i = 0;i < bmpHeight;i++)
	{
		for (j = 0;j < bmpWidth;j++)
		{
			pxl = pixels[i][j];
			xSum += pxl * (j + 1);
			ySum += pxl * (i + 1);
			pxlSum += pxl;
		}
	}
	// ��18������������ǿ��(Strength)  λͼ�е��������ص� ����ֵ֮��
	mGLCMFeats[Strength] = (double)pxlSum;
	// ��19������������MassX
	mGLCMFeats[MassX] = (double)xSum / pxlSum;
	// ��20������������MassY
	mGLCMFeats[MassY] = (double)ySum / pxlSum;

	// �ͷ��� new ���ٵĶ�̬�ռ� 
	delete[] matNorm;
	delete[] matNormData;

	return true;
}
