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
	char buf[1024];
	while ((a = strchr(mes, '\n')) != NULL) {
		if ((b = strrchr(mes, '\r', a - mes - 2)) != NULL)
			mes = b + 1;
		*a = '\0';
		if (NULL == strstr(mes, "fps       kb/s     elapsed")) //x264 rev2207以降の進捗タイトル行を弾く
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
		//ウィンドウタイトルの整形
		const char *ptr_start = strchr(mes, ']');
		if (ptr_start == NULL) ptr_start = mes;
		int a_tmp, b_tmp, tm[3];
		double fps, bitrate;
		const char *ptr_remain = strrchr(ptr_start+1, ' ');
		const char *ptr_tmp = strchr(ptr_start+1, '/');
		if (ptr_remain && 
			6 == sscanf_s(1 + ((ptr_tmp) ? ptr_tmp : ptr_start), "%d %lf %lf %d:%d:%d", &b_tmp, &fps, &bitrate, &tm[0], &tm[1], &tm[2])) {
			//x264 rev2207以降の出力
			a_tmp = 0;
			if (ptr_tmp) {
				//通常時
				a_tmp = strtol(ptr_start+1, NULL, 0);
				a_tmp = (int)sprintf_s(buf, _countof(buf), "[%.1f%%] %d/", (100.0 * a_tmp) / b_tmp, a_tmp);
				ptr_tmp = ", remain";
			} else {
				//afs時
				ptr_remain = "";
				ptr_tmp = "";
			}
			a_tmp += sprintf_s(buf + a_tmp, _countof(buf) - a_tmp, "%d frames, %.2f fps, %.2f kb/s%s%s", b_tmp, fps, bitrate, ptr_tmp, ptr_remain);
			ptr_start = buf;
		} else {
			//そのまま出力
			ptr_start = mes;
		}
		set_window_title_x264_mes(ptr_start, total_drop, current_frames);
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