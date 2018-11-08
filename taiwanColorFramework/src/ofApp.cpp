#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofLog()<<"screen resolution: "<<ofGetWidth()<<" x "<<ofGetHeight();
    screenWidth = ofGetWidth();
    screenHeight = ofGetHeight();
    state = 0;

    serial.listDevices();
  	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 9600;
  	serial.setup(1, baud); //open the first device
    detectedColor = 0;

    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);
    ofEnableAlphaBlending();
    ofEnableSmoothing();

    //square canvas
    movieFbo.allocate(screenHeight,screenHeight,GL_RGBA);
    kPhotoFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample

    movieFbo.begin();
    ofClear(255, 255, 255,0);
    movieFbo.end();

    kPhotoFbo.begin();
    ofClear(255, 255, 255,0);
    kPhotoFbo.end();

    searchingFont.load("Skia.ttf", 17);
    frameFont.load("Skia.ttf", 24);

    kPhotoSize = kPhotoFbo.getWidth()*0.75;
    kPhotoInfoAlpha = 0;
    kPhotoInfoWaitCount = 0;
    kColorDelay = 100;
    kPhotoFboXoffset = 0;

    colorWheel.setup(screenHeight*0.051,screenHeight*0.037,5.625); //outter radius, inner radius, rotation speed

    totalMovies = 2;
    nowPlayer = 0;
    loadingMovieId = 0;
    playAndPreload();

    showInfo = true;

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
        case STATE_START:
            //play movie loop;
            checkSerial();
            playMovie();
        break;
        case STATE_DETECTED:
            //object detected! searching;
            movies[nowPlayer].stop();
            if(colorWheel.times<2){
                colorWheel.update();
            }else{
                colorWheel.resetTimes();
                state = STATE_KPHOTO_IN;
            }
        break;
        case STATE_KPHOTO_IN:
            //show kyoto photo, zoom, show text;
            if(kPhotoSize<kPhotoFbo.getHeight()){
                kPhotoSize+=5; //todo::add easing
            }else{
                kPhotoSize = kPhotoFbo.getHeight();
                if(kPhotoInfoAlpha<255){
                    kPhotoInfoAlpha+=5;
                }else{
                    resetKPhoto();
                    state =STATE_KCOLOR_IN;
                }
            }
            kPhotoFbo.begin();
                ofPushMatrix();
                ofPushStyle();
                ofTranslate(kPhotoFbo.getWidth()/2, kPhotoFbo.getHeight()/2);
                ofSetRectMode(OF_RECTMODE_CENTER);

                drawKPhoto(0,0,kPhotoSize);
                drawKPhotoInfo(0,0,kPhotoFbo.getWidth(),kPhotoInfoAlpha);
            
                ofSetRectMode(OF_RECTMODE_CORNER);
                drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::white);
            
                ofPopStyle();
                ofPopMatrix();

            kPhotoFbo.end();

        break;
        case STATE_KCOLOR_IN:
            //show kyoto color;
            if(kPhotoInfoWaitCount<kColorDelay){
                kPhotoInfoWaitCount++;
            }else{
                kPhotoFboXoffset+=5;
            }


        break;
        case 4:
            //show taiwan color, pan in;

        break;
        case 5:
            //show taiwan photo, mono tone, show text;

        break;
        case 6:
            //show taiwan color;

        break;
        case 7:
            //show photo bombs;

        break;
        case 8:
            //show ending;

            playAndPreload();
            state = 0;

        break;


        default:
          break;
    }


//     colorCardFbo.begin();
//
//     ofBackground(39, 85, 128);
//     ofFill();
//     ofPushMatrix();
//     ofPushStyle();
//     ofSetRectMode(OF_RECTMODE_CENTER);
// //        glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//    // glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//     //ofTranslate(colorCardFbo.getWidth()/2, colorCardFbo.getHeight()/2);
//     //ofSetColor(255,255,255,255);
//     logoTop.draw(ofGetWidth()/2,ofGetHeight()/2);
//   //  ofSetColor(255,0,0);
// //    ofFill();
// //    logoTopSVG.draw();
//     //if(titleScale<1.5) titleScale+=0.01;
//     //else    titleScale = 1;
//     //titleFont.drawString("byColors", ofGetWidth()/2,ofGetHeight()/2);
//     ofPopStyle();
//     ofPopMatrix();
//     colorCardFbo.end();

    //show fps on title bar
    std::stringstream strm;
	strm << "fps: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());


}

//--------------------------------------------------------------
void ofApp::draw(){
    switch (state) {
        case STATE_START:
            //play movie loop;
            
            movieFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY);
        break;
        case STATE_DETECTED:
            //object detected! searching;

            ofPushMatrix();
            ofPushStyle();
            ofTranslate(screenWidth/2, screenHeight/2);
            
            colorWheel.draw(0,-kPhotoFbo.getWidth()*0.045);
            drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::black);
            ofSetColor(ofColor::black);
            searchingFont.drawString("searching...", -searchingFont.stringWidth("searching...")/2, kPhotoFbo.getWidth()*0.04);
            ofPopStyle();
            ofPopMatrix();


        break;
        case STATE_KPHOTO_IN:
            //show kyoto photo, zoom, show text;
            kPhotoFbo.draw((ofGetWidth()-ofGetHeight())/2,0);


        break;
        case STATE_KCOLOR_IN:
            //show kyoto color;
            kPhotoFbo.draw((screenWidth-screenHeight)/2-kPhotoFboXoffset,0);
            kColor.draw((screenWidth+screenHeight)/2-kPhotoFboXoffset,0,screenHeight,screenHeight);

            
        break;
        case 4:
            //show taiwan color, pan in;

        break;
        case 5:
            //show taiwan photo, mono tone, show text;

        break;
        case 6:
            //show taiwan color;

        break;
        case 7:
            //show photo bombs;

        break;
        case 8:
            //show ending;

        break;

        default:
        break;
    }


