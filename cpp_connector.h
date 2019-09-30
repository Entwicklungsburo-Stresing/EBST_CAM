#pragma once

//cpp_connector.h is an example for a wrapper of cpp to provide c api
//inspired by this tutorial to call cpp methods from c: https://bytes.com/topic/c/insights/921728-calling-c-class-methods-c

#ifdef __cplusplus
extern "C"
{
#endif
	void* testcpp_new();
	void createNewWindow_c(void* testcpp);

#ifdef __cplusplus    
}
#endif