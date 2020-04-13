#include <iostream>
#include <examples/examples.hpp>
#include <fpp/core/FFmpegException.hpp>

auto main() -> int {

    std::cout << "Started\n";

    try {

        // Common
//        void transmuxing_file();
//        void webcam_to_file();
//        void record_screen_win();

        // YouTube stream
//        void youtube_stream_copy();
//        void youtube_stream_transcode();

        // RTP stream
//        void rtp_video_stream();
//        void rtp_audio_stream();
//        void rtp_video_and_audio_stream();
//        void rtp_video_stream_transcoded();

        // Filters
//        void text_on_video();
//        void timelapase();

    } catch (const fpp::FFmpegException& e) {
        std::cout << "FFmpegException: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception\n";
    }

    std::cout << "Finished\n";

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
