#include "launcher.h"
#include "funkeymenu.h"
#include "debug.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef BIND_CONSOLE
#include <linux/vt.h>
#endif

using namespace std;


Launcher::Launcher(vector<string> const& commandLine, bool consoleApp)
	: commandLine(commandLine)
	, consoleApp(consoleApp)
{
}

Launcher::Launcher(vector<string> && commandLine, bool consoleApp)
	: commandLine(commandLine)
	, consoleApp(consoleApp)
{
}

void Launcher::exec()
{
	FILE *fp;

	/* Start audio amp */
	system(SHELL_CMD_AUDIO_AMP_ON);

	if (consoleApp) {
#ifdef BIND_CONSOLE
		/* Enable the framebuffer console */
		char c = '1';
		int fd = open("/sys/devices/virtual/vtconsole/vtcon1/bind", O_WRONLY);
		if (fd < 0) {
			WARNING("Unable to open fbcon handle\n");
		} else {
			write(fd, &c, 1);
			close(fd);
		}

		fd = open("/dev/tty1", O_RDWR);
		if (fd < 0) {
			WARNING("Unable to open tty1 handle\n");
		} else {
			if (ioctl(fd, VT_ACTIVATE, 1) < 0)
				WARNING("Unable to activate tty1\n");
			close(fd);
		}
#endif
	}

	INFO("Launching '");
	vector<const char *> args;
	args.reserve(commandLine.size() + 1);
	for (auto arg : commandLine) {
		args.push_back(strdup(arg.c_str()));
		INFO("%s ", args.back() );
	}
	args.push_back(nullptr);
	INFO("'\n");

	execvp(commandLine[0].c_str(), (char * const *)args.data());
	WARNING("Failed to exec '%s': %s\n",
			commandLine[0].c_str(), strerror(errno));
}
