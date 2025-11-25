# MiraTherm radiator thermostat software

## Build Instructions

To build this project, you must first generate missing code from the `.ioc` project file using **STM32CubeMX 6.16.0**:

1. Open `mt-rt.ioc` in STM32CubeMX 6.16.0
2. Generate the code
3. Then build the project using CMake
4. Pull submodules with `git submodule update --init --recursive`
5. Remove some submodule files, executing the following commands from the project root:
```bash
cd Drivers/ssd1306
git sparse-checkout init
git sparse-checkout set --no-cone 'ssd1306/' 'README.md' 'LICENSE'
cd ../st7735
git sparse-checkout init
git sparse-checkout set --no-cone 'st7735/' 'README.md' 'LICENSE'
```

**Note:** Some generated files contain STMicroelectronics code under AS-IS license. See [LEGAL_NOTICES](LEGAL_NOTICES) for details.

## License

This project is licensed under **GPL-3.0** for MiraTherm original code. Generated files from STM32CubeMX are covered under STMicroelectronics' AS-IS license.

See [LICENSE](LICENSE) and [LEGAL_NOTICES](LEGAL_NOTICES) for details.

Copyright (c) 2025 MiraTherm.
