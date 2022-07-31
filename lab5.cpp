//lab5

//include openCV
#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;


int main(int argc, char** argv)
{	
	// Modify these two values depending on the desired mode
	// The flow should be: preprocessing -> neural network -> postprocessing
	bool preprocessing = false;
	bool postprocessing;
	if (preprocessing) {
		postprocessing = false;
	}
	else {
		postprocessing = true;
	}

	// Load all the images in a directory
	vector<String> fn;
	glob("CNR-EXT_FULL_IMAGE_1000x750/FULL_IMAGE_1000x750/RAINY/2015-11-21/camera2/*.jpg", fn, true);
	vector<Mat> images;
	size_t count = fn.size();
	for (size_t i = 0; i < count; i++) {  
		images.push_back(imread(fn[i]));
	}

	size_t index = 1;		// chose the image from the folder
	vector<int> id;
	vector<Point> points;
	vector<int> width;
	vector<int> height;
	vector<Vec2i> predictions;

	string lines;
	string slotId;
	string slotX;
	string slotY;
	string slotWidth;
	string slotHeight;
	Mat img = images[index];
	fstream fs;
	string res;

	// Parsing the coordinates of the parking slots
	fs.open("CNR-EXT_FULL_IMAGE_1000x750/camera2.csv", ios::in);
	while (getline(fs, lines)) {
		int comma = lines.find(',');
		slotId = lines.substr(0, comma);
		if (slotId == "SlotId") {
			continue;
		}
		int comma1 = lines.find(',', comma + 1);
		slotX = lines.substr(comma + 1, comma1 - comma - 1);
		int comma2 = lines.find(',', comma1 + 1);
		slotY = lines.substr(comma1 + 1, comma2 - comma1 - 1);
		int comma3 = lines.find(',', comma2 + 1);
		slotWidth = lines.substr(comma2 + 1, comma3 - comma2 - 1);
		int comma4 = lines.find(',', comma3 + 1);
		slotHeight = lines.substr(comma3 + 1, comma4 - comma3);
		id.push_back(stoi(slotId));
		Point p;
		p.x = static_cast<int>(stod(slotX) / 2.592);
		p.y = static_cast<int>((stod(slotY) / 1944) * 750);
		points.push_back(p);
		width.push_back(static_cast<int>(stod(slotWidth) / 2.592));
		height.push_back(static_cast<int>(((stod(slotHeight) * 750) / 1944)));
		//cout << slotId << endl << points.back().x << endl << points.back().y << endl << width.back() << endl << height.back() << endl;
	}
	fs.close();

	if (postprocessing) {
		// Parsing the prediction outputs from the neural network
		fs.open("results.txt", ios::in);
		while (getline(fs, res, '.')) {
			cout << res.substr(res.find(',') + 1) << endl << res.substr(0, res.find(',')) << endl;
			Vec2i predict(stoi(res.substr(res.find(',') + 1)), stoi(res.substr(0, res.find(','))));
			predictions.push_back(predict);
		}
		fs.close();
	}

	// Draw rectangles that identify the parking slots
	for (size_t i = 0; i < points.size(); i++) {
		Point p1 = points.at(i);
		Point p2 = p1;
		p2.x = p2.x + width.at(i);
		Point p3 = p2;
		p3.y = p3.y + height.at(i);
		Point p4 = p1;
		p4.y = p4.y + height.at(i);

		if (preprocessing) {
			// Crop the frames from the orginal image
			Mat frame = img(Range(p1.y, p4.y), Range(p1.x, p2.x));
			// Save parking slot frames in the directory frames
			bool result = imwrite("frames/frame" + to_string(id.at(i)) + "_" + to_string(index) + ".jpg", frame);
			if (result) {
				cout << "frame" + to_string(id.at(i)) + "_" + to_string(index) + ".jpg saved succesfully" << endl;
			}
			else {
				cout << "Something went wrong" << endl;
				break;
			}
		}
		if (postprocessing) {
			// checking the prediction of the neural network for the specified slotId
			int prediction = 2;
			for (size_t j = 0; j < predictions.size(); j++) {
				if (predictions.at(j)[0] == id.at(i)) {
					prediction = predictions.at(j)[1];
				}
			}
			cout << to_string(prediction) << endl;
			if (prediction == 2) {
				cout << "Something went wrong with the predictions" << endl;
			}
			// Drawing green or red rectangles based on the prediction
			if (prediction == 0) {
				line(img, p1, p2, Scalar(0, 255, 0), 1);
				line(img, p2, p3, Scalar(0, 255, 0), 1);
				line(img, p4, p3, Scalar(0, 255, 0), 1);
				line(img, p1, p4, Scalar(0, 255, 0), 1);
			}
			else {
				line(img, p1, p2, Scalar(0, 0, 255), 1);
				line(img, p2, p3, Scalar(0, 0, 255), 1);
				line(img, p4, p3, Scalar(0, 0, 255), 1);
				line(img, p1, p4, Scalar(0, 0, 255), 1);
			}
		}		
	}
	if (postprocessing) {
		imwrite("output/img20.jpg", img);
	}
	imshow("img", img);
	waitKey(0);

	return 0;
}