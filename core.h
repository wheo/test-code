#ifndef _CCORE_H_
#define _CCORE_H_
#define MAX_AUDIO_STREAM 8
#include "main.h"

class CCore : public PThread
{
public:
	CCore(void);
	~CCore(void);

	bool Create(string filename, int sec);
	void Delete();
	bool SetSocket();
	bool send_bitstream(uint8_t *stream, int size);

	//bool GetOutputs(string basepath);

protected:
	pthread_mutex_t m_mutex_core;
	string m_filename;
	int m_sec;
	int m_nVideoStream;
	int m_nAudioStream[MAX_AUDIO_STREAM];

	int m_nPlayAudioStream;
	int m_nAudioStreamCount;
	double m_fDuration;
	double m_fFPS;
	int m_sock;
	sockaddr_in m_mcast_group;

private:
protected:
	void Run();
	void OnTerminate(){};
};
#endif // _CCORE_H_