#ifndef _WAVEOUT_
#define _WAVEOUT_

#include "alsa/asoundlib.h"
#include <iostream>
using namespace std;

class WaveOut
{
private:
    snd_pcm_t *pcmHandle;
    snd_pcm_hw_params_t *params;

public:
    WaveOut(unsigned int SampleRate,unsigned long BufferSize)
    {
        int err = snd_pcm_open(&pcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if (err < 0)
        {
            std::cout << "无法打开PCM设备: " << snd_strerror(err) << std::endl;
            exit(-1);
        }
        snd_pcm_hw_params_alloca(&params);
        snd_pcm_hw_params_any(pcmHandle, params);
        snd_pcm_hw_params_set_access(pcmHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(pcmHandle, params, SND_PCM_FORMAT_FLOAT_LE); // SND_PCM_FORMAT_U16_LE
        snd_pcm_hw_params_set_channels(pcmHandle, params, 2);
        snd_pcm_hw_params_set_rate_near(pcmHandle, params, &SampleRate, 0);
        snd_pcm_hw_params_set_buffer_size(pcmHandle, params, BufferSize);
        
        snd_pcm_hw_params(pcmHandle, params);
    }
    int PlayAudio(const void *buffer, size_t buflen)
    {
        int written = snd_pcm_writei(pcmHandle, buffer, buflen / 2);
        if (written == -EPIPE)
        {
            // 检测到溢出错误，重新启动PCM设备
            snd_pcm_prepare(pcmHandle);
        }
        else if (written < 0)
        {
            std::cout << "写入PCM设备时发生错误: " << snd_strerror(written) << std::endl;
            return -1;
        }
        return 0;
    }
    void Stop()
    {
        // 关闭PCM设备
        snd_pcm_close(pcmHandle);
    }
};

#endif