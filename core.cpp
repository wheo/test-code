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
	m_mcast_group.sin_port = htons(19262);
	m_mcast_group.sin_addr.s_addr = inet_addr("228.67.45.91");

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
	Demux();
#if 1
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

int CCore::Demux()
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

	FILE *fp;
	string dump_name = m_filename + ".bin";
	fp = fopen(dump_name.c_str(), "wb");

	AVFormatContext *fmt_ctx;

	if (avformat_open_input(&fmt_ctx, m_filename.c_str(), NULL, NULL) < 0)
	{
		cout << "[CORE] Could not open source file : " << m_filename << endl;
		return false;
	}
	else
	{
		cout << "[CORE] " << m_filename << " file is opened" << endl;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0)
	{
		cout << "[CORE] failed to find proper stream in FILE : " << m_filename << endl;
	}

	if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0)
	{
		video_stream = fmt_ctx->streams[video_stream_idx];

		/* dump input information to stderr */
		av_dump_format(fmt_ctx, 0, m_filename.c_str(), 0);

		if (!video_stream)
		{
			fprintf(stderr, "[DEMUXER] Could not find audio or video stream in the input, aborting\n");
			ret = 1;
		}
	}

	int64_t ts = 0;

	begin = high_resolution_clock::now();
	int readcnt = 0;
	int nFrame = 0;

	while (!m_bExit)
	{

		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;

		if (av_read_frame(fmt_ctx, &pkt) < 0)
		{
			cout << "[CORE] meet EOF(" << fmt_ctx->filename << ")" << endl;
			avformat_close_input(&fmt_ctx);
			// 메모리릭 나면 아래 avformat_free_context 추가할 것
			//avformat_free_context(fmt_ctx);
			break;
		}

#if 0
		if ((ret = av_bsf_send_packet(m_bsfc, &pkt)) < 0)
		{
			av_log(fmt_ctx, AV_LOG_ERROR, "Failed to send packet to filter %s for stream %d\n", m_bsfc->filter->name, pkt.stream_index);
			//return ret;
		}
		// TODO: when any automatically-added bitstream filter is generating multiple
		// output packets for a single input one, we'll need to call this in a loop
		// and write each output packet.
		if ((ret = av_bsf_receive_packet(m_bsfc, &pkt)) < 0)
		{
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				av_log(fmt_ctx, AV_LOG_ERROR, "Failed to receive packet from filter %s for stream %d\n", m_bsfc->filter->name, pkt.stream_index);
			}
			if (fmt_ctx->error_recognition & AV_EF_EXPLODE)
			{
				//
			}
			//return ret;
			//return 0;
		}
#endif
		nFrame++;
		cout << "[CORE] (" << nFrame << ")pkt.flag : " << pkt.flags << ", size : " << pkt.size << endl;

		if (!send_bitstream(pkt.data, pkt.size))
		{
			cout << "[DEMUXER] send_bitstream failed" << endl;
		}
		else
		{
			//cout << "[DEMUXER.ch" << m_nChannel << "] send_bitstream success" << endl;
			fwrite(pkt.data, pkt.size, 1, fp);
		}

		av_packet_unref(&pkt);

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
	}
	fclose(fp);
}

int CCore::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret, stream_index;
	AVStream *st;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0)
	{
		fprintf(stderr, "Could not find %s stream in input file");
		return ret;
	}
	else
	{
		stream_index = ret;
		st = fmt_ctx->streams[stream_index];

		/* find decoder for the stream */
		dec = avcodec_find_decoder(st->codecpar->codec_id);
		if (!dec)
		{
			fprintf(stderr, "Failed to find %s codec\n", av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}

		/* Allocate a codec context for the decoder */
		*dec_ctx = avcodec_alloc_context3(dec);
		if (!*dec_ctx)
		{
			fprintf(stderr, "Failed to allocate the %s codec context\n", av_get_media_type_string(type));
			return AVERROR(ENOMEM);
		}

		/* Copy codec parameters from input stream to output codec context */
		if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
		{
			fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n", av_get_media_type_string(type));
			return ret;
		}

		/* Init the decoders, with or without reference counting */
		av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
		if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
		{
			fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(type));
			return ret;
		}
		*stream_idx = stream_index;
	}

	return 0;
}

void CCore::Delete()
{
}
