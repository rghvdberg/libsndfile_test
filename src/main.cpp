
/*
ISC License (ISC)
Copyright <2020> <Rob van den Berg>
Permission to use, copy, modify, and/or distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright notice
and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/


#include <sndfile.hh>
#include <iostream>
#include "include/audio.hpp"
#include <cstring>
#include <vector>
#include <jack/jack.h>
#include <unistd.h>

#define SIGNED_SIZEOF(x) ((int64_t)(sizeof(x)))

using namespace std;

struct VIO_DATA
{
    sf_count_t offset, length;
    unsigned char *data;
};

sf_count_t vfget_filelen(void *user_data);
sf_count_t vfseek(sf_count_t offset, int whence, void *user_data);
sf_count_t vfread(void *ptr, sf_count_t count, void *user_data);
sf_count_t vfwrite(const void *ptr, sf_count_t count, void *user_data);
sf_count_t vftell(void *user_data);

int process(jack_nframes_t nframes, void *);
jack_port_t *outputPort = 0;
int playbackIndex = 0;
vector<float> sample;

int main(void)
{
    SF_VIRTUAL_IO vio;
    vio.get_filelen = vfget_filelen;
    vio.seek = vfseek;
    vio.read = vfread;
    vio.write = vfwrite;
    vio.tell = vftell;

    VIO_DATA vio_data;
    size_t data_length;
    vio_data.data =
      (unsigned char *)
      audio::get_sine_data (&data_length);
    vio_data.length = (sf_count_t) data_length;
    vio_data.offset = 0;

    SndfileHandle file(vio, &vio_data);
    sample.resize(file.frames() * file.channels());
    file.read(&sample.at(0), file.frames());

    free (vio_data.data);

    jack_client_t *client = jack_client_open("LoopedSample", JackNullOption, 0, 0);
    jack_set_process_callback(client, process, 0);
    outputPort = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    jack_activate(client);

    sleep(10);
};

sf_count_t vfget_filelen(void *user_data)
{
    VIO_DATA *vf = (VIO_DATA *)user_data;

    return vf->length;
}

sf_count_t vfseek(sf_count_t offset, int whence, void *user_data)
{
    VIO_DATA *vf = (VIO_DATA *)user_data;

    switch (whence)
    {
    case SEEK_SET:
        vf->offset = offset;
        break;

    case SEEK_CUR:
        vf->offset = vf->offset + offset;
        break;

    case SEEK_END:
        vf->offset = vf->length + offset;
        break;
    default:
        break;
    };

    return vf->offset;
}

sf_count_t vfread(void *ptr, sf_count_t count, void *user_data)
{

    VIO_DATA *vf = (VIO_DATA *)user_data;
    if (vf->offset + count > vf->length)
        count = vf->length - vf->offset;
    memcpy(ptr, vf->data + vf->offset, count);
    vf->offset += count;

    return count;
}

sf_count_t vfwrite(const void *ptr, sf_count_t count, void *user_data)
{
    VIO_DATA *vf = (VIO_DATA *)user_data;

    if (vf->offset >= SIGNED_SIZEOF(vf->data))
        return 0;

    if (vf->offset + count > SIGNED_SIZEOF(vf->data))
        count = sizeof(vf->data) - vf->offset;

    memcpy(vf->data + vf->offset, ptr, (size_t)count);
    vf->offset += count;

    if (vf->offset > vf->length)
        vf->length = vf->offset;

    return count;
}

sf_count_t vftell(void *user_data)
{
    VIO_DATA *vf = (VIO_DATA *)user_data;

    return vf->offset;
}
int process(jack_nframes_t nframes, void *)
{
    float *outputBuffer = (float *)jack_port_get_buffer(outputPort, nframes);

    for (int i = 0; i < (int)nframes; i++)
    {
        if (playbackIndex >= sample.size())
        {
            playbackIndex = 0;
        }

        outputBuffer[i] = sample.at(playbackIndex);
        playbackIndex++;
    }

    return 0;
}
