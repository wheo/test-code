#include "main.h"
#include "core.h"

bool exit_flag_main = false;

// 2019-12-19 clone 후 commit 후 push 되는지 테스트

pthread_mutex_t sleepMutex;
pthread_cond_t sleepCond;

void sigfunc(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
	{
		exit_flag_main = true;
	}
	pthread_cond_signal(&sleepCond);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "[MAIN] example) ./test-code filename" << endl;
		exit(0);
	}

	string filename = argv[1];
	int sec = atoi(argv[2]);

	signal(SIGINT, sigfunc);
	signal(SIGTERM, sigfunc);
	signal(SIGHUP, sigfunc);

	CCore *core = new CCore();
	core->Create(filename, sec);

	high_resolution_clock::time_point begin = high_resolution_clock::now();
	while (!exit_flag_main)
	{
		pthread_mutex_lock(&sleepMutex);
		pthread_cond_wait(&sleepCond, &sleepMutex);
		pthread_mutex_unlock(&sleepMutex);
	}
	high_resolution_clock::time_point end = high_resolution_clock::now();
	int64_t tick_diff = duration_cast<microseconds>(end - begin).count();
	cout << "[MAIN] time elapsed : " << tick_diff << endl;
	delete core;
	cout << "[MAIN] Exited" << endl;
	return 0;
}