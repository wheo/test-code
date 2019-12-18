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
	int64_t tick_diff = 0;
	int64_t num, den;
	int64_t acc_time = 0;
	int64_t acc_sec = 0;
	double target_time;
	num = 1001;
	den = 30000;
	target_time = (double)num / (double)den * 1000000;
	high_resolution_clock::time_point begin;
	high_resolution_clock::time_point end;
	cout << tick_diff << endl;
	begin = high_resolution_clock::now();
	while (!m_bExit)
	{

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
