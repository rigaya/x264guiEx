//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_PIPE_H_
#define _AUO_PIPE_H_

#include <Windows.h>
#include <stdio.h>
#include "auo_frm.h"

const int RP_USE_NO_PIPE = -1;
const int RP_SUCCESS = 0;
const int RP_ERROR_OPEN_PIPE = 1;
const int RP_ERROR_GET_STDIN_FILE_HANDLE = 2;
const int RP_ERROR_CREATE_PROCESS = 3;

const int PIPE_READ_BUF = 2048;

typedef struct {
	HANDLE h_read;
	HANDLE h_write;
	BOOL enable;
	DWORD bufferSize;
} PIPE;

typedef struct {
	PIPE stdIn;
	PIPE stdOut;
	PIPE stdErr;
	FILE *f_stdin;
	DWORD buf_len;
	char read_buf[PIPE_READ_BUF];
} PIPE_SET;

void InitPipes(PIPE_SET *pipes);
int RunProcess(char *args, const char *exe_dir, PROCESS_INFORMATION *pi, PIPE_SET *pipes, DWORD priority, BOOL hidden, BOOL minimized);
void CloseStdIn(PIPE_SET *pipes);
BOOL get_exe_message(const char *exe_path, const char *args, char *buf, size_t nSize);

#endif //_AUO_PIPE_H_