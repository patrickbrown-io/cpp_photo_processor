/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Patrick Brown

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    n/a

- Did you do any optional enhancements? If so, please explain:
    n/a
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


// Quick terminal command
// g++ -std=c++11 -o test main.cpp (change test to whatever you wanna call it)

/**
 * Process 1: Applies a vignette effect to the specified image
 * @param image The Vector Image
 */
void applyVignetteEffect(vector<vector<Pixel>>& image) {
    int height = image.size();
    int width = image[0].size();
    double centerX = width / 2.0;
    double centerY = height / 2.0;

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            double distance = sqrt(pow(col - centerX, 2) + pow(row - centerY, 2));
            double scaling_factor = (height - distance) / height;
            image[row][col].red = image[row][col].red * scaling_factor;
            image[row][col].green = image[row][col].green * scaling_factor;
            image[row][col].blue =image[row][col].blue * scaling_factor;
        }
    }
}
/**
 * Process 2: Applies a clarendon effect to the specified image
 * @param image The Vector Image
 * @param scaling_factor a double value you'd like to apply to the effect
 */
void applyClarendonEffect(vector<vector<Pixel>>& image, double scaling_factor) {
    int height = image.size();
    int width = image[0].size();

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            double average = (image[row][col].red + image[row][col].green + image[row][col].blue)/3;
//             If cell is light, make it lighter
            if(average >= 170)
            {
                image[row][col].red = (225 - (225 - image[row][col].red) * scaling_factor);
                image[row][col].green = (225 - (225 - image[row][col].green) * scaling_factor);
                image[row][col].blue = (225 - (225 - image[row][col].blue) * scaling_factor);
            
            } else if(average < 90)
            {
                image[row][col].red = image[row][col].red * scaling_factor;
                image[row][col].green = image[row][col].green* scaling_factor;
                image[row][col].blue = image[row][col].blue * scaling_factor;
            } else {
                image[row][col].red = image[row][col].red;
                image[row][col].green = image[row][col].green;
                image[row][col].blue = image[row][col].blue;
            }

    }
}
}
/**
 * Process 3: Applies a grayscale effect to the specified image
 * @param image The Vector Image
 */
void applyGrayscaleEffect(vector<vector<Pixel>>& image) {
    int height = image.size();
    int width = image[0].size();

        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                double average = (image[row][col].red + image[row][col].green + image[row][col].blue)/3;
                image[row][col].red = average;
                image[row][col].green = average;
                image[row][col].blue = average;
            }
        }
}
/**
 * Process 4: Rotates the specified image 90 degrees
 * @param image The Vector Image
 * @param rotations The number of rotations
 */
vector<vector<Pixel>> apply90Rotation(vector<vector<Pixel>>& image, int rotations) {
    int actual_rotations = rotations % 4;
    vector<vector<Pixel>> rotatedImage = image; // Start with the original image

    for (int i = 0; i < actual_rotations; ++i) {
        int height = rotatedImage.size();
        int width = rotatedImage[0].size();
        vector<vector<Pixel>> tempImage(width, vector<Pixel>(height));

        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                int newRow = col;
                int newCol = height - 1 - row;
                tempImage[newRow][newCol] = rotatedImage[row][col];
            }
        }
        
        rotatedImage = tempImage;
    }

    return rotatedImage;
}
/**
 * Process 6: Enlarges the image in the x and y direction
 * @param x_scale The x scale direction
 * @param y_scale The y scale direction
 */
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale){
    int height = image.size();
    int width = image[0].size();
    int newHeight = height * y_scale;
    int newWidth = width * x_scale;

    vector<vector<Pixel>> newImage(newHeight, vector<Pixel>(newWidth));

    for (int row = 0; row < newHeight; ++row) {
        for (int col = 0; col < newWidth; ++col) {
            int origRow = row / y_scale;
            int origCol = col / x_scale;
            newImage[row][col] = image[origRow][origCol];
        }
    }
    return newImage;

}
/**
 * Process 7: Convert image to high contrast (black and white only)
 * @param image The Vector Image
 */
