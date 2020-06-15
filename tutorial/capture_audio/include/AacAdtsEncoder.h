/**
@author Tao Zhang
@since 2020/6/8
@version 0.0.1-SNAPSHOT 2020/6/15
*/
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

// TODO to be test.

class AacAdtsEncoder {
 private:
  AVCodecContext *codecCtx;
  std::list<AVPacket *> pktList{};
  AVFrame *tmpFrame;

  bool finished = false;

  int bitsPerSample;

  static AVFrame *allocAudioFrame(AVSampleFormat sample_fmt, uint64_t channel_layout,
                                  int sample_rate, int nb_samples) {
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
      throw std::runtime_error("Error allocating an audio frame");
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
      ret = av_frame_get_buffer(frame, 0);
      if (ret < 0) {
        throw std::runtime_error("AacAdtsEncoder: Error allocating an audio buffer");
      }
    }

    D_LOG("allocAudioFrame, frame->linesize[0]={}", frame->linesize[0]);

    return frame;
  }

 public:
  AacAdtsEncoder(int64_t bitRate = 64000,
                 AVSampleFormat sampleFmt = AV_SAMPLE_FMT_S16,
                 int sampleRate = 44100,
                 int channels = 1 /*only support 1 and 2*/) {
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (codec == nullptr) {
      throw std::runtime_error(fmt::format("AacAdtsEncoder. Could not find codec:{}", (int)AV_CODEC_ID_AAC));
    }


    codecCtx = avcodec_alloc_context3(codec);

    codecCtx->frame_size = 1024;

    codecCtx->sample_fmt = sampleFmt;

    codecCtx->bit_rate = bitRate;

    codecCtx->sample_rate = sampleRate;

    codecCtx->channels = channels;

    if (channels == 1) {
      codecCtx->channel_layout = AV_CH_LAYOUT_MONO;
    } else if (channels == 2) {
      codecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    } else {
      throw std::runtime_error(fmt::format("AacAdtsEncoder: channels must be 1 or 2, but got [{}]", channels));
    }

    //FIXME need resamples
    //AAC only support AV_SAMPLE_FMT_FLTP.


    switch (sampleFmt) {
      case AV_SAMPLE_FMT_FLTP:
        bitsPerSample = 32;
        break;
      case AV_SAMPLE_FMT_U8:
      case AV_SAMPLE_FMT_U8P:
      case AV_SAMPLE_FMT_S16:
      case AV_SAMPLE_FMT_S16P:
      case AV_SAMPLE_FMT_S32:
      case AV_SAMPLE_FMT_FLT:
      case AV_SAMPLE_FMT_NONE:
      case AV_SAMPLE_FMT_DBL:
      case AV_SAMPLE_FMT_S32P:
      case AV_SAMPLE_FMT_DBLP:
      case AV_SAMPLE_FMT_S64:
      case AV_SAMPLE_FMT_S64P:
      case AV_SAMPLE_FMT_NB:
        throw std::runtime_error(fmt::format("AacAdtsEncoder: unsupported fmt: {}", sampleFmt));
      default:
        throw std::runtime_error(fmt::format("AacAdtsEncoder: unknown fmt: {}", sampleFmt));
    }

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
      throw std::runtime_error(fmt::format("AacAdtsEncoder: Could not open codec: {}", codec->name));
    }

    tmpFrame =
        allocAudioFrame(sampleFmt, codecCtx->channel_layout, sampleRate, codecCtx->frame_size);

    D_LOG("codecCtx->frame_size = {}", codecCtx->frame_size);
  }


  ~AacAdtsEncoder() {
    if (codecCtx != nullptr) {
      avcodec_free_context(&codecCtx);
    }
    if (tmpFrame != nullptr) {
      av_frame_free(&tmpFrame);
    }
  }

  /*
   * PCM format :https://blog.csdn.net/haima1998/article/details/49103055
   * @return 1 got pkt,
   *         0 no pkt got.
   */
  int encode(uint8_t *buf, const size_t bufSize, bool endOfStream = false) {
    if (finished) {
      throw std::runtime_error("AacAdtsEncoder: data stream has been finished.");
    }

    static AVPacket *tmpPkt;

    //size_t expectedSize = getFrameSize() * codecCtx->channels * bitsPerSample / 8;
    //if (bufSize != expectedSize) {
    //  throw std::runtime_error(
    //      fmt::format("AacAdtsEncoder: encode error: bufSize[{}] != expectedSize[{}]", bufSize, expectedSize));
    //}

    // fill data into frame.
    memcpy((int8_t *)tmpFrame->data[0], buf, bufSize);

    int ret;
    ret = avcodec_send_frame(codecCtx, tmpFrame);
    if (ret == AVERROR(EAGAIN)) {
      throw std::runtime_error("AacAdtsEncoder: input is not accepted in the current state");
    } else if (ret == AVERROR_EOF) {
      throw std::runtime_error(
          "the encoder has been flushed, and no new frames can be sent to it");
    } else if (ret == AVERROR(EINVAL)) {
      throw std::runtime_error(
          "AacAdtsEncoder: codec not opened, refcounted_frames not set, it is a decoder, or requires flush");
    } else if (ret == AVERROR(ENOMEM)) {
      throw std::runtime_error("failed to add packet to internal queue, or similar");
    } else if (ret < 0) {
      throw std::runtime_error("AacAdtsEncoder: legitimate decoding errors: " + ret);
    }

    if (endOfStream) {
      avcodec_send_frame(codecCtx, nullptr);
      finished = true;
    }

    if (tmpPkt == nullptr) {
      tmpPkt = av_packet_alloc();
    }
    ret = avcodec_receive_packet(codecCtx, tmpPkt);
    if (ret == 0) {
      pktList.push_back(tmpPkt);
      tmpPkt = nullptr;
      return 1;
    } else if (ret == AVERROR(EAGAIN)) {
      I_LOG("output is not available in the current state, user must try to send input");
      return 0;
    } else if (ret == AVERROR_EOF) {
      W_LOG("the encoder has been fully flushed, and there will be no more output packet");
      return 0;
    } else if (ret == AVERROR(EINVAL)) {
      throw std::runtime_error("AacAdtsEncoder: codec not opened, or it is an encoder");
    } else {
      throw std::runtime_error(fmt::format("AacAdtsEncoder: legitimate decoding errors: {}", ret));
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
      } else if (ret == AVERROR(EAGAIN)) {
        I_LOG("output is not available in the current state, user must try to send input");
        rstPkt = nullptr;
      } else if (ret == AVERROR_EOF) {
        W_LOG("the encoder has been fully flushed, and there will be no more output packets");
        rstPkt = nullptr;
      } else if (ret == AVERROR(EINVAL)) {
        throw std::runtime_error("AacAdtsEncoder: codec not opened, or it is an encoder");
      } else {
        throw std::runtime_error("AacAdtsEncoder: legitimate decoding errors");
      }
    } else {
      rstPkt = pktList.front();
      pktList.pop_front();
    }
    return rstPkt;
  }

  bool isFinished() { return finished; }

  int getFrameSize() { return codecCtx->frame_size; }
};