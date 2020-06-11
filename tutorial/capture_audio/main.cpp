#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <iostream>
#include "seeker/logger.h"
#include "AudioCollector.h"

extern int capture_to_aac();
extern int capture_to_wav();

int main() { 
  seeker::Logger::init("", true);


  capture_to_wav();
  return 0;

}