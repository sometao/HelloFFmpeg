/**
@author Tao Zhang
@since 2020/3/1
@version 0.0.1-SNAPSHOT 2020/6/15
*/
#pragma once

#ifdef _WIN32
// Windows
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};
#else
// Linux...
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
};
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <tuple>
#include "seeker/loggerApi.h"

namespace ffmpegUtil {

using std::cout;
using std::endl;
using std::string;
using std::stringstream;

struct AudioInfo {
  int64_t layout;
  int sampleRate;
  int channels;
  AVSampleFormat format;

  AudioInfo() {
    layout = -1;
    sampleRate = -1;
    channels = -1;
    format = AV_SAMPLE_FMT_S16;
  }

  AudioInfo(int64_t l, int rate, int c, AVSampleFormat f)
      : layout(l), sampleRate(rate), channels(c), format(f) {}
};

class ReSampler {
  SwrContext* swr;
  uint8_t* outBuf = nullptr;
  size_t bufSize = 0;

 public:
  ReSampler(const ReSampler&) = delete;
  ReSampler(ReSampler&&) noexcept = delete;
  ReSampler operator=(const ReSampler&) = delete;
  ~ReSampler() {
    cout << "deconstruct ReSampler called." << endl;
    if (swr != nullptr) {
      swr_free(&swr);
    }
  }

  const AudioInfo in;
  const AudioInfo out;

  static AudioInfo getDefaultAudioInfo(int sr) {
    int64_t layout = AV_CH_LAYOUT_STEREO;
    int sampleRate = sr;
    int channels = 2;
    AVSampleFormat format = AV_SAMPLE_FMT_S16;

    return AudioInfo(layout, sampleRate, channels, format);
  }

  ReSampler(AudioInfo input, AudioInfo output, int frameSize = -1) : in(input), out(output) {
    swr = swr_alloc_set_opts(nullptr, out.layout, out.format, out.sampleRate, in.layout,
                             in.format, in.sampleRate, 0, nullptr);

    if (swr_init(swr)) {
      throw std::runtime_error("swr_init error.");
    }

    if (frameSize > 0) {
      bufSize = allocDataBuf(&outBuf, frameSize);
    }
  }


  size_t allocDataBuf(uint8_t** outDataBuffer, int frameSize) {
    int bytePerOutSample = -1;
    switch (out.format) {
      case AV_SAMPLE_FMT_U8:
        bytePerOutSample = 1;
        break;
      case AV_SAMPLE_FMT_S16P:
      case AV_SAMPLE_FMT_S16:
        bytePerOutSample = 2;
        break;
      case AV_SAMPLE_FMT_S32:
      case AV_SAMPLE_FMT_S32P:
      case AV_SAMPLE_FMT_FLT:
      case AV_SAMPLE_FMT_FLTP:
        bytePerOutSample = 4;
        break;
      case AV_SAMPLE_FMT_DBL:
      case AV_SAMPLE_FMT_DBLP:
      case AV_SAMPLE_FMT_S64:
      case AV_SAMPLE_FMT_S64P:
        bytePerOutSample = 8;
        break;
      default:
        bytePerOutSample = 2;
        break;
    }

    int guessOutSamplesPerChannel =
        av_rescale_rnd(frameSize, out.sampleRate, in.sampleRate, AV_ROUND_UP);
    size_t guessOutSize = guessOutSamplesPerChannel * out.channels * bytePerOutSample;

    D_LOG("GuessOutSamplesPerChannel=[{}] GuessOutSize=[{}]", guessOutSamplesPerChannel,
          guessOutSize);

    guessOutSize *= 1.2;  // just make sure.

    *outDataBuffer = (uint8_t*)av_malloc(sizeof(uint8_t) * guessOutSize);
    // av_samples_alloc(&outData, NULL, outChannels, guessOutSamplesPerChannel,
    // AV_SAMPLE_FMT_S16, 0);
    return guessOutSize;
  }

  std::tuple<int, int> reSample(uint8_t** outBuffer, int outBufferSize,
                                const uint8_t** inBuffer, int inputSamplesInOneChannel) {
    int outSamples = swr_convert(swr, outBuffer, outBufferSize, inBuffer, inputSamplesInOneChannel);

    T_LOG("reSample1: outBufferSize=[{}] inCount=[{}]", outBufferSize, inputSamplesInOneChannel);

    if (outSamples <= 0) {
      throw std::runtime_error("error: outSamples=" + outSamples);
    }

    int outDataSize =
        av_samples_get_buffer_size(NULL, out.channels, outSamples, out.format, 1);

    if (outDataSize <= 0) {
      throw std::runtime_error("error: outDataSize=" + outDataSize);
    }

    return {outSamples, outDataSize};
  }

  std::tuple<int, uint8_t*, int> reSample2(const uint8_t** inBuffer, int inputSamplesInOneChannel) {

    //D_LOG("reSample2: bufSize=[{}] inputSamplesInOneChannel=[{}]", bufSize, inputSamplesInOneChannel);
    int outSamples = swr_convert(swr, &outBuf, bufSize, inBuffer, inputSamplesInOneChannel);

    if (outSamples <= 0) {
      throw std::runtime_error("error: outSamples=" + outSamples);
    }

    int outDataSize =
        av_samples_get_buffer_size(NULL, out.channels, outSamples, out.format, 1);

    if (outDataSize <= 0) {
      throw std::runtime_error("error: outDataSize=" + outDataSize);
    }

    return {outSamples, outBuf, outDataSize};
  }
};

}  // namespace ffmpegUtil
