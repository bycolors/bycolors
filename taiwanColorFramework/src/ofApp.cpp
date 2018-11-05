#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    state = 0;
    
    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    movieFbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA);
    colorCardFbo.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA,8);
    
    movieFbo.begin();
    ofClear(255, 255, 255,0);
    movieFbo.end();
    
    colorCardFbo.begin();
    ofClear(255,0);
    colorCardFbo.end();

    
    mask.load("mask.png");
    logoTop.load("logoTop-screenshot.png");
    logoTopSVG.load("logo-top.svg");
    
    infoFont.load("DIN.otf", 18);
    titleFont.load("Tribute Roman", 64);
    
    ofTrueTypeFontSettings settings("Adobe Heiti Std R.ttf", 64);
    settings.antialiased = true;
    settings.dpi = 72;
    settings.direction = OF_TTF_LEFT_TO_RIGHT;
    settings.addRanges(ofAlphabet::Japanese);
    jpFontBig.load(settings);
    
    
    titleScale = 1;

    totalMovies = 3;
    nowPlayer = 0;
    loadingMovieId = 0;
//    playAndPreload();
    
    showInfo = false;
    
    if (XML.load("mySettings.xml")) {
        ofLog()<<"mySettings.xml loaded!";
        canvasOffsetX=XML.getValue("position:canvasOffsetX", 0);
        canvasOffsetY=XML.getValue("position:canvasOffsetY", 0);
        maskOffsetX=XML.getValue("position:maskOffsetX", 0);
        maskOffsetY=XML.getValue("position:maskOffsetY", 0);

    }else{
        XML.setValue("position:canvasOffsetX", 0);
        XML.setValue("position:canvasOffsetY", 0);
        XML.setValue("position:maskOffsetX", 0);
        XML.setValue("position:maskOffsetY", 0);
        XML.saveFile("mySettings.xml"); //puts settings.xml file in the bin/data folder
    }

}

//--------------------------------------------------------------
void ofApp::update(){
    switch (state) {
        case 0:
            //play movie loop;
        break;
        case 1: 
            //object detected!;
        break;
        
        default:
            break;
    }
    
    
    movies[nowPlayer].update();
    int totalFrames = movies[nowPlayer].getTotalNumFrames();
    int currentFrame = movies[nowPlayer].getCurrentFrame();
    if((currentFrame>0) && (totalFrames-currentFrame <1)){
        ofLog()<<"movie is done";
        // swap video player ID
        switchMovie();
    }
    
    
    movieFbo.begin();
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CENTER); //set rectangle mode to the center
    movies[nowPlayer].draw(movieFbo.getWidth()/2, movieFbo.getHeight()/2);
    ofPopStyle();
    movieFbo.end();
    
    colorCardFbo.begin();
    
    

    ofBackground(39, 85, 128);
    ofFill();
    ofPushMatrix();
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CENTER);
//        glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

   // glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    //ofTranslate(colorCardFbo.getWidth()/2, colorCardFbo.getHeight()/2);
    //ofSetColor(255,255,255,255);
    logoTop.draw(ofGetWidth()/2,ofGetHeight()/2);
  //  ofSetColor(255,0,0);
//    ofFill();
//    logoTopSVG.draw();
    //if(titleScale<1.5) titleScale+=0.01;
    //else    titleScale = 1;
    //titleFont.drawString("byColors", ofGetWidth()/2,ofGetHeight()/2);
    ofPopStyle();
    ofPopMatrix();
    colorCardFbo.end();
    
    std::stringstream strm;
	strm << "fps: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());

    
}

//--------------------------------------------------------------
void ofApp::draw(){
    movieFbo.draw(canvasOffsetX,canvasOffsetY);
    
  //  glEnable(GL_BLEND);
//    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    //    glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    ofPushStyle();
//    ofSetColor(255,255,255,255);
    ofSetRectMode(OF_RECTMODE_CENTER);
    colorCardFbo.draw(ofGetWidth()/2+maskOffsetX, ofGetHeight()/2+maskOffsetY);
    mask.draw(ofGetWidth()/2+maskOffsetX, ofGetHeight()/2+maskOffsetY);
    ofPopStyle();
    

    
    
    if (showInfo) {
        infoFont.drawString("Next Movie ID: "+ofToString(loadingMovieId), 50, 70);
        infoFont.drawString("Total Frames: "+ofToString(movies[nowPlayer].getTotalNumFrames()), 50, 100);
        infoFont.drawString("Current Frame: "+ofToString(movies[nowPlayer].getCurrentFrame()), 50, 130);
    }

//    ofSetColor(255,0,0);

  //  logoTopSVG.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch (key) {
        case OF_KEY_LEFT:
            canvasOffsetX--;
        break;
        case OF_KEY_RIGHT:
            canvasOffsetX++;
        break;
        case OF_KEY_UP:
            canvasOffsetY++;
        break;
        case OF_KEY_DOWN:
            canvasOffsetY--;
        break;
        case 'a':
            maskOffsetX--;
            break;
        case 'd':
            maskOffsetX++;
            break;
        case 'w':
            maskOffsetY++;
            break;
        case 's':
            maskOffsetY--;
            break;

        case 'r':
            switchMovie();
        break;

        case 'i':
            showInfo=!showInfo;
        break;

            
        default:
            break;
    }
    
    XML.setValue("position:canvasOffsetX", canvasOffsetX);
    XML.setValue("position:canvasOffsetY", canvasOffsetY);
    XML.setValue("position:maskOffsetX", maskOffsetX);
    XML.setValue("position:maskOffsetY", maskOffsetY);

    XML.saveFile("mySettings.xml"); //puts settings.xml file in the bin/data folder
}


//--------------------------------------------------------------
void ofApp::playAndPreload(){
    // PLAY the current movie
    
    // need to load the first movie : should only happen once
    if( !movies[nowPlayer].isLoaded() ){
        movies[nowPlayer].load( ofToString(loadingMovieId) + ".mp4" );
    }
    
    movies[nowPlayer].setLoopState(OF_LOOP_NORMAL);
    movies[nowPlayer].play();
    
    ofLog()<<"playing: "<<loadingMovieId<<".mp4";
    
    // PRELOAD the next movie
    
    // get a unique random ID
    int preloadId = floor(ofRandom(totalMovies));
    while (preloadId == loadingMovieId ) {
        preloadId = floor(ofRandom(totalMovies));
    }

    // set the loading movie to the new ID
    loadingMovieId = preloadId;
    
    // load movie in the player that isn't 'nowPlayer'
    int nextPlayer = nowPlayer == 0 ? 1 : 0;
    movies[nextPlayer].load( ofToString(loadingMovieId) + ".mp4" );
    
    ofLog()<<"loading: : "<<loadingMovieId<<".mp4";

}

//--------------------------------------------------------------
void ofApp::switchMovie(){
    // swap video player ID
    nowPlayer = ( nowPlayer == 0 ) ? 1 : 0;
    
    playAndPreload();
    
    // stop currently playing video
    movies[!nowPlayer].stop();

}


//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
