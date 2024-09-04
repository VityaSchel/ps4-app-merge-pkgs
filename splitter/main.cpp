#include <fstream>
#include <iostream>
#include <vector>
#include <string>

// const std::size_t CHUNK_SIZE = 15 * 1024 * 1024 * 1024; // 15 GB
const std::size_t CHUNK_SIZE = 5 * 1024 * 1024;

void splitFile(const std::string& filePath, const std::string& outputPrefix) {
  std::ifstream inputFile(filePath, std::ios::binary);
  if (!inputFile) {
    std::cerr << "Error: Could not open input file\n";
    return;
  }

  std::size_t partNum = 0;
  char buffer[1024 * 1024]; // 1 MB

  while (!inputFile.eof()) {
    std::ofstream outputFile(outputPrefix + std::to_string(partNum), std::ios::binary);
    std::size_t bytesWritten = 0;

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
  std::cout << "File split into " << partNum << " parts\n";
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <input-file> <output-prefix>\n";
    return 1;
  }

  std::string inputFilePath = argv[1];
  std::string outputPrefix = argv[2];

  splitFile(inputFilePath, outputPrefix);

  return 0;
}
