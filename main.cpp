#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// Define M_PI if it's not already defined. This is a common fix
// as M_PI is not part of the standard C++ library but is often
// provided by headers like <cmath> with certain flags.
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// --- Quality Metrics ---

// Calculates the Peak Signal-to-Noise Ratio (PSNR)
double calculatePSNR(const unsigned char* original, const unsigned char* compressed, int width, int height, int channels) {
    double mse = 0;
    long total_pixels = static_cast<long>(width) * height * channels;

    for (long i = 0; i < total_pixels; ++i) {
        double diff = static_cast<double>(original[i]) - static_cast<double>(compressed[i]);
        mse += diff * diff;
    }
    mse /= total_pixels;

    if (mse == 0) {
        return 100.0; // PSNR is infinite for identical images
    }

    double max_pixel_value = 255.0; // 8-bit image
    double psnr = 10.0 * log10(pow(max_pixel_value, 2) / mse);
    return psnr;
}


// --- Lossless Compression (Run-Length Encoding) ---

// Compresses data using Run-Length Encoding
std::vector<unsigned char> runLengthEncode(const std::vector<unsigned char>& data) {
    if (data.empty()) return {};

    std::vector<unsigned char> compressed_data;
    size_t i = 0;
    while (i < data.size()) {
        unsigned char current_byte = data[i];
        unsigned char count = 1;
        i++;
        while (i < data.size() && data[i] == current_byte && count < 255) {
            count++;
            i++;
        }
        compressed_data.push_back(count);
        compressed_data.push_back(current_byte);
    }
    return compressed_data;
}

// Decompresses data encoded with Run-Length Encoding
std::vector<unsigned char> runLengthDecode(const std::vector<unsigned char>& data) {
    if (data.empty()) return {};

    std::vector<unsigned char> decompressed_data;
    for (size_t i = 0; i < data.size(); i += 2) {
        unsigned char count = data[i];
        unsigned char byte = data[i + 1];
        for (unsigned char j = 0; j < count; j++) {
            decompressed_data.push_back(byte);
        }
    }
    return decompressed_data;
}


// --- Lossy Compression (Simplified DCT) ---

// Simplified 1D DCT (Conceptual, for demonstration)
std::vector<double> simplifiedDCT(const std::vector<unsigned char>& data) {
    std::vector<double> transformed(data.size());
    size_t N = data.size();
    for (size_t k = 0; k < N; ++k) {
        double sum = 0.0;
        for (size_t n = 0; n < N; ++n) {
            sum += (data[n] - 128.0) * cos(M_PI * (n + 0.5) * k / N);
        }
        transformed[k] = sum * (k == 0 ? sqrt(1.0 / N) : sqrt(2.0 / N));
    }
    return transformed;
}

// Simplified 1D Inverse DCT (Conceptual)
std::vector<unsigned char> simplifiedIDCT(const std::vector<double>& transformed) {
    std::vector<unsigned char> reconstructed(transformed.size());
    size_t N = transformed.size();
    for (size_t n = 0; n < N; ++n) {
        double sum = 0.0;
        for (size_t k = 0; k < N; ++k) {
            double C = (k == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
            sum += C * transformed[k] * cos(M_PI * (n + 0.5) * k / N);
        }
        int pixel_val = static_cast<int>(sum + 128.0);
        // Clamp values to the valid range [0, 255]
        reconstructed[n] = static_cast<unsigned char>(std::max(0, std::min(255, pixel_val)));
    }
    return reconstructed;
}

// Quantization (the lossy step)
std::vector<double> quantize(const std::vector<double>& transformed, int quality) {
    std::vector<double> quantized = transformed;
    double scale = static_cast<double>(quality);
    for (double& val : quantized) {
        val = round(val / scale) * scale;
    }
    return quantized;
}


// --- Main Program ---
int main() {
    // --- Step 1: Load an image ---
    int width, height, channels;
    unsigned char* img_data = stbi_load("input.png", &width, &height, &channels, 0);

    if (img_data == nullptr) {
        std::cerr << "Error: Failed to load image. Make sure 'input.png' exists in the same directory." << std::endl;
        return 1;
    }

    std::cout << "Image loaded successfully: " << width << "x" << height << " with " << channels << " channels." << std::endl;
    std::vector<unsigned char> original_vector(img_data, img_data + (static_cast<long>(width) * height * channels));


    // --- Step 2: Lossless Compression Demonstration (RLE) ---
    std::cout << "\n--- Lossless Compression (RLE) ---" << std::endl;
    std::vector<unsigned char> compressed_rle = runLengthEncode(original_vector);
    std::cout << "Original size: " << original_vector.size() << " bytes" << std::endl;
    std::cout << "Compressed (RLE) size: " << compressed_rle.size() << " bytes" << std::endl;

    std::vector<unsigned char> decompressed_rle = runLengthDecode(compressed_rle);
    bool is_lossless = (original_vector == decompressed_rle);
    std::cout << "Is RLE lossless? " << (is_lossless ? "Yes" : "No") << std::endl;


    // --- Step 3: Lossy Compression Demonstration (Simplified DCT) ---
    std::cout << "\n--- Lossy Compression (Simplified DCT) ---" << std::endl;

    // A higher quality factor (e.g., 20) means more aggressive compression and lower quality.
    // A lower quality factor (e.g., 5) means less compression and higher quality.
    int quality_factor = 10;
    std::cout << "Using quality factor: " << quality_factor << std::endl;

    // Convert pixel data to double for mathematical operations
    std::vector<double> original_double(original_vector.begin(), original_vector.end());

    // 1. Apply a transform (DCT)
    std::vector<double> dct_transformed = simplifiedDCT(original_vector);

    // 2. Quantize (the key lossy step)
    std::vector<double> quantized_data = quantize(dct_transformed, quality_factor);

    // 3. Inverse transform to reconstruct the image
    std::vector<unsigned char> decompressed_dct = simplifiedIDCT(quantized_data);

    // Save the compressed image for visual inspection
    stbi_write_png("output_compressed.png", width, height, channels, decompressed_dct.data(), width * channels);
    std::cout << "Lossy compressed image saved as 'output_compressed.png'." << std::endl;

    // Calculate and print quality metrics
    double psnr = calculatePSNR(img_data, decompressed_dct.data(), width, height, channels);
    std::cout << "PSNR of lossy compression: " << psnr << " dB" << std::endl;

    // --- Cleanup ---
    stbi_image_free(img_data); // Free the memory allocated by stbi_load
    return 0;
}
