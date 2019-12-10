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

bool CCore::Create()
{
	Start();
	return true;
}

void CCore::Run()
{
	int64_t tick_diff;
	int64_t num, den;
	int64_t acc_time = 0;
	double sec = 0;
	double fps;
	num = 1001;
	den = 60000;
	fps = (double)num / (double)den * 1000000;
	high_resolution_clock::time_point begin = high_resolution_clock::now();
	while (!m_bExit)
	{
#if 0
		usleep(1); // 1000000 ms = 1 sec
		if (acc_time < fps)
		{
			acc_time += tick_diff;
		}
		else
		{
			cout << "[CORE] diff : " << acc_time << ", fps : " << fps << endl;
			//cout << "[CORE] sec : " << sec << endl;
			acc_time = 0;
		}
		high_resolution_clock::time_point end = high_resolution_clock::now();
		tick_diff = duration_cast<microseconds>(end - begin).count();
		//sec += (double)tick_diff / 1000000;
		begin = high_resolution_clock::now();
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

					writer->write(root, &cout);

					root["channel"] = "testjson1";

					root["channel"].clear();
					//root["channels"].clear();
					//root["channels"].empty();
					writer->write(root, &cout);

					//info["test"] = "testjson-append";
					//root["channels"].append(info);
					writer->write(root, &cout);

					std::ofstream ofs(path);
					writer->write(root, &ofs);
					ofs.close();
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
