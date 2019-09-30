#include "cppTest.h"

void Testcpp::createNewWindow() {
	int msgboxID = MessageBox(
		NULL,
		"hi, i'm cpp",
		"hi",
		MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
	);
	return;
}