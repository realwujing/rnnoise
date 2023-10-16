# rnnoise

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

### 卸载

```bash
sudo ./uninstall.sh
```
