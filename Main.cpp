// Main.cpp
#include "tchar.h"
#include <iostream>
#include<conio.h>
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include "sqlext.h"
#include<string.h>
#include<stdio.h>
#include<fstream>
#include<dirent.h>
#pragma warning(disable:4996)
#include "Main.h"
using namespace std;
void rename1()
{
	std::string directory = "C:\\Users\\DELL\\Google Drive\\hi\\";
	
	DIR *pdir = NULL;
	struct dirent *pent = NULL;
	const char * DIRECTORY;

	// convert directory string to const char
	DIRECTORY = directory.c_str();

	pdir = opendir(DIRECTORY);
	int i = 1;
	std::string s, oldname, newname;
	const char * OLDNAME, *NEWNAME;

	while (pent = readdir(pdir))
	{


		oldname = (std::string(DIRECTORY) + pent->d_name).c_str();
		newname = (std::string(DIRECTORY) + "image.png " + s).c_str();

		OLDNAME = oldname.c_str();
		NEWNAME = newname.c_str();

		rename(OLDNAME, NEWNAME);

		i++;
	}
}
void delete1()
{
	int status;
	char fname[20];
	status = remove("C:\\Users\\DELL\\Google Drive\\hi\\image.png");
	
}
void show_error(unsigned int handletype, const SQLHANDLE& handle)
{
	SQLCHAR sqlstate[1024];
	SQLCHAR message[1024];
	if (SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
		cout << "Message: " << message << "\nSQLSTATE: " << sqlstate << endl;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//int _tmain(int argc, _TCHAR* argv[])
int main()
{
	rename1();
	bool blnKNNTrainingSuccessful = loadKNNDataAndTrainKNN();           // attempt KNN training

	if (blnKNNTrainingSuccessful == false) {                            // if KNN training was not successful
																		// show error message
		std::cout << std::endl << std::endl << "error: error: KNN traning was not successful" << std::endl << std::endl;
		return(0);                                                      // and exit program
	}

	cv::Mat imgOriginalScene;           // input image

	imgOriginalScene = cv::imread("C:\\Users\\DELL\\Google Drive\\hi\\image.png");         // open image

	if (imgOriginalScene.empty()) {                             // if unable to open image
		std::cout << "error: image not read from file\n\n";     // show error message on command line
		_getch();                                               // may have to modify this line if not using Windows
		return(0);                                              // and exit program
	}

	std::vector<PossiblePlate> vectorOfPossiblePlates = detectPlatesInScene(imgOriginalScene);          // detect plates

	vectorOfPossiblePlates = detectCharsInPlates(vectorOfPossiblePlates);                               // detect chars in plates

	cv::imshow("imgOriginalScene", imgOriginalScene);           // show scene image

	if (vectorOfPossiblePlates.empty()) {                                               // if no plates were found
		std::cout << std::endl << "no license plates were detected" << std::endl;       // inform user no plates were found
	}
	else {                                                                            // else
																					  // if we get in here vector of possible plates has at leat one plate

																					  // sort the vector of possible plates in DESCENDING order (most number of chars to least number of chars)
		std::sort(vectorOfPossiblePlates.begin(), vectorOfPossiblePlates.end(), PossiblePlate::sortDescendingByNumberOfChars);

		// suppose the plate with the most recognized chars (the first plate in sorted by string length descending order) is the actual plate
		PossiblePlate licPlate = vectorOfPossiblePlates.front();

		cv::imshow("imgPlate", licPlate.imgPlate);            // show crop of plate and threshold of plate
		cv::imshow("imgThresh", licPlate.imgThresh);

		if (licPlate.strChars.length() == 0) {                                                      // if no chars were found in the plate
			std::cout << std::endl << "no characters were detected" << std::endl << std::endl;      // show message
			return(0);                                                                              // and exit program
		}

		drawRedRectangleAroundPlate(imgOriginalScene, licPlate);                // draw red rectangle around plate
		string number;
		number = licPlate.strChars;
		int n;
		n = number.length();
		char char_array[20];
		strcpy_s(char_array, number.c_str());

		std::cout << std::endl << "license plate read from image = " << char_array << std::endl;     // write license plate text to std out
		std::cout << std::endl << "-----------------------------------------" << std::endl;

		SQLHANDLE sqlenvhandle;
		SQLHANDLE sqlconnectionhandle = NULL;
		SQLHANDLE sqlstatementhandle = NULL;
		SQLRETURN retcode;
		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle))
			goto FINISHED;
		if (SQL_SUCCESS != SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
			goto FINISHED;
		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle))
			goto FINISHED;

		SQLCHAR retconstring[1024];
		switch (SQLDriverConnect(sqlconnectionhandle,
			NULL,
			(SQLCHAR*)"DSN=OracleTest;UID=krysia;PWD=krysia;",
			SQL_NTS,
			retconstring,
			1024,
			NULL,
			SQL_DRIVER_NOPROMPT))
		{
		case SQL_SUCCESS_WITH_INFO:
			show_error(SQL_HANDLE_DBC, sqlconnectionhandle);
			break;
		case SQL_INVALID_HANDLE:
		case SQL_ERROR:
			show_error(SQL_HANDLE_DBC, sqlconnectionhandle);
			goto FINISHED;
		default:
			break;
		}

		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle))
			goto FINISHED;

		if (SQL_SUCCESS != SQLExecDirect(sqlstatementhandle, (SQLCHAR*)" select * from vehicle full outer join owner_details on vehicle.number_plate_no = owner_details.number_plate_no full outer join fingerprint_scanner on  fingerprint_scanner.phone_no = owner_details.phone_no", SQL_NTS))
		{
			show_error(SQL_HANDLE_STMT, sqlstatementhandle);
			goto FINISHED;
	    }
		else
		{
			
			char number_plate_no[64], model_name[64], vehicle_type[64];
			int registration_number,user_id;
			char phone_no[64];
			char address[64], name[64], email_ID[64];
			int id, x, flag = 0;
			while (SQLFetch(sqlstatementhandle) == SQL_SUCCESS) {
				SQLGetData(sqlstatementhandle, 1, SQL_C_CHAR, number_plate_no, 64, NULL);
				SQLGetData(sqlstatementhandle, 2, SQL_C_CHAR, model_name, 64, NULL);
				SQLGetData(sqlstatementhandle, 3, SQL_C_CHAR, vehicle_type, 64, NULL);
				SQLGetData(sqlstatementhandle, 4, SQL_C_ULONG, &registration_number, 0, NULL);
				SQLGetData(sqlstatementhandle, 5, SQL_C_CHAR, number_plate_no, 64, NULL);
				SQLGetData(sqlstatementhandle, 6, SQL_C_CHAR, name, 64, NULL);
				SQLGetData(sqlstatementhandle, 7, SQL_C_CHAR,phone_no, 64, NULL);
				SQLGetData(sqlstatementhandle, 8, SQL_C_CHAR, email_ID, 64, NULL);
				SQLGetData(sqlstatementhandle, 9, SQL_C_CHAR, address, 64, NULL);
				SQLGetData(sqlstatementhandle, 10, SQL_C_ULONG, &user_id, 64, 0);
				if (strcmp(number_plate_no, char_array) == 0)
				{
					flag++;



					if (flag == 1)
					{
						cout << endl<<endl;
						cout << "\t\t\t\tVEHICLE'S DETAILS" << endl<<endl;
					    cout << "number plate no ="<<" " <<number_plate_no << " " <<"model name ="<< " " << model_name << " " <<" vehicle type ="<< " " << vehicle_type << " " << "registration number ="<< " " << registration_number << endl;
						cout <<endl<<endl<< "\t\t\t\tOWNERS'S DETAILS" << endl<<endl;
					    cout<<" "<<"owner's name="<< name << " "<<"phone_no ="<< " " << phone_no << " " <<"email_ID ="<< " " << email_ID << " " << "address ="<< " " << address << endl;
						
						cout << " "<<"User-ID=" << user_id << endl;
					
					}
				}

			}
			if (flag != 0)
			{cout<<endl<<endl;
				cout << "vehicle registered:number plate matched";
			}
			else
			{
				cout << "vehicle unregistered:number plate not found ";
			}
		}

	FINISHED:
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);

























		writeLicensePlateCharsOnImage(imgOriginalScene, licPlate);              // write license plate text on the image

		cv::imshow("imgOriginalScene", imgOriginalScene);                       // re-show scene image

		cv::imwrite("imgOriginalScene.png", imgOriginalScene);                  // write image out to file
	}
	delete1();
	cv::waitKey(0);                 // hold windows open until user presses a key
	_getch();
	return(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawRedRectangleAroundPlate(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
	cv::Point2f p2fRectPoints[4];

	licPlate.rrLocationOfPlateInScene.points(p2fRectPoints);            // get 4 vertices of rotated rect

	for (int i = 0; i < 4; i++) {                                       // draw 4 red lines
		cv::line(imgOriginalScene, p2fRectPoints[i], p2fRectPoints[(i + 1) % 4], SCALAR_RED, 2);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void writeLicensePlateCharsOnImage(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
	cv::Point ptCenterOfTextArea;                   // this will be the center of the area the text will be written to
	cv::Point ptLowerLeftTextOrigin;                // this will be the bottom left of the area that the text will be written to

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;                              // choose a plain jane font
	double dblFontScale = (double)licPlate.imgPlate.rows / 30.0;            // base font scale on height of plate area
	int intFontThickness = (int)std::round(dblFontScale * 1.5);             // base font thickness on font scale
	int intBaseline = 0;

	cv::Size textSize = cv::getTextSize(licPlate.strChars, intFontFace, dblFontScale, intFontThickness, &intBaseline);      // call getTextSize

	ptCenterOfTextArea.x = (int)licPlate.rrLocationOfPlateInScene.center.x;         // the horizontal location of the text area is the same as the plate

	if (licPlate.rrLocationOfPlateInScene.center.y < (imgOriginalScene.rows * 0.75)) {      // if the license plate is in the upper 3/4 of the image
																							// write the chars in below the plate
		ptCenterOfTextArea.y = (int)std::round(licPlate.rrLocationOfPlateInScene.center.y) + (int)std::round((double)licPlate.imgPlate.rows * 1.6);
	}
	else {                                                                                // else if the license plate is in the lower 1/4 of the image
																						  // write the chars in above the plate
		ptCenterOfTextArea.y = (int)std::round(licPlate.rrLocationOfPlateInScene.center.y) - (int)std::round((double)licPlate.imgPlate.rows * 1.6);
	}

	ptLowerLeftTextOrigin.x = (int)(ptCenterOfTextArea.x - (textSize.width / 2));           // calculate the lower left origin of the text area
	ptLowerLeftTextOrigin.y = (int)(ptCenterOfTextArea.y + (textSize.height / 2));          // based on the text area center, width, and height

																							// write the text on the image
	cv::putText(imgOriginalScene, licPlate.strChars, ptLowerLeftTextOrigin, intFontFace, dblFontScale, SCALAR_YELLOW, intFontThickness);
}


