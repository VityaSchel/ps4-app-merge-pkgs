#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstdint>

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

void splitFile(const std::string& filePath) {
  std::ifstream inputFile(filePath, std::ios::binary);
  if (!inputFile) {
    std::cerr << "Error: Could not open input file\n";
    return;
  }

  std::string baseName = getFileNameWithoutExtension(filePath);
  std::size_t partNum = 1;
  char buffer[1024 * 1024]; // 1 MiB

  constexpr std::uint64_t CHUNK_SIZE = 15ULL * 1000 * 1000 * 1000; // 15 GB

  while (!inputFile.eof()) {
    std::string chunkFileName = getChunkFileName(baseName, partNum);
    std::ofstream outputFile(chunkFileName, std::ios::binary);
    std::uint64_t bytesWritten = 0;

    while (bytesWritten < CHUNK_SIZE && inputFile) {
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
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input-file>\n";
    return 1;
  }

  std::string inputFilePath = argv[1];
  splitFile(inputFilePath);

  return 0;
}
