
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <iostream>
#include "AudioCollector.h"
#include "seeker/loggerApi.h"
#include "seeker/common.h"
#include "wav_reader.hpp"
#include "wav_writer.hpp"

#define OUTPUT_WAVE_FILE "Capture2.wav"
#define BUFFERSIZE 4410

int capture_to_wav2() {


  ALchar Buffer[BUFFERSIZE];

  const int channels = 1;
  const int sampleRate = 22050;
  const int bitsPerSample = 16;
  const int blockAlign = channels * bitsPerSample / 8;

  AudioCollector audioCollector(BUFFERSIZE, sampleRate, AL_FORMAT_MONO16);

  audioCollector.open();

  auto deviceName = audioCollector.getDeviceName();
  std::cout << "deviceName: " << deviceName << std::endl;
  audioCollector.start();

  void *wavWriter = wav_write_open(OUTPUT_WAVE_FILE, sampleRate, bitsPerSample, channels);


  size_t iSamplesAvailable;
  // Record for 5 seconds
  auto dwStartTime = seeker::Time::currentTime();
  while ((seeker::Time::currentTime() <= (dwStartTime + 8000))) {
    Sleep(2);

    // Find out how many samples have been captured
    iSamplesAvailable = audioCollector.availableSamples();

    D_LOG("Samples available: {} ? {}", iSamplesAvailable, (BUFFERSIZE / blockAlign));

    // When we have enough data to fill our BUFFERSIZE byte buffer, grab the samples
    if (iSamplesAvailable > (BUFFERSIZE / blockAlign)) {
      int dataOut = audioCollector.captureSamples((uint8_t*)Buffer, BUFFERSIZE, BUFFERSIZE / blockAlign );
      D_LOG("dataOut: {}", dataOut);
      wav_write_data(wavWriter, (unsigned char *)&Buffer, dataOut);
    }
  }

  audioCollector.stop();


  iSamplesAvailable = audioCollector.availableSamples();

  while (iSamplesAvailable) {
    if (iSamplesAvailable > (BUFFERSIZE / blockAlign)) {
      int dataOut = audioCollector.captureSamples((uint8_t*)Buffer, BUFFERSIZE, BUFFERSIZE / blockAlign );
      wav_write_data(wavWriter, (unsigned char *)&Buffer, dataOut);
      iSamplesAvailable -= (BUFFERSIZE / blockAlign);
    } else {
      int dataOut = audioCollector.captureSamples((uint8_t*)Buffer, BUFFERSIZE, iSamplesAvailable );
      wav_write_data(wavWriter, (unsigned char *)&Buffer, dataOut);
      iSamplesAvailable = 0;
    }
  }

  wav_write_close(wavWriter);
  audioCollector.close();


  return 0;
}