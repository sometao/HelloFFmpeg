
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <iostream>
#include <fstream>
#include <tuple>
#include "AudioCollector.h"
#include "seeker/loggerApi.h"
#include "ReSampler.h"
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
  AudioCollector audioCollector(FRAME_SIZE * 2 * 4, sampleRate, AL_FORMAT_MONO16);
  std::cout << "capture_to_aac2 ------- 2 ----------" << std::endl;


  ffmpegUtil::AudioInfo inputAudio(AV_CH_LAYOUT_MONO, sampleRate, channels, AV_SAMPLE_FMT_S16);
  ffmpegUtil::AudioInfo outputAudio(AV_CH_LAYOUT_MONO, sampleRate, channels, AV_SAMPLE_FMT_FLTP);

  ffmpegUtil::ReSampler reSampler(inputAudio, outputAudio, FRAME_SIZE);

  audioCollector.open();

  auto deviceName = audioCollector.getDeviceName();
  std::cout << "deviceName: " << deviceName << std::endl;
  audioCollector.start();

  // void *wavWriter = wav_write_open(OUTPUT_WAVE_FILE, sampleRate, bitsPerSample, channels);

  size_t pktCount = 0;
  size_t pktByteCount = 0;

  std::ofstream fout(OUTPUT_AAC_FILE, std::ios::binary);


  auto encodeSamplesAndProcess = [&](size_t sampleNumber, bool endOfStream = false) {
    int capturedDataSize =
        audioCollector.captureSamples(buffer, FRAME_SIZE * 2, sampleNumber);

    //D_LOG("captureSamples done. sampleNumber={}, capturedDataSize={}", sampleNumber, capturedDataSize);


    const uint8_t* d = &buffer[0];

    int outSamples;
    int outDataSize;
    uint8_t* outData;
    std::tie(outSamples, outData, outDataSize) = reSampler.reSample2( (const uint8_t **)&d, sampleNumber);

    //D_LOG("reSample2 done. outSamples={}, outDataSize={}", outSamples, outDataSize);


    int gotPkt = encoder.encode(outData, outDataSize, endOfStream);
    if (gotPkt) {
      AVPacket* pkt = encoder.getPacket();
      pktCount += 1;
      fout.write((char*)pkt->data, pkt->size);
      pktByteCount += pkt->size;
      D_LOG("encodeSamplesAndProcess pkt->size={} pktCount={}, pktByteCount={}", pkt->size, pktCount, pktByteCount);
      av_packet_free(&pkt);
    }
  };

  size_t iSamplesAvailable;
  // Record for 5 seconds
  auto dwStartTime = seeker::Time::currentTime();
  while ((seeker::Time::currentTime() <= (dwStartTime + 5000))) {
    Sleep(2);

    // Find out how many samples have been captured
    iSamplesAvailable = audioCollector.availableSamples();

    //D_LOG("Samples available: {}", iSamplesAvailable);

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