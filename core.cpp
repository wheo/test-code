#include "core.h"

extern char __BUILD_DATE;
extern char __BUILD_NUMBER;

CCore::CCore(void)
{
	m_bExit = false;
	pthread_mutex_init(&m_mutex_core, 0);
}

CCore::~CCore(void)
{
	m_bExit = true;
	pthread_mutex_destroy(&m_mutex_core);
	cout << "[CORE] Trying to exit thread" << endl;
	Terminate();
	cout << "[CORE] has been exited..." << endl;
}

bool CCore::Create(string filename, int sec)
{
	m_filename = filename;
	m_sec = sec;
	SetSocket();
	Start();
	return true;
}

bool CCore::SetSocket()
{
	struct ip_mreq mreq;
	int state;

	memset(&m_mcast_group, 0x00, sizeof(m_mcast_group));
	m_mcast_group.sin_family = AF_INET;
	m_mcast_group.sin_port = htons(44223);
	m_mcast_group.sin_addr.s_addr = inet_addr("239.0.0.1");

	m_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == m_sock)
	{
		cout << "[CORE] cket createion error" << endl;
		return false;
	}

	if (-1 == bind(m_sock, (struct sockaddr *)&m_mcast_group, sizeof(m_mcast_group)))
	{
		cout << "[CORE] nd error" << endl;
		return false;
	}

	uint reuse = 1;
	state = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (state < 0)
	{
		cout << "[CORE] tting SO_REUSEADDR error" << endl;
		return false;
	}

	uint ttl = 16;
	state = setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	if (state < 0)
	{
		cout << "[CORE] tting IP_MULTICAST_TTL error" << endl;
		;
		return false;
	}

	mreq.imr_multiaddr = m_mcast_group.sin_addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(m_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		cout << "[CORE] d membership setsocket opt" << endl;
		return false;
	}

	struct timeval read_timeout;
	read_timeout.tv_sec = 1;
	read_timeout.tv_usec = 0;
	if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) < 0)
	{
		cout << "[CORE] t timeout error" << endl;
		return false;
	}

	return true;
}

bool CCore::send_bitstream(uint8_t *stream, int size)
{
	int tot_packet = 0;
	int cur_packet = 1;

	int tot_size;
	int cur_size;

	int remain;

	int nSendto;

	char video_type = 0;	   // 1(NTSC), 2(PAL), 3(PANORAMA), 4(FOCUS)
	char video_codec_type = 0; // 1(HEVC), 2(H264)
	char video_frame_type = 0; // 0(P frame), 1(I frame)
	char reserve[5] = {
		0,
	};

	uint8_t *p = stream;
	uint8_t buffer[PACKET_HEADER_SIZE + PACKET_SIZE];

	tot_size = remain = size;

	if ((tot_size % PACKET_SIZE) == 0)
	{
		tot_packet = tot_size / PACKET_SIZE;
	}
	else
	{
		tot_packet = tot_size / PACKET_SIZE + 1;
	}
	while (remain > 0)
	{
		if (remain > PACKET_SIZE)
		{
			cur_size = PACKET_SIZE;
		}
		else
		{
			cur_size = remain;
		}

		memcpy(&buffer[0], &tot_size, 4);
		memcpy(&buffer[4], &cur_size, 4);
		memcpy(&buffer[8], &tot_packet, 4);
		memcpy(&buffer[12], &cur_packet, 4);
		memcpy(&buffer[16], &video_type, 1);
		memcpy(&buffer[17], &video_codec_type, 1);
		memcpy(&buffer[18], &video_frame_type, 1);
		memcpy(&buffer[19], &reserve, sizeof(reserve));
		memcpy(&buffer[24], p, cur_size);

		nSendto = sendto(m_sock, buffer, PACKET_HEADER_SIZE + cur_size, 0, (struct sockaddr *)&m_mcast_group, sizeof(m_mcast_group));
		if (nSendto < 0)
		{
			return false;
		}
		else
		{
#if 0
			cout << "[DEMUXER.ch" << m_nChannel << "] "
				 << "tot_size : " << tot_size << ", cur_size : " << cur_size << ", tot_packet : " << tot_packet << ", cur_packet : " << cur_packet << ", nSendto : " << nSendto << endl;
#endif
		}

		remain -= cur_size;
		p += cur_size;
		cur_packet++;
	}

	return true;
}

