// Copyright (c) 2014 The cefcapi authors. All rights reserved.
// License: BSD 3-clause.
// Website: https://github.com/CzarekTomczak/cefcapi

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "cef_base.h"
#include "cef_app.h"
#include "cef_client.h"
#include "winapi.h"

// TODO: call cef_quit_message_loop() when window is closed,
//       otherwise app is still hanging. You can do it in
//       OnBeforeClose() callback or implement WndProc::WM_CLOSE
//       by creating window on your own.

typedef int (*p_cef_string_utf8_to_utf16)(const char* src, size_t src_len,
	cef_string_utf16_t* output);
typedef int (*p_cef_browser_host_create_browser)(
	const cef_window_info_t* windowInfo, struct _cef_client_t* client,
	const cef_string_t* url, const struct _cef_browser_settings_t* settings,
	struct _cef_request_context_t* request_context);
typedef int (*p_cef_execute_process)(const struct _cef_main_args_t* args,
	cef_app_t* application, void* windows_sandbox_info);
typedef int (*p_cef_initialize)(const struct _cef_main_args_t* args,
	const struct _cef_settings_t* settings, cef_app_t* application,
	void* windows_sandbox_info);
typedef void (*p_cef_shutdown)();
typedef void (*p_cef_run_message_loop)();


int main(int argc, char** argv) {
	if (argc < 2)
	{
		printf("get url error!");
		return 0;
	}
	//get function from libcef.dll
	HANDLE hLibcef = LoadLibrary("libcef.dll");

	if (!hLibcef)
	{
		printf("can note load the libcef\n");
		return 0;
	}
	p_cef_string_utf8_to_utf16 cef_string_utf8_to_utf16 = (p_cef_string_utf8_to_utf16)GetProcAddress(hLibcef,"cef_string_utf8_to_utf16");
	p_cef_browser_host_create_browser cef_browser_host_create_browser = (p_cef_browser_host_create_browser)GetProcAddress(hLibcef, "cef_browser_host_create_browser");
	p_cef_execute_process cef_execute_process = (p_cef_execute_process)GetProcAddress(hLibcef, "cef_execute_process");
	p_cef_initialize cef_initialize = (p_cef_initialize)GetProcAddress(hLibcef, "cef_initialize");
	p_cef_shutdown cef_shutdown = (p_cef_shutdown)GetProcAddress(hLibcef, "cef_shutdown");
	p_cef_run_message_loop cef_run_message_loop = (p_cef_run_message_loop)GetProcAddress(hLibcef, "cef_run_message_loop");;

	if (!cef_string_utf8_to_utf16 || !cef_browser_host_create_browser || !cef_execute_process || !cef_initialize || !cef_shutdown || !cef_run_message_loop)
	{
		printf("get function error\n");
		return 0;
	}

	// Main args.
	cef_main_args_t mainArgs = {0};
	mainArgs.instance = GetModuleHandle(NULL);

	// Application handler and its callbacks.
	// cef_app_t structure must be filled. It must implement
	// reference counting. You cannot pass a structure 
	// initialized with zeroes.
	cef_app_t app = {0};
	initialize_app_handler(&app);

	// Execute subprocesses.
	printf("cef_execute_process, argc=%d\n", argc);
	int code = cef_execute_process(&mainArgs, &app, NULL);
	if (code >= 0) {
		_exit(code);
	}

	// Application settings.
	// It is mandatory to set the "size" member.
	cef_settings_t settings = {0};
	settings.size = sizeof(cef_settings_t);
	settings.no_sandbox = TRUE;

	// Initialize CEF.
	printf("cef_initialize\n");
	cef_initialize(&mainArgs, &settings, &app, NULL);

	// Create GTK window. You can pass a NULL handle 
	// to CEF and then it will create a window of its own.
	//initialize_gtk();
	//GtkWidget* hwnd = create_gtk_window("cefcapi example", 1024, 768);
	//cef_window_info_t windowInfo = {};
	//windowInfo.parent_widget = hwnd;

	cef_window_info_t windowInfo = {0};
	windowInfo.style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN \
		| WS_CLIPSIBLINGS | WS_VISIBLE;
	windowInfo.parent_window = NULL;
	windowInfo.x = CW_USEDEFAULT;
	windowInfo.y = CW_USEDEFAULT;
	windowInfo.width = CW_USEDEFAULT;
	windowInfo.height = CW_USEDEFAULT;

	// Initial url.
	char url[1024];
	snprintf(url, sizeof(url), argv[1]);
	printf("load url: %s\n", url);
	// There is no _cef_string_t type.
	cef_string_t cefUrl = {0};
	cef_string_utf8_to_utf16(url, strlen(url), &cefUrl);

	// Browser settings.
	// It is mandatory to set the "size" member.
	cef_browser_settings_t browserSettings = {0};
	browserSettings.size = sizeof(cef_browser_settings_t);

	// Client handler and its callbacks.
	// cef_client_t structure must be filled. It must implement
	// reference counting. You cannot pass a structure 
	// initialized with zeroes.
	cef_client_t client = {0};
	initialize_client_handler(&client);

	// Create browser.
	printf("cef_browser_host_create_browser\n");
	cef_browser_host_create_browser(&windowInfo, &client, &cefUrl,&browserSettings, NULL);

	// Message loop.
	printf("cef_run_message_loop\n");
	cef_run_message_loop();

	// Shutdown CEF.
	printf("cef_shutdown\n");
	cef_shutdown();

	return 0;
}
