﻿// -----------------------------------------------------------------------------------------
// x264guiEx by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2016 rigaya
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

#include <Windows.h>
#pragma comment(lib, "user32.lib") //WaitforInputIdle
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <Process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") 
#include <limits.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <vector>
#include <set>

#include "output.h"
#include "vphelp_client.h"

#pragma warning( push )
#pragma warning( disable: 4127 )
#include "afs_client.h"
#pragma warning( pop )

#include "auo.h"
#include "auo_frm.h"
#include "auo_pipe.h"
#include "auo_error.h"
#include "auo_conf.h"
#include "auo_util.h"
#include "auo_convert.h"
#include "auo_system.h"
#include "auo_version.h"
#include "auo_chapter.h"

#include "auo_encode.h"
#include "auo_video.h"
#include "auo_audio_parallel.h"
#include "cpu_info.h"

const int DROP_FRAME_FLAG = INT_MAX;

typedef struct video_output_thread_t {
    CONVERT_CF_DATA *pixel_data;
    FILE *f_out;
    BOOL abort;
    HANDLE thread;
    HANDLE he_out_start;
    HANDLE he_out_fin;
    int repeat;
} video_output_thread_t;

static const char * specify_input_csp(int output_csp) {
    return specify_csp[output_csp];
}

int get_aviutl_color_format(int use_highbit, int output_csp, int input_as_lw48) {
    //Aviutlからの入力に使用するフォーマット

    const int cf_aviutl_pixel48 = (input_as_lw48) ? CF_LW48 : CF_YC48;
    switch (output_csp) {
        case OUT_CSP_YUV444:
            return cf_aviutl_pixel48;
        case OUT_CSP_RGB:
            return CF_RGB;
        case OUT_CSP_NV12:
        case OUT_CSP_NV16:
        case OUT_CSP_YUY2:
        default:
            return (use_highbit) ? cf_aviutl_pixel48 : CF_YUY2;
    }
}

static int calc_input_frame_size(int width, int height, int color_format) {
    width = (color_format == CF_RGB) ? (width+3) & ~3 : (width+1) & ~1;
    return width * height * COLORFORMATS[color_format].size;
}

BOOL setup_afsvideo(const OUTPUT_INFO *oip, const SYSTEM_DATA *sys_dat, CONF_GUIEX *conf, PRM_ENC *pe) {
    //すでに初期化してある または 必要ない
    if (pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED || !conf->vid.afs)
        return TRUE;

    const int color_format = get_aviutl_color_format(conf->x264.use_highbit_depth, conf->x264.output_csp, conf->vid.input_as_lw48);
    const int frame_size = calc_input_frame_size(oip->w, oip->h, color_format);
    //Aviutl(自動フィールドシフト)からの映像入力
    if (afs_vbuf_setup((OUTPUT_INFO *)oip, conf->vid.afs, frame_size, COLORFORMATS[color_format].FOURCC)) {
        pe->afs_init = TRUE;
        return TRUE;
    } else if (conf->vid.afs && sys_dat->exstg->s_local.auto_afs_disable) {
        afs_vbuf_release(); //一度解放
        warning_auto_afs_disable();
        conf->vid.afs = FALSE;
        //再度使用するmuxerをチェックする
        pe->muxer_to_be_used = check_muxer_to_be_used(conf, sys_dat, pe->temp_filename, pe->video_out_type, (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0);
        return TRUE;
    }
    //エラー
    error_afs_setup(conf->vid.afs, sys_dat->exstg->s_local.auto_afs_disable);
    return FALSE;
}

void close_afsvideo(PRM_ENC *pe) {
    if (!pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED)
        return;

    afs_vbuf_release();

    pe->afs_init = FALSE;
}

static AUO_RESULT check_cmdex(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    DWORD ret = AUO_RESULT_SUCCESS;
    const int color_format = get_aviutl_color_format(conf->x264.use_highbit_depth, conf->x264.output_csp, conf->vid.input_as_lw48); //現在の色形式を保存
    if (conf->oth.disable_guicmd) 
        get_default_conf_x264(&conf->x264, FALSE); //CLIモード時はとりあえず、デフォルトを呼んでおく
    //cmdexを適用
    set_cmd_to_conf(conf->vid.cmdex, &conf->x264);

    if (color_format != get_aviutl_color_format(conf->x264.use_highbit_depth, conf->x264.output_csp, conf->vid.input_as_lw48)) {
        //cmdexで入力色形式が変更になる場合、再初期化
        close_afsvideo(pe);
        if (!setup_afsvideo(oip, sys_dat, conf, pe)) {
            ret |= AUO_RESULT_ERROR; //Aviutl(afs)からのフレーム読み込みに失敗
        }
    }
    return ret;
}

static AUO_RESULT tcfile_out(int *jitter, int frame_n, double fps, BOOL afs, const PRM_ENC *pe) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    char auotcfile[MAX_PATH_LEN];
    FILE *tcfile = NULL;

    if (afs)
        fps *= 4; //afsなら4倍精度
    const double tm_multi = 1000.0 / fps;

    //ファイル名作成
    apply_appendix(auotcfile, _countof(auotcfile), pe->temp_filename, pe->append.tc);

    if (NULL != fopen_s(&tcfile, auotcfile, "wb")) {
        ret |= AUO_RESULT_ERROR; warning_auo_tcfile_failed();
    } else {
        fprintf(tcfile, "# timecode format v2\r\n");
        if (afs) {
            int time_additional_frame = 0;
            if (pe->delay_cut_additional_vframe) {
                const int multi_for_additional_vframe = 4 + !!fps_after_afs_is_24fps(frame_n, pe);
                for (int i = 0; i < pe->delay_cut_additional_vframe; i++)
                    fprintf(tcfile, "%.6lf\r\n", i * multi_for_additional_vframe * tm_multi);

                time_additional_frame = pe->delay_cut_additional_vframe * multi_for_additional_vframe;
            }
            for (int i = 0; i < frame_n; i++)
                if (jitter[i] != DROP_FRAME_FLAG)
                    fprintf(tcfile, "%.6lf\r\n", (i * 4 + jitter[i] + time_additional_frame) * tm_multi);
        } else {
            frame_n += pe->delay_cut_additional_vframe;
            for (int i = 0; i < frame_n; i++)
                fprintf(tcfile, "%.6lf\r\n", i * tm_multi);
        }
        fclose(tcfile);
    }
    return ret;
}

static AUO_RESULT set_keyframe_from_aviutl(std::vector<int> *keyframe_list, const OUTPUT_INFO *oip) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    const int prev_chap_count = keyframe_list->size();
    const char * const MES_SEARCH_KEYFRAME = "Aviutl キーフレーム検出中…";
    DWORD tm = 0, tm_prev = 0;
    set_window_title(MES_SEARCH_KEYFRAME, PROGRESSBAR_CONTINUOUS);

    //検出ループ
    for (int i = 0; i < oip->n; i++) {
        //中断
        if (oip->func_is_abort()) {
            ret |= AUO_RESULT_ABORT; write_log_auo_line(LOG_INFO, "Aviutl キーフレーム検出を中断しました。");
            break;
        }
        //フラグ検出
        if (oip->func_get_flag(i) & OUTPUT_INFO_FRAME_FLAG_KEYFRAME)
            keyframe_list->push_back(i);
        //進捗表示 (自動24fps化などしていると時間がかかる)
        if ((tm = timeGetTime()) - tm_prev > LOG_UPDATE_INTERVAL * 5) {
            set_log_progress(i / (double)oip->n);
            tm_prev = tm;
        }
    }
    set_window_title(MES_SEARCH_KEYFRAME, PROGRESSBAR_DISABLED);
    write_log_auo_line_fmt(LOG_INFO, "Aviutlから %d箇所 キーフレーム設定を検出しました。", keyframe_list->size() - prev_chap_count);
    return ret;
}

