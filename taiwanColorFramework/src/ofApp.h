#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "spinningWheel.h"
#include "ofxAnimatableFloat.h"
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector



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
		void updateMovie();

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
    
        //photo related;
        ofImage kPhotoImg;
        ofImage kPhotoInfoImg;
        float   kPhotoInfoAlpha;
        ofFbo   kPhotoFbo;
        ofFbo   tPhotoFbo;
        ofFbo   photoBombFbo;
        ofImage kColorImg;
        ofImage tColorImg;
        ofImage tPhotoImg;
        ofImage tPhotoInfoImg;
        ofImage tPhotoMonoImg;
        ofImage tPhotoMonoInfoImg;
        ofImage tNameImg;
        ofImage photoBombImg;
        vector<int>    photoBombOrder; //storing the order of photo bomb
        int photoBombTotalNumb;
        int photoBombOrderIndex =0;
        ofImage photoBombClosingImg;
        ofImage mapAImg;
        ofImage mapBImg;
        int   mapSpeed;
        ofImage endingImg;
        float   endingAlpha;
    
        //timing related
        int kPhotoSmallWaitCount;
        int kPhotoSmallHoldTime;
    
        int kPhotoInfoWaitCount; //counter for kPhotoInfo being Idle
        int kPhotoInfoHoldTime;    //how long should kPhotoInfo hold before next (kcolor)comes in
    
        int kColorWaitCount; //counter for kColor being Idle
        int kColorHoldTime;    //how long should kColor hold
    
        int tColorWaitCount; //counter for tColor being Idle
        int tColorHoldTime;    //how long should tColor hold
    
        int tPhotoWaitCount;
        int tPhotoHoldTime;
        int tPhotoMonoHoldTime;
        int tPhotoMonoInfoHoldTime;
    
        int tColorNameWaitCount;
        int tColorNameHoldTime;

        int photoBombWaitCount;
        int photoBombHoldTime; //delay before photo bomb

        int photoBombSwitchWaitCount;
        int photoBombSwitchHoldTime; //delay between each photo
    
        int photoBombSwitchSpeed; //basic delay of switching, the higher the slower
        int photoBombSwitchAccel; //acceleration of switching, the higher the faster
    
        int photoBombClosingWaitCount;
        int photoBombClosingHoldTime;

        int mapWaitCount;
        int mapHoldTime;
    
        int endingWaitCount;
        int endingHoldTime;
    
        //ofxAnimatable related;
        ofxAnimatableFloat animatedPhotoPos;
        ofxAnimatableFloat animatedPhotoSize;
        ofxAnimatableFloat kPhotoPosYoffset;
        ofxAnimatableFloat animatedCircleSize;

    
        //sensor related
        ofSerial    serial;
        int detectedColor;
    
        enum Constants
        {
            STATE_START = 0,
            STATE_DETECTED = 1,
            STATE_KPHOTO_IN = 2,
            STATE_KCOLOR_IN = 3,
            STATE_TCOLOR_IN = 4,
            STATE_TPHOTO_IN = 5,
            STATE_TCOLOR_FILL = 6,
            STATE_TNAME_IN = 7,
            STATE_PHOTO_BOMB = 8,
            STATE_PHOTO_BOMB_CLOSING = 9,
            STATE_MAP = 10,
            STATE_ENDING = 11

        };
};
