// -----------------------------------------------------------------------------------------
// x264guiEx/x265guiEx/svtAV1guiEx/ffmpegOut/QSVEnc/NVEnc/VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2022 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <emmintrin.h>
#include <chrono>

#include "output.h"
#include "auo.h"
#include "auo_version.h"
#include "auo_util.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"
#include "auo_frm.h"
#include "auo_error.h"
#include "auo_encode.h"
#include "auo_audio.h"
#include "auo_audio_parallel.h"
#include "auo_faw2aac.h"
#include "auo_mes.h"
#include "rgy_faw.h"

struct faw2aac_data_t {
    int id;
    char audfile[MAX_PATH_LEN];
    FILE *fp_out;
};

static size_t write_file(faw2aac_data_t *aud_dat, const PRM_ENC *pe, const void *buf, size_t size) {
    return _fwrite_nolock(buf, 1, size, aud_dat->fp_out);
}

AUO_RESULT audio_faw2aac(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;

    set_window_title(L"faw2aac", PROGRESSBAR_CONTINUOUS);
    write_log_auo_line_fmt(LOG_INFO, L"faw2aac %s", g_auo_mes.get(AUO_AUDIO_START_ENCODE));
    const int bufsize = sys_dat->exstg->s_local.audio_buffer_size;

    faw2aac_data_t aud_dat[2];
    //パイプ or ファイルオープン
    for (int i_aud = 0; !ret && i_aud < pe->aud_count; i_aud++) {
        // 初期化
        aud_dat[i_aud].id = i_aud;
        memset(aud_dat[i_aud].audfile, 0, sizeof(aud_dat[i_aud].audfile));
        aud_dat[i_aud].fp_out = nullptr;
    }

    //確実なfcloseのために何故か一度ここで待機する必要あり
    if_valid_set_event(pe->aud_parallel.he_vid_start);
    if_valid_wait_for_single_object(pe->aud_parallel.he_aud_start, INFINITE);

    //パイプ or ファイルオープン
    for (int i_aud = 0; !ret && i_aud < pe->aud_count; i_aud++) {
        const AUDIO_SETTINGS *aud_stg = &sys_dat->exstg->s_aud[conf->aud.encoder];
        strcpy_s(pe->append.aud[i_aud], _countof(pe->append.aud[i_aud]), aud_stg->aud_appendix); //pe一時パラメータにコピーしておく
        if (i_aud)
            insert_before_ext(pe->append.aud[i_aud], _countof(pe->append.aud[i_aud]), i_aud);
        get_aud_filename(aud_dat[i_aud].audfile, _countof(aud_dat[i_aud].audfile), pe, i_aud);
        if (fopen_s(&aud_dat[i_aud].fp_out, aud_dat[i_aud].audfile, "wbS")) {
            ret |= AUO_RESULT_ABORT;
            break;
        }
    }

    if (!ret) {
        const int elemsize = sizeof(short);
        const int wav_sample_size = oip->audio_ch * elemsize;

        RGYWAVHeader wavheader = { 0 };
        wavheader.file_size = 0;
        wavheader.subchunk_size = 16;
        wavheader.audio_format = 1;
        wavheader.number_of_channels = oip->audio_ch;
        wavheader.sample_rate = oip->audio_rate;
        wavheader.byte_rate = oip->audio_rate * oip->audio_ch * elemsize;
        wavheader.block_align = wav_sample_size;
        wavheader.bits_per_sample = elemsize * 8;
        wavheader.data_size = oip->audio_n * wavheader.number_of_channels * elemsize;

        RGYFAWDecoder fawdec;
        fawdec.init(&wavheader);

        RGYFAWDecoderOutput output;
        int samples_read = 0;
        int samples_get = bufsize;

        //wav出力ループ
        while (oip->audio_n - samples_read > 0 && samples_get) {
            //中断
            if ((pe->aud_parallel.he_aud_start) ? pe->aud_parallel.abort : oip->func_is_abort()) {
                ret |= AUO_RESULT_ABORT;
                break;
            }
            uint8_t *audio_dat = (uint8_t *)get_audio_data(oip, pe, samples_read, std::min(oip->audio_n - samples_read, bufsize), &samples_get);
            samples_read += samples_get;
            set_log_progress(samples_read / (double)oip->audio_n);

            fawdec.decode(output, audio_dat, samples_get * wav_sample_size);
            for (int i_aud = 0; i_aud < pe->aud_count; i_aud++) {
                if (output[i_aud].size() > 0) {
                    if (write_file(&aud_dat[i_aud], pe, output[i_aud].data(), output[i_aud].size()) == 0) {
                        ret |= AUO_RESULT_ABORT;
                        break;
                    }
                }
            }
        }

        fawdec.fin(output);
        for (int i_aud = 0; i_aud < pe->aud_count; i_aud++) {
            if (output[i_aud].size() > 0) {
                if (write_file(&aud_dat[i_aud], pe, output[i_aud].data(), output[i_aud].size()) == 0) {
                    ret |= AUO_RESULT_ABORT;
                    break;
                }
            }
        }

        //ファイルクローズ
        for (int i_aud = 0; i_aud < pe->aud_count; i_aud++) {
            if (aud_dat[i_aud].fp_out) {
                fclose(aud_dat[i_aud].fp_out);
                aud_dat[i_aud].fp_out = nullptr;
            }
        }
    }

    //動画との音声との同時処理が終了
    release_audio_parallel_events(pe);

    set_window_title(g_auo_mes.get(AUO_GUIEX_FULL_NAME), PROGRESSBAR_DISABLED);
    return ret;
}
