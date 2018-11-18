#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "spinningWheel.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"

#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector
#include <math.h>


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
        void loadAssets(float _numOfColor);
        void PhotoCrossFade();
    
        void resetGridAnimation();


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
    
        void loadFadeImages(ofImage* _foreground, ofImage* _background);

    
        //photo related;
        ofImage kPhotoImg;
        ofImage kPhotoMonoInfoImg;
        ofImage kPhotoMonoImg;
    
        ofFbo   kPhotoFbo;
        ofFbo   fadingFbo;
        ofFbo   tPhotoFbo;
        ofFbo   tPhotoGridFbo;

        ofImage tPhotoGridImg;
        ofImage kColorImg;
        ofImage tColorImg;
        ofImage tPhotoInfoImg;
        ofImage tPhotoMonoImg;
        ofImage tPhotoMonoInfoImg;
        ofImage tClosingImg;
        ofImage tClosingMask;
        ofImage tGridPhotos[8];
        ofImage mapAImg;
        ofImage mapBImg;
        ofImage searchingImg;
        ofImage byColorImg;
        ofImage wrongColorImg;
        int   mapSpeed;
        ofImage endingImg;
        float   endingAlpha;
        float   tPhotoScanBarPos;
        float   tClosingAlpha;
    
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
        int tPhotoMono2HoldTime;
        int wrongColorHoldTime;
    
    
        int tColorNameHoldTime;

        int tClosingHoldTime;

        int mapHoldTime;
    
        int endingHoldTime;
    
        //ofxAnimatable related;
        ofxAnimatableFloat animatedPhotoPos;
        ofxAnimatableFloat animatedPhotoSize;
        ofxAnimatableFloat kPhotoPosYoffset;
        ofxAnimatableFloat animatedCircleSize;
    
        ofxAnimatableFloat tPhotoGridSizeScale;
        ofxAnimatableOfPoint tGridPos;
    
        ofxAnimatableOfPoint tGridPhotoPos[8];
    
    
        float gridSize;
        float photoSize;

    
        //sensor related
        ofSerial    serial;
        int detectedColor;
        int selectedTPhoto;
        int selectedGridIndex;
        bool		bSendSerialMessage;			// a flag for sending serial

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
            STATE_TPHOTO_MONO_IN_2 = 10,
            STATE_TPHOTO_GRID = 11,
            STATE_TCOLOR_FILL = 12,
            STATE_TCLOSING_IN = 13,
            STATE_MAP = 14,
            STATE_ENDING = 15,
            STATE_WRONG_COLOR = 16

        };
    
    
        struct gridLocator{
            float x;
            float y;
        };
    
        gridLocator allGridLocators[8]={
            {2582,2582},
            {230,230},
            {230,2582},
            {2582,230},
            {1406,230},
            {230,1406},
            {2582,1406},
            {1406,2582}
        };
    
    
        ofColor allColorPickers[14]={
            //tPhoto-c1-1-mono-info ~ tPhoto-c1-10-mono-info
            //color code for t color
            ofColor::fromHex(0xD34D42), //c1 red1
            ofColor::fromHex(0xB87167),
            ofColor::fromHex(0xDE632E), //c3 orange1
            ofColor::fromHex(0xE18738),
            ofColor::fromHex(0xE4B57F), //c5 yellow1
            ofColor::fromHex(0xF0B537),
            ofColor::fromHex(0x1E4327), //c6 green1
            ofColor::fromHex(0x585F36),
            ofColor::fromHex(0x2E3A79), //c9 blue1
            ofColor::fromHex(0x4AB6C8),
            ofColor::fromHex(0x443120), //c11 brown1
            ofColor::fromHex(0xA57C66),
            ofColor::fromHex(0xE2DDDB), //c13 gray1
            ofColor::fromHex(0x707580),

        };
};
