/* alsa_rnnoise - rnnoise-based alsa noise removal
 * Copyright (C) 2021  Arsen Arsenović <arsen@aarsen.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <alsa/asoundlib.h>
#include <alsa/pcm_external.h>
#include <rnnoise.h>

#ifdef NO_RNNOISE_GET_FRAME_SIZE
/* backwards compat with older rnnoise revisions */
static inline int rnnoise_get_frame_size() {
	return 480;
}
#endif

struct alsa_rnnoise_info {
	snd_pcm_extplug_t ext;
	/* instance and intermedate buffer */
	DenoiseState *rnnoise;

	/* factor of original audio to let through mixed with filtered audio */
	double wet_dry_factor;

	float *buf;
	float *srcbuf;
	size_t filled;
};

static void *area_start_address(
	const snd_pcm_channel_area_t *area,
	snd_pcm_uframes_t offset
) {
	size_t bit_offset = area->first + area->step * offset;
	assert(bit_offset % 8 == 0); /* always true? */
	return (char *) area->addr + bit_offset / 8;
}

static snd_pcm_sframes_t
arnn_transfer(
	snd_pcm_extplug_t *ext,
	const snd_pcm_channel_area_t *dst_area,
	snd_pcm_uframes_t dst_offset,
	const snd_pcm_channel_area_t *src_area,
	snd_pcm_uframes_t src_offset,
	snd_pcm_uframes_t size
) {
	struct alsa_rnnoise_info *pdata = ext->private_data;
	float *src = area_start_address(src_area, src_offset);
	float *dst = area_start_address(dst_area, dst_offset);
	size_t count = size;

	/* this is pretty much the algorithm in pcm_speex.c in alsa-plugins
	 * I've reorganized it to fit code style and added code to handle the
	 * S16 -> fake float conversion.
	 */
	while (count > 0) {
		size_t chunk = rnnoise_get_frame_size() - pdata->filled;
		if (chunk > count) {
			chunk = count;
		}

		for (size_t i = 0; i < chunk; i++) {
			dst[i] = pdata->buf[pdata->filled + i] / 32768;
			pdata->buf[pdata->filled + i] = src[i];
		}
		dst += chunk;

		for (size_t i = 0; i < chunk; i++) {
			pdata->buf[pdata->filled + i] = src[i];
			pdata->srcbuf[pdata->filled + i] = src[i];
		}
		pdata->filled += chunk;

		src += chunk;
		count -= chunk;

		if (pdata->filled != rnnoise_get_frame_size()) {
			continue;
		}

		for (size_t i = 0; i < rnnoise_get_frame_size(); i++) {
			pdata->buf[i] *= 32768;
		}

		rnnoise_process_frame(pdata->rnnoise, pdata->buf, pdata->buf);
		pdata->filled = 0;

		if (pdata->wet_dry_factor == 0) {
			continue;
		}

		for (size_t i = 0; i < rnnoise_get_frame_size(); i++) {
			pdata->buf[i] *= 1 - pdata->wet_dry_factor;
			pdata->buf[i] += pdata->wet_dry_factor
					* pdata->srcbuf[i];
		}
	}

	return size;
}

static int arnn_init_buf(float **buf, size_t s) {
	*buf = realloc(*buf, s * sizeof(float));
	if (!*buf) {
		return -ENOMEM;
	}
	memset(*buf, 0, s * sizeof(float));
	return 0;
}

static int arnn_init(snd_pcm_extplug_t *ext) {
	struct alsa_rnnoise_info *pdata = ext->private_data;
	int r;

	pdata->filled = 0;

	r = arnn_init_buf(&pdata->buf, rnnoise_get_frame_size());
	if (r) {
		return r;
	}

	r = arnn_init_buf(&pdata->srcbuf, rnnoise_get_frame_size());
	if (r) {
		return r;
	}

	if (pdata->rnnoise) {
		rnnoise_destroy(pdata->rnnoise);
	}

	pdata->rnnoise = rnnoise_create(NULL);
	if (!pdata->rnnoise) {
		return -ENOMEM;
	}
	return 0;
}

static int arnn_close(snd_pcm_extplug_t *ext) {
	struct alsa_rnnoise_info *pdata = ext->private_data;
	free(pdata->buf);
	free(pdata->srcbuf);
	/* rnnoise_destroy does not null check */
	if (pdata->rnnoise) {
		rnnoise_destroy(pdata->rnnoise);
	}
	return 0;
}

static const snd_pcm_extplug_callback_t rnnoise_callback = {
	.transfer = arnn_transfer,
	.init = arnn_init,
	.close = arnn_close,
};

SND_PCM_PLUGIN_DEFINE_FUNC(rnnoise) {
	snd_config_iterator_t i, next;
	struct alsa_rnnoise_info *arnn;
	snd_config_t *slave = NULL;
	int err;

	arnn = calloc(1, sizeof(*arnn));
	if (!arnn) {
		return -ENOMEM;
	}

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0) {
			continue;
		}
		if (strcmp(id, "comment") == 0 || strcmp(id, "type") == 0
			|| strcmp(id, "hint") == 0) {
			continue;
		}
		if (strcmp(id, "slave") == 0) {
			slave = n;
			continue;
		}
		if (strcmp(id, "wet_dry_factor") == 0) {
			int r = snd_config_get_ireal(n, &arnn->wet_dry_factor);
			if (r >= 0) {
				continue;
			}
			if (arnn->wet_dry_factor > 1
				|| arnn->wet_dry_factor < 0) {
				SNDERR("wet_dry_factor out of range [0, 1]");
				return -EINVAL;
			}
			SNDERR("expected double for wet_dry_factor");
			return -EINVAL;
		}
		SNDERR("unknown field %s", id);
		return -EINVAL;
	}

	if (!slave) {
		SNDERR("no slave configuration for rnnoise pcm");
		return -EINVAL;
	}

	arnn->ext.version = SND_PCM_EXTPLUG_VERSION;
	arnn->ext.name = "rnnoise-based denoiser for alsa";
	arnn->ext.callback = &rnnoise_callback;
	arnn->ext.private_data = arnn;

	err = snd_pcm_extplug_create(&arnn->ext, name, root, slave,
				     stream, mode);
	if (err < 0) {
		free(arnn);
		return err;
	}

	snd_pcm_extplug_set_param(&arnn->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 1);
	snd_pcm_extplug_set_slave_param(&arnn->ext,
					SND_PCM_EXTPLUG_HW_CHANNELS, 1);

	/* RNNoise, for some reason, does not work for floats ranged -1 to 1
	 * but does work with floats in the int16_t range, so we request that
	 * and convert to floats in the transfer function
	 */
	snd_pcm_extplug_set_param(&arnn->ext, SND_PCM_EXTPLUG_HW_FORMAT,
				  SND_PCM_FORMAT_FLOAT);
	snd_pcm_extplug_set_slave_param(&arnn->ext, SND_PCM_EXTPLUG_HW_FORMAT,
					SND_PCM_FORMAT_FLOAT);

	*pcmp = arnn->ext.pcm;
	return 0;
}

SND_PCM_PLUGIN_SYMBOL(rnnoise);
