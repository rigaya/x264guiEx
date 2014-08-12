//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include "auo_frm.h"
#include "auo_util.h"

static inline int check_log_type(char *mes) {
	if (strstr(mes, "warning")) return LOG_WARNING;
	if (strstr(mes, "error")) return LOG_ERROR;
	return LOG_INFO;
}

void write_log_x264_mes(char *const msg, DWORD *log_len, int total_drop, int current_frames) {
	char *a, *b, *mes = msg;
	char * const fin = mes + *log_len; //null文字の位置
	*fin = '\0';
	while ((a = strchr(mes, '\n')) != NULL) {
		if ((b = strrchr(mes, '\r', a - mes - 2)) != NULL)
			mes = b + 1;
		*a = '\0';
		write_log_line(check_log_type(mes), mes);
		mes = a + 1;
	}
	if ((a = strrchr(mes, '\r', fin - mes - 1)) != NULL) {
		b = a - 1;
		while (*b == ' ' || *b == '\r')
			b--;
		*(b+1) = '\0';
		if ((b = strrchr(mes, '\r', b - mes - 2)) != NULL)
			mes = b + 1;
		set_window_title_x264_mes(mes, total_drop, current_frames);
		mes = a + 1;
	}
	if (mes == msg && *log_len) {
		write_log_line(check_log_type(mes), mes);
		mes += strlen(mes);
	}
	memmove(msg, mes, ((*log_len = fin - mes) + 1) * sizeof(msg[0]));
}

void write_args(const char *args) {
	const int NEW_LINE_THRESHOLD = 110;
	size_t len = strlen(args);
	char *const c = (char *)malloc((len+1)*sizeof(c[0]));
	char *const fin = c + len;
	char *p = c;
	char *q;
	memcpy(c, args, (len+1)*sizeof(c[0]));
	while (p + NEW_LINE_THRESHOLD < fin && (q = strrchr(p, ' ', NEW_LINE_THRESHOLD)) != NULL) {
		*q = '\0';
		write_log_line(LOG_INFO, p);
		p = q+1;
	}
	write_log_line(LOG_INFO, p);
	free(c);
}