static AUO_RESULT set_keyframe_from_chapter(std::vector<int> *keyframe_list, const CONF_GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    //mux設定がなければスキップ
    if (pe->muxer_to_be_used == MUXER_DISABLED) {
        //スキップ
        write_log_auo_line(LOG_INFO, "使用するmuxerが設定されていないため、チャプターからのキーフレーム検出は行いません。");
    } else {
        //チャプターファイル名作成
        char chap_file[MAX_PATH_LEN] = { 0 };
        const MUXER_SETTINGS *mux_stg = &sys_dat->exstg->s_mux[(pe->muxer_to_be_used == MUXER_TC2MP4) ? MUXER_MP4 : pe->muxer_to_be_used];
        const MUXER_CMD_EX *muxer_mode = &mux_stg->ex_cmd[get_mux_excmd_mode(conf, pe)];
        strcpy_s(chap_file, _countof(chap_file), muxer_mode->chap_file);
        cmd_replace(chap_file, _countof(chap_file), pe, sys_dat, conf, oip);

        chapter_file chapter;
        if (!str_has_char(chap_file) || !PathFileExists(chap_file)) {
            write_log_auo_line(LOG_INFO, "チャプターファイルが存在しません。");
        //チャプターリストを取得
        } else if (AUO_CHAP_ERR_NONE != chapter.read_file(chap_file, CODE_PAGE_UNSET, 0.0)) {
            ret |= AUO_RESULT_ERROR; write_log_auo_line(LOG_WARNING, "チャプターファイルからチャプター設定を読み取れませんでした。");
        //チャプターがない場合
        } else if (0 == chapter.chapters.size()) {
            write_log_auo_line(LOG_WARNING, "チャプターファイルからチャプター設定を読み取れませんでした。");
        } else {
            const double fps = oip->rate / (double)oip->scale;
            //QPファイルを出力
            for (const auto& chap : chapter.chapters) {
                double chap_time_s = chap->get_ms() / 1000.0;
                int i_frame = (int)(chap_time_s * fps + 0.5);
                keyframe_list->push_back(i_frame);
            }
            write_log_auo_line_fmt(LOG_INFO, "チャプターファイルから %d箇所 キーフレーム設定を行いました。", chapter.chapters.size());
        }
    }
    return ret;
}

//自動フィールドシフトが、動きを検出できない部分では正しく設定できないこともある
static AUO_RESULT adjust_keyframe_as_afs_24fps(std::vector<int> &keyframe_list, const std::set<int> &keyframe_set, const OUTPUT_INFO *oip) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
#if 0 //デバッグ用
    {
        const char * const MES_AFS_DEBUG = "Aviutl afs検出中…";
        set_window_title(MES_AFS_DEBUG, PROGRESSBAR_CONTINUOUS);
        DWORD tm = 0, tm_prev = 0;
        char afs_csv_file[MAX_PATH_LEN] = { 0 };
        strcpy_s(afs_csv_file, _countof(afs_csv_file), oip->savefile);
        change_ext(afs_csv_file, _countof(afs_csv_file), "x264guiEx_afs_log.csv");
        FILE *fp_log = NULL;
        if (fopen_s(&fp_log, afs_csv_file, "wb") || fp_log == NULL) {
            write_log_auo_line(LOG_WARNING, "x264guiEx_afsログファイルを開けませんでした。");
        } else {
            int drop = FALSE, next_jitter = 0;
            for (int i = 0; i < oip->n; i++) {
                afs_get_video((OUTPUT_INFO *)oip, i, &drop, &next_jitter);
                fprintf(fp_log, "%d\r\n%d,%d,", drop, next_jitter, 4*(i+1) + next_jitter);
                //進捗表示 (自動24fps化などしていると時間がかかる)
                if ((tm = timeGetTime()) - tm_prev > LOG_UPDATE_INTERVAL * 5) {
                    set_log_progress(i / (double)oip->n);
                    log_process_events();
                    tm_prev = tm;
                }
            }
            fclose(fp_log);
        }
        set_window_title(MES_AFS_DEBUG, PROGRESSBAR_DISABLED);
    }
#endif
    //24fps化を仮定して設定し直す
    keyframe_list.clear();
    const char * const MES_CHAPTER_AFS_ADJUST = "チャプター 補正計算中(afs 24fps化)...";
    set_window_title(MES_CHAPTER_AFS_ADJUST, PROGRESSBAR_CONTINUOUS);

    int last_chapter = 0;
    for (auto keyframe : keyframe_set) {
        DWORD tm = 0, tm_prev = 0;
        const int check_start = (std::max)(0, ((keyframe - 300) / 5) * 5);
        int drop_count = 0;
        for (int i_frame = check_start, drop = FALSE, next_jitter = 0; i_frame < (std::min)(keyframe, oip->n); i_frame++) {
            afs_get_video((OUTPUT_INFO *)oip, i_frame, &drop, &next_jitter);
            drop_count += !!drop;
            //中断
            if (oip->func_is_abort()) {
                ret |= AUO_RESULT_ABORT; write_log_auo_line(LOG_INFO, "Aviutl キーフレーム検出を中断しました。");
                break;
            }
            //進捗表示
            if ((tm = timeGetTime()) - tm_prev > LOG_UPDATE_INTERVAL * 5) {
                double progress_current_chapter = (i_frame - check_start) / (double)(keyframe - check_start);
                set_log_progress((last_chapter + progress_current_chapter * (keyframe - last_chapter)) / (double)oip->n);
                log_process_events();
                tm_prev = tm;
            }
        }
        last_chapter = keyframe;

        keyframe_list.push_back(4*check_start/5 + (keyframe - check_start) - drop_count);
    }
    set_window_title(MES_CHAPTER_AFS_ADJUST, PROGRESSBAR_DISABLED);
    write_log_auo_line(LOG_INFO, "チャプター 補正計算(afs 24fps化)が完了しました。");
    return ret;
}

