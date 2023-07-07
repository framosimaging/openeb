/**********************************************************************************************************************
 * Copyright (c) Prophesee S.A.                                                                                       *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License");                                                    *
 * you may not use this file except in compliance with the License.                                                   *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0                                 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   *
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                      *
 * See the License for the specific language governing permissions and limitations under the License.                 *
 **********************************************************************************************************************/

#include <exception>
#include <iostream>
#include <boost/program_options.hpp>
#include <type_traits>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <vector>
#include <queue>
#include <thread>

#include <metavision/hal/decoders/evt21/evt21_decoder.h>
#include <metavision/hal/decoders/evt3/evt3_decoder.h>
#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/embedded/v4l2_device.h>

using namespace Metavision;
using EventCdDecoder  = I_EventDecoder<EventCD>;
using EventExtDecoder = I_EventDecoder<EventExtTrigger>;
using EventErcDecoder = I_EventDecoder<EventERCCounter>;
using RawEvent        = Evt21Raw::RawEvent;
using RawData         = I_Decoder::RawData;

enum EvtFmt { EVT21, EVT3 };

class EventAnalyzer {
private:
    int _width, _height;
    cv::Mat img0, img1;
    std::mutex m;
    bool updated;
    std::size_t nb_cd_evt;

    // Display colors
    const cv::Vec3b color_bg  = cv::Vec3b(52, 37, 30);
    const cv::Vec3b color_on  = cv::Vec3b(236, 223, 216);
    const cv::Vec3b color_off = cv::Vec3b(201, 126, 64);

public:
    EventAnalyzer(const int width, const int height) : _width(width), _height(height), updated{false}, nb_cd_evt{0} {
        img0 = cv::Mat(height, width, CV_8UC3);
        img1 = cv::Mat(height, width, CV_8UC3);

        img0.setTo(color_bg);
    }
    ~EventAnalyzer() = default;

    void add_cd_evt(const EventCD &evt) {
	if ((evt.x < _width) && (evt.y < _height))
        	img0.at<cv::Vec3b>(evt.y, evt.x) = (evt.p) ? color_on : color_off;
	//else
	//	std::cout << "Out of bond: (" << evt.x << ", " << evt.y << ")"<< std::endl;
        ++nb_cd_evt;
    }
    std::size_t capture_nb_cd_evt() {
        std::size_t res = nb_cd_evt;
        nb_cd_evt       = 0;
        return res;
    }
    // Update the display
    void update() {
        {
            std::unique_lock<std::mutex> lock(m);
            std::swap(img0, img1);
            img0.setTo(color_bg);
            updated = true;
        }
    }

    // Called from main Thread
    bool get_display_frame(cv::Mat &display) {
        if (updated == true) {
            std::unique_lock<std::mutex> lock(m);
            img1.copyTo(display);
            updated = false;
            return true;
        }
        return false;
    }
};

