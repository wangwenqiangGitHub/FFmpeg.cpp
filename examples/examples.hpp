
// Common
void transmuxing_file();
void transrating_file();
void transsizing();
void webcam_to_file();
void webcam_to_udp();
void mic_to_file();
void record_screen_win();
void save_picture();
void cutVideo(double start_seconds,
		double end_seconds,
		const char *inputFileName,
		const char *outputFileName);
int rtsp2mp4(const char* pInputFileName, const char* pOutputFileName);
// Memory stuff
void write_to_memory();
void read_from_memory();

// YouTube stream
void youtube_stream_copy();
void youtube_stream_transcode();
void youtube_stream_copy_with_silence();
void youtube_stream_transcode_with_silence();

// RTP stream
void rtp_video_stream();
void rtp_audio_stream();
void rtp_video_and_audio_stream();
void rtp_video_stream_transcoded();

// Filters
void text_on_video();
void timelapase();
void complex();

void concatenate();
void multiple_outputs_sequence();
void multiple_outputs_parallel();
