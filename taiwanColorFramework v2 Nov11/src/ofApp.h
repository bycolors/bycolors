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

        void resetkPhotoMonoInfo();
        void loadAssets(int _numOfColor);
        void kPhotoCrossFade();


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
    
        //cross fade related:
        ofShader shader;
        ofImage* backgroundImage;
        ofImage* foregroundImage;
        float fadeMaskAlpha;
        int crossFadeSpeed;

        ofFbo fadeMaskFbo;
        void updateFadeMask(int _fadeSpeed);
        void resetFadeMask();


    
        //photo related;
        ofImage kPhotoImg;
        ofImage kPhotoMonoInfoImg;
        ofImage kPhotoMonoImg;
    
        ofFbo   kPhotoFbo;
        ofFbo   fadingFbo;
        ofFbo   tPhotoFbo;
        ofFbo   photoBombFbo;
        ofImage kColorImg;
        ofImage tColorImg;
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
        float   tPhotoScanBarPos;
    
        //timing related
        int frameCounter;

        int kPhotoSmallHoldTime;
    
        int kPhotoHoldTime;
        int kPhotoMonoInfoHoldTime;    //how long should kPhotoMonoInfo hold before kPhotoMono)comes in
        int kPhotoMonoHoldTime;
    
        int kColorHoldTime;    //how long should kColor hold
    
        int tColorHoldTime;    //how long should tColor hold
    
        int tPhotoHoldTime;
        int tPhotoInfoHoldTime;
        int tPhotoMonoHoldTime;
        int tPhotoMonoInfoHoldTime;
    
        int tColorNameHoldTime;

        int photoBombHoldTime; //delay before photo bomb

        int photoBombSwitchWaitCount;
        int photoBombSwitchHoldTime; //delay between each photo
    
        int photoBombSwitchSpeed; //basic delay of switching, the higher the slower
        int photoBombSwitchAccel; //acceleration of switching, the higher the faster
    
        int photoBombClosingHoldTime;

        int mapHoldTime;
    
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
            STATE_KPHOTO_MONO_INFO_IN = 3,
            STATE_KPHOTO_MONO_IN = 4,
            STATE_KCOLOR_IN = 5,
            STATE_TCOLOR_IN = 6,
            STATE_TPHOTO_MONO_IN = 7,
            STATE_TPHOTO_MONO_INFO_IN = 8,
            STATE_TPHOTO_INFO_IN = 9,
            STATE_TCOLOR_FILL = 10,
            STATE_TNAME_IN = 11,
            STATE_PHOTO_BOMB = 12,
            STATE_PHOTO_BOMB_CLOSING = 13,
            STATE_MAP = 14,
            STATE_ENDING = 15

        };
    
    
        struct colorPicker{
            float x;
            float y;
            ofColor hexColor;
        };
    
        colorPicker allColorPickers[10]={
            //tPhoto-c1-1-mono-info ~ tPhoto-c1-10-mono-info
            {980,270,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)},
            {0,0,ofColor::fromHex(0xB87167)}
        };
};