namespace po = boost::program_options;
int main(int argc, char *argv[]) {
    std::string sensor    = "imx636";
    std::string dev_name  = "/dev/video0";
    std::string heap_path = "/dev/dma_heap";
    std::string heap_name = "linux,cma";

    const std::string program_desc("Visualize of events.\n");
    const int fps           = 50; // event-based cameras do not have a frame rate, but we need one for visualization
    const int wait_time     = static_cast<int>(std::round(1000 / fps)); // how much we should wait between two frames
    unsigned int nb_buffers = 32;
    EvtFmt fmt              = EVT21;
    int height              = 720;
    int width               = 1280;

    po::options_description options_desc("Options");
    // clang-format off
    options_desc.add_options()
            ("help,h", "Produce help message.")
            ("sensor,s", po::value<std::string>(&sensor), "Sensor name.")
            ("device,d",  po::value<std::string>(&dev_name), "Device name.")
            ("heap_path,p",  po::value<std::string>(&heap_path), "Path to the dma_heap directory in the system.")
            ("heap_name,n",  po::value<std::string>(&heap_name), "Name of the dma_heap char driver.")
            ("buffers,b",  po::value<unsigned int>(&nb_buffers), "Number of buffers shared with the driver.")
            ;
    // clang-format on

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(options_desc).run(), vm);
        po::notify(vm);
    } catch (po::error &e) {
        std::cerr << program_desc << std::endl;
        std::cerr << options_desc << std::endl;
        std::cerr << "Parsing error: " << e.what() << std::endl;
        return 1;
    }

    if (vm.count("help")) {
        std::cout << program_desc << std::endl;
        std::cout << options_desc << std::endl;
        return 0;
    }
    /* Default sensor is supposed to be IMX636 */
    std::transform(sensor.begin(), sensor.end(), sensor.begin(), ::toupper);
    if (sensor == "GENX320") {
        fmt    = EVT21;
        height = 320;
        width  = 320;
    }

    cv::Mat display; // frame where events will be accumulated
    const std::string window_name = "E-OpenEB Viewer";
    cv::namedWindow(window_name, cv::WINDOW_GUI_EXPANDED);
    cv::resizeWindow(window_name, width, height);

    // Create the event framer
    EventAnalyzer evtFramer(width, height);

    // Create a decoder and set the callback to push decoded events in the event framer
    auto event_cd_decoder  = std::make_shared<EventCdDecoder>();
    auto event_ext_decoder = std::make_shared<EventExtDecoder>();
    auto event_erc_decoder = std::make_shared<EventErcDecoder>();
    event_cd_decoder->add_event_buffer_callback([&](auto beg, auto end) {
        for (auto it = beg; it != end; ++it)
            evtFramer.add_cd_evt(*it);
    });

    std::unique_ptr<I_EventsStreamDecoder> decoder;
    if (fmt == EVT21)
        decoder = std::make_unique<EVT21Decoder>(false, event_cd_decoder, event_ext_decoder, event_erc_decoder);
    else
        decoder = std::make_unique<RobustEVT3Decoder>(false, height, width, event_cd_decoder, event_ext_decoder,
                                                      event_erc_decoder);

    bool stop_processing = false;
    std::thread processing_loop([&]() {
        std::cout << "Open " << dev_name << std::endl;

	
        auto p_buf_heap = std::make_unique<Metavision::DmaBufHeap>(heap_path, heap_name);
        auto p_dev      = std::make_unique<Metavision::V4l2DeviceUserPtr>(dev_name.c_str(), std::move(p_buf_heap),
                                                                     8 * 1024 * 1024, nb_buffers);
        try {
            p_dev->start();
        } catch (const std::exception &e) { std::cerr << e.what() << std::endl; }

        while (stop_processing == false) {
            // Grab a MIPI frame
            int idx = p_dev->get_buffer();

            auto buffer_desc  = p_dev->get_buffer_desc(idx);
            auto raw_data_beg = static_cast<RawData *>(buffer_desc.first);
            auto raw_data_end = raw_data_beg + buffer_desc.second / sizeof(RawData);
            std::cout << "Grab buffer " << idx << "from: " << std::hex << raw_data_beg << " of: " << std::dec
                      << buffer_desc.second << " Bytes." << std::endl;

            // Decode the MIPI frame
            auto start_dec = std::chrono::high_resolution_clock::now();
            decoder->decode(raw_data_beg, raw_data_end);
            auto dec_duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start_dec);
            auto nb_evts = evtFramer.capture_nb_cd_evt();
            std::cout << nb_evts << " events decoded in " << dec_duration.count() << "ms ("
                      << (nb_evts * 1000) / dec_duration.count() << " Kevt/s)" << std::endl;

            // Reset the buffer data
            memset(buffer_desc.first, 0, buffer_desc.second);

            // Release the buffer
            p_dev->release_buffer(idx);

            // Update the display
            evtFramer.update();
        }

        std::cout << "Close " << dev_name << std::endl;
        try {
            p_dev->stop();
        } catch (const std::exception &e) { std::cerr << e.what() << std::endl; }
    });

    while (stop_processing == false) {
        // Update the display frame if required
        if (evtFramer.get_display_frame(display) == true) {
            cv::imshow(window_name, display);
        }

        // if user presses `q` key, quit the loop
        int key = cv::waitKey(wait_time);
        if ((key & 0xff) == 'q') {
            stop_processing = true;
            std::cout << "q pressed, exiting." << std::endl;
        }
    }
    // Wait end of decoding loop
    processing_loop.join();

    return 0;
}
