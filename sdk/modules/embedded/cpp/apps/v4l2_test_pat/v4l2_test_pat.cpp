#include <exception>
#include <iostream>
#include <boost/program_options.hpp>
#include <type_traits>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <vector>
#include <queue>
#include <thread>

#include <include/v4l2_device.h>
#include <include/raw_evt3_decoder_1.h>

template<typename DataIt>
std::size_t process_data(DataIt begin, DataIt end) {
    DataIt cur       = begin;
    std::size_t size = 0;

	auto val = *cur;
	while (cur != end) {
		if (size++ < 10) {
			printf("%08x ", *cur);
		}
		else {
			if (*cur == 0)
				break;
			else if (*cur != val + 1) {
				std::cout << "Error @" << size << " " << std::hex << *cur << " " << val << " ";
				break;
			}
		}
		val = *cur;
		++cur;
	}
	return size;
}

namespace po = boost::program_options;
int main(int argc, char *argv[]) {
    std::string dev_name  = "/dev/video0";
    std::string heap_path = "/dev/dma_heap";
    std::string heap_name = "linux,cma";
    bool use_mmap         = false;

    const std::string program_desc("Visualize of events.\n");
    const int fps       = 50; // event-based cameras do not have a frame rate, but we need one for visualization
    const int wait_time = static_cast<int>(std::round(1.f / fps * 1000)); // how much we should wait between two frames
    unsigned int nb_buffers = 32;

    po::options_description options_desc("Options");
    // clang-format off
    options_desc.add_options()
            ("help,h", "Produce help message.")
            ("device,d",  po::value<std::string>(&dev_name), "Device name.")
            ("heap_path,p",  po::value<std::string>(&heap_path), "Path to the dma_heap directory in the system.")
            ("heap_name,n",  po::value<std::string>(&heap_name), "Name of the dma_heap char driver.")
            ("use_mmap,m",  po::value<bool>(&use_mmap), "Use mmap instead of User Ptr IO method.")
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
	std::cout << "Open " << dev_name << std::endl;
	std::unique_ptr<Metavision::V4l2Device> p_dev = std::make_unique<Metavision::V4l2DeviceMmap>(dev_name.c_str(), nb_buffers);

	try {
		p_dev->start();
	} catch (const std::exception &e) { std::cerr << e.what() << std::endl; }

	bool stop_processing = false;
	while (stop_processing == false) {
		// Grab a MIPI frame
		int idx = p_dev->get_buffer();
		std::cout << "Buffer (" << idx << ")";

		// Decode the MIPI frame
		void *buffer_addr       = p_dev->get_buffer_addr(idx);
		std::size_t buffer_size = p_dev->get_buffer_size(idx);
		auto raw_begin = static_cast<uint32_t *>(buffer_addr);
		auto raw_end   = raw_begin + buffer_size / sizeof(*raw_begin);
		auto nb_evts = process_data(raw_begin, raw_end);
		std::cout << "Size: " << std::dec << nb_evts << std::endl;

		// Release a MIPI frame
		p_dev->release_buffer(idx);
	}

	return 1;
}