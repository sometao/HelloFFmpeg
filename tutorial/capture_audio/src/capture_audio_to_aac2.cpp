
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <iostream>
#include <fstream>
#include "AudioCollector.h"
#include "seeker/loggerApi.h"
#include "AacAdtsEncoder.h"


#define OUTPUT_AAC_FILE "Capture2.aac"
#define FRAME_SIZE 1024


int capture_to_aac2() {
  uint8_t buffer[FRAME_SIZE * 2];

  const int channels = 1;
  const int sampleRate = 22050;
  const int bitsPerSample = 16;
  const int blockAlign = channels * bitsPerSample / 8;

  std::cout << "capture_to_aac2 ------- 0 ----------" << std::endl;
  AacAdtsEncoder encoder(32000, AV_SAMPLE_FMT_FLTP, sampleRate, channels);
  std::cout << "capture_to_aac2 ------- 1 ----------" << std::endl;
  AudioCollector audioCollector(FRAME_SIZE * 2 * 4, sampleRate, bitsPerSample, channels);
  std::cout << "capture_to_aac2 ------- 2 ----------" << std::endl;

  audioCollector.open();

  auto deviceName = audioCollector.getDeviceName();
  std::cout << "deviceName: " << deviceName << std::endl;
  audioCollector.start();

  // void *wavWriter = wav_write_open(OUTPUT_WAVE_FILE, sampleRate, bitsPerSample, channels);

  size_t pktCount = 0;
  size_t pktByteCount = 0;

  std::ofstream fout(OUTPUT_AAC_FILE, std::ios::binary);


  auto encodeSamplesAndProcess = [&](size_t sampleNumber, bool endOfStream = false) {
    int dataOut =
        audioCollector.captureSamples(buffer, sampleNumber * blockAlign, sampleNumber);
    int gotPkt = encoder.encode(buffer, sampleNumber * blockAlign, endOfStream);
    if (gotPkt) {
      AVPacket* pkt = encoder.getPacket();
      // TODO process pkt.
      pktCount += 1;
      fout.write((char*)pkt->data, pkt->size);
      pktByteCount += pkt->size;
      av_packet_free(&pkt);
      D_LOG("encodeSamplesAndProcess pktCount={}, pktByteCount={}", pktCount, pktByteCount);
    }
  };

  size_t iSamplesAvailable;
  // Record for 5 seconds
  auto dwStartTime = seeker::Time::currentTime();
  while ((seeker::Time::currentTime() <= (dwStartTime + 8000))) {
    Sleep(2);

    // Find out how many samples have been captured
    iSamplesAvailable = audioCollector.availableSamples();

    D_LOG("Samples available: {}", iSamplesAvailable);

    // When we have enough data to fill our BUFFERSIZE byte buffer, grab the samples
    if (iSamplesAvailable > FRAME_SIZE) {
      encodeSamplesAndProcess(FRAME_SIZE);
    }
  }

  audioCollector.stop();


  iSamplesAvailable = audioCollector.availableSamples();

  while (iSamplesAvailable) {
    if (iSamplesAvailable > FRAME_SIZE) {
      encodeSamplesAndProcess(FRAME_SIZE);
      iSamplesAvailable -= FRAME_SIZE;
    } else {
      encodeSamplesAndProcess(iSamplesAvailable, true);
      iSamplesAvailable = 0;
    }
  }

  audioCollector.close();


  return 0;
}