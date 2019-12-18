#ifndef _MAIN_H_
#define _MAIN_H_

#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>

#include <signal.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/random_seed.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#include "misc.h"
#include "json/json.h"

#define PACKET_HEADER_SIZE 24
#define PACKET_SIZE 4096

#define _d(fmt, args...)     \
    {                        \
        printf(fmt, ##args); \
    }

using namespace std;
using namespace std::chrono;

#endif // __MAIN_H