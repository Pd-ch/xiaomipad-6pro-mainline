// SPDX-License-Identifier: MIT
/*
 * Open the liuqin AudioReach FE, apply the vendor-backed format, prepare it,
 * and close without writing or starting a PCM stream.  This is a diagnostic
 * admission probe, not a playback program.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct _snd_pcm snd_pcm_t;
typedef struct _snd_pcm_hw_params snd_pcm_hw_params_t;

extern int snd_pcm_open(snd_pcm_t **pcm, const char *name, int stream, int mode);
extern int snd_pcm_close(snd_pcm_t *pcm);
extern int snd_pcm_prepare(snd_pcm_t *pcm);
extern int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **ptr);
extern void snd_pcm_hw_params_free(snd_pcm_hw_params_t *obj);
extern int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
extern int snd_pcm_hw_params_set_access(snd_pcm_t *pcm,
					snd_pcm_hw_params_t *params, int access);
extern int snd_pcm_hw_params_set_format(snd_pcm_t *pcm,
					snd_pcm_hw_params_t *params, int format);
extern int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm,
				      snd_pcm_hw_params_t *params,
				      unsigned int rate, int dir);
extern int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm,
					  snd_pcm_hw_params_t *params,
					  unsigned int channels);
extern int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
extern const char *snd_strerror(int errnum);

enum {
	SND_PCM_STREAM_PLAYBACK = 0,
	SND_PCM_ACCESS_RW_INTERLEAVED = 3,
	SND_PCM_FORMAT_S24_LE = 6,
};

static int failed(const char *stage, int error)
{
	fprintf(stderr, "liuqin-audio-hwparams-probe: %s: %s (%d)\n",
		stage, snd_strerror(error), error);
	return 1;
}

int main(int argc, char **argv)
{
	snd_pcm_hw_params_t *params = NULL;
	snd_pcm_t *pcm = NULL;
	int ret;

	if (argc == 2 && !strcmp(argv[1], "--help")) {
		puts("usage: liuqin-audio-hwparams-probe");
		puts("opens hw:0,0 as S24_LE/48000/4ch, prepares, writes no frames, closes");
		return 0;
	}
	if (argc != 1)
		return 64;

	ret = snd_pcm_open(&pcm, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0)
		return failed("snd_pcm_open", ret);
	ret = snd_pcm_hw_params_malloc(&params);
	if (ret < 0)
		goto fail;
	ret = snd_pcm_hw_params_any(pcm, params);
	if (ret < 0)
		goto fail;
	ret = snd_pcm_hw_params_set_access(pcm, params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0)
		goto fail;
	ret = snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_S24_LE);
	if (ret < 0)
		goto fail;
	ret = snd_pcm_hw_params_set_rate(pcm, params, 48000, 0);
	if (ret < 0)
		goto fail;
	ret = snd_pcm_hw_params_set_channels(pcm, params, 4);
	if (ret < 0)
		goto fail;
	ret = snd_pcm_hw_params(pcm, params);
	if (ret == -EOPNOTSUPP) {
		/*
		 * This probe accepts only the firmware's legacy TDM module
		 * identity, not its private configuration payload.  Reaching this
		 * marker proves GRAPH_OPEN accepted that identity and the kernel
		 * stopped locally before SET_CFG/PREPARE/START or any audio frame.
		 */
		puts("__LIUQIN_AUDIO_LEGACY_GRAPH_OPEN_PASS_CONFIG_UNSUPPORTED__");
		goto fail;
	}
	if (ret < 0)
		goto fail;
	ret = snd_pcm_prepare(pcm);
	if (ret < 0)
		goto fail;

	snd_pcm_hw_params_free(params);
	ret = snd_pcm_close(pcm);
	if (ret < 0)
		return failed("snd_pcm_close", ret);
	puts("__LIUQIN_AUDIO_HWPARAMS_PREPARE_PASS_NO_FRAMES__");
	return 0;

fail:
	if (params)
		snd_pcm_hw_params_free(params);
	if (pcm)
		(void)snd_pcm_close(pcm);
	return failed("hwparams-or-prepare", ret);
}
