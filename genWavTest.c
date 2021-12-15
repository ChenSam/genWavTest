#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define FORMAT_PCM 1

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};



int main(int argc, char **argv)
{


    FILE *file;
    struct wav_header header;
    unsigned int channels = 2;
    unsigned int rate = 44100;
    unsigned int bits = 16;
    unsigned int frames;
    unsigned int patten;
    unsigned int cap_time;
    char buffer[4] = {0};
    unsigned int size;


    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.wav"
                " [-c channels] [-r rate] [-b bits]"
                " [-T capture time]\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "wb");
    if (!file) {
        fprintf(stderr, "Unable to create file '%s'\n", argv[1]);
        return 1;
    }

    /* parse command line arguments */
    argv += 2;
    while (*argv) {
        if (strcmp(*argv, "-r") == 0) {
            argv++;
            rate = atoi(*argv);
        } else if (strcmp(*argv, "-c") == 0) {
            argv++;
            channels = atoi(*argv);
        } else if (strcmp(*argv, "-b") == 0) {
            argv++;
            bits = atoi(*argv);
        } else if (strcmp(*argv, "-p") == 0) {
            argv++;
            unsigned long tmp;
            tmp = strtoul(*argv, NULL, 16);
            patten = (unsigned int) tmp;
            printf("patten=%x\n", patten);
        } else if (strcmp(*argv, "-T") == 0) {
            argv++;
            if (*argv)
                cap_time = atoi(*argv);
        }
        argv++;
    }

    header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = FORMAT_PCM;
    header.num_channels = channels;
    header.sample_rate = rate;
    header.bits_per_sample = bits;
    header.byte_rate = (header.bits_per_sample / 8) * channels * rate;
    header.block_align = channels * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;

    /* leave enough room for header */
    fseek(file, sizeof(struct wav_header), SEEK_SET);

    if (bits == 16) {
        *buffer = (char) (patten & 0xff);
        *(buffer + 1) = (char) ((patten >> 8) & 0xff);
        printf("16bit buf=%x\n", (short)*buffer);
        size = 2;
    } else {
        *buffer = (char) (patten & 0xff);
        *(buffer + 1) = (char) ((patten >> 8) & 0xff);
        *(buffer + 2) = (char) ((patten >> 16) & 0xff);
        *(buffer + 3) = (char) ((patten >> 24) & 0xff);
        printf("32bit buf=%x\n", *buffer);
        size = 4;
    }

    for (int i = 0; i < 4; i ++) {
        printf("buffer[%d]=%x\n", i, buffer[i] & 0xff);
    }

    frames = channels * rate * cap_time;

    for (int i = 0; i < frames; i ++) {
        if (fwrite(buffer, 1, size, file) != size) {
            fprintf(stderr,"Error writing sample\n");
            break;
        }
    }

    /* write header now all information is known */
    header.data_sz = frames * header.block_align;
    header.riff_sz = header.data_sz + sizeof(header) - 8;
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct wav_header), 1, file);
    fclose(file);


    return 0;
}
