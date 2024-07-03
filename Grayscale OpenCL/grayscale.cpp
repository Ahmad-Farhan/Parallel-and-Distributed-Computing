#include <CL/cl2.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define namesfile "filenames.txt"
#define kernelfile "kernel.cl"
namespace fs = std::filesystem;


// 1D Representation of RG Image
struct rgb_uchar3 {
    int w, h;   //Dimensions
    uchar* r;   //Red Channel
    uchar* g;   //Green Channel
    uchar* b;   //Blue Channel
};
// Free rgb_uchar3 Structure Memory
void clearImageSpace(rgb_uchar3* image) {
    delete[] image->r;
    delete[] image->g;
    delete[] image->b;
}
// Function to Read Image as rgb_uchar3
bool readImage(std::string filename, rgb_uchar3* resImage) {
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        //Error when invalid image filename
        std::cerr << "Could not open image." << std::endl;
        return false;
    }
    // Initializing Structure
    const int width = image.cols, height = image.rows;
    cl_uchar* r = new cl_uchar[height * width];
    cl_uchar* g = new cl_uchar[height * width];
    cl_uchar* b = new cl_uchar[height * width];
    // Generate RGB arrays from cv::Mat
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
            r[i * width + j] = pixel[2];
            g[i * width + j] = pixel[1];
            b[i * width + j] = pixel[0];
        }
    // Initialize rgb_uchar3 Structure
    resImage->w = width;
    resImage->h = height;
    resImage->r = r;
    resImage->g = g;
    resImage->b = b;
    return true;
}
// Generate File containing names of all images in folder
void makeFilenames(std::string inputfolder) {
    std::fstream file("filenames.txt", std::ios::out);
    for (const auto& entry : fs::directory_iterator(inputfolder)) {
        std::string filename = entry.path().filename().string();
        file << filename.c_str() << "\n";
    }
    file.close();
}
// Convert rgb_uchar3 into cv::Mat. Note: Clears inputImage memory
cv::Mat cvtLinear2Mat(cl_uchar* inputImage, const int width, const int height) {
    cv::Mat resImage(height, width, CV_8UC1);   // Initializing 2D cv::Mat in Gray
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            resImage.at<uchar>(i, j) = inputImage[i * width + j];
    delete[] inputImage;
    return resImage;
}
// Converts RGB Image into Grayscale using OpenCL
cl_uchar* applyTransformation(cl::Kernel& kernel, cl::Context& context, cl::CommandQueue& queue, rgb_uchar3& image) {
    int width = image.w, height = image.h;
    const int imageSize = height * width;
    cl_uchar* gray = new cl_uchar[imageSize];
    cl_uchar* red = image.r, * green = image.g, * blue = image.b;

    //Kernel Function Input/Output Buffers
    cl::Buffer redBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, imageSize * sizeof(cl_uchar), red);
    cl::Buffer greenBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, imageSize * sizeof(cl_uchar), green);
    cl::Buffer blueBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, imageSize * sizeof(cl_uchar), blue);
    cl::Buffer widthBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &width);
    cl::Buffer grayBuffer(context, CL_MEM_WRITE_ONLY, imageSize * sizeof(cl_uchar));//Output Buffer

    //Setting Kernel Arguements
    kernel.setArg(0, redBuffer);
    kernel.setArg(1, greenBuffer);
    kernel.setArg(2, blueBuffer);
    kernel.setArg(3, grayBuffer);
    kernel.setArg(4, widthBuffer);

    //Setting Work size to number of rows in image
    cl::NDRange globalWorkSize(height);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalWorkSize, cl::NullRange, nullptr);
    queue.enqueueReadBuffer(grayBuffer, CL_TRUE, 0, sizeof(cl_uchar) * imageSize, gray);
    queue.flush();  // Reading Output and clearing queue
    return gray;
}
// Loads Kernel Code from .cl file
const char* loadKernelCode() {
    std::string filename = kernelfile;
    std::ifstream kernelStream(filename);
    if (!kernelStream.is_open()) throw std::runtime_error("Failed to open file: " + filename);
    std::string kernelSrc((std::istreambuf_iterator<char>(kernelStream)), std::istreambuf_iterator<char>());
    return kernelSrc.c_str();
}
std::string outputfoldername(std::string inputfolder) {
    std::string outputfolder = inputfolder;
    size_t found = outputfolder.find("Input");
    if (found != std::string::npos)
        outputfolder.replace(found, 5, "Output");
    else outputfolder += "Output";
    return outputfolder;
}
// Execute Transformation Function on a Single Image
void imageConvert(cl::Kernel& kernel, cl::Context& context, cl::CommandQueue& queue) {
    rgb_uchar3 image;
    std::string filename = "";
    printf("Enter RGB Image Filename: ");
    std::getline(std::cin, filename);      //Take Image Filename as Input
    if (!readImage(filename, &image)) {
        printf("File Not Found\n");
        return;
    }
    // Call RGB to GRayscale Transformation Function
    cl_uchar* gray = applyTransformation(kernel, context, queue, image);    // RGB to Gray
    cv::Mat grayImage = cvtLinear2Mat(gray, image.w, image.h);      // Convert rgb_uchar3 to cv::Mat
    cv::imwrite("GRAY-" + filename, grayImage);       // Save Output Image as cv::Mat
    clearImageSpace(&image);        //Clear rgb_uchar3 space
}
// Execute Transformation Function on all Images of a folder
void folderConvert(cl::Kernel& kernel, cl::Context& context, cl::CommandQueue& queue) {
    std::string filename;       //Predefined Input and Output Folders
    std::string inputfolder, outputfolder;
    printf("Enter RGB Images Folder: ");
    std::getline(std::cin, inputfolder);      //Take Image Folder name as Input
    if (!(fs::exists(inputfolder) && fs::is_directory(inputfolder))) {
        printf("Invalid Input Folder\n");
        return;
    }
    // Generate Output Folder Name
    outputfolder = outputfoldername(inputfolder);
    fs::create_directory(outputfolder);

    std::ifstream file(namesfile, std::ios::in);
    if (!file.is_open()) {          //Check imagenames file of given folder
        printf("Error: Dataset Images Names File Not Found\n");
        printf("Creating file...");
        makeFilenames(inputfolder);
        file.open(namesfile, std::ios::in);
    }
    // Iterate over each image and apply transformation
    while (getline(file, filename)) {
        rgb_uchar3 image;
        if (!readImage(inputfolder + '/' + filename, &image)) continue;      //Read Image
        cl_uchar* gray = applyTransformation(kernel, context, queue, image);    //Rgb to Gray
        cv::Mat grayImage = cvtLinear2Mat(gray, image.w, image.h);      //rgb_uchar3 to cv::Mat
        cv::imwrite(outputfolder + '/' + filename, grayImage);        // Save cv::Mat to file
        clearImageSpace(&image);            // Clear rgb_uchar3 space
    }
    file.close();
}
int main() {
    // Select First Platform on Machine
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front();

    // Select First GPU Device on Machine
    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device = devices.front();

    // Create OpenCL context for device
    cl::Context context({ device });

    // Create command queue for device
    cl::CommandQueue queue(context, device);

    // Create OpenCL program from source
    cl::Program::Sources sources;
    const char* kernelSource = loadKernelCode();
    sources.push_back({ kernelSource , strlen(kernelSource) });

    //Build the Program for device
    cl::Program program(context, sources);
    program.build({ device });

    // Create kernel object for convert function
    cl::Kernel kernel(program, "convert", 0);

    // Choose Input Type and Apply Transformation
    std::string inputtype = "folder";
    printf("Enter Input Type(image, folder): ");
    std::getline(std::cin, inputtype);
    if (inputtype == "image")
        imageConvert(kernel, context, queue);
    else if (inputtype == "folder")
        folderConvert(kernel, context, queue);
    else printf("Invalid Input Type\n");
    return -1;
}
