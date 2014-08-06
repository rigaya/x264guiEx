//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_ERROR_H_
#define _AUO_ERROR_H_

void warning_failed_getting_temp_path();
void warning_no_temp_root(const char *dir);
void warning_no_aud_temp_root(const char *dir);
void error_filename_too_long();
void error_nothing_to_output();
void error_invalid_resolution(BOOL width, int mul, int w, int h);
void error_no_exe_file(const char *name, const char *path);
void error_afs_setup(BOOL afs);
void error_run_process(const char *exe_name, int rp_ret);
void warning_auto_qpfile_failed();
void warning_auo_tcfile_failed();
void error_open_wavfile();

void error_malloc_pixel_data();
void error_malloc_tc();
void error_malloc_8bit();
void error_x264_dead();
void error_afs_get_frame();
void error_batfile_original_open();
void error_batfile_temp_open();
void error_audenc_failed(const char *name, const char *args);
void error_no_wavefile();
void error_mux_failed(const char *name, const char *args);

void warning_no_mux_tmp_root(const char *dir);
void warning_failed_mux_tmp_drive_space();
void warning_failed_get_aud_size();
void warning_failed_get_vid_size();
void warning_mux_tmp_not_enough_space();

void info_encoding_aborted();

#endif //_AUO_ERROR_H_