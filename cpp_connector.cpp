#include "cpp_connector.h"
#include "cppTest.h"

void* testcpp_new() {
	return new Testcpp();
}

void createNewWindow_c(void* testcpp) {
	Testcpp *T = (Testcpp *)testcpp;
	T->createNewWindow();
}