void process_7(vector<vector<Pixel>>& image){
    int height = image.size();
    int width = image[0].size();

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            // Grey value
            double average = (image[row][col].red + image[row][col].green + image[row][col].blue)/3;

            if(average >= 255/2){
                image[row][col].red = 255;
                image[row][col].green = 255;
                image[row][col].blue = 255;
            } else{
                image[row][col].red = 0;
                image[row][col].green = 0;
                image[row][col].blue = 0;
            }
        }
    }
}
/**
 * Process 8: Lightens image by a scaling factor
 * @param image The Vector Image
 * @param scaling_factor The scaling factor
 */
void process_8(vector<vector<Pixel>>& image, double scaling_factor){
    int height = image.size();
    int width = image[0].size();

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {

            image[row][col].red = (255 - (255 - image[row][col].red) * scaling_factor);
            image[row][col].green = (255 - (255 - image[row][col].green) * scaling_factor);
            image[row][col].blue = (255 - (255 - image[row][col].blue) * scaling_factor);
        }
    }
}
/**
 * Process 9: Darkens image by a scaling factor
 * @param image The Vector Image
 * @param scaling_factor The scaling factor
 */
void process_9(vector<vector<Pixel>>& image, double scaling_factor){
    int height = image.size();
    int width = image[0].size();

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {

            image[row][col].red *= scaling_factor;
            image[row][col].green *= scaling_factor;
            image[row][col].blue *= scaling_factor;
        }
    }
}
/**
 * Process 10: Converts image to only black, white, red, blue, and green
 * @param image The Vector Image
 */
void process_10(vector<vector<Pixel>>& image){
    int height = image.size();
    int width = image[0].size();

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            double maximum = image[row][col].red;

            if(image[row][col].blue > maximum){
                maximum = image[row][col].blue;
            }
            if(image[row][col].green > maximum){
                maximum = image[row][col].green;
            }

            if(image[row][col].red + image[row][col].blue + image[row][col].green >= 550){
                image[row][col].red = 255;
                image[row][col].blue = 255;
                image[row][col].green = 255;
            } else if(image[row][col].red + image[row][col].blue + image[row][col].green <= 150) {
                image[row][col].red = 0;
                image[row][col].blue = 0;
                image[row][col].green = 0;
            } else if(maximum == image[row][col].red){
                image[row][col].red = 255;
                image[row][col].blue = 0;
                image[row][col].green = 0;
            } else if(maximum == image[row][col].green){
                image[row][col].red = 0;
                image[row][col].blue = 0;
                image[row][col].green = 255;
            } else {
                image[row][col].red = 0;
                image[row][col].blue = 255;
                image[row][col].green = 0;
            }
        }
    }
}


