#include <windows.h>
#include <process.h>

int WINAPI WinMain(
  HINSTANCE hInstance,  // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,      // pointer to command line
  int nCmdShow          // show state of window
) 
{
	char *args[4], *prog;

	prog = "c:\\php4\\php.exe";
	args[0] = prog;
	args[1] = "-q";
	args[2] = lpCmdLine;
	args[3] = NULL;

	return _spawnvp(_P_DETACH, prog, args);
}