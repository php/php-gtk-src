#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>

int WINAPI WinMain(
	HINSTANCE hInstance,		// handle to current instance
	HINSTANCE hPrevInstance,	// handle to previous instance
	LPSTR lpCmdLine,			// pointer to command line
	int nCmdShow				// show state of window
) 
{
	char *args[4], *prog = NULL;
	char pathbuffer[_MAX_PATH];
	char searchfile[] = "php.exe";
	char envvar[] = "PATH";
	char *path = getenv("PATH");
	char newpath[2048];

	sprintf(newpath, "PATH=c:\\php4;d:\\php4;%s", path);
	putenv(newpath);

	if (strlen(lpCmdLine) == 0)
		MessageBox(NULL, "Usage: php_win <path to php-gtk script>\n", "Error", MB_OK);
	if (!(prog = getenv("PHP_BIN"))) {
		_searchenv( searchfile, envvar, pathbuffer );
		if (strlen(pathbuffer))
			prog = pathbuffer;
	}
	args[0] = prog;
	args[1] = "-q";
	args[2] = lpCmdLine;
	args[3] = NULL;

	if (prog)
		return _spawnvp(_P_DETACH, prog, args);
	else {
		MessageBox(NULL, "PHP.EXE was not found.", "Error", MB_OK);
		return -1;
	}
}