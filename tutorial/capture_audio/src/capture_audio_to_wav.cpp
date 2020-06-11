
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <iostream>
#include "al.h"
#include "alc.h"
#include "seeker/loggerApi.h"
#include "seeker/common.h"
#include "wav_reader.hpp"
#include "wav_writer.hpp"

#define OUTPUT_WAVE_FILE "Capture.wav"
#define BUFFERSIZE 4410

int capture_to_wav() {
  ALCdevice *pDevice;
  ALCcontext *pContext;
  ALCdevice *pCaptureDevice;
  const ALCchar *szDefaultCaptureDevice;
  ALint iSamplesAvailable;
  FILE *pFile;
  ALchar Buffer[BUFFERSIZE];



  const int channels = 1;
  const int sampleRate = 22050;
  const int bitsPerSample = 16;
  const int blockAlign = channels * bitsPerSample / 8 ;

  // NOTE : This code does NOT setup the Wave Device's Audio Mixer to select a recording input
  // or a recording level.


  // Check for Capture Extension support
  pContext = alcGetCurrentContext();
  pDevice = alcGetContextsDevice(pContext);
  if (alcIsExtensionPresent(pDevice, "ALC_EXT_CAPTURE") == AL_FALSE) {
    printf("Failed to detect Capture Extension\n");
    return 0;
  }

  // Get list of available Capture Devices
  const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
  if (pDeviceList) {
    printf("\nAvailable Capture Devices are:-\n");

    while (*pDeviceList) {
      printf("%s\n", pDeviceList);
      pDeviceList += strlen(pDeviceList) + 1;
    }
  }

  // Get the name of the 'default' capture device
  szDefaultCaptureDevice = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
  printf("\nDefault Capture Device is '%s'\n\n", szDefaultCaptureDevice);

  // Open the default Capture device to record a 22050Hz 16bit Mono Stream using an internal
  // buffer of BUFFERSIZE Samples (== BUFFERSIZE * 2 bytes)
  pCaptureDevice =
    alcCaptureOpenDevice(szDefaultCaptureDevice, sampleRate, AL_FORMAT_MONO16, BUFFERSIZE);


  if (pCaptureDevice) {
    I_LOG("Opened '{}' Capture Device\n\n",  alcGetString(pCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER));
    // Create / open a file for the captured data
    void *wavWriter = wav_write_open(OUTPUT_WAVE_FILE, sampleRate, bitsPerSample, channels);


    auto b = Buffer;

    auto writeSamples = [=](size_t sampleNumber){
      // Consume Samples
      alcCaptureSamples(pCaptureDevice, (unsigned char *)&Buffer, sampleNumber);

      // Write the audio data to a file
      wav_write_data(wavWriter, (unsigned char *)&Buffer, sampleNumber * blockAlign);
    };

    // Start audio capture
    alcCaptureStart(pCaptureDevice);

    // Record for 5 seconds
    auto dwStartTime = seeker::Time::currentTime();
    while ((seeker::Time::currentTime() <= (dwStartTime + 5000))) {
      // Release some CPU time ...
      Sleep(2);

      // Find out how many samples have been captured
      alcGetIntegerv(pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &iSamplesAvailable);

      D_LOG("Samples available: {} ? {}", iSamplesAvailable, (BUFFERSIZE / blockAlign));

      // When we have enough data to fill our BUFFERSIZE byte buffer, grab the samples
      if (iSamplesAvailable > (BUFFERSIZE / blockAlign)) {
        writeSamples(BUFFERSIZE / blockAlign);
      }
    }

    // Stop capture
    alcCaptureStop(pCaptureDevice);

    // Check if any Samples haven't been consumed yet
    alcGetIntegerv(pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &iSamplesAvailable);
    while (iSamplesAvailable) {
      if (iSamplesAvailable > (BUFFERSIZE / blockAlign )) {
        writeSamples(BUFFERSIZE / blockAlign);
        iSamplesAvailable -= (BUFFERSIZE / blockAlign);
      } else {
        writeSamples(iSamplesAvailable);
        iSamplesAvailable = 0;
      }
    }


    I_LOG("Saved captured audio data to {}", OUTPUT_WAVE_FILE);
    wav_write_close(wavWriter);

    // Close the Capture Device
    alcCaptureCloseDevice(pCaptureDevice);
  }


  return 0;
}