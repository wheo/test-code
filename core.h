#ifndef _CCORE_H_
#define _CCORE_H_

#include "main.h"

class CCore : public PThread
{
public:
	CCore(void);
	~CCore(void);

	bool Create();
	void Delete();

	//bool GetOutputs(string basepath);

protected:
	pthread_mutex_t m_mutex_core;

private:
protected:
	void Run();
	void OnTerminate(){};
};
#endif // _CCORE_H_