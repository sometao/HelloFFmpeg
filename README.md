### HelloFFmpeg v0.0.1
---




### Goals
- 学习ffmpeg api编程
- 学习音视频的采集


### 功能
- 通过麦克风采集音频，并保存成HLS格式

### TODOs
- [X] 采集音频并保存成PCM
- [X] 采集音频并保存为aac
- [X] 将openAL采集流程封装成一个类
- [X] 引入Resample类
- [X] 将ffmpeg AAC编码过程封装为一个类
- [ ] 给AAC编码添加 ADTS 头：https://blog.csdn.net/chailongger/article/details/84376914
- [ ] 使用ffmpeg muxing api对音频进行封装



### 参考
- ffmpeg官网
- Tutorial: https://think-async.com/Asio/asio-1.16.1/doc/asio/tutorial.html