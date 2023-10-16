============
alsa_rnnoise
============
|builds.sr.ht status|

RNNoise_ based noise removal plugin for ALSA

alsa_rnnoise provides ALSA with a high-performance denoiser in real-time,
perfect for VoIP conferences and calls, streaming, screencasting, and chatting
with friends. To hear a demo of RNNoise_ in action, on your voice or a voice
sample, see `this page`_.


.. _RNNoise: https://gitlab.xiph.org/xiph/rnnoise/
.. _`this page`: https://jmvalin.ca/demo/rnnoise/
.. |builds.sr.ht status| image:: https://builds.sr.ht/~arsen/alsa_rnnoise/commits/debian.yml.svg
   :target: https://builds.sr.ht/~arsen/alsa_rnnoise/commits/debian.yml?

Building
--------
``alsa_rnnoise`` depends on ``alsa-lib`` and ``rnnoise``.
You can get development files for ALSA through a package called
``libasound2-dev`` on Debian, ``alsa-lib-devel`` on Fedora, or
``media-libs/alsa-lib`` on Gentoo. RNNoise_ might require manual installation,
since it is not packaged by many distributions. See the RNNoise README_

Please configure RNNoise with ``--prefix=/usr``, or configure
``PKG_CONFIG_PATH`` to include the right path for your prefix.

After installing RNNoise and ALSA, building ``alsa_rnnoise`` is a standard
meson process:

.. code-block:: sh

    $ meson build  # generates the ``build'' directory
    $ ninja -C build
    $ sudo ninja -C build install

.. _README: https://gitlab.xiph.org/xiph/rnnoise/-/blob/master/README

Build options
-------------
Build options are specified via ``-Doption=value`` flags to the ``meson``
invocation.

``plugin_dir``
    ALSA plugin directory location, usually ``/usr/lib/alsa-lib``.
    Depends on ALSA build time configuration
``static_rnnoise``
    Whether to static link RNNoise.
    Enabled by default since RNNoise lacks stable releases.

Usage
-----
The plugin defines the following options:

``wet_dry_control``
    How much of the original audio to mix into the resulting denoised signal,
    range [0, 1], where 0 means none and 1 means all. The loudness of the audio
    should be preserved, but this may affect how nice your recordings sound,
    in terms of comfort noise and speech quality.

There is currently a known bug where if something requests to know the accepted
sample rate interval, the plugin reports an empty interval. Please make sure to
specify the sample rate as exactly ``48000`` as seen in the ``rnnoise`` PCM
below. I'm currently not aware of a way to fix this with ALSAs external plugin
API.

.. code-block::

    pcm.!default {
        type asym
        playback.pcm "cards.pcm.default"
        capture.pcm "plug:rnnoise"
        # there is a short way to add the wet/dry (and other args):
        #     capture.pcm "plug:\"rnnoise:sysdefault,0.5\""
        # without the plug wrapper this becomes ``rnnoise:slave,wetdry'', both
        # of the arguments are optional
        # this is provided by 60-rnnoise.conf, takes two args: slave, wet_dry
        # see 60-rnnoise.conf for the default values
    }

License
-------
``alsa_rnnoise`` is Free software licensed under the GNU General Public
License, version 3.

.. vim: et sw=4 :
