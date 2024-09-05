#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <cstdlib>

std::string getFileNameWithoutExtension(const std::string& filePath) {
  std::string::size_type lastSlashPos = filePath.find_last_of("/\\");
  std::string::size_type lastDotPos = filePath.find_last_of('.');
  
  std::string baseName = filePath.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
  return baseName;
}

std::string getChunkFileName(const std::string& baseName, std::size_t partNum) {
  std::ostringstream oss;
  oss << baseName << "_" << std::setw(3) << std::setfill('0') << partNum << ".pkgpart";
  return oss.str();
}

void splitFile(const std::string& filePath, std::uint64_t chunkSize) {
  std::ifstream inputFile(filePath, std::ios::binary);
  if (!inputFile) {
    std::cerr << "Error: Could not open input file\n";
    return;
  }

  std::string baseName = getFileNameWithoutExtension(filePath);
  std::size_t partNum = 1;
  char buffer[1024 * 1024]; // 1 MiB

  while (!inputFile.eof()) {
    std::string chunkFileName = getChunkFileName(baseName, partNum);
    std::ofstream outputFile(chunkFileName, std::ios::binary);
    std::uint64_t bytesWritten = 0;

    while (bytesWritten < chunkSize && inputFile) {
      inputFile.read(buffer, sizeof(buffer));
      std::streamsize bytesRead = inputFile.gcount();
      outputFile.write(buffer, bytesRead);
      bytesWritten += bytesRead;
    }

    outputFile.close();
    partNum++;
  }

  inputFile.close();
  std::cout << "File split into " << partNum - 1 << " parts\n";
}

int main(int argc, char* argv[]) {
  std::uint64_t chunkSize = 15ULL * 1000 * 1000 * 1000; // Default is 15 GB

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--chunk-size") {
      if (i + 1 < argc) {
        chunkSize = std::strtoull(argv[++i], nullptr, 10);
        if (chunkSize == 0) {
          std::cerr << "Error: Invalid chunk size\n";
          return 1;
        }
      } else {
        std::cerr << "Error: Missing value for chunk size\n";
        return 1;
      }
    } else {
      std::string inputFilePath = argv[i];
      splitFile(inputFilePath, chunkSize);
      return 0;
    }
  }

  std::cerr << "Usage: " << argv[0] << " [-c chunk-size-in-bytes] <input-file>\n";
  return 1;
}
