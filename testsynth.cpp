#include <stdio.h>
#include <iostream>

#include "WaveOut.h"
#include "MidiIn.h"

#include "VCO.h"
#include "Filter.h"
#include "ADSR.h"
#include "limit.h"

using namespace std;
#define SAMPLE_RATE (48000)

#define buflen (160)
float wavbuf[buflen];

WaveOut hWout(SAMPLE_RATE, 2048);
MidiIn hMin;

#define MaxPolyN (6)
VCO vcos[MaxPolyN];
Filter filts_l[MaxPolyN];
Filter filts_r[MaxPolyN];
// IIRFilter filts_l[MaxPolyN];
// IIRFilter filts_r[MaxPolyN];
IIRFilter hpfs_l;
IIRFilter hpfs_r;
ADSR enves[MaxPolyN];
float vols[MaxPolyN];
float freqs[MaxPolyN];
int pitchs[MaxPolyN];
int pos = 0;

float vunin = 3, vdtune = 0.725, vctof = 1.0, vreso = 0.5;
float va = 0, vd = 0, vs = 0, vr = 0;
int main()
{
    hMin.Start("MPK mini 3");
    for (int t = 0;;)
    {
        while (hMin.midimsg()) // midi state
        {
            snd_seq_event_t dat = hMin.GetMidiMsg();
            if (dat.type == SND_SEQ_EVENT_CONTROLLER)
            {
                // printf("ID:%d Param:%d\n", dat.data.control.param, dat.data.control.value);
                int ID = dat.data.control.param;
                float Value = (float)dat.data.control.value / 127.0;
                if (ID == 70)
                    vunin = Value * 8 + 1;
                else if (ID == 71)
                    vdtune = Value * 1.5;
                else if (ID == 72)
                    // vctof = Value * Value * 1.5;
                    vctof = Value * Value * 0.995 * 1.5;
                else if (ID == 73)
                    // vreso = Value * 16;
                    vreso = sqrt(Value);
                else if (ID == 74)
                    va = Value * Value * Value * Value * 0.25;
                else if (ID == 75)
                    vd = Value * Value * Value * Value * 0.25;
                else if (ID == 76)
                    vs = Value * Value;
                else if (ID == 77)
                    vr = Value * Value * Value * Value * 0.25;
            }
            if (dat.type == SND_SEQ_EVENT_NOTEON)
            {
                int pitch = dat.data.note.note;
                freqs[pos] = 440.0 * pow(2.0, (float)(pitch - 3 - 12 * 6) / 12.0);
                pitchs[pos] = pitch;
                enves[pos].SetGate(1);
                vols[pos] = (dat.data.note.velocity / 128.0) * 0.4 + 0.6;
                printf("pitch:%d freq:%.5f\n", pitch, freqs[pos]);
                pos = (pos + 1) % MaxPolyN;
            }
            else if (dat.type == SND_SEQ_EVENT_NOTEOFF)
            {
                int pitch = dat.data.note.note;
                for (int i = 0; i < MaxPolyN; ++i)
                {
                    if (pitchs[i] == pitch)
                    {
                        enves[i].SetGate(0);
                        pitchs[i] = 0;
                        printf("pitch:%d noteoff\n", pitch);
                    }
                }
            }
        }

        for (int i = 0; i < MaxPolyN; ++i)
        {
            enves[i].SetADSR(va, vd, vs, vr);
            /*
                        float t_ctof = enves[i].proc() * vctof;
                        float t_reso = vreso;
                        t_reso *= 1.0 - pow(1.0 - t_ctof, 16.0); // 包络
                        t_ctof += 0.0 / SAMPLE_RATE;

                        if (t_ctof > 1.0)
                            t_ctof = 1.0;
                        if (t_ctof < 0.0)
                            t_ctof = 0.0;

                        filts_l[i].UpdataLPF(t_ctof, t_reso + 0.125);
                        filts_r[i].UpdataLPF(t_ctof, t_reso + 0.125);
            */
        }
        hpfs_l.UpdataHPF(5.0 / SAMPLE_RATE, 0.5);
        hpfs_r.UpdataHPF(5.0 / SAMPLE_RATE, 0.5);

        for (int i = 0; i < buflen; i += 2, t++)
        {
            float datl = 0, datr = 0;
            for (int j = 0; j < MaxPolyN; ++j)
            {
                StereoSignal sign = vcos[j].Saw(freqs[j], vunin, vdtune);
                float tmpl = sign.l * vols[j];
                float tmpr = sign.r * vols[j];
                // tmpl = filts_l[j].proc(tmpl);
                // tmpr = filts_r[j].proc(tmpr);
                float tctof = enves[j].proc() * vctof;
                tmpl = filts_l[j].LPF2_Oversampling_ResoLimit_limit(tmpl, tctof, vreso);
                tmpr = filts_r[j].LPF2_Oversampling_ResoLimit_limit(tmpr, tctof, vreso);
                // tmpl *= tctof;
                // tmpr *= tctof;
                datl += tmpl;
                datr += tmpr;
            }
             datl = hpfs_l.proc(datl);
            datr = hpfs_r.proc(datr);

            // limit
            datl = limiter(datl / 131072.0 / 4.0, 2.5, 0.25);
            datr = limiter(datr / 131072.0 / 4.0, 2.5, 0.25);

            wavbuf[i + 0] = datl;
            wavbuf[i + 1] = datr;
        }
        // printf ("aaa\n");
        hWout.PlayAudio(wavbuf, buflen);
    }
}