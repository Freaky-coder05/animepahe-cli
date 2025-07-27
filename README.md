# AnimePahe CLI (Beta)

[![Build Status](https://github.com/Danushka-Madushan/animepahe-cli/workflows/Build%20and%20Release/badge.svg)](https://github.com/Danushka-Madushan/animepahe-cli/actions)
[![Release](https://img.shields.io/github/v/release/Danushka-Madushan/animepahe-cli?include_prereleases)](https://github.com/Danushka-Madushan/animepahe-cli/releases)
[![License](https://img.shields.io/github/license/Danushka-Madushan/animepahe-cli)](LICENSE)

A command-line interface for downloading anime episodes from AnimePahe.ru with support for batch downloads, episode ranges, and export functionality.

## ⚠️ Beta Notice

This is a **beta version** and may encounter issues during operation. The current version has the following limitations:
- Maximum of 30 episodes per download session
- Episode ranges are limited to 30-episode spans (e.g., 1-30 is supported, but 20-50 is not)
- Some edge cases may cause unexpected behavior

## 📋 Features

- **Batch Downloads**: Download multiple episodes or entire series
- **Episode Range Selection**: Choose specific episode ranges for targeted downloads
- **Export Functionality**: Generate download links without downloading
- **Archive Support**: Compress downloaded episodes into ZIP archives
- **Cross-Platform**: Native Windows executable with plans for multi-platform support

## 🚀 Installation

### Windows
1. Download the latest `animepahe-cli-beta.exe` from the [Releases](https://github.com/yourusername/animepahe-cli/releases) page
2. Place the executable in your desired directory
3. Open Command Prompt or PowerShell in that directory

### Building from Source
```bash
git clone https://github.com/yourusername/animepahe-cli.git
cd animepahe-cli
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 📖 Usage

### Command Syntax
```
animepahe-cli-beta.exe [OPTIONS]
```

### Required Arguments
| Flag | Long Form | Description | Example |
|------|-----------|-------------|---------|
| `-l` | `--link` | Valid AnimePahe.ru anime URL | `"https://animepahe.ru/anime/dcb2b21f-a70d-84f7-fbab-580701484066"` |
| `-e` | `--episodes` | Episode selection (`all` or range like `1-12`) | `all`, `1-12`, `5-25` |

### Optional Arguments
| Flag | Long Form | Description |
|------|-----------|-------------|
| `-x` | `--export` | Export download links to `links.txt` (cancels download) |
| `-z` | `--zip` | Compress all downloaded episodes into a single ZIP archive |

### Examples

#### Download All Episodes
```bash
animepahe-cli-beta.exe -l "https://animepahe.ru/anime/dcb2b21f-a70d-84f7-fbab-580701484066" -e all
```

#### Download Specific Episode Range
```bash
animepahe-cli-beta.exe -l "https://animepahe.ru/anime/dcb2b21f-a70d-84f7-fbab-580701484066" -e 1-12
```

#### Export Download Links Only
```bash
animepahe-cli-beta.exe -l "https://animepahe.ru/anime/dcb2b21f-a70d-84f7-fbab-580701484066" -e all -x
```

#### Download and Create ZIP Archive
```bash
animepahe-cli-beta.exe -l "https://animepahe.ru/anime/dcb2b21f-a70d-84f7-fbab-580701484066" -e 1-24 -z
```

## 🔧 Technical Details

### Dependencies
- **CPR**: HTTP client library for C++
- **FMT**: Modern formatting library
- **RE2**: Regular expression engine
- **Abseil**: Google's C++ common libraries
- **cxxopts**: Command line argument parsing
- **PugiXML**: XML processing library
- **nlohmann/json**: JSON parsing library

### Build Requirements
- CMake 3.5 or higher
- C++20 compatible compiler
- Git (for dependency fetching)

## 🐛 Known Issues

- Episode ranges spanning more than 30 episodes are not supported
- Some anime series may have parsing issues
- Network timeouts may occur with slow connections
- Large batch downloads may consume significant system resources

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## ⚖️ Disclaimer

This tool is for educational purposes only. Users are responsible for complying with AnimePahe.ru's terms of service and applicable copyright laws. The developers do not condone piracy or copyright infringement.

## 🔗 Links

- [AnimePahe.ru](https://animepahe.ru) - Source website
- [Issues](https://github.com/yourusername/animepahe-cli/issues) - Bug reports and feature requests
- [Releases](https://github.com/yourusername/animepahe-cli/releases) - Download latest versions