int main()
{

    string filename;
    cout << "**********************************************" << endl;
    cout << "*** CSPB 1300 Image Processing Application ***" << endl;
    cout << "************** by Patrick Brown **************" << endl;
    cout << "**********************************************" << endl << endl;
    cout << "Enter input BMP filename: ";
    cin >> filename;
    cout << endl << endl;
    string input;
    int choice;
    do
    {

        // Present menu
        cout << "0) Change image (current: " << filename << ")" << endl;
        cout << "1) Vignette" << endl;
        cout << "2) Clarendon" << endl;
        cout << "3) Greyscale" << endl;
        cout << "4) Rotate 90 degrees" << endl;
        cout << "5) Rotate multiple 90 degrees" << endl;
        cout << "6) Enlarge" << endl;
        cout << "7) High contrast" << endl;
        cout << "8) Lighten" << endl;
        cout << "9) Darken" << endl;
        cout << "10) Black, white, red, green, blue" << endl;
        cout << endl;
        cout << "Enter menu selection (Q to quit): ";
        cin >> input;
        cout << endl;
        if (input == "Q" || input == "q")
        {
            cout << "Thank you for using my program!" << endl;
            cout << "Quitting..." << endl;
            break;
        }

        // This is needed to convert input to an int and then turn that into choice, since input is setup to also hand the q's
        choice = (isdigit(input[0]) || (input[0] == '-' && input.size() > 1 && isdigit(input[1]))) ? stoi(input) : -1;

        switch (choice)
        {
        case 0:
            cout << "Change image selected" << endl;
                cout << "Enter input BMP filename: ";
                cin >> filename;
                cout << "Successfully changed input image!" << endl << endl;
            break;
        case 1:
            {
            cout << "Vignette selected" << endl;
             // Get output name
            string outputname;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            // Get image
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            applyVignetteEffect(image);
             // Save
            write_image(outputname, image);
            cout << endl;
            cout << "Successfully applied vignette!" << endl << endl;
            break;
            }
        case 2:
            {
            cout << "Clarendon selected" << endl;
             // Get output name + scaling factor
            string outputname;
            double scaling;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            cout << "Enter scaling factor: ";
            cin >> scaling;
            // Get image 
            vector<vector<Pixel>> image = read_image(filename);
            applyClarendonEffect(image, scaling);
            // Save
            write_image(outputname, image);
            cout << endl;
            cout << "Successfully applied Clarendon!" << endl << endl;
            break;
            }
        case 3:
            {
            cout << "Grayscale selected" << endl;
             // Get output name
            string outputname;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            // Get image
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            applyGrayscaleEffect(image);
             // Save
            write_image(outputname, image);
            cout << endl;
            cout << "Successfully applied grayscale!" << endl << endl;
            }
            break;
        case 4:
            {
            cout << "Rotate 90 degrees selected" << endl;
              // Get output name
            string outputname;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            // Get image
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            write_image(outputname, apply90Rotation(image,1));
            cout << "Successfully applied 90 degree rotation!" << endl << endl;
            break;
            }
        case 5:
            {
            cout << "Rotate multiple 90 degrees selected" << endl;
              // Get output name
            string outputname;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            // Get number of rotations
            int rotation_num;
            cout << endl;
            cout << "Enter number of 90 degree rotations: ";
            cin >> rotation_num;
            // Get image
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            write_image(outputname, apply90Rotation(image,rotation_num));
            cout << endl;
            cout << "Successfully applied multiple 90 degree rotations!";
            break;
            }
        case 6:
        {
            cout << "Enlarge selected" << endl;
             // Get output name
            string outputname;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            // Get X and Y
            int x_val;
            int y_val;
            cout << endl;
            cout << "Enter X scale: ";
            cin >> x_val;
            cout << endl;
            cout << "Enter Y scale: ";
            cin >> y_val;
            // Get image
            vector<vector<Pixel>> image = read_image(filename);
            // Run effect
            write_image(outputname, process_6(image,x_val,y_val));
            cout << endl;
            cout << "Enlarge successfully applied!";
            break;
        }
        case 7:
        {
            cout << "High contrast selected" << endl;
             // Get output name
            string outputname;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            // Get image
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            process_7(image);
             // Save
            write_image(outputname, image);
            cout << endl;
            cout << "Successfully applied vignette!" << endl << endl;
            break;
        }
        case 8:
        {
            cout << "Lighten selected" << endl;
            // Get output name + scaling factor
            string outputname;
            double scaling;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            cout << "Enter scaling factor: ";
            cin >> scaling;
            // Get image 
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            process_8(image, scaling);
            write_image(outputname, image);
            cout << endl;
            cout << "Successfully applied lighten!" << endl << endl;
            break;
        }
        case 9:
        {
            cout << "Darken selected" << endl;
            // Get output name + scaling factor
            string outputname;
            double scaling;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            cout << "Enter scaling factor: ";
            cin >> scaling;
            // Get image 
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            process_9(image, scaling);
            write_image(outputname, image);
            cout << endl;
            cout << "Successfully applied darken!" << endl << endl;
            break;
        }
        case 10:
            {
            cout << "Black, white, red, green, blue selected" << endl;
            string outputname;
            cout << "Enter output BMP filename: ";
            cin >> outputname;
            // Get image 
            vector<vector<Pixel>> image = read_image(filename);
            // Apply effect
            process_10(image);
            write_image(outputname, image);
            cout << endl;
            cout << "Successfully applied black, white, red, green, blue filter!" << endl << endl;
            break;
            }
        default:
            cout << "Invalid choice!" << endl;
            break;
        }
    } while (true);
    return 0;
}