void CCore::Run()
{
	int64_t tick_diff = 0;
	double num, den;
	int64_t acc_time = 0;
	int64_t acc_sec = 0;
	double target_time;
	double fTime;
	num = 1001;
	den = 60000;
	double fps = den / num;
	target_time = (double)num / (double)den * AV_TIME_BASE;

	high_resolution_clock::time_point begin;
	high_resolution_clock::time_point end;
	cout << tick_diff << endl;
	begin = high_resolution_clock::now();

	int ret;

	AVFormatContext *fmt_ctx;

	if (avformat_open_input(&fmt_ctx, m_filename.c_str(), NULL, NULL) < 0)
	{
		cout << "[CORE] Could not open source file : " << m_filename << endl;
	}
	else
	{
		cout << "[CORE] " << m_filename << " file is opened, max stream : " << fmt_ctx->nb_streams << endl;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0)
	{
		cout << "[CORE] failed to find proper stream in FILE : " << m_filename << endl;
	}

	double fFPS = 0.;

	for (int i = 0; i < (int)fmt_ctx->nb_streams; i++)
	{
		AVCodec *pCodec = NULL;
		AVCodecContext *pCodecContext = NULL;
		AVStream *pStream = NULL;

		pStream = fmt_ctx->streams[i];
		pCodecContext = pStream->codec;

		if (pCodecContext->codec_type == AVMEDIA_TYPE_AUDIO || pCodecContext->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			AVDictionary *opts = NULL;

			pCodec = avcodec_find_decoder(pCodecContext->codec_id);

			pCodecContext->workaround_bugs = 1;
			pCodecContext->lowres = 0;
			pCodecContext->idct_algo = FF_IDCT_AUTO;
			pCodecContext->skip_frame = AVDISCARD_DEFAULT;
			pCodecContext->skip_idct = AVDISCARD_DEFAULT;
			pCodecContext->skip_loop_filter = AVDISCARD_DEFAULT;
			pCodecContext->error_concealment = 3;
			if (pCodec->capabilities & AV_CODEC_CAP_DR1)
			{
				pCodecContext->flags |= 0x4000;
			}

			av_dict_set(&opts, "threads", "1", 0);
			if ((ret = avcodec_open2(pCodecContext, pCodec, &opts)) < 0)
			{
				_d("[CORE] st.%d >> Failed to open codec (0x%x)\n", i, pCodecContext->codec_id);
			}

			_d("[CORE] st.%d info >> start = %lld, duration = %lld, Timebase = %d/%d\n", i, pStream->start_time, pStream->duration, pStream->time_base.num, pStream->time_base.den);
			if (pCodecContext->codec_type == AVMEDIA_TYPE_VIDEO && pStream->duration != AV_NOPTS_VALUE)
			{
				m_fDuration = (double)(pStream->duration * pStream->time_base.num) / pStream->time_base.den;
			}
			if (pCodecContext->codec_type == AVMEDIA_TYPE_AUDIO && m_fDuration == 0.)
			{
				m_fDuration = (double)(pStream->duration * pStream->time_base.num) / pStream->time_base.den;
			}
			/*
			if (pCodecContext->codec_type == AVMEDIA_TYPE_AUDIO && !m_pMD) {
				if (pDec->codec_id >= AV_CODEC_ID_FIRST_AUDIO && pDec->codec_id <= AV_CODEC_ID_PCM_S16BE_PLANAR) {
					m_pMD = new CMyDPM();
					m_pMD->Create(pDec->codec_id, pDec->channels);
				}
			}
			*/

			if (pCodecContext->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				if (m_nVideoStream < 0)
				{
					m_nVideoStream = i;

					fFPS = (double)pStream->avg_frame_rate.num / pStream->avg_frame_rate.den;
					fFPS = rnd(fFPS, 2);
				}
			}
			if (pCodecContext->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				m_nAudioStream[m_nAudioStreamCount] = i;
				m_nAudioStreamCount++;

				if (m_nPlayAudioStream < 0)
				{
					m_nPlayAudioStream = i;
				}
			}
		}
	}

	if (m_nVideoStream < 0)
	{
		_d("[CORE] 비디오 스트림 정보 없음. path: %s\n", m_filename.c_str());
	}

	if (m_nPlayAudioStream < 0)
	{
		_d("[CORE] 오디오 스트림 정보 없음. path: %s\n", m_filename.c_str());
	}

	_d("[CORE] fps : %.3f\n", fFPS);
	int cnt = 0;

	while (!m_bExit)
	{
		AVStream *pStream = fmt_ctx->streams[m_nVideoStream];
		int nFrame = m_sec * fps; // 0초로 이동하고 싶다.
		fTime = (((double)nFrame * pStream->avg_frame_rate.den) / pStream->avg_frame_rate.num) - 0.5;
		fTime = max(fTime, 0.);

		AVRational timeBaseQ;
		AVRational timeBase = pStream->time_base;

		timeBaseQ.num = 1;
		timeBaseQ.den = AV_TIME_BASE;

		int64_t tm = (int64_t)(fTime * AV_TIME_BASE);
		tm = av_rescale_q(tm, timeBaseQ, timeBase);
		ret = 0;
		//avcodec_flush_buffers(fmt_ctx->streams[m_nVideoStream]->codec);
		if (cnt == 0)
		{
			ret = avformat_seek_file(fmt_ctx, m_nVideoStream, 0, tm, tm, 0);
			cnt++;
		}
		printf("[CORE] ret : %d , m_sec : %d, fps : %.3f, fTime : %.3f, tm : %lld, nFrame : %d, avg_frame_rate.den : %d, avg_frame_rate.num : %d, timebase.den : %d, timebase.num : %d\n", ret, m_sec, fps, fTime, tm, nFrame, pStream->avg_frame_rate.den, pStream->avg_frame_rate.num, timeBase.num, timeBase.den);

		AVPacket pkt;
		av_init_packet(&pkt);

		if (av_read_frame(fmt_ctx, &pkt) < 0)
		{
			cout << "[CORE] meet EOF(" << fmt_ctx->filename << "), tm : " << tm << endl;
			avformat_close_input(&fmt_ctx);

			//avformat_free_context(fmt_ctx);
			break;
		}
		if (!send_bitstream(pkt.data, pkt.size))
		{
			cout << "[CORE] send_bitstream failed" << endl;
		}
		else
		{
			cout << "[CORE] send_bitstream success" << endl;
		}

#if 1
		while (tick_diff < target_time)
		{
			//usleep(1); // 1000000 us = 1 sec
			end = high_resolution_clock::now();
			tick_diff = duration_cast<microseconds>(end - begin).count();
			//cout << tick_diff << endl;
			this_thread::sleep_for(microseconds(1));
			//acc_time += tick_diff;
			//begin = high_resolution_clock::now();
		}
		begin = end;
		cout << "[CORE] num : " << num << ", den : " << den << ", target_time : " << target_time << ", tick_diff : " << tick_diff << endl;
		tick_diff = 0;
#else
		string path = "./dummy.json";
		if (false)
		{
			Json::Value root;
			Json::Value info;
			info["test"] = "testjson1";
			root["channels"].append(info);
			info["test"] = "testjson2";
			root["channels"].append(info);
			info["test"] = "testjson3";
			root["channels"].append(info);
			info["test"] = "testjson4";
			root["channels"].append(info);
			info["test"] = "testjson5";
			root["channels"].append(info);
			Json::StreamWriterBuilder builder;
			builder["commentStyle"] = "None";
			builder["indentation"] = "   "; // tab
			std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
			std::ofstream ofs(path);
			writer->write(root, &ofs);
			ofs.close();
			cout << "[MISC] Create Json meta completed : " << path << endl;
		}
		else
		{
			ifstream ifs("./dummy.json", ifstream::binary);
			if (!ifs.is_open())
			{
				ifs.close();
			}
			else
			{
				Json::Reader reader;
				Json::Value root;
				Json::Value info;
				if (reader.parse(ifs, root, true))
				{
					ifs.close();

					Json::StreamWriterBuilder builder;
					builder["commentStyle"] = "None";
					builder["indentation"] = "   "; // tab
					std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

					info["path"] = "output/blabla";

					root["files-array"].append(info);

					writer->write(root, &cout);

					//info["test"] = "testjson-append";
					//root["channels"].append(info);
					//writer->write(root, &cout);
					root["channels"][0].clear();
					root["channels"][0] = "";

					std::ofstream ofs(path);
					writer->write(root, &ofs);
					ofs.close();
				}
				else
				{
					cout << "[CORE] failed to parse" << endl;
					usleep(100000);
				}
			}
		}

		m_bExit = true;
#endif
	}
}

void CCore::Delete()
{
}
