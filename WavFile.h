#ifndef __wavfile__h
#define __wavfile__h

#include <vector>

class WavFile
{
public:
	WavFile(const char *name,bool read=true,int c=1,int r=44100,bool cached=true);
		
	virtual	~WavFile();
	
	virtual	void	Close();
	
	virtual void	AddSample(float s);

	virtual	float	ReadSample();
	virtual	int		CurrentSample(){return current;}
	
	virtual	int		GetChannels(){return channels;}
	virtual	int		GetSamples(){return samples/channels;}
	virtual	int		GetRate(){return rate;}
	virtual int		GetByteSize(){return bytesize;}
	

	virtual	float	GetSample(int which){if (cached) return data[which];return 0;}
	virtual	void	Expect(size_t num){if (cached) dataw.reserve(num);}
protected:
	FILE *f;
	int samples;
	int channels,rate,bytesize;
	bool read;
	int current;
	long pfmt,pdata;
	float *data;
	std::vector<float> dataw;
	char name[256];
	bool cached;
};

#endif
