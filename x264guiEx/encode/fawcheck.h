//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _FAWCHECK_H_
#define _FAWCHECK_H_

const int NON_FAW  = 0;
const int FAW_FULL = 1;
const int FAW_HALF = 2;
const int FAWCHECK_ERROR_TOO_SHORT = -1;
const int FAWCHECK_ERROR_OTHER     = -2;

int FAWCheck(short *audio_dat, int audio_n, int audio_rate, int audio_size); //FAWCheckを行い、判定結果を返す

#endif //_FAWCHECK_H_