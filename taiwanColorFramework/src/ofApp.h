#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "spinningWheel.h"



class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();


		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void checkSerial();
		void playMovie();

        void playAndPreload();
        void switchMovie();
        void drawFrame(float _x, float _y, float _w, float _h, ofColor c);
        void drawKPhoto(float _x, float _y, float _size);
        void resetKPhoto();

        void drawKPhotoInfo(float _x, float _y, float _size, float _alpha);
        void resetKPhotoInfo();


        //layout related
        ofFbo	movieFbo;
        int	canvasOffsetX;
        int	canvasOffsetY;
        int	maskOffsetX;
        int	maskOffsetY;

        //movie related
        ofAVFoundationPlayer	movies[2];
        int	nowPlayer; //which movie player (only 2);
        int	loadingMovieId; //what's name of next movie file
        int	totalMovies;

        //control related
        int	state;
        bool    showInfo;
        ofxXmlSettings XML;
        float screenWidth;
        float screenHeight;

        //graphics related
        ofTrueTypeFont	searchingFont;
        ofTrueTypeFont	frameFont;

        SpinningWheel colorWheel;
        float kPhotoSize;
    
        //photo related;
        ofImage kPhoto;
        ofImage kPhotoInfo;
        float   kPhotoInfoAlpha;
        ofFbo   kPhotoFbo;
        int kPhotoInfoWaitCount;
        int kColorDelay;
        float kPhotoFboXoffset;
        ofImage kColor;



    
        //sensor related
        ofSerial    serial;
        int detectedColor;
    
        enum Constants
        {
            STATE_START = 0,
            STATE_DETECTED = 1,
            STATE_KPHOTO_IN = 2,
            STATE_KCOLOR_IN = 3

        };
};
