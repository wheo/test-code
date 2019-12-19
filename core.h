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
	int Demux();
	bool SetSocket();
	bool send_bitstream(uint8_t *stream, int size);

	//bool GetOutputs(string basepath);

protected:
	pthread_mutex_t m_mutex_core;
	string m_filename;
	int m_sec;

	const AVBitStreamFilter *m_bsf = NULL;
	AVBSFContext *m_bsfc = NULL;

	int m_nPlayAudioStream;
	int m_nAudioStreamCount;
	double m_fDuration;
	double m_fFPS;
	int m_sock;
	sockaddr_in m_mcast_group;

	int video_stream_idx;

	AVCodecContext *video_dec_ctx = NULL;
	AVStream *video_stream = NULL, *audio_stream = NULL;

	int refcount;

private:
	int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);

protected:
	void Run();
	void OnTerminate(){};
};
#endif // _CCORE_H_