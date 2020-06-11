
#pragma once


#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <iostream>
#include <string>

#include "seeker/loggerApi.h"
#include "seeker/common.h"

#include "al.h"
#include "alc.h"

enum class AudioFormat {};

class AudioCollector {
 private:
  const std::string deviceName;

  const size_t bufSize;

  const int sampleRate;
  const int bitsPerSample;
  const int channels;
  const int blockAlign;

  uint8_t *buffer = nullptr;
  ALCdevice *pCaptureDevice = nullptr;
  ALCcontext *pContext = nullptr;

 public:
  static void listDevices();

  static bool checkSupport();

  static std::string getDefaultDeviceName();

  AudioCollector(size_t bufSize_, int sampleRate_, int bitsPerSample_, int channels_,
                 std::string deviceName_ = "")
      : bufSize(bufSize),
        sampleRate(sampleRate_),
        bitsPerSample(bitsPerSample_),
        channels(channels_),
        blockAlign(channels_ * bitsPerSample_ / 8),
        deviceName(deviceName_.length() > 0 ? deviceName_ : getDefaultDeviceName()) {

    if (bitsPerSample != 8 && bitsPerSample != 16) {
      std::string msg =
        fmt::format("AudioCollector init failed: bitsPerSample must be 8 or 16, bitsPerSample=[{}] is not supported.",
          bitsPerSample);
      E_LOG(msg);
      throw std::exception(msg.c_str());
    }


    if (channels != 1 && channels != 2) {
      std::string msg =
        fmt::format("AudioCollector init failed: channels must be 1 or 2, channels=[{}] is not supported.",
          channels);
      E_LOG(msg);
      throw std::exception(msg.c_str());
    }


    if (!checkSupport()) {
      auto msg = "AudioCollector init failed: Failed to detect Capture Extension";
      E_LOG(msg);
      throw std::exception(msg);
    }

    int format = 0;
    if(bitsPerSample == 16 && channels == 1)  {
      format = AL_FORMAT_MONO16;
    } else if(bitsPerSample == 16 && channels == 2) {
      format = AL_FORMAT_STEREO16;
    } else if(bitsPerSample == 8 && channels == 1) {
      format = AL_FORMAT_MONO8;
    } else if(bitsPerSample == 8 && channels == 2) {
      format = AL_FORMAT_STEREO8;
    } else {
      throw std::exception("It will never happened.");
    }

    pCaptureDevice =
      alcCaptureOpenDevice(deviceName.c_str(), sampleRate, AL_FORMAT_MONO16, bufSize);

    if(pCaptureDevice == nullptr) {
      auto msg = fmt::format("AudioCollector init failed: can not open device: {}", deviceName);
      E_LOG(msg);
      throw std::exception(msg.c_str());
    }

    buffer = new uint8_t[bufSize];

  }

  ~AudioCollector() { delete[] buffer; }

  void start() {



    // Start audio capture
    alcCaptureStart(pCaptureDevice);
  }


  void close();
};