static AUO_RESULT set_keyframe(const CONF_GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    std::vector<int> keyframe_list;

    //自動フィールドシフトならスキップ
    BOOL disable_keyframe_afs = conf->vid.afs && !sys_dat->exstg->s_local.set_keyframe_as_afs_24fps;
    if (disable_keyframe_afs)
        return ret;

    //ファイル名作成
    char auoqpfile[MAX_PATH_LEN] = { 0 };
    apply_appendix(auoqpfile, _countof(auoqpfile), pe->temp_filename, pe->append.qp);
    if (PathFileExists(auoqpfile))
        remove(auoqpfile);

    //Aviutlのキーフレーム検出からキーフレーム設定
    if (!ret && conf->vid.check_keyframe & CHECK_KEYFRAME_AVIUTL)
        ret |= set_keyframe_from_aviutl(&keyframe_list, oip);

    //チャプターからキーフレーム設定
    if (!ret && conf->vid.check_keyframe & CHECK_KEYFRAME_CHAPTER)
        ret |= set_keyframe_from_chapter(&keyframe_list, conf, oip, pe, sys_dat);

    if (ret) {
        //エラーないし中断
    } else if (!keyframe_list.size()) {
        write_log_auo_line(LOG_INFO, "キーフレーム探索を行いましたが、キーフレーム設定を検出できませんでした。");
    } else {
        //重複要素削除 + ソートを自動でやってくれる
        std::set<int> keyframe_set(keyframe_list.begin(), keyframe_list.end());

        if (!conf->vid.afs) {
            keyframe_list.assign(keyframe_set.begin(), keyframe_set.end());
        } else {
            ret |= adjust_keyframe_as_afs_24fps(keyframe_list, keyframe_set, oip);
        }

        if (!ret) {
            FILE *qpfile = NULL;
            if (NULL != fopen_s(&qpfile, auoqpfile, "wb")) {
                ret |= AUO_RESULT_ERROR; warning_auto_qpfile_failed();
            } else {
                //出力
                for (auto i_keyframe : keyframe_list)
                    fprintf(qpfile, "%d I\r\n", (i_keyframe) ? i_keyframe + pe->delay_cut_additional_vframe : 0);
                fclose(qpfile);
            }
        }
    }
    return ret;
}

static AUO_RESULT write_log_x264_version(const char *x264fullpath) {
    AUO_RESULT ret = AUO_RESULT_WARNING;
    static const int REQUIRED_X264_CORE = 104;
    static const int REQUIRED_X264_REV = 1673;
    char buffer[2048] = { 0 };
    if (get_exe_message(x264fullpath, "--version", buffer, _countof(buffer), AUO_PIPE_MUXED) == RP_SUCCESS) {
        char print_line[512] = { 0 };
        const char *LINE_HEADER = "x264 version: ";
        const char *LINE_CONFIGURATION = "configuration:";
        BOOL line_configuration = FALSE; //現在取得中かどうか
        BOOL got_line_configuration = FALSE; //一度取得したかどうか
        int configuration_pos = 0; //取得開始の文字の位置
        sprintf_s(print_line, _countof(print_line), LINE_HEADER);
        int a = -1, b = -1, c = -1;
        for (char *ptr = buffer, *qtr = NULL; NULL != (ptr = strtok_s(ptr, "\r\n", &qtr)); ) {
            if (ptr == buffer) {
                strcat_s(print_line, _countof(print_line), ptr + strlen("x264 ") * (NULL == strncmp(ptr, "x264 ", strlen("x264 "))));
                if (3 != sscanf_s(ptr, "x264 %d.%d.%d", &a, &b, &c))
                    a = b = c = -1;
            } else if (strstr(ptr, LINE_CONFIGURATION) && !got_line_configuration) {
                const char *rtr = strstr(ptr, LINE_CONFIGURATION) + strlen(LINE_CONFIGURATION);
                while (*rtr == ' ') rtr++;
                strcat_s(print_line, _countof(print_line), " ");
                strcat_s(print_line, _countof(print_line), rtr);
                configuration_pos = rtr - ptr;
                line_configuration = TRUE;
                got_line_configuration = TRUE;
            } else if (line_configuration) {
                const char *rtr = ptr;
                while (*rtr == ' ') rtr++;
                if ((int)(rtr - ptr) == configuration_pos) {
                    strcat_s(print_line, _countof(print_line), " ");
                    strcat_s(print_line, _countof(print_line), rtr);
                } else {
                    line_configuration = FALSE;
                }
            } else {
                line_configuration = FALSE;
            }
            ptr = NULL;
        }
        if (strlen(print_line) > strlen(LINE_HEADER)) {
            write_log_auo_line(LOG_INFO, print_line);
        }
        if (a >= 0 && b >= 0 && c >= 0) {
            ret = (b >= REQUIRED_X264_CORE || c >= REQUIRED_X264_REV) ? AUO_RESULT_SUCCESS : AUO_RESULT_ERROR;
        }
        if (ret & AUO_RESULT_ERROR) {
            char required_ver[128] = { 0 };
            char current_ver[128] = { 0 };
            sprintf_s(required_ver, _countof(required_ver), "Core:%4d, Rev: %4d", REQUIRED_X264_CORE, REQUIRED_X264_REV);
            sprintf_s(current_ver,  _countof(current_ver),  "Core:%4d, Rev: %4d", b, c);
            error_x264_version(required_ver, current_ver);
        }
    }
    return ret;
}

//auo_pipe.cppのread_from_pipeの特別版
static int ReadLogEnc(PIPE_SET *pipes, int total_drop, int current_frames) {
    DWORD pipe_read = 0;
    if (!PeekNamedPipe(pipes->stdErr.h_read, NULL, 0, NULL, &pipe_read, NULL))
        return -1;
    if (pipe_read) {
        ReadFile(pipes->stdErr.h_read, pipes->read_buf + pipes->buf_len, sizeof(pipes->read_buf) - pipes->buf_len - 1, &pipe_read, NULL);
        pipes->buf_len += pipe_read;
        write_log_enc_mes(pipes->read_buf, &pipes->buf_len, total_drop, current_frames);
    } else {
        log_process_events();
    }
    return pipe_read;
}

//cmdexのうち、guiから発行されるオプションとの衝突をチェックして、読み取られなかったコマンドを追加する
static void append_cmdex(char *cmd, size_t nSize, const char *cmdex, BOOL disble_guicmd, const CONF_GUIEX *conf) {
    const size_t cmd_len = strlen(cmd);
    const size_t cmdex_len = strlen(cmdex);

    //CLIモードなら常にチェックをスキップ
    BOOL skip_check_if_imported = disble_guicmd;
    //--presetなどの特殊なオプションがあったらチェックをスキップ
    const char * const IRREGULAR_OPTIONS[] = { "--preset", "--tune", "--profile", NULL };
    for (int i = 0; IRREGULAR_OPTIONS[i] && !skip_check_if_imported; i++)
        skip_check_if_imported = (NULL != strstr(cmdex, IRREGULAR_OPTIONS[i]));

    sprintf_s(cmd + cmd_len, nSize - cmd_len, " %s", cmdex);

    if (skip_check_if_imported) {
        //改行のチェックのみ行う
        replace_cmd_CRLF_to_Space(cmd + cmd_len + 1, nSize - cmd_len - 1);
    } else {
        CONF_GUIEX cnf = *conf;
        //confに読み込ませ、読み取られなかった部分のみを得る
        set_cmd_to_conf(cmd + cmd_len + 1, &cnf.x264, cmdex_len, TRUE);
    }
}

