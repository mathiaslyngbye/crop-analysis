// OpenCV includes
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

// Standard includes
#include <iostream>
#include <string>
#include <regex>
#include <cmath>
#include <experimental/filesystem>

using namespace std;

const double halfC = M_PI / 180;

const int z_slider_max = 1.5*640;
int z_slider = 0;
const int tilt_slider_max = 360;
int tilt_slider = 0;
cv::Mat base_image;
cv::Mat dst;

static void update(int z, int tilt)
{
    z_slider = z;
    tilt_slider = tilt-180;

    size_t height = dst.rows / 2;
    int tilt_amount = height*std::tan(tilt_slider*halfC);

    dst = base_image.clone();
    cv::Point  middleTop(z-tilt_amount,0);
    cv::Point  middleBottom(z+tilt_amount,dst.rows);

    cv::line(dst,middleTop,middleBottom,cv::Scalar(0,255,0),2);
    cv::circle(dst, cv::Point(z,dst.rows/2),3, cv::Scalar(0,0,255),CV_FILLED, 8,0);
    std::string text = "z=" + to_string(z_slider) + ", tilt=" + to_string(tilt_slider) + "deg";
    cv::rectangle(dst, cv::Point2f(5,3), cv::Point2f(225,26), cv::Scalar(0,0,0), -1);
    cv::putText(dst, text, cv::Point2f(10,20), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0,0,255),2);
    cv::imshow("Crop image", dst );
}

static void on_trackbar( int, void* )
{
    size_t height = dst.rows / 2;
    int tilt_amount = height*std::tan(tilt_slider*halfC);

    cv::Point  middleTop(z_slider-tilt_amount,0);
    cv::Point  middleBottom(z_slider+tilt_amount,dst.rows);
    dst = base_image.clone();
    cv::line(dst,middleTop,middleBottom,cv::Scalar(0,255,0));
    cv::circle(dst, cv::Point(z_slider,dst.rows/2),2, cv::Scalar(0,0,255),CV_FILLED, 8,0);
    cv::imshow("Crop image", dst );
}

static void on_mouse(int event,int x,int y,int,void*)
{
    update(x,y);
}

void fetch_test_image_paths(std::string path, std::vector<std::string> &test_image_paths)
{
    std::cout << "Scanning dataset path \"" << path << "\"..." << std::endl;

    // Generate vector of paths to supported files
    test_image_paths.clear();
    std::vector<std::string> supported_file_extensions = {"jpg", "png"};
    for (const auto & entry : std::experimental::filesystem::directory_iterator(path))
    {
        std::string path_string = entry.path();
        std::string file_extension = path_string.substr(path_string.find_last_of(".") + 1);

        for(const auto & supported_file_extension : supported_file_extensions)
        {
            if(file_extension == supported_file_extension)
            {
                test_image_paths.push_back(path_string);
                break;
            }
        }
    }
    std::cout << "Found " << test_image_paths.size() << " image(s)!" << std::endl;
}

void get_data(std::vector<int> &image_data, std::string image_name, std::vector<std::string> delimiters = {"_i", "_s", "_z", "_e", ".jpg"})
{
    for(int j = 0; j < delimiters.size()-1; j++)
    {
        std::regex base_regex(delimiters[j] + "(.*)" + delimiters[j+1]);
        std::smatch base_match;
        std::regex_search(image_name, base_match, base_regex);
        image_data.push_back(std::stoi(base_match[1].str()));
    }
}

int main(int argc, char* argv[])
{
    cv::namedWindow("Crop image");

    // Fetch input file path
    std::string path = "./";
    if( argc < 2)
        return -1; // Error
    path = argv[1];

    // Fetch image paths
    std::vector<std::string> test_image_paths;
    fetch_test_image_paths(path, test_image_paths);
    std::sort(test_image_paths.begin(), test_image_paths.end());

    std::ofstream fout;
    fout.open("data.csv", std::ofstream::trunc);
    fout << "image_z,image_tilt,robot_z\n";
    fout.close();

    int export_count = 0;
    for(size_t i = 0; i < test_image_paths.size(); i++)
    {
        std::string image_path = test_image_paths[i];
        std::vector<int> image_data;
        get_data(image_data,image_path);
        cv::Mat test_image = cv::imread(image_path);
        if (test_image.empty())
        {
            std::cout << "!!! Failed imread(): image no is here" << std::endl;
            continue;
        }

        cv::Mat tmp_image = test_image.clone();
        cv::cvtColor( tmp_image, tmp_image, cv::COLOR_BGR2GRAY );
        cv::equalizeHist( tmp_image, tmp_image );
        cv::cvtColor( tmp_image, test_image, cv::COLOR_GRAY2BGR );


        base_image = test_image.clone();
        dst = base_image.clone();

        // Set initial guess
        int data_z = image_data[2];
        int image_z = -13.873 + 0.68037*data_z;
        int image_tilt = -0.75685  + 0.054274*data_z;
        std::cout << image_tilt << "\t" << data_z << std::endl;
        // Retired peasant trackbar
        //cv::createTrackbar( "Crop z:\t", "Crop image", &z_slider, z_slider_max, on_trackbar );
        //cv::createTrackbar( "Crop tilt:\t", "Crop image", &tilt_slider, tilt_slider_max, on_trackbar );
        //on_trackbar( z_slider, 0 );

        // Mouse master race
        update(image_z,image_tilt);
        cv::setMouseCallback("Crop image",on_mouse);
        //update(image_z,image_tilt);
        //update(z_slider, tilt_slider+180);
        char k = cv::waitKey(0); // Wait for a keystroke in the window

        // Save image if enter is pressed
        if(k == 10)
        {
            fout.open("data.csv",std::ios_base::app);
            fout << z_slider << ',' << tilt_slider << ',' << data_z << '\n';
            fout.close();
            std::cout << "z_slider: " << z_slider << "\ttilt_slider: " << tilt_slider << "\tz_robot: " << data_z << std::endl;
        }
        else if (k == 8)
            std::cout << "Skipping..." << std::endl;
        else if (k == 115)
        {
            cv::imwrite("./export_"+to_string(export_count++)+".png", dst);
            std::cout << "Saving image!" << std::endl;
        }
        else
        {
            std::cout << "Showing the same picture again..." << std::endl;
            i--;
        }
    }

    cout << "End of main!" << endl;
    return 0;
}
