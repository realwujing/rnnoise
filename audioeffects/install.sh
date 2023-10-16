#!/bin/sh


#rnnCfgFile='/etc/alsa/conf.d/60-rnnoise.conf'
#if [ -f "$rnnCfgFile" ]; then
#	echo "audio effects config file already exists."
#	rm -f /etc/alsa/conf.d/60-rnnoise.conf
	
#fi
cp -f 60-rnnoise.conf /etc/alsa/conf.d/60-rnnoise.conf

#rnnLibFile= '/usr/lib/x86_64-linux-gnu/alsa-lib/libasound_module_pcm_rnnoise.so'
#if [ -f "$rnnLibFile" ];then
#	rm -f /usr/lib/x86_64-linux-gnu/alsa-lib/libasound_module_pcm_rnnoise.so
#fi

cp libasound_module_pcm_rnnoise.so /usr/lib/x86_64-linux-gnu/alsa-lib/libasound_module_pcm_rnnoise.so

echo "Now installing the asound.conf file..."
#alsaCfg= '/etc/asound.conf'
#if [ -f "$alsaCfg" ];then
#	echo "alsa config file already exists in /etc/asound.conf"
rm -f /etc/asound.conf
#fi
cp -f asound.conf /etc/asound.conf

#pulseCfgFile= '/usr/share/alsa/pulse-alsa.conf'
#if [ -f "$pulseCfgFile" ];then
#	echo "PulseAudio file exists. Now to remove it."
#mv -f /usr/share/alsa/pulse-alsa.conf /usr/share/alsa/pulse-alsa.conf.bbk
#fi

#if [ ! -f "$pulseCfgFile" ]; then
	cp -f pulse-alsa.conf /usr/share/alsa/pulse-alsa.conf
#fi

echo "Install audio effects finished."





