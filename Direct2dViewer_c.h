#pragma once

//cpp_connector.h is a wrapper of Direct2dViewer.cpp to provide c api
//inspired by this tutorial to call cpp methods from c: https://bytes.com/topic/c/insights/921728-calling-c-class-methods-c


#ifdef __cplusplus
extern "C"
{
#endif
	void* Direct2dViewer_new();
	void Direct2dViewer_Initialize(void* D2dV);
	void Direct2dViewer_delete(void *D2dV);

#ifdef __cplusplus    
}
#endif