//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "auo.h"
#include "auo_util.h"
#include "auo_pipe.h"

//参考 : http://support.microsoft.com/kb/190351/ja
//参考 : http://www.autch.net/page/tips/win32_anonymous_pipe.html
//参考 : http://www.monzen.org/blogn/index.php?e=43&PHPSESSID=o1hmtphk82cd428g8p09tf84e6

void InitPipes(PIPE_SET *pipes) {
	ZeroMemory(pipes, sizeof(PIPE_SET));
}

static int StartPipes(PIPE_SET *pipes) {
	int ret = RP_USE_NO_PIPE;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	if (pipes->stdOut.enable) {
		if (!CreatePipe(&pipes->stdOut.h_read, &pipes->stdOut.h_write, &sa, pipes->stdOut.bufferSize) || 
			!SetHandleInformation(pipes->stdOut.h_read, HANDLE_FLAG_INHERIT, 0))
			return RP_ERROR_OPEN_PIPE;
		ret = RP_SUCCESS;
	}
	if (pipes->stdErr.enable) {
		if (!CreatePipe(&pipes->stdErr.h_read, &pipes->stdErr.h_write, &sa, pipes->stdErr.bufferSize) ||
			!SetHandleInformation(pipes->stdErr.h_read, HANDLE_FLAG_INHERIT, 0))
			return RP_ERROR_OPEN_PIPE;
		ret = RP_SUCCESS;
	}
	if (pipes->stdIn.enable) {
		if (!CreatePipe(&pipes->stdIn.h_read, &pipes->stdIn.h_write, &sa, pipes->stdIn.bufferSize) ||
			!SetHandleInformation(pipes->stdIn.h_write, HANDLE_FLAG_INHERIT, 0))
			return RP_ERROR_OPEN_PIPE;
		if ((pipes->f_stdin = _fdopen(_open_osfhandle((intptr_t)pipes->stdIn.h_write, _O_BINARY), "wb")) == NULL) {
			return RP_ERROR_GET_STDIN_FILE_HANDLE;
		}
		ret = RP_SUCCESS;
	}
	return ret;
}

int RunProcess(char *args, const char *exe_dir, PROCESS_INFORMATION *pi, PIPE_SET *pipes, DWORD priority, BOOL hidden, BOOL minimized) {
	BOOL Inherit = FALSE;
	DWORD flag = priority;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(pi, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);

	int ret = (pipes) ? StartPipes(pipes) : RP_USE_NO_PIPE;
	if (ret > RP_SUCCESS)
		return ret;

	if (ret == RP_SUCCESS) {
		if (pipes->stdOut.enable)
			si.hStdOutput = pipes->stdOut.h_write;
		if (pipes->stdErr.enable)
			si.hStdError = pipes->stdErr.h_write;
		if (pipes->stdIn.enable)
			si.hStdInput = pipes->stdIn.h_read;
		si.dwFlags |= STARTF_USESTDHANDLES;
		Inherit = TRUE;
		flag |= DETACHED_PROCESS;
	}
	if (minimized) {
		si.dwFlags |= STARTF_USESHOWWINDOW;
		si.wShowWindow |= SW_SHOWMINNOACTIVE;
	}
	if (hidden)
		flag |= CREATE_NO_WINDOW;

	if (!PathIsDirectory(exe_dir))
		exe_dir = NULL; //とりあえずカレントディレクトリで起動しとく

	ret = (CreateProcess(NULL, args, NULL, NULL, Inherit, flag, NULL, exe_dir, &si, pi)) ? RP_SUCCESS : RP_ERROR_CREATE_PROCESS;

	if (pipes) {
		if (pipes->stdOut.enable) {
			CloseHandle(pipes->stdOut.h_write);
			if (ret != RP_SUCCESS) {
				CloseHandle(pipes->stdOut.h_read);
				pipes->stdOut.enable = FALSE;
			}
		}
		if (pipes->stdErr.enable) {
			CloseHandle(pipes->stdErr.h_write);
			if (ret != RP_SUCCESS) {
				CloseHandle(pipes->stdErr.h_read);
				pipes->stdErr.enable = FALSE;
			}
		}
		if (pipes->stdIn.enable) {
			CloseHandle(pipes->stdIn.h_read);
			if (ret != RP_SUCCESS) {
				CloseHandle(pipes->stdIn.h_write);
				pipes->stdIn.enable = FALSE;
			}
		}
	}

	return ret;
}

void CloseStdIn(PIPE_SET *pipes) {
	if (pipes->stdIn.enable) {
		_fclose_nolock(pipes->f_stdin);
		CloseHandle(pipes->stdIn.h_write);
		pipes->stdIn.enable = FALSE;
	}
}

//PeekNamedPipeが失敗→プロセスが終了していたら-1
static int read_from_pipe(PIPE_SET *pipes, BOOL fromStdErr) {
	DWORD pipe_read = 0;
	HANDLE h_read = (fromStdErr) ? pipes->stdErr.h_read : pipes->stdOut.h_read;
	if (!PeekNamedPipe(h_read, NULL, 0, NULL, &pipe_read, NULL))
		return -1;
	if (pipe_read) {
		ReadFile(h_read, pipes->read_buf + pipes->buf_len, sizeof(pipes->read_buf) - pipes->buf_len - 1, &pipe_read, NULL);
		pipes->buf_len += pipe_read;
		pipes->read_buf[pipes->buf_len] = '\0';
	}
	return pipe_read;
}

BOOL get_exe_message(const char *exe_path, const char *args, char *buf, size_t nSize) {
	BOOL ret = FALSE;
	char exe_dir[MAX_PATH_LEN];
	size_t len = strlen(exe_path) + strlen(args) + 5;
	char *const fullargs = (char*)malloc(len);
	PROCESS_INFORMATION pi;
	PIPE_SET pipes;

	InitPipes(&pipes);
	pipes.stdErr.enable = TRUE;
	pipes.stdOut.enable = TRUE;

	strcpy_s(exe_dir, _countof(exe_dir), exe_path);
	PathRemoveFileSpecFixed(exe_dir);

	sprintf_s(fullargs, len, "\"%s\" %s", exe_path, args);
	if ((ret = RunProcess(fullargs, exe_dir, &pi, &pipes, NORMAL_PRIORITY_CLASS, TRUE, FALSE)) == RP_SUCCESS) {
		WaitForSingleObject(pi.hProcess, INFINITE);

		read_from_pipe(&pipes, TRUE);
		read_from_pipe(&pipes, FALSE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	free(fullargs);
	CloseHandle(pipes.stdErr.h_read);
	CloseHandle(pipes.stdOut.h_read);

	strcpy_s(buf, nSize, pipes.read_buf);

	return ret;
}
