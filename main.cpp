#include <examples/examples.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

auto main() -> int {

    fpp::static_log_info() << "Program started";
    fpp::static_log_info() << "FFmpeg version:" << fpp::utils::ffmpeg_version();
    fpp::set_ffmpeg_log_level(fpp::LogLevel::Error);

    try {

        // Common
//        transmuxing_file();
//        webcam_to_file();
//        webcam_to_udp();
//        mic_to_file();
//        record_screen_win();
       // transsizing();
	// 保存图片
	   // save_picture();
	   // cutVideo(0.0, 2.0, "test.mp4", "tt.mp4");
	   rtsp2mp4("rtsp://127.0.0.1:8090/h264/stream1", "rtsp.mp4");

        // Memory stuff
//        write_to_memory();
//        read_from_memory();

        // YouTube stream
//        youtube_stream_copy();
//        youtube_stream_transcode();
//        youtube_stream_copy_with_silence();
//        youtube_stream_transcode_with_silence();

        // RTP stream
//        rtp_video_stream();
//        rtp_audio_stream();
//        rtp_video_and_audio_stream();
//        rtp_video_stream_transcoded();

        // Filters
//        text_on_video();
//        timelapase();
//        complex();

//        concatenate();
//        multiple_outputs_sequence();
//        multiple_outputs_parallel();

    } catch (const fpp::FFmpegException& e) {
        fpp::static_log_error() << "FFmpegException:" << e.what();
    } catch (const std::exception& e) {
        fpp::static_log_error() << "Exception:" << e.what();
    } catch (...) {
        fpp::static_log_error() << "Unknown exception";
    }

    fpp::static_log_info() << "Program finished";
    return 0;

}

/*

======================== USB video ========================

    video=HP Wide Vision FHD Camera
    video=HP Wide Vision HD
    video=Webcam C170
    video=USB2.0 PC CAMERA
    video=KVYcam Video Driver

======================== USB audio ========================

    audio=Набор микрофонов (Realtek High Definition Audio)

========================== RTSP ===========================

local:

    rtsp://admin:admin@192.168.10.189:554/ch01.264
    rtsp://admin:Admin2019@192.168.10.12:554
    rtsp://admin:admin@192.168.10.3:554 (1080p + audio)

public:

    (!)Check channels ch01_0, ch02_0 and ch03_0
    for lower quality but lower latency

    rtsp://80.26.155.227/live/ch00_0    (food store)
    rtsp://195.46.114.132/live/ch00_0   (building)
    rtsp://87.197.138.187/live/ch00_0   (sports nutrition store (with audio))
    rtsp://213.129.131.54/live/ch00_0   (parking place)
    rtsp://90.80.246.160/live/ch00_0    (lobby)
    rtsp://186.38.89.5/live/ch00_0      (tree)
    rtsp://82.79.117.37/live/ch00_0     (auto parts store)
    rtsp://75.147.239.197/live/ch00_0   (street)
    rtsp://109.183.182.53/live/ch00_0   (yard)
    rtsp://205.120.142.79/live/ch00_0   (server room)
    rtsp://79.101.6.26/live/ch00_0      (tennis tables)
    rtsp://186.1.213.236/live/ch00_0    (street)
    rtsp://185.41.129.79/live/ch00_0    (aquapark)
    rtsp://109.70.190.112/live/ch00_0   (street)
    rtsp://109.73.212.169/live/ch00_0   (turtles)
    rtsp://91.197.91.139/live/ch00_0    (park)
    rtsp://82.150.185.64/live/ch00_0    (field)
    rtsp://80.76.108.241/live/ch00_0    (yard)
    rtsp://193.124.147.207/live/ch00_0  (tree)
    rtsp://163.47.188.104/live/ch00_0
    rtsp://98.163.61.242/live/ch00_0
    rtsp://98.163.61.243/live/ch00_0
    rtsp://38.130.64.34/live/ch00_0

    http://soemon-cho.miemasu.net:63109/nphMotionJpeg?Resolution=640x480&Quality=Motion (Osaka)

*/
