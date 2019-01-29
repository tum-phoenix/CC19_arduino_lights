[![Build Status](https://travis-ci.org/tum-phoenix/CC19_arduino_lights.svg?branch=master)](https://travis-ci.org/tum-phoenix/CC19_arduino_lights)


# Introduction
TUM Phoenix Head- and Rearlight firmware repository for Arduino Leonardo compatible boards! Open this directory as project with [PlatformIO](https://platformio.org/). It is structured in two [environments](http://docs.platformio.org/en/latest/projectconf/section_env.html) for front and rear light.

## Libraries
Environment specific libraries can be specified in `platformio.ini`.

## Sources
Sources are stored in `src/<environment>`. Add new environments in the `platformio.ini` file.
To only build / upload one specific environment you can:
* recommended: use the command line ([more info](http://docs.platformio.org/en/latest/userguide/cmd_run.html#cmdoption-platformio-run-e)) `pio run -e <env>` (build) and `pio run -t upload -e <env>` (upload) for uploading you will need to specify the board and Serial Port in the right bottom corner
* uncomment all other environments
* use `env_default` ([more info](http://docs.platformio.org/en/latest/projectconf/section_platformio.html#projectconf-pio-env-default))
