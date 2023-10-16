# rnnoise

## 安装依赖

```bash
sudo apt install meson cmake autoconf libtool libasound2-dev audacity
```

## 获取源码

```bash
git clone https://github.com/realwujing/rnnoise.git
git submodule update --init --recursive
```

## rnnoise

```bash
cd rnnoise
./autogen.sh
export CFLAGS="-g -O0 -fPIC"
export LDFLAGS="-g -O0 -fPIC"
mkdir build
../configure
make
sudo make install
```

## alsa_rnnoise-v1.0

```bash
cd alsa_rnnoise-v1.0
meson build  # generates the ``build'' directory
ninja -C build
```

## audioeffects

### 安装

```bash
cd audioeffects
cp ../alsa_rnnoise-v1.0/build/libasound_module_pcm_rnnoise.so .
sudo ./install.sh
```

### 测试

```bash
pulseaudio -k
audacity
```

### 卸载

```bash
sudo ./uninstall.sh
```

## More

- [https://github.com/xiph/rnnoise](https://github.com/xiph/rnnoise)
- [https://sr.ht/~arsen/alsa_rnnoise/](https://sr.ht/~arsen/alsa_rnnoise/)
- [语音降噪是如何扼住噪声的咽喉的](https://www.toutiao.com/article/7189091215299019320)
- [https://github.com/werman/noise-suppression-for-voice](https://github.com/werman/noise-suppression-for-voice)
