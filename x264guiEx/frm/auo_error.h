﻿// -----------------------------------------------------------------------------------------
// x264guiEx by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2017 rigaya
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

#ifndef _AUO_ERROR_H_
#define _AUO_ERROR_H_

#include <Windows.h>

void warning_conf_not_initialized(const char *default_stg_file);
void warning_failed_getting_temp_path();
void warning_no_temp_root(const char *dir);
void warning_no_aud_temp_root(const char *dir);
void error_filename_too_long();
void error_savdir_do_not_exist(const char *savname, const char *savdir);
void error_file_is_already_opened_by_aviutl();
void error_nothing_to_output();
void error_amp_bitrate_confliction();
void warning_amp_bitrate_confliction(int lower, int upper);
void error_amp_afs_audio_delay_confliction();
void info_afs_audio_delay_confliction();
void info_amp_do_aud_enc_first(DWORD target_limit);
void error_amp_aud_too_big(DWORD target_limit);
void error_amp_target_bitrate_too_small(DWORD target_limit);
void warning_amp_change_bitrate(int bitrate_old, int bitrate_new, DWORD target_limit);
void error_invalid_resolution(BOOL width, int mul, int w, int h);
void error_log_line_cache();
void error_no_exe_file(const char *name, const char *path);
void warning_use_default_audio_encoder(const char *name);
void error_invalid_ini_file();
void error_unsupported_audio_format_by_muxer(const int video_out_type, const char *selected_aud, const char *default_aud);
void info_use_exe_found(const char *target, const char *path);
void error_mp4_muxer_unmatch_of_ini_and_exe(BOOL exe_file_is_lsmash);
void warning_auto_afs_disable();
void error_afs_setup(BOOL afs, BOOL auto_afs_disable);
void warning_x264_mp4_output_not_supported();
void error_run_process(const char *exe_name, int rp_ret);
void error_video_output_thread_start();
void warning_auto_qpfile_failed();
void warning_auo_tcfile_failed();
void error_open_wavfile();
void warning_audio_length(const double video_length, const double audio_length, const BOOL exedit_is_used);

void error_malloc_pixel_data();
void error_malloc_tc();
void error_malloc_8bit();
void error_afs_interlace_stg();
void error_x264_dead();
void error_x264_dead_and_nodiskspace(const char *drive, uint64_t diskspace);
void error_x264_version(const char *required_ver, const char *current_ver);
void error_afs_get_frame();
void error_audenc_failed(const char *name, const char *args);
void error_no_wavefile();
void error_mux_failed(const char *name, const char *args);

void warning_no_mux_tmp_root(const char *dir);
void warning_failed_mux_tmp_drive_space();
void warning_failed_muxer_drive_space();
void warning_failed_out_drive_space();
void warning_failed_get_aud_size();
void warning_failed_get_vid_size();
void error_no_vid_file();
void error_no_aud_file();
void error_vid_file_zero_byte();
void error_aud_file_zero_byte();
void warning_mux_tmp_not_enough_space();
void error_muxer_drive_not_enough_space();
void error_out_drive_not_enough_space();
void warning_failed_to_get_duration_from_timecode();
void error_check_muxout_exist();
void error_check_muxout_too_small(int expected_filesize_KB, int muxout_filesize_KB);
void warning_failed_check_muxout_filesize();
void warning_amp_failed();
void warning_amp_filesize_over_limit();
void info_amp_result(DWORD status, int amp_result, UINT64 filesize, double file_bitrate, double limit_filesize, double limit_filebitrate_upper, double limit_filebitrate_lower, int retry_count, int new_bitrate);

void warning_no_auto_save_log_dir();

void info_encoding_aborted();

void warning_mux_no_chapter_file();
void warning_mux_chapter(int sts);
void warning_chapter_convert_to_utf8(int sts);

void error_select_convert_func(int width, int height, BOOL use16bit, BOOL interlaced, int output_csp);

void warning_no_batfile(const char *batfile);
void warning_malloc_batfile_tmp();
void warning_failed_open_bat_orig();
void warning_failed_open_bat_new();

#endif //_AUO_ERROR_H_