static void build_full_cmd(char *cmd, size_t nSize, const CONF_GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *input) {
    CONF_GUIEX prm;
    //パラメータをコピー
    memcpy(&prm, conf, sizeof(CONF_GUIEX));
    //共通置換を実行
    cmd_replace(prm.vid.cmdex,     sizeof(prm.vid.cmdex),     pe, sys_dat, conf, oip);
    cmd_replace(prm.vid.stats,     sizeof(prm.vid.stats),     pe, sys_dat, conf, oip);
    cmd_replace(prm.vid.tcfile_in, sizeof(prm.vid.tcfile_in), pe, sys_dat, conf, oip);
    cmd_replace(prm.vid.cqmfile,   sizeof(prm.vid.cqmfile),   pe, sys_dat, conf, oip);
    if (!prm.oth.disable_guicmd) {
        //cliモードでない
        //自動設定の適用
        apply_guiEx_auto_settings(&prm.x264, oip->w, oip->h, oip->rate, oip->scale, sys_dat->exstg->s_local.auto_ref_limit_by_level);
        //GUI部のコマンドライン生成
        build_cmd_from_conf(cmd, nSize, &prm.x264, &prm.vid, FALSE);
    }
    //cmdexのうち、読み取られなかったコマンドを追加する
    if (str_has_char(prm.vid.cmdex))
        append_cmdex(cmd, nSize, prm.vid.cmdex, prm.oth.disable_guicmd, conf);
    //メッセージの発行
    if ((conf->x264.vbv_bufsize != 0 || conf->x264.vbv_maxrate != 0) && prm.vid.afs)
        write_log_auo_line(LOG_INFO, "自動フィールドシフト使用時はvbv設定は正確に反映されません。");
    //キーフレーム検出を行い、そのQPファイルが存在し、かつ--qpfileの指定がなければ、それをqpfileで読み込む
    char auoqpfile[MAX_PATH_LEN];
    apply_appendix(auoqpfile, _countof(auoqpfile), pe->temp_filename, pe->append.qp);
    BOOL disable_keyframe_afs = conf->vid.afs && !sys_dat->exstg->s_local.set_keyframe_as_afs_24fps;
    if (prm.vid.check_keyframe && !disable_keyframe_afs && PathFileExists(auoqpfile) && strstr(cmd, "--qpfile") == NULL)
        sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --qpfile \"%s\"", auoqpfile);
    //1pass目でafsでない、--framesがなければ--framesを指定
    if ((!prm.vid.afs || pe->current_x264_pass > 1) && strstr(cmd, "--frames") == NULL)
        sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --frames %d", oip->n - pe->drop_count + pe->delay_cut_additional_vframe);
    //解像度情報追加(--input-res)
    if (strcmp(input, PIPE_FN) == NULL)
        sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --input-res %dx%d", oip->w, oip->h);
    //rawの形式情報追加
    sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --input-csp %s", specify_input_csp(prm.x264.output_csp));
    //fps//tcfile-inが指定されていた場合、fpsの自動付加を停止]
    if (!prm.x264.use_tcfilein && strstr(cmd, "--tcfile-in") == NULL) {
        int gcd = get_gcd(oip->rate, oip->scale);
        sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --fps %d/%d", oip->rate / gcd, oip->scale / gcd);
    }
    //出力ファイル
    const char * const outfile = (prm.x264.nul_out) ? "nul" : pe->temp_filename;
    sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " -o \"%s\"", outfile);
    //入力
    sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " \"%s\"", input);
}

static void set_pixel_data(CONVERT_CF_DATA *pixel_data, const CONF_GUIEX *conf, int w, int h) {
    const int byte_per_pixel = (conf->x264.use_highbit_depth) ? sizeof(short) : sizeof(BYTE);
    ZeroMemory(pixel_data, sizeof(CONVERT_CF_DATA));
    switch (conf->x264.output_csp) {
        case OUT_CSP_NV16: //nv16 (YUV422)
            pixel_data->count = 2;
            pixel_data->size[0] = w * h * byte_per_pixel;
            pixel_data->size[1] = pixel_data->size[0];
            break;
        case OUT_CSP_YUY2: //yuy2 (YUV422)
            pixel_data->count = 1;
            pixel_data->size[0] = w * h * byte_per_pixel * 2;
            break;
        case OUT_CSP_YUV444: //i444 (YUV444 planar)
            pixel_data->count = 3;
            pixel_data->size[0] = w * h * byte_per_pixel;
            pixel_data->size[1] = pixel_data->size[0];
            pixel_data->size[2] = pixel_data->size[0];
            break;
        case OUT_CSP_RGB: //RGB packed
            pixel_data->count = 1;
            pixel_data->size[0] = w * h * 3 * sizeof(BYTE); //8bit only
            break;
        case OUT_CSP_NV12: //nv12 (YUV420)
        default:
            pixel_data->count = 2;
            pixel_data->size[0] = w * h * byte_per_pixel;
            pixel_data->size[1] = pixel_data->size[0] / 2;
            break;
    }
    //サイズの総和計算
    for (int i = 0; i < pixel_data->count; i++)
        pixel_data->total_size += pixel_data->size[i];
}

static inline void check_enc_priority(HANDLE h_aviutl, HANDLE h_x264, DWORD priority) {
    if (priority == AVIUTLSYNC_PRIORITY_CLASS)
        priority = GetPriorityClass(h_aviutl);
    SetPriorityClass(h_x264, priority);
}

//並列処理時に音声データを取得する
static AUO_RESULT aud_parallel_task(const OUTPUT_INFO *oip, PRM_ENC *pe) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    AUD_PARALLEL_ENC *aud_p = &pe->aud_parallel; //長いんで省略したいだけ
    if (aud_p->th_aud) {
        //---   排他ブロック 開始  ---> 音声スレッドが止まっていなければならない
        if_valid_wait_for_single_object(aud_p->he_vid_start, INFINITE);
        if (aud_p->he_vid_start && aud_p->get_length) {
            DWORD required_buf_size = aud_p->get_length * (DWORD)oip->audio_size;
            if (aud_p->buf_max_size < required_buf_size) {
                //メモリ不足なら再確保
                if (aud_p->buffer) free(aud_p->buffer);
                aud_p->buf_max_size = required_buf_size;
                if (NULL == (aud_p->buffer = malloc(aud_p->buf_max_size)))
                    aud_p->buf_max_size = 0; //ここのmallocエラーは次の分岐でAUO_RESULT_ERRORに設定
            }
            void *data_ptr = NULL;
            if (NULL == aud_p->buffer || 
                NULL == (data_ptr = oip->func_get_audio(aud_p->start, aud_p->get_length, &aud_p->get_length))) {
                ret = AUO_RESULT_ERROR; //mallocエラーかget_audioのエラー
            } else {
                //自前のバッファにコピーしてdata_ptrが破棄されても良いようにする
                memcpy(aud_p->buffer, data_ptr, aud_p->get_length * oip->audio_size);
            }
            //すでにTRUEなら変更しないようにする
            aud_p->abort |= oip->func_is_abort();
        }
        flush_audio_log();
        if_valid_set_event(aud_p->he_aud_start);
        //---   排他ブロック 終了  ---> 音声スレッドを開始
    }
    return ret;
}

