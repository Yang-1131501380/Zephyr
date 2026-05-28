# Rocket-Pi Zephyr Learning

Zephyr learning workspace application for the Rocket-Pi STM32F401RE board.

This repository follows Zephyr's workspace application style: the application
is outside the `zephyr/` source tree, and the custom Rocket-Pi board port is
kept in this application repository under `boards/`.

## Build

From the west workspace root:

```sh
env CCACHE_DISABLE=1 .venv/bin/west build \
  -p always \
  -b rocket_pi \
  apps/rocketpi_zephyr_learning \
  -d build/rocketpi_zephyr_learning \
  -- -DCMAKE_C_COMPILER_LAUNCHER= -DCMAKE_CXX_COMPILER_LAUNCHER=
```

## Flash

```sh
.venv/bin/west flash -d build/rocketpi_zephyr_learning -r openocd
```

## Serial Console

The board console is USART2 at 115200 baud.

```sh
picocom -b 115200 /dev/ttyACM0
```

## Layout

```text
CMakeLists.txt
prj.conf
VERSION
src/main.c
boards/others/rocket_pi/
```
