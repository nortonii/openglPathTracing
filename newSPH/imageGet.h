#define BMP_Header_Length 54
class frame2Real
{
	float* pPixelData;
	float* realData;
	int pixelDataLength;
	int width, height;
	int spp;
	public:
	frame2Real(int width,int height,int spp):width(width),height(height),spp(spp)
	{
		GLint i, j;
		i = width * 3;      // 得到每一行的像素数据长度
		while (i % 4 != 0)  // 补充数据，直到 i 是的倍数
			++i;
		pixelDataLength = i * height;
		pPixelData = (float*)malloc(pixelDataLength * sizeof(float));
		realData = (float*)malloc(pixelDataLength * sizeof(float));
		iniRealData();
		if (pPixelData == 0)
			exit(0);
	}
	void iniRealData(){
		for (int i = 0; i < pixelDataLength; i++) {
			realData[i] = 0.0f;
		}
	}
	void frame2local(){
		glReadBuffer(GL_FRONT);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glReadPixels(0, 0, width, height, GL_BGR, GL_FLOAT, pPixelData);
		countReal();
	}
	void countReal() {
		for (int i = 0; i < pixelDataLength; i++) {
			realData[i] += pPixelData[i];
		}
	}
	void saveFrameBuff(const char* fileName, int width, int height)
	{
		for (int i = 0; i < pixelDataLength; i++) {
			realData[i] = realData[i]/spp;
		}
		cv::Mat localImage = cv::Mat(height, width, CV_32FC3);
		cv::Mat flipImage;
		localImage.data = (uchar*)realData;
		cv::flip(localImage,flipImage,0);
		cv::imshow("原始图像", flipImage);
		cv::waitKey(0);
	}
	
};