//音声処理をどんどん回して終了させる
static AUO_RESULT finish_aud_parallel_task(const OUTPUT_INFO *oip, PRM_ENC *pe, AUO_RESULT vid_ret) {
    //エラーが発生していたら音声出力ループをとめる
    pe->aud_parallel.abort |= (vid_ret != AUO_RESULT_SUCCESS);
    if (pe->aud_parallel.th_aud) {
        for (int wait_for_audio_count = 0; pe->aud_parallel.he_vid_start; wait_for_audio_count++) {
            vid_ret |= aud_parallel_task(oip, pe);
            if (wait_for_audio_count == 5)
                write_log_auo_line(LOG_INFO, "音声処理の終了を待機しています...");
        }
    }
    return vid_ret;
}

//並列処理スレッドの終了を待ち、終了コードを回収する
static AUO_RESULT exit_audio_parallel_control(const OUTPUT_INFO *oip, PRM_ENC *pe, AUO_RESULT vid_ret) {
    vid_ret |= finish_aud_parallel_task(oip, pe, vid_ret); //wav出力を完了させる
    release_audio_parallel_events(pe);
    if (pe->aud_parallel.buffer) free(pe->aud_parallel.buffer);
    if (pe->aud_parallel.th_aud) {
        //音声エンコードを完了させる
        //2passエンコードとかだと音声エンコーダの終了を待機する必要あり
        int wait_for_audio_count = 0;
        while (WaitForSingleObject(pe->aud_parallel.th_aud, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT) {
            if (wait_for_audio_count == 10)
                set_window_title("音声処理の終了を待機しています...", PROGRESSBAR_MARQUEE);
            pe->aud_parallel.abort |= oip->func_is_abort();
            log_process_events();
            wait_for_audio_count++;
        }
        flush_audio_log();
        if (wait_for_audio_count > 10)
            set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

        DWORD exit_code = 0;
        //GetExitCodeThreadの返り値がNULLならエラー
        vid_ret |= (NULL == GetExitCodeThread(pe->aud_parallel.th_aud, &exit_code)) ? AUO_RESULT_ERROR : exit_code;
        CloseHandle(pe->aud_parallel.th_aud);
    }
    //初期化 (重要!!!)
    ZeroMemory(&pe->aud_parallel, sizeof(pe->aud_parallel));
    return vid_ret;
}

static UINT64 get_amp_filesize_limit(const CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    UINT64 filesize_limit = MAXUINT64;
    if (conf->x264.use_auto_npass) {
        //上限ファイルサイズのチェック
        if (conf->vid.amp_check & AMPLIMIT_FILE_SIZE) {
            filesize_limit = min(filesize_limit, (UINT64)(conf->vid.amp_limit_file_size*1024*1024));
        }
        //上限ビットレートのチェック
        if (conf->vid.amp_check & AMPLIMIT_BITRATE_UPPER) {
            const double duration = get_duration(conf, sys_dat, pe, oip);
            filesize_limit = min(filesize_limit, (UINT64)(conf->vid.amp_limit_bitrate_upper * 1000 / 8 * duration));
        }
    }
    //音声のサイズはここでは考慮しない
    //音声のサイズも考慮してしまうと、のちの判定で面倒なことになる
    //(muxをしないファイルを評価してしまい、上限をクリアしていると判定されてしまう)
    return (MAXUINT64 == filesize_limit) ? 0 : filesize_limit;
}

static unsigned __stdcall video_output_thread_func(void *prm) {
    video_output_thread_t *thread_data = reinterpret_cast<video_output_thread_t *>(prm);
    CONVERT_CF_DATA *pixel_data = thread_data->pixel_data;
    WaitForSingleObject(thread_data->he_out_start, INFINITE);
    while (false == thread_data->abort) {
        //映像データをパイプに
        for (int i = 0; i < 1 + thread_data->repeat; i++)
            for (int j = 0; j < pixel_data->count; j++)
                _fwrite_nolock((void *)pixel_data->data[j], 1, pixel_data->size[j], thread_data->f_out);

        thread_data->repeat = 0;
        SetEvent(thread_data->he_out_fin);
        WaitForSingleObject(thread_data->he_out_start, INFINITE);
    }
    return 0;
}

static int video_output_create_thread(video_output_thread_t *thread_data, CONVERT_CF_DATA *pixel_data, FILE *pipe_stdin) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    thread_data->abort = false;
    thread_data->pixel_data = pixel_data;
    thread_data->f_out = pipe_stdin;
    if (   NULL == (thread_data->he_out_start = (HANDLE)CreateEvent(NULL, false, false, NULL))
        || NULL == (thread_data->he_out_fin   = (HANDLE)CreateEvent(NULL, false, true,  NULL))
        || NULL == (thread_data->thread       = (HANDLE)_beginthreadex(NULL, 0, video_output_thread_func, thread_data, 0, NULL))) {
        ret = AUO_RESULT_ERROR;
    }
    return ret;
}

static void video_output_close_thread(video_output_thread_t *thread_data, AUO_RESULT ret) {
    if (thread_data->thread) {
        if (!ret)
            while (WAIT_TIMEOUT == WaitForSingleObject(thread_data->he_out_fin, LOG_UPDATE_INTERVAL))
                log_process_events();
        thread_data->abort = true;
        SetEvent(thread_data->he_out_start);
        WaitForSingleObject(thread_data->thread, INFINITE);
        CloseHandle(thread_data->thread);
        CloseHandle(thread_data->he_out_start);
        CloseHandle(thread_data->he_out_fin);
    }
    memset(thread_data, 0, sizeof(thread_data[0]));
}

