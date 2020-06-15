/**
@author Tao Zhang
@since 2020/6/12
@version 0.0.1-SNAPSHOT 2020/6/15
*/
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
  int bitsPerSample;
  int channels;
  int blockAlign;
  ALCenum soundFormat;

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

  AudioCollector(size_t bufSize_, int sampleRate_, ALCenum alSoundFormat,
                 std::string deviceName_ = "")
      : internalBufSize(bufSize_),
        sampleRate(sampleRate_),
        soundFormat(alSoundFormat),
        deviceName(deviceName_.length() > 0 ? deviceName_ : getDefaultDeviceName()) {
    pContext = alcGetCurrentContext();

    if (!checkSupport()) {
      auto msg = "AudioCollector init failed: Failed to detect Capture Extension";
      E_LOG(msg);
      throw std::exception(msg);
    }


    switch (alSoundFormat) {
      case AL_FORMAT_MONO8:
        bitsPerSample = 8;
        channels = 1;
        break;
      case AL_FORMAT_STEREO8:
        bitsPerSample = 8;
        channels = 2;
        break;
      case AL_FORMAT_MONO16:
        bitsPerSample = 16;
        channels = 1;
        break;
      case AL_FORMAT_STEREO16:
        bitsPerSample = 16;
        channels = 2;
        break;
      default:
        throw std::runtime_error("Can not find al sound format: " +
                                 std::to_string(alSoundFormat));
        break;
    }

    blockAlign = channels * bitsPerSample / 8;

    buffer = new uint8_t[internalBufSize];
  }

  ~AudioCollector() { delete[] buffer; }

  void open() {
    pCaptureDevice = alcCaptureOpenDevice(deviceName.c_str(), sampleRate, soundFormat, internalBufSize);

    if (pCaptureDevice == nullptr) {
      throw std::runtime_error(
          fmt::format("AudioCollector init failed: can not open device: {}", deviceName));
    }
  }

  void start() { alcCaptureStart(pCaptureDevice); }

  void stop() { alcCaptureStop(pCaptureDevice); }

  int availableSamples() {
    int iSamplesAvailable;
    alcGetIntegerv(pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &iSamplesAvailable);
    return iSamplesAvailable;
  }

  size_t captureSamples(uint8_t *buf, size_t bufSize, uint16_t sampleNumber) {
    size_t outSize = sampleNumber * blockAlign;
    if (outSize > bufSize) {
      throw std::runtime_error(
          fmt::format("bufSize[{}] is too small to get {} samples", bufSize, sampleNumber));
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