# MiraTherm radiator thermostat software

## Build Instructions

To build this project, you must first generate the code from the `.ioc` project file using **STM32CubeMX 6.16.0**:

1. Open `mt-rt.ioc` in STM32CubeMX 6.16.0
2. Generate the code (this creates the necessary Generated and Drivers folders)
3. Then build the project using CMake

**Note:** Some generated files contain STMicroelectronics code under AS-IS license. See [LEGAL_NOTICES](LEGAL_NOTICES) for details.

## License

This project is licensed under **GPL-3.0** for MiraTherm original code. Generated files from STM32CubeMX are covered under STMicroelectronics' AS-IS license.

See [LICENSE](LICENSE) and [LEGAL_NOTICES](LEGAL_NOTICES) for details.

Copyright (c) 2025 MiraTherm.
