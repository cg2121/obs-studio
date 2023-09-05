#include "audio-helpers.h"

#include <util/dstr.h>

void setup_audio_source(obs_source_t *parent, obs_source_t **child,
			obs_data_t *settings, const char *window, bool enabled,
			enum window_priority priority)
{
	if (enabled) {
		obs_data_t *audio_settings = NULL;
		if (window) {
			audio_settings = obs_data_create();
			obs_data_set_string(audio_settings, "window", window);
			obs_data_set_int(audio_settings, "priority", priority);
		}

		if (!*child) {
			struct dstr name = {0};
			dstr_printf(&name, "%s (%s)",
				    obs_source_get_name(parent),
				    obs_module_text("AudioSuffix"));

			obs_data_t *audio_data =
				obs_data_get_obj(settings, "audio_data");

			// Restore saved source if possible
			if (audio_data) {
				*child = obs_load_private_source(audio_data);
				obs_source_set_name(*child, name.array);

				obs_data_release(audio_data);
			} else {
				*child = obs_source_create_private(
					"wasapi_process_output_capture",
					name.array, NULL);
			}

			obs_source_add_active_child(parent, *child);

			dstr_free(&name);
		}

		// Apply new settings (if any)
		if (audio_settings) {
			obs_source_update(*child, audio_settings);
			obs_data_release(audio_settings);
		}

	} else if (*child) {
		obs_source_remove_active_child(parent, *child);
		// Save audio source settings (filters, etc.) before destroying it
		obs_data_t *audio_data = obs_save_source(*child);
		obs_data_set_obj(settings, "audio_data", audio_data);
		obs_data_release(audio_data);

		obs_source_release(*child);
		*child = NULL;
	}
}

static inline void encode_dstr(struct dstr *str)
{
	dstr_replace(str, "#", "#22");
	dstr_replace(str, ":", "#3A");
}

void reconfigure_audio_source(obs_source_t *source, HWND window)
{
	struct dstr title = {0};
	struct dstr class = {0};
	struct dstr exe = {0};
	struct dstr encoded = {0};

	ms_get_window_title(&title, window);
	ms_get_window_class(&class, window);
	ms_get_window_exe(&exe, window);

	encode_dstr(&title);
	encode_dstr(&class);
	encode_dstr(&exe);

	dstr_cat_dstr(&encoded, &title);
	dstr_cat(&encoded, ":");
	dstr_cat_dstr(&encoded, &class);
	dstr_cat(&encoded, ":");
	dstr_cat_dstr(&encoded, &exe);

	obs_data_t *audio_settings = obs_data_create();
	obs_data_set_string(audio_settings, "window", encoded.array);
	obs_data_set_int(audio_settings, "priority", WINDOW_PRIORITY_CLASS);

	obs_source_update(source, audio_settings);

	obs_data_release(audio_settings);
	dstr_free(&encoded);
	dstr_free(&title);
	dstr_free(&class);
	dstr_free(&exe);
}

void rename_audio_source(void *param, calldata_t *data)
{
	obs_source_t *src = *(obs_source_t **)param;
	if (!src)
		return;

	struct dstr name = {0};
	dstr_printf(&name, "%s (%s)", calldata_string(data, "new_name"),
		    TEXT_CAPTURE_AUDIO_SUFFIX);

	obs_source_set_name(src, name.array);
	dstr_free(&name);
}

bool render_child_audio(obs_source_t *source, uint64_t *ts_out,
			struct obs_source_audio_mix *output, uint32_t mixers,
			size_t channels)
{
	if (obs_source_audio_pending(source))
		return false;

	uint64_t source_ts = obs_source_get_audio_timestamp(source);
	if (!source_ts)
		return false;

	struct obs_source_audio_mix audio;
	obs_source_get_audio_mix(source, &audio);
	for (size_t mix = 0; mix < MAX_AUDIO_MIXES; mix++) {
		if ((mixers & (1 << mix)) == 0) // Skip disabled mixes
			continue;

		memcpy(output->output[mix].data[0], audio.output[mix].data[0],
		       AUDIO_OUTPUT_FRAMES * sizeof(float) * channels);
	}

	*ts_out = source_ts;

	return true;
}
