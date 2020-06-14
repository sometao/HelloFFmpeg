#pragma once

#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <iostream>
#include <string>
#include <list>

#include "seeker/loggerApi.h"
#include "seeker/common.h"


extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include "libswresample/swresample.h"
}



class AacAdtsEncoder {
 private:
  AVCodecContext *codecCtx;
  std::list<AVPacket *> pktList{};

 public:
  AacAdtsEncoder(int64_t bitRate = 64000,
                 AVSampleFormat sampleFmt = AV_SAMPLE_FMT_S16,
                 int sampleRate = 44100,
                 int channels = 1 /*only support 1 and 2*/) {
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (codec == nullptr) {
      throw std::runtime_error(fmt::format("Could not find codec:{}", (int)AV_CODEC_ID_AAC));
    }

    avcodec_alloc_context3(codec);

    codecCtx->sample_fmt = sampleFmt;

    codecCtx->bit_rate = bitRate;

    codecCtx->sample_rate = sampleRate;

    codecCtx->channels = channels;

    if (channels == 1) {
      codecCtx->channel_layout = AV_CH_LAYOUT_MONO;
    } else if (channels == 2) {
      codecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    } else {
      throw std::runtime_error(fmt::format("channels must be 1 or 2, but got [{}]", channels));
    }

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
      throw std::runtime_error(fmt::format("Could not open codec: {}", codec->name));
    }


    D_LOG("codecCtx->frame_size = {}", codecCtx->frame_size);
  }


  ~AacAdtsEncoder() {
    if (codecCtx != nullptr) {
      avcodec_free_context(&codecCtx);
    }
  }

  /*
   * @return 0 on success, otherwise negative error code:
   *      AVERROR(EAGAIN):   output is not available in the current state - user
   *                         must try to send input
   *      AVERROR_EOF:       the encoder has been fully flushed, and there will be
   *                         no more output packets
   *      AVERROR(EINVAL):   codec not opened, or it is an encoder
   *      other errors: legitimate decoding errors
   */
  int encode(AVFrame *f) {
    static AVPacket *tmpPkt;
    avcodec_send_frame(codecCtx, f);
    if (tmpPkt == nullptr) {
      tmpPkt = av_packet_alloc();
    }
    int ret = avcodec_receive_packet(codecCtx, tmpPkt);
    if (ret == 0) {
      pktList.push_back(tmpPkt);
      tmpPkt = nullptr;
      return 1;
    } else if (ret == AVERROR(EAGAIN)) {
      return 0;
    } else if (ret == AVERROR_EOF) {
      throw std::runtime_error(
          "the encoder has been fully flushed, and there will be no more output packet");
    } else if (ret == AVERROR(EINVAL)) {
      throw std::runtime_error("codec not opened, or it is an encoder");
    } else {
      throw std::runtime_error(fmt::format("legitimate decoding errors: {}", ret));
    }
  }

  AVPacket *getPacket() {
    static AVPacket *tmpPkt;
    AVPacket *rstPkt;
    if (pktList.empty()) {
      if (tmpPkt == nullptr) {
        tmpPkt = av_packet_alloc();
      }
      int ret = avcodec_receive_packet(codecCtx, tmpPkt);
      if (ret == 0) {
        rstPkt = tmpPkt;
        tmpPkt = nullptr;
      } else {
        rstPkt = nullptr;
      }
    } else {
      rstPkt = pktList.front();
      pktList.pop_front();
    }
    return rstPkt;
  }



};