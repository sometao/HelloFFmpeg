#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <iostream>
#include "seeker/logger.h"
#include "AudioCollector.h"

extern int capture_to_aac();
extern int capture_to_wav();
extern int capture_to_wav2();
extern int capture_to_aac2();

int main() {
  seeker::Logger::init("", true);

  try {
    capture_to_aac2();
  } catch (std::exception ex) {
    std::cout << "exception: " << ex.what() << std::endl;
  }


  return 0;
}