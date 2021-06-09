#define LZY_ASSERT_ENABLED
#define LZY_LOG_TRACE_ENABLED
#define LZY_LOG_INFO_ENABLED
#define LZY_LOG_WARN_ENABLED
#define LZY_LOG_ERROR_ENABLED
#define LZY_LOG_FATAL_ENABLED

#include "Lzy.h"

int main()
{

	LzyApplicationConfig appConfig;
	appConfig.pApplicationName = "Lzy Engine";
	appConfig.uResX = 1280;
	appConfig.uResY = 720;

	lzy_application_create(&appConfig);
	lzy_application_run();

	return 0;
}