static AUO_RESULT x264_out(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    PIPE_SET pipes = { 0 };
    PROCESS_INFORMATION pi_enc = { 0 };

    char x264cmd[MAX_CMD_LEN]  = { 0 };
    char x264args[MAX_CMD_LEN] = { 0 };
    char x264dir[MAX_PATH_LEN] = { 0 };
    char *x264fullpath = (conf->x264.use_highbit_depth) ? sys_dat->exstg->s_x264.fullpath_highbit : sys_dat->exstg->s_x264.fullpath;
    
    const BOOL afs = conf->vid.afs != 0;
    CONVERT_CF_DATA pixel_data = { 0 };
    video_output_thread_t thread_data = { 0 };
    thread_data.repeat = pe->delay_cut_additional_vframe;
    set_pixel_data(&pixel_data, conf, oip->w, oip->h);
    
    int *jitter = NULL;
    int rp_ret = 0;

    //x264優先度関連の初期化
    DWORD set_priority = (pe->h_p_aviutl || conf->vid.priority != AVIUTLSYNC_PRIORITY_CLASS) ? priority_table[conf->vid.priority].value : NORMAL_PRIORITY_CLASS;

    //プロセス用情報準備
    if (!PathFileExists(x264fullpath)) {
        ret |= AUO_RESULT_ERROR; error_no_exe_file("x264", x264fullpath);
        return ret;
    }
    PathGetDirectory(x264dir, _countof(x264dir), x264fullpath);

    //YUY2/YC48->NV12/YUV444, RGBコピー用関数
    const int input_csp_idx = get_aviutl_color_format(conf->x264.use_highbit_depth, conf->x264.output_csp, conf->vid.input_as_lw48);
    const func_convert_frame convert_frame = get_convert_func(oip->w, input_csp_idx, (conf->x264.use_highbit_depth) ? 16 : 8, conf->x264.interlaced, conf->x264.output_csp);
    if (convert_frame == NULL) {
        ret |= AUO_RESULT_ERROR; error_select_convert_func(oip->w, oip->h, conf->x264.use_highbit_depth, conf->x264.interlaced, conf->x264.output_csp);
        return ret;
    }
    //映像バッファ用メモリ確保
    if (!malloc_pixel_data(&pixel_data, oip->w, oip->h, conf->x264.output_csp, (conf->x264.use_highbit_depth) ? 16 : 8)) {
        ret |= AUO_RESULT_ERROR; error_malloc_pixel_data();
        return ret;
    }

    //パイプの設定
    pipes.stdIn.mode = AUO_PIPE_ENABLE;
    pipes.stdErr.mode = AUO_PIPE_ENABLE;
    pipes.stdIn.bufferSize = pixel_data.total_size * 2;

    //x264バージョン情報表示・チェック
    if (AUO_RESULT_ERROR == write_log_x264_version(x264fullpath)) {
        return (ret | AUO_RESULT_ERROR);
    }

    //コマンドライン生成
    build_full_cmd(x264cmd, _countof(x264cmd), conf, oip, pe, sys_dat, PIPE_FN);
    write_log_auo_line(LOG_INFO, "x264 options...");
    write_args(x264cmd);
    sprintf_s(x264args, _countof(x264args), "\"%s\" %s", x264fullpath, x264cmd);
    remove(pe->temp_filename); //ファイルサイズチェックの時に旧ファイルを参照してしまうのを回避
    
    if (conf->vid.afs && conf->x264.interlaced) {
        ret |= AUO_RESULT_ERROR; error_afs_interlace_stg();
    //jitter用領域確保
    } else if ((jitter = (int *)calloc(oip->n + 1, sizeof(int))) == NULL) {
        ret |= AUO_RESULT_ERROR; error_malloc_tc();
    //Aviutl(afs)からのフレーム読み込み
    } else if (!setup_afsvideo(oip, sys_dat, conf, pe)) {
        ret |= AUO_RESULT_ERROR; //Aviutl(afs)からのフレーム読み込みに失敗
    //x264プロセス開始
    } else if ((rp_ret = RunProcess(x264args, x264dir, &pi_enc, &pipes, (set_priority == AVIUTLSYNC_PRIORITY_CLASS) ? GetPriorityClass(pe->h_p_aviutl) : set_priority, TRUE, FALSE)) != RP_SUCCESS) {
        ret |= AUO_RESULT_ERROR; error_run_process("x264", rp_ret);
    //書き込みスレッドを開始
    } else if (video_output_create_thread(&thread_data, &pixel_data, pipes.f_stdin)) {
        ret |= AUO_RESULT_ERROR; error_video_output_thread_start();
    } else {
        //全て正常
        int i = 0;
        void *frame = NULL;
        int *next_jitter = NULL;
        UINT64 amp_filesize_limit = (UINT64)(1.02 * get_amp_filesize_limit(conf, oip, pe, sys_dat));
        BOOL enc_pause = FALSE, copy_frame = FALSE, drop = FALSE;
        const DWORD aviutl_color_fmt = COLORFORMATS[get_aviutl_color_format(conf->x264.use_highbit_depth, conf->x264.output_csp, conf->vid.input_as_lw48)].FOURCC;

        //Aviutlの時間を取得
        PROCESS_TIME time_aviutl;
        GetProcessTime(pe->h_p_aviutl, &time_aviutl);

        //x264が待機に入るまでこちらも待機
        while (WaitForInputIdle(pi_enc.hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
            log_process_events();

        //ログウィンドウ側から制御を可能に
        DWORD tm_vid_enc_start = timeGetTime();
        enable_x264_control(&set_priority, &enc_pause, afs, afs && pe->current_x264_pass == 1, tm_vid_enc_start, oip->n);

        //------------メインループ------------
        for (i = 0, next_jitter = jitter + 1, pe->drop_count = 0; i < oip->n; i++, next_jitter++) {
            //中断を確認
            ret |= (oip->func_is_abort()) ? AUO_RESULT_ABORT : AUO_RESULT_SUCCESS;

            //x264が実行中なら、メッセージを取得・ログウィンドウに表示
            if (ReadLogEnc(&pipes, pe->drop_count, i) < 0) {
                //勝手に死んだ...
                ret |= AUO_RESULT_ERROR; error_x264_dead();
                break;
            }

            if (!(i & 7)) {
                //Aviutlの進捗表示を更新
                oip->func_rest_time_disp(i + oip->n * (pe->current_x264_pass - 1), oip->n * pe->total_x264_pass);

                //x264優先度
                check_enc_priority(pe->h_p_aviutl, pi_enc.hProcess, set_priority);

                //音声同時処理
                ret |= aud_parallel_task(oip, pe);
                
                //上限をオーバーしていないかチェック
                if (!(i & 63)
                    && amp_filesize_limit //上限設定が存在する
                    && !(1 == pe->current_x264_pass && 1 < pe->total_x264_pass)) { //multi passエンコードの1pass目でない
                    UINT64 current_filesize = 0;
                    if (GetFileSizeUInt64(pe->temp_filename, &current_filesize) && current_filesize > amp_filesize_limit) {
                        warning_amp_filesize_over_limit();
                        pe->muxer_to_be_used = MUXER_DISABLED; //muxをスキップ
                        break;
                    }
                }
            }

            //一時停止
            while (enc_pause & !ret) {
                Sleep(LOG_UPDATE_INTERVAL);
                ret |= (oip->func_is_abort()) ? AUO_RESULT_ABORT : AUO_RESULT_SUCCESS;
                log_process_events();
            }

            //標準入力への書き込み完了をチェック
            while (WAIT_TIMEOUT == WaitForSingleObject(thread_data.he_out_fin, LOG_UPDATE_INTERVAL)) {
                ret |= (oip->func_is_abort()) ? AUO_RESULT_ABORT : AUO_RESULT_SUCCESS;
                log_process_events();
            }

            //中断・エラー等をチェック
            if (AUO_RESULT_SUCCESS != ret)
                break;

            //コピーフレームフラグ処理
            copy_frame = (!!i & (oip->func_get_flag(i) & OUTPUT_INFO_FRAME_FLAG_COPYFRAME));

            //Aviutl(afs)からフレームをもらう
            if (NULL == (frame = ((afs) ? afs_get_video((OUTPUT_INFO *)oip, i, &drop, next_jitter) : oip->func_get_video_ex(i, aviutl_color_fmt)))) {
                ret |= AUO_RESULT_ERROR; error_afs_get_frame();
                break;
            }

            drop |= (afs & copy_frame);

            if (!drop) {
                //コピーフレームの場合は、映像バッファの中身を更新せず、そのままパイプに流す
                if (!copy_frame)
                    convert_frame(frame, &pixel_data, oip->w, oip->h);  /// YUY2/YC48->NV12/YUV444変換, RGBコピー
                //標準入力への書き込みを開始
                SetEvent(thread_data.he_out_start);
            } else {
                *(next_jitter - 1) = DROP_FRAME_FLAG;
                pe->drop_count++;
                //次のフレームの変換を許可
                SetEvent(thread_data.he_out_fin);
            }

            // 「表示 -> セーブ中もプレビュー表示」がチェックされていると
            // func_update_preview() の呼び出しによって func_get_video_ex() の
            // 取得したバッファが書き換えられてしまうので、呼び出し位置を移動 (拡張AVI出力 plus より)
            oip->func_update_preview();
        }
        //------------メインループここまで--------------

        //書き込みスレッドを終了
        video_output_close_thread(&thread_data, ret);

        //ログウィンドウからのx264制御を無効化
        disable_x264_control();

        //パイプを閉じる
        CloseStdIn(&pipes);

        if (!ret) oip->func_rest_time_disp(oip->n * pe->current_x264_pass, oip->n * pe->total_x264_pass);

        //音声の同時処理を終了させる
        ret |= finish_aud_parallel_task(oip, pe, ret);
        //音声との同時処理が終了
        release_audio_parallel_events(pe);

        //タイムコード出力
        if (!ret && (afs || conf->vid.auo_tcfile_out))
            tcfile_out(jitter, oip->n, (double)oip->rate / (double)oip->scale, afs, pe);

        //エンコーダ終了待機
        while (WaitForSingleObject(pi_enc.hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
            ReadLogEnc(&pipes, pe->drop_count, i);

        DWORD tm_vid_enc_fin = timeGetTime();

        //最後にメッセージを取得
        while (ReadLogEnc(&pipes, pe->drop_count, i) > 0);

        if (!(ret & AUO_RESULT_ERROR) && afs)
            write_log_auo_line_fmt(LOG_INFO, "drop %d / %d frames", pe->drop_count, i);
        
        write_log_auo_line_fmt(LOG_INFO, "CPU使用率: Aviutl: %.2f%% / x264: %.2f%%", GetProcessAvgCPUUsage(pe->h_p_aviutl, &time_aviutl), GetProcessAvgCPUUsage(pi_enc.hProcess));
        write_log_auo_enc_time("x264エンコード時間", tm_vid_enc_fin - tm_vid_enc_start);
    }

    //解放処理
    if (pipes.stdErr.mode)
        CloseHandle(pipes.stdErr.h_read);
    CloseHandle(pi_enc.hProcess);
    CloseHandle(pi_enc.hThread);

    free_pixel_data(&pixel_data);
    if (jitter) free(jitter);

    ret |= exit_audio_parallel_control(oip, pe, ret);

    return ret;
}

BOOL check_x264_mp4_output(const char *exe_path, const char *temp_filename) {
    BOOL ret = FALSE;
    std::string exe_message;
    PROCESS_INFORMATION pi = { 0 };

    const int TEST_WIDTH = 160;
    const int TEST_HEIGHT = 120;
    std::vector<char> test_buffer(TEST_WIDTH * TEST_HEIGHT * 3 / 2, 0);

    PIPE_SET pipes = { 0 };
    InitPipes(&pipes);
    pipes.stdIn.mode  = AUO_PIPE_ENABLE;
    pipes.stdOut.mode = AUO_PIPE_DISABLE;
    pipes.stdErr.mode = AUO_PIPE_ENABLE;
    pipes.stdIn.bufferSize = test_buffer.size();

    char test_path[1024] = { 0 };
    for (int i = 0; !i || PathFileExists(test_path); i++) {
        char test_filename[32] = { 0 };
        sprintf_s(test_filename, _countof(test_filename), "_test_%d.mp4", i);
        PathCombineLong(test_path, _countof(test_path), temp_filename, test_filename);
    }

    char exe_dir[1024] = { 0 };
    strcpy_s(exe_dir, _countof(exe_dir), exe_path);
    PathRemoveFileSpecFixed(exe_dir);

    char fullargs[8192] = { 0 };
    sprintf_s(fullargs, _countof(fullargs), "\"%s\" --fps 1 --frames 1 --input-depth 8 --input-res %dx%d -o \"%s\" --input-csp nv12 -", exe_path, TEST_WIDTH, TEST_HEIGHT, test_path);
    if ((ret = RunProcess(fullargs, exe_dir, &pi, &pipes, NORMAL_PRIORITY_CLASS, TRUE, FALSE)) == RP_SUCCESS) {

        while (WAIT_TIMEOUT == WaitForInputIdle(pi.hProcess, LOG_UPDATE_INTERVAL))
            log_process_events();

        _fwrite_nolock(&test_buffer[0], 1, test_buffer.size(), pipes.f_stdin);

        auto read_stderr = [](PIPE_SET *pipes) {
            DWORD pipe_read = 0;
            if (!PeekNamedPipe(pipes->stdErr.h_read, NULL, 0, NULL, &pipe_read, NULL))
                return -1;
            if (pipe_read) {
                ReadFile(pipes->stdErr.h_read, pipes->read_buf + pipes->buf_len, sizeof(pipes->read_buf) - pipes->buf_len - 1, &pipe_read, NULL);
                pipes->buf_len += pipe_read;
                pipes->read_buf[pipes->buf_len] = '\0';
            }
            return (int)pipe_read;
        };

        while (WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, 10)) {
            if (read_stderr(&pipes)) {
                exe_message += pipes.read_buf;
                pipes.buf_len = 0;
            } else {
                log_process_events();
            }
        }

        CloseStdIn(&pipes);

        while (read_stderr(&pipes) > 0) {
            exe_message += pipes.read_buf;
            pipes.buf_len = 0;
        }
        log_process_events();

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (std::string::npos == exe_message.find("not compiled with MP4 output"))
            ret = TRUE;
    }

    if (pipes.stdIn.mode)  CloseHandle(pipes.stdIn.h_read);
    if (pipes.stdOut.mode) CloseHandle(pipes.stdOut.h_read);
    if (pipes.stdErr.mode) CloseHandle(pipes.stdErr.h_read);
    if (PathFileExists(test_path)) remove(test_path);
    return ret;
}

static void set_window_title_x264(const PRM_ENC *pe) {
    char mes[256] = { 0 };
    strcpy_s(mes, _countof(mes), "x264エンコード");
    if (pe->total_x264_pass > 1)
        sprintf_s(mes + strlen(mes), _countof(mes) - strlen(mes), "   %d / %d pass", pe->current_x264_pass, pe->total_x264_pass);
    if (pe->aud_parallel.th_aud)
        strcat_s(mes, _countof(mes), " + 音声エンコード");
    set_window_title(mes, PROGRESSBAR_CONTINUOUS);
}

static AUO_RESULT check_amp(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    if (!(conf->x264.use_auto_npass && conf->x264.rc_mode == X264_RC_BITRATE) || !conf->vid.amp_check)
        return AUO_RESULT_SUCCESS; //上限確認付きcrfはここで抜ける
    //音声ファイルサイズ取得
    double aud_bitrate = 0.0;
    const double duration = get_duration(conf, sys_dat, pe, oip);
    UINT64 aud_filesize = 0;
    if (oip->flag & OUTPUT_INFO_FLAG_AUDIO) {
        if (!str_has_char(pe->append.aud[0])) { //音声エンコがまだ終了していない
            info_amp_do_aud_enc_first(conf->vid.amp_check);
            return AUO_RESULT_ABORT; //音声エンコを先にやるべく、動画エンコを終了する
        }
        for (int i_aud = 0; i_aud < pe->aud_count; i_aud++) {
            char aud_file[MAX_PATH_LEN];
            apply_appendix(aud_file, _countof(aud_file), pe->temp_filename, pe->append.aud[i_aud]);
            if (!PathFileExists(aud_file)) {
                error_no_aud_file();
                return AUO_RESULT_ERROR;
            }
            UINT64 filesize_tmp = 0;
            if (!GetFileSizeUInt64(aud_file, &filesize_tmp)) {
                warning_failed_get_aud_size(); warning_amp_failed();
                return AUO_RESULT_ERROR;
            }
            aud_filesize += filesize_tmp;
        }
        if ((conf->vid.amp_check & AMPLIMIT_FILE_SIZE) && 
            aud_filesize >= conf->vid.amp_limit_file_size * 1024 * 1024) {
            error_amp_aud_too_big(AMPLIMIT_FILE_SIZE);
            return AUO_RESULT_ERROR;
        }
        aud_bitrate = aud_filesize * 8.0 / 1000.0 / duration;
        if ((conf->vid.amp_check & AMPLIMIT_BITRATE_UPPER) &&
            aud_bitrate >= conf->vid.amp_limit_bitrate_upper) {
            error_amp_aud_too_big(AMPLIMIT_BITRATE_UPPER);
            return AUO_RESULT_ERROR;
        }
    }
    //目標ビットレートの計算
    DWORD target_limit = NULL; //上限ファイルサイズと上限ビットレートどちらによる制限か
    double required_file_bitrate = DBL_MAX;
    //上限ファイルサイズのチェック
    if (conf->vid.amp_check & AMPLIMIT_FILE_SIZE) {
        required_file_bitrate = conf->vid.amp_limit_file_size*1024*1024 * 8.0/1000.0 / duration; //動画に割り振ることのできる最大ビットレート
        target_limit = AMPLIMIT_FILE_SIZE;
    }
    //上限ビットレートのチェック
    if ((conf->vid.amp_check & AMPLIMIT_BITRATE_UPPER) &&
        required_file_bitrate > conf->vid.amp_limit_bitrate_upper) {
        required_file_bitrate = conf->vid.amp_limit_bitrate_upper;
        target_limit = AMPLIMIT_BITRATE_UPPER;
    }
    //下限ビットレートのチェック
    //上限設定と矛盾した場合は、上限設定を優先させる
    if ((conf->vid.amp_check & AMPLIMIT_BITRATE_LOWER) && required_file_bitrate < conf->vid.amp_limit_bitrate_lower) {
        warning_amp_bitrate_confliction((int)conf->vid.amp_limit_bitrate_lower, (int)required_file_bitrate);
        conf->vid.amp_check &= ~AMPLIMIT_BITRATE_LOWER;
    }
    const double required_vid_bitrate_upper = get_amp_margin_bitrate(required_file_bitrate - aud_bitrate, sys_dat->exstg->s_local.amp_bitrate_margin_multi);
    //あまりにも計算したビットレートが小さすぎたらエラーを出す
    if (required_vid_bitrate_upper <= 1.0) {
        error_amp_target_bitrate_too_small(target_limit);
        return AUO_RESULT_ERROR;
    }
    const int bitrate_vid_old = conf->x264.bitrate;
    //まず上限のほうをチェック、下限設定よりも優先させる
    //計算されたビットレートが目標ビットレートを上回っていたら、目標ビットレートを変更する
    //conf->x264.bitrate = -1は自動であるが、
    //これをDWORDとして扱うことでUINT_MAX扱いとし、自動的に反映する
    if (required_vid_bitrate_upper < (double)((DWORD)conf->x264.bitrate)) {
        //下限のほうもぎりぎり超えないよう確認
        const double limit_bitrate_lower = (conf->vid.amp_check & AMPLIMIT_BITRATE_LOWER) ? conf->vid.amp_limit_bitrate_lower - aud_bitrate : 0.0;
        conf->x264.bitrate = (int)((std::max)(required_vid_bitrate_upper, limit_bitrate_lower) + 0.5);
    } else {
        //あとから下限のほうもチェック
        const double required_vid_bitrate_lower = get_amp_margin_bitrate(conf->vid.amp_limit_bitrate_lower - aud_bitrate, -1.0 * sys_dat->exstg->s_local.amp_bitrate_margin_multi);
        if ((conf->vid.amp_check & AMPLIMIT_BITRATE_LOWER) && conf->x264.bitrate < required_vid_bitrate_lower) {
            //上限のほうもぎりぎり超えないよう確認
            const double limit_bitrate_upper = (target_limit) ? required_file_bitrate - aud_bitrate : DBL_MAX;
            conf->x264.bitrate = (int)((std::min)(required_vid_bitrate_lower, limit_bitrate_upper) + 0.5);
        }
    }
    //ビットレートを変更したらメッセージ
    if (bitrate_vid_old != conf->x264.bitrate) {
        warning_amp_change_bitrate(bitrate_vid_old, conf->x264.bitrate, target_limit);
    }
    return AUO_RESULT_SUCCESS;
}

static AUO_RESULT video_output_inside(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    //動画エンコードの必要がなければ終了
    if (pe->video_out_type == VIDEO_OUTPUT_DISABLED)
        return ret;

    //最初のみ実行する部分
    if (pe->current_x264_pass <= 1) {
        //自動マルチパス用チェック
        if ((ret |= check_amp(conf, oip, pe, sys_dat)) != AUO_RESULT_SUCCESS) {
            return (ret & ~AUO_RESULT_ABORT); //AUO_RESULT_ABORTなら、音声を先にエンコードするため、動画エンコードを一時的にスキップ
        }
        //追加コマンドをパラメータに適用する
        ret |= check_cmdex(conf, oip, pe, sys_dat);

        //キーフレーム検出 (cmdexのほうに--qpfileの指定があればそれを優先する)
        if (!ret && conf->vid.check_keyframe && strstr(conf->vid.cmdex, "--qpfile") == NULL)
            set_keyframe(conf, oip, pe, sys_dat);
    }

    for (; !ret && pe->current_x264_pass <= pe->total_x264_pass; pe->current_x264_pass++) {
        if (conf->x264.use_auto_npass) {
            //自動npass出力
            switch (pe->current_x264_pass) {
                case 1:
                    conf->x264.pass = 1;
                    break;
                case 2:
                    if (conf->vid.afs && conf->vid.afs_bitrate_correction)
                        conf->x264.bitrate = (conf->x264.bitrate * oip->n) / (oip->n - pe->drop_count);
                    //下へフォールスルー
                default:
                    open_log_window(oip->savefile, sys_dat, pe->current_x264_pass, pe->total_x264_pass);
                    if (pe->current_x264_pass == pe->total_x264_pass)
                        conf->x264.nul_out = FALSE;
                    conf->x264.pass = 3;
                    break;
            }
        }
        set_window_title_x264(pe);
        ret |= x264_out(conf, oip, pe, sys_dat);
    }

    set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);
    return ret;
}

AUO_RESULT video_output(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    return exit_audio_parallel_control(oip, pe, video_output_inside(conf, oip, pe, sys_dat));
}
