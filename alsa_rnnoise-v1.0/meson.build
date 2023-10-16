project('alsa_rnnoise', 'c', version: '1.0')
cc = meson.get_compiler('c')

static_rnnoise = get_option('static_rnnoise')
pdir = get_option('plugin_dir')
cdir = get_option('config_dir')

libm = dependency('', required: false)
rnnoise = dependency('rnnoise', static: static_rnnoise)
alsa = dependency('alsa')

if static_rnnoise
	# the rnnoise .pc does not declare this dep
	libm = cc.find_library('m', required: false)
endif

if pdir == ''
	pdir = get_option('prefix') / get_option('libdir') / 'alsa-lib'
endif

if cdir == ''
	cdir = get_option('prefix') / get_option('datadir') \
		/ 'alsa/alsa.conf.d'
endif

has_frame_func = cc.has_function('rnnoise_get_frame_size',
				prefix: '#include <rnnoise.h>',
				dependencies: [rnnoise, libm])

if not has_frame_func
	add_project_arguments('-DNO_RNNOISE_GET_FRAME_SIZE', language: 'c')
endif

summary({
	'plugin directory': pdir,
	'config directory': cdir,
	'statically link rnnoise': static_rnnoise,
	'rnnoise_get_frame_size() detected?': has_frame_func
}, section: 'build configuration')

shared_library('asound_module_pcm_rnnoise',
	'src/main.c',
	dependencies: [rnnoise, alsa, libm],
	install_dir: pdir,
	c_args: ['-DPIC'],
	install: true)

install_data('60-rnnoise.conf',
	install_dir: cdir)

# vim: sw=8 noet :
