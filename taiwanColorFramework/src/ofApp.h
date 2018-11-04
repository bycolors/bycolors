#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

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
        void playAndPreload();
        void switchMovie();
    
        ofFbo   movieFbo;
        ofFbo   colorCardFbo;
        int     canvasOffsetX;
        int     canvasOffsetY;
        int     maskOffsetX;
        int     maskOffsetY;
        ofImage mask;

    
        ofAVFoundationPlayer  movies[2];
        int            nowPlayer; //which movie player (only 2);
        int            loadingMovieId; //what's name of next movie file
        int            totalMovies;
    
        ofxXmlSettings XML;
    
        int     state;
    
        ofTrueTypeFont		infoFont;
        ofTrueTypeFont		titleFont;

        bool    showInfo;

};
