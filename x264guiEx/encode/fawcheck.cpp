﻿//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <limits.h>
#include <Math.h>
#include <vector>

#include "fawcheck.h"
#include "auo_util.h"

const int    FAW_ZERO_FULL              = 0;
const int    FAW_ZERO_HALF              = SHRT_MIN;
const double ZERO_BLOCK_THRESHOLD_RATIO = 0.005;
const double FAW_ERROR_TOO_SHORT_RATIO  = 0.2; //音声サンプリングレートに対する割合
const int    ZERO_BLOCK_COUNT_THRESHOLD = 16; //ゼロブロックが最低秒間いくつあるか
const double ZERO_SUM_RATIO_MIN         = 0.5; //全体に対するゼロの数下限
const double ZERO_SUM_RATIO_MAX         = 0.99479; //全体に対するゼロの数上限
const double ZERO_SD_RATIO              = 0.25; //ゼロブロック内のゼロの平均数に対する標準偏差

//int		audio_rate;		//	音声サンプリングレート
//int		audio_ch;		//	音声チャンネル数
//int		audio_n;		//	音声サンプリング数
//int		audio_size;		//	音声１サンプルのバイト数

//音声データは16bitのみということで
int FAWCheck(short *audio_dat, int audio_n, int audio_rate, int audio_size) {
	std::vector<int> zero_blocks[2];
	int current_zero_blocks[2] = { 0, 0 };
	int ret = NON_FAW;
	int i;

	short *data = NULL;
	const int step = audio_size / sizeof(short);
	const short * const fin = audio_dat + audio_n * step;

	const int zero_block_threshold = (int)(audio_rate * ZERO_BLOCK_THRESHOLD_RATIO);

	//十分な音声があるかチェック
	if (audio_n < audio_rate * FAW_ERROR_TOO_SHORT_RATIO)
		return FAWCHECK_ERROR_TOO_SHORT;

	//ゼロブロックを数える
	for (data = audio_dat; data < fin; data += step) {
		switch (*data) {
			case FAW_ZERO_FULL:
				current_zero_blocks[0]++;
				break;
			case FAW_ZERO_HALF:
				current_zero_blocks[1]++;
				break;
			default:
				for (i = 0; i < 2; i++) {
					if (current_zero_blocks[i] >= zero_block_threshold)
						zero_blocks[i].push_back(current_zero_blocks[i]);
					current_zero_blocks[i] = 0;
				}
				break;
		}
	}

	//ゼロブロックをチェック
	for (i = 0; (i < 2 && ret == NON_FAW); i++) {
		if (zero_blocks[i].size() < (size_t)(audio_n * ZERO_BLOCK_COUNT_THRESHOLD / audio_rate))
			continue;
		int zero_sum = 0;
		foreach(std::vector<int>, it_zero_len, &zero_blocks[i])
			zero_sum += *it_zero_len;
		if (zero_sum < audio_n * ZERO_SUM_RATIO_MIN || zero_sum > audio_n * ZERO_SUM_RATIO_MAX)
			continue;
		double zero_avg = zero_sum / (double)(zero_blocks[i].size());
		double zero_sd = 0;
		foreach(std::vector<int>, it_zero_len, &zero_blocks[i])
			zero_sd += pow2(*it_zero_len - zero_avg);
		zero_sd = sqrt(zero_sd / (zero_blocks[i].size() - 1));
		if (zero_sd > zero_avg * ZERO_SD_RATIO)
			continue;
		//ここまで来たらFAW
		ret = FAW_FULL + i;
	}
	return ret;
}