#ifndef _MIDIIN_
#define _MIDIIN_

#include <iostream>
#include <string.h>
#include <cmath>
#include <alsa/asoundlib.h>
using namespace std;

class MidiIn
{
private:
    snd_seq_t *seq;
    snd_seq_event_t *ev;
    int myport;
    int myclient;

public:
    int Start(string ClientName) // 根据设备名称来打开midi
    {
        snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
        snd_seq_set_client_name(seq, "MIDI Input");

        myport = snd_seq_create_simple_port(seq, "MIDI Input", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
        myclient = snd_seq_client_id(seq);

        snd_seq_client_info_t *client_info;
        snd_seq_client_info_alloca(&client_info);
        snd_seq_client_info_set_client(client_info, -1);

        int FromClient = -1;
        while (snd_seq_query_next_client(seq, client_info) >= 0)
        {
            string name = snd_seq_client_info_get_name(client_info);

            printf("name:%s\n", name.c_str());

            if (name == ClientName)
            {
                FromClient = snd_seq_client_info_get_client(client_info);
                break;
            }
        }
        if (FromClient == -1)
            return -1;

        int FromPort = snd_seq_client_info_get_num_ports(client_info); // 原来是最大端口数  还要枚举。。。

        printf("ClientID:%d\n", FromClient);
        snd_seq_connect_from(seq, myport, FromClient, 0); // 一般midi设备插上去port默认为0
        return 0;
    }
    void Start(int FromClient, int FromPort) // 24,0
    {
        snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
        snd_seq_set_client_name(seq, "MIDI Input");
        myport = snd_seq_create_simple_port(seq, "MIDI Input", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
        myclient = snd_seq_client_id(seq);
        snd_seq_connect_from(seq, myport, FromClient, FromPort);
    }
    int midimsg() // 接收一个midi事件并返回还有多少个没接受
    {
        return snd_seq_event_input(seq, &ev)>0;
    }
    snd_seq_event_t GetMidiMsg()
    {
        snd_seq_event_t tmp;
        memset(&tmp, 0, sizeof(snd_seq_event_t));
        if (ev == NULL)
            return tmp;
        memcpy(&tmp, ev, sizeof(snd_seq_event_t));
        snd_seq_free_event(ev);
        return tmp;
    }
    void Stop()
    {
        snd_seq_close(seq);
    }
};

#endif