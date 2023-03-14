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
	int i, j, k;	//临时变量：循环索引
	double t, t2;	//临时变量：存储计算中间过程
	
	// matNorm[256][256]：中间结果，归一化后的 GLCM 矩阵，数组下标 0-255，mMatNorm=mMat/sum
	// 之所以使用动态内存分配，时担心数据过大导致堆栈溢出
	double* matNormData, ** matNorm;
	matNormData = new double[mcGLCMMatDim * mcGLCMMatDim];	// 将动态二维数组的所有元素定义为线性空间
	matNorm = new double* [mcGLCMMatDim];	// 指针数组保存各行首地址
	for (i = 0; i < mcGLCMMatDim; i++)
		matNorm[i] = matNormData + mcGLCMMatDim * i;

	int elemMat;								// 使用 elemMat 保存矩阵元素的值，而不要经常使用 mMat[][] 二维数组的下标,提高效率
	double elemNorm;							// 使用 elemNorm 保存标准化后的矩阵元素的值，而不要经常通过给出下标使用 matNorm[][] 二维数组
	double pXYPlus[2 * (mcGLCMMatDim - 1) + 1];	// 0 to 510, p_x+y (k) 表示i+j=k 的那些元素之和
	double pXYMinus[mcGLCMMatDim];				// 0 to 255, p_x-y (k)，表示|i-j|=k 的那些元素之和
	double pXYMinusMean = 1.0 / mcGLCMMatDim;	// pXYMinus[] 均值永远为 1/256，因 pXYMinus[] 之和为标准化后的矩阵的所有元素之和为1

	memset(mGLCMFeats, 0, sizeof(double) * (cntTotalGLCMFeatures + 1));
	memset(pXYPlus, 0, sizeof(double) * (2 * (mcGLCMMatDim - 1) + 1));
	memset(pXYMinus, 0, sizeof(double) * (mcGLCMMatDim));

	// ============ 统计矩阵中的所有元素和，以及最大、最小值 ============
	int sum = 0, max = 0 - 2147483648, min = 2147483647;
	for (i = 0;i < mcGLCMMatDim;i++)
	{
		for (j = mcGLCMMatDim - 1;j >= i;j--)
		{
			elemMat = mGLCMMat[i][j];
			max = (elemMat > max ? elemMat : max);
			min = (elemMat < min ? elemMat : min);
			sum = sum + elemMat * 2;//如果是非对角线元素，那么加2*elemMat
		}
		sum -= elemMat;//如果是对角线元素，那么只要加elemMat，相当于加2*elemMat再减elemMat
		//内层for循环每次结束时，必然i=j, 遍历至对角线元素
		//代码这样写，避开了if判断语句，提高了程序性能
	}
	if (sum == 0) // 若像素对数量总和为0，无法计算特征
		return false;	

	mGLCMFeats[MaxProb] = (double)max / sum;
	mGLCMFeats[MinProb] = (double)min / sum;

	// 计算
	// 注意：以下循环只循环了矩阵的上三角，没有循环矩阵的下三角！
	//       若某些计算若还需要下三角元素时，请根据上三角位置(i,j)对应
	//       获得下三角位置的值，自行处理（下三角对应位置的值与上三角此值是相等的）
	//       例如，再需要累加计算矩阵中的所有元素的平方和时：
	for (i = 0; i < mcGLCMMatDim; i++)		// i表示GLCM矩阵的行号：高度
		for (j = i; j < mcGLCMMatDim; j++)	// j表示GLCM矩阵的列号：宽度
		{
			elemNorm = (double)mGLCMMat[i][j] / sum;
			matNorm[i][j] = elemNorm;
			if (i != j) 
				matNorm[j][i] = elemNorm;	// 将下三角一起填充为：与上三角对应位置的相同值

			// ---- 计算几个特征
			// 如果 elemNorm=0 则不计算（以 mMat[i][j]==0 判断 double 型的 elemNorm==0）
			if (mGLCMMat[i][j] == 0) continue;

			// 【3】计算特征：能量ASM（or energy or homogeneity）（角二阶距Angular Second Moment ）
			t = elemNorm * elemNorm;
			mGLCMFeats[ASM] += t;
			if (i != j) mGLCMFeats[ASM] += t;		// 若非主对角线，再加下三角元素值

			// 【4】计算特征：熵(Entropy)
			t = elemNorm * log(elemNorm);
			mGLCMFeats[Entropy] -= t;
			if (i != j) mGLCMFeats[Entropy] -= t;
			// 【5】计算特征：Contrast(对比度)
			t = elemNorm * pow((i - j), 2);
			mGLCMFeats[Contrast] += t;
			if (i != j) mGLCMFeats[Contrast] += t;
			// 【6】计算特征：Dissimilarity
			t = elemNorm * abs(i - j);
			mGLCMFeats[Dissimilarity] += t;
			if (i != j) mGLCMFeats[Dissimilarity] += t;
			// 【7】计算特征：逆差距或均匀性(Homogeneity) 
			t = elemNorm * (1. / (1 + pow(i - j, 2)));
			mGLCMFeats[Homogeneity] += t;
			if (i != j) mGLCMFeats[Homogeneity] += t;


			// 【8】计算特征：均值(Mean)（后面计算 方差(Variance)，也要用此均值）
			t = i * elemNorm;		// i and j are symmetric，用 j* 也是可以的
			mGLCMFeats[Mean] += t;
			if (i != j) mGLCMFeats[Mean] += j * elemNorm;	// 若非主对角线，再加下三角元素值

			// 【8.5】计算 pXYPlus()、pXYMinus()
			pXYPlus[i + j] += elemNorm;
			if (i != j) pXYPlus[i + j] += elemNorm;	// 若非主对角线，再加下三角元素值
			pXYMinus[j - i] += elemNorm;				// i-j 的绝对值：|i-j| ，此值就 = j-i, 因为 j>=i
			if (i != j) pXYMinus[j - i] += elemNorm;	// 若非主对角线，再加下三角元素值

		}  // for (j=i; j<mcGLCMMatDim; j++)	// j表示GLCM矩阵的列号：宽度

	for (k = 0;k <= mcGLCMMatDim - 1;k++)
	{
		t = pXYMinus[k];
		// 【12】计算特征：差熵(Difference Entropy)
		mGLCMFeats[DiffEntropy] -= (t > 0 ? t * log(t) : 0);
		// 【13】计算特征：差方差(Difference Variance)
		mGLCMFeats[DiffVar] += (pXYMinus[k] - pXYMinusMean) * (pXYMinus[k] - pXYMinusMean);
		t = pXYPlus[k];
		// 【9】计算特征：和均值(Sum Average)
		mGLCMFeats[SumAverage] += k * t;
		// 【10】计算特征：和熵(Sum Entropy)（计算 和方差(Sum Variance) 也要用和熵的结果）
		mGLCMFeats[SumEntropy] -= (t > 0 ? t * log(t) : 0);
	}

	mGLCMFeats[DiffVar] /= (mcGLCMMatDim - 1);
	
	for (k = mcGLCMMatDim;k <= 2 * mcGLCMMatDim - 2;k++)
	{
		t = pXYPlus[k];
		// 继续计算【9】和均值(Sum Average)
		mGLCMFeats[SumAverage] += k * t;
		// 继续计算【10】和熵(Sum Entropy)（计算 和方差(Sum Variance) 也要用和熵的结果）
		mGLCMFeats[SumEntropy] -= (t > 0 ? t * log(t) : 0);
	}
	// 【11】计算特征：和方差(Sum Variance)
	for (k = 0;k <= 2 * mcGLCMMatDim - 2;k++)
		mGLCMFeats[SumVar] += (pXYPlus[k] > 0 ? pXYPlus[k] : 0) *pow(k - mGLCMFeats[SumEntropy], 2);


	//计算第四批特征
	double matMean = mGLCMFeats[Mean];
	for (i = 0; i < mcGLCMMatDim; i++)		// i表示GLCM矩阵的行号：高度
	{
		for (j = 0; j < mcGLCMMatDim; j++)	// j表示GLCM矩阵的列号：宽度
		{
			elemNorm = matNorm[i][j];
			// 【14】计算特征：方差(Sum of Squares, Variance)，需要用到平均值 mGLCMFeats[Mean]
			t = (i - matMean) * (i - matMean) * elemNorm;
			mGLCMFeats[Variance] += t;
			// 【15】计算特征：三阶聚集性(Cluster Shade) 
			t = i + j - matMean - matMean;
			t2 = t * t * t * elemNorm;
			mGLCMFeats[ClusterShade] += t2;
			// 【16】计算特征：四阶聚集性(Cluster Prominence)
			t2 *= t;
			mGLCMFeats[ClusterProm] += t2;
			// 【17】计算特征：相关性(Correlation)
			t = i * j * elemNorm;
			mGLCMFeats[Correlation] += t;
		}
	}
	// 继续求:【17】相关性(Correlation)
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
	// 【18】计算特征：强度(Strength)  位图中的所有像素的 像素值之和
	mGLCMFeats[Strength] = (double)pxlSum;
	// 【19】计算特征：MassX
	mGLCMFeats[MassX] = (double)xSum / pxlSum;
	// 【20】计算特征：MassY
	mGLCMFeats[MassY] = (double)ySum / pxlSum;

	// 释放用 new 开辟的动态空间 
	delete[] matNorm;
	delete[] matNormData;

	return true;
}
