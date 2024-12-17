#include <iostream>
#include <fstream>
#include <stdexcept>

std::pair<char*, size_t> readFileToMemory(const std::string& filename) {
    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open the file: " + filename);
    }

    // Get the size of the file
    file.seekg(0, std::ios::end); // Move to the end of the file
    size_t fileSize = file.tellg(); // Get the current position (size of the file)
    file.seekg(0, std::ios::beg); // Move back to the beginning of the file

    // Allocate memory for the file contents
    char* buffer = new char[fileSize];
    if (!buffer) {
        throw std::runtime_error("Memory allocation failed");
    }

    // Read the file contents into the buffer
    file.read(buffer, fileSize);
    if (!file) {
        delete[] buffer; // Clean up memory in case of an error
        throw std::runtime_error("Error reading the file: " + filename);
    }

    // Close the file
    file.close();

    // Return the buffer and its size
    return { buffer, fileSize };
}