//
//   //  glEnable(GL_BLEND);
// //    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//     //    glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//     ofPushStyle();
// //    ofSetColor(255,255,255,255);
//     ofSetRectMode(OF_RECTMODE_CENTER);
//     colorCardFbo.draw(ofGetWidth()/2+maskOffsetX, ofGetHeight()/2+maskOffsetY);
//     mask.draw(ofGetWidth()/2+maskOffsetX, ofGetHeight()/2+maskOffsetY);
//     ofPopStyle();
//
    
    



    if (showInfo) {
        ofDrawBitmapString("Next Movie ID: "+ofToString(loadingMovieId), 50, 70);
        ofDrawBitmapString("Total Frames: "+ofToString(movies[nowPlayer].getTotalNumFrames()), 50, 100);
        ofDrawBitmapString("Current Frame: "+ofToString(movies[nowPlayer].getCurrentFrame()), 50, 130);
    }



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

        case '0':
            playAndPreload();
            state = STATE_START;
        break;
        case '1':
            kPhoto.load("kPhotos/1.jpg");
            kPhotoInfo.load("kPhotos/1-info.jpg");
            kColor.load("kColors/1.jpg");


            state = STATE_DETECTED;
        break;
        case '2':
            state = 2;
        break;
        case '3':
            state = 3;
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
        movies[nowPlayer].load( "movies/"+ofToString(loadingMovieId) + ".mp4" );
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
    movies[nextPlayer].load(  "movies/"+ofToString(loadingMovieId) + ".mp4" );
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
void ofApp::playMovie(){
  movies[nowPlayer].update();
  int totalFrames = movies[nowPlayer].getTotalNumFrames();
  int currentFrame = movies[nowPlayer].getCurrentFrame();
  if((currentFrame>0) && (totalFrames-currentFrame <2000)){
      ofLog()<<"movie is done";
      // swap video player I    D
      switchMovie();
  }


  movieFbo.begin();
  // ofPushStyle();
  // ofSetRectMode(OF_RECTMODE_CENTER); //set rectangle mode to the center
  // movies[nowPlayer].draw(movieFbo.getWidth()/2, movieFbo.getHeight()/2);
  movies[nowPlayer].draw(0,0,movieFbo.getWidth(),movieFbo.getHeight());
  // ofPopStyle();
  movieFbo.end();

}
//--------------------------------------------------------------
void ofApp::checkSerial(){
  int myByte = 0;
  myByte = serial.readByte();
  if ( myByte == OF_SERIAL_NO_DATA ){}
    // printf("no data was read");
  else if ( myByte == OF_SERIAL_ERROR ){}
    // printf("an error occurred");
  else if ( myByte == 99){
    ofLog()<<"no matching color";
  }
  else{
    ofLog()<<"detected color: "<< myByte;
    detectedColor = myByte;
      
//      kPhoto.load("kPhotos/1.jpg");
//      kPhotoInfo.load("kPhotos/1-info.jpg");
//      kColor.load("kColors/1.jpg");

    state = STATE_DETECTED;
  }
}
//--------------------------------------------------------------


void ofApp::drawFrame(float _x, float _y, float _w, float _h, ofColor c){

    ofPath path;
    path.setFillColor(c);
//    path.rectangle(-120, -195, 240, 335);
//    path.rectangle(-110, -173, 220, 220);
    path.rectangle(_x, _y, _w, _h);
    path.rectangle(_x+_w*0.04, _y+_w*0.083, _w*0.92, _w*0.92);

    path.draw();

    
    ofSetColor(c.getInverted());
    ofPushMatrix();
    ofTranslate(_x+_w/2, _y+_h);
    float strWidth = frameFont.stringWidth("#by colors");
    frameFont.drawString("#by colors",-strWidth/2,-_h*.1);
    ofPopMatrix();

}

//--------------------------------------------------------------


void ofApp::drawKPhoto(float _x, float _y, float _size){
    
    kPhoto.draw(_x,_y,_size,_size);
}

//--------------------------------------------------------------


void ofApp::drawKPhotoInfo(float _x, float _y, float _size, float _alpha){
    ofSetColor(255, 255, 255, _alpha);
    kPhotoInfo.draw(_x,_y,_size,_size);
}

//--------------------------------------------------------------


void ofApp::resetKPhoto(){
    kPhotoFbo.getWidth()*0.75;
}

void ofApp::resetKPhotoInfo(){
    kPhotoInfoAlpha = 0;
    kPhotoInfoWaitCount = 0;
}



//--------------------------------------------------------------


void ofApp::exit(){

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
