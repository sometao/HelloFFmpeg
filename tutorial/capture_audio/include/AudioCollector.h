#pragma once

#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <iostream>
#include <string>
#include <vector>

#include "seeker/loggerApi.h"
#include "seeker/common.h"

#include "al.h"
#include "alc.h"

enum class AudioFormat {};

class AudioCollector {
 private:
  const std::string deviceName;

  const size_t internalBufSize;

  const int sampleRate;
  const int bitsPerSample;
  const int channels;
  const int blockAlign;

  uint8_t *buffer = nullptr;
  ALCcontext *pContext = nullptr;
  ALCdevice *pCaptureDevice = nullptr;

  bool checkSupport() {
    ALCdevice *pDevice = alcGetContextsDevice(pContext);
    if (alcIsExtensionPresent(pDevice, "ALC_EXT_CAPTURE") == AL_FALSE) {
      // printf("Failed to detect Capture Extension\n");
      return false;
    } else {
      return true;
    }
  }

 public:
  static std::vector<std::string> listDevices() {
    std::vector<std::string> rst;
    // Get list of available Capture Devices
    const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (pDeviceList) {
      // printf("\nAvailable Capture Devices are:-\n");
      while (*pDeviceList) {
        // printf("%s\n", pDeviceList);
        rst.emplace_back(std::string(pDeviceList));
        pDeviceList += strlen(pDeviceList) + 1;
      }
    }
    return rst;
  }


  static std::string getDefaultDeviceName() {
    auto s = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    return std::string(s);
  }

  AudioCollector(size_t bufSize_, int sampleRate_, int bitsPerSample_, int channels_,
                 std::string deviceName_ = "")
      : internalBufSize(bufSize_),
        sampleRate(sampleRate_),
        bitsPerSample(bitsPerSample_),
        channels(channels_),
        blockAlign(channels_ * bitsPerSample_ / 8),
        deviceName(deviceName_.length() > 0 ? deviceName_ : getDefaultDeviceName()) {
    if (bitsPerSample != 8 && bitsPerSample != 16) {
      std::string msg = fmt::format(
          "AudioCollector init failed: bitsPerSample must be 8 or 16, bitsPerSample=[{}] is "
          "not supported.",
          bitsPerSample);
      E_LOG(msg);
      throw std::exception(msg.c_str());
    }


    if (channels != 1 && channels != 2) {
      std::string msg = fmt::format(
          "AudioCollector init failed: channels must be 1 or 2, channels=[{}] is not "
          "supported.",
          channels);
      E_LOG(msg);
      throw std::exception(msg.c_str());
    }


    pContext = alcGetCurrentContext();


    if (!checkSupport()) {
      auto msg = "AudioCollector init failed: Failed to detect Capture Extension";
      E_LOG(msg);
      throw std::exception(msg);
    }

    int format = 0;
    if (bitsPerSample == 16 && channels == 1) {
      format = AL_FORMAT_MONO16;
    } else if (bitsPerSample == 16 && channels == 2) {
      format = AL_FORMAT_STEREO16;
    } else if (bitsPerSample == 8 && channels == 1) {
      format = AL_FORMAT_MONO8;
    } else if (bitsPerSample == 8 && channels == 2) {
      format = AL_FORMAT_STEREO8;
    } else {
      throw std::exception("It will never happened.");
    }

    buffer = new uint8_t[internalBufSize];
  }

  ~AudioCollector() { delete[] buffer; }

  void open() {
    pCaptureDevice = alcCaptureOpenDevice(deviceName.c_str(), sampleRate, AL_FORMAT_MONO16,
                                          internalBufSize);

    if (pCaptureDevice == nullptr) {
      auto msg =
          fmt::format("AudioCollector init failed: can not open device: {}", deviceName);
      E_LOG(msg);
      throw std::exception(msg.c_str());
    }
  }

  void start() { alcCaptureStart(pCaptureDevice); }

  void stop() { alcCaptureStop(pCaptureDevice); }

  int availableSamples() {
    int iSamplesAvailable;
    alcGetIntegerv(pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &iSamplesAvailable);
    return iSamplesAvailable;
  }

  size_t captureSamples(uint8_t *buf, size_t bufSize, int sampleNumber) {
    size_t outSize = sampleNumber * blockAlign;
    if (outSize > bufSize) {
      throw std::exception(
          fmt::format("bufSize[{}] is too small to get {} samples", bufSize, sampleNumber)
              .c_str());
    }
    alcCaptureSamples(pCaptureDevice, buf, sampleNumber);
    return outSize;
  }

  std::string getDeviceName() {
    std::string name = alcGetString(pCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
    return name;
  }

  void close() {
    if (pCaptureDevice != nullptr) {
      // Close the Capture Device
      alcCaptureCloseDevice(pCaptureDevice);
    }
  }
};