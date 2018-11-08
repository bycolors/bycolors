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

    //square canvas
    movieFbo.allocate(screenHeight,screenHeight,GL_RGBA);
    kPhotoFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample
    tPhotoFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample
    photoBombFbo.allocate(screenHeight,screenHeight,GL_RGBA);

    movieFbo.begin();
    ofClear(255, 255, 255,0);
    movieFbo.end();

    kPhotoFbo.begin();
    ofClear(255, 255, 255,0);
    kPhotoFbo.end();

    tPhotoFbo.begin();
    ofClear(255, 255, 255,0);
    tPhotoFbo.end();

    photoBombFbo.begin();
    ofClear(255, 255, 255,0);
    photoBombFbo.end();

    searchingFont.load("Skia.ttf", 17);
    frameFont.load("Skia.ttf", 24);

    kPhotoInfoAlpha = 0;

    // timing variable
    kPhotoInfoWaitCount = 0;
    kColorDelay = 100;
    kColorWaitCount = 0;
    tColorDelay = 100;
    tColorWaitCount = 0;
    tPhotoDelay = 100;

    tPhotoWaitCount = 0;
    tPhotoMonoDelay = 100;
    tPhotoMonoInfoDelay = 50;
    tPhotoColorFillDelay = 100;

    tColorWaitCount = 0;
    photoBombDelay = 100;


    photoBombTotalNumb = 10;
    photoBombOrderIndex = 0;
    photoBombSwitchWaitCount = 0;
    photoBombSwitchSpeed = 10; //the higher the slower
    photoBombSwitchAccel = 7; //the higher the faster
    photoBombSwitchDelay = photoBombSwitchSpeed+(int)(pow(photoBombTotalNumb,2)/photoBombSwitchAccel);

    photoBombClosingWaitCount = 0;
    mapDelay = 100;


    ofLog()<<"photo bomb order";
    // set some values and shuffle it
    for (int i=0; i<photoBombTotalNumb; ++i) photoBombOrder.push_back(i); // 1 2 3 4 5 6 7 8 9
    random_shuffle ( photoBombOrder.begin(), photoBombOrder.end() );

    photoBombClosing.load("photoBombClosing.jpg");
    
    animatedPhotoPos.animateFromTo(0, screenHeight);
    animatedPhotoPos.setDuration(1);
    AnimCurve curve = (AnimCurve) (EASE_IN_EASE_OUT);
    animatedPhotoPos.setCurve( curve );

    animatedPhotoSize.animateFromTo(screenHeight*0.75, screenHeight);
    animatedPhotoSize.setDuration(1);
    animatedPhotoSize.setCurve( curve );

    animatedCircleSize.animateFromTo(25, screenHeight*2);
    animatedCircleSize.setDuration(2);
    AnimCurve curve2 = (AnimCurve) (TANH);
//    AnimCurve curve2 = (AnimCurve) (CUBIC_EASE_IN);

    animatedCircleSize.setCurve( curve2 );


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
            if(!animatedPhotoSize.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedPhotoSize.update( dt );
            }else{
                if(kPhotoInfoAlpha<255){
                    kPhotoInfoAlpha+=5;
                }else{
                    resetKPhoto();
                    animatedPhotoSize.animateFromTo(screenHeight*0.75, screenHeight);
                    state =STATE_KCOLOR_IN;
                }
            }


            kPhotoFbo.begin();
                ofPushMatrix();
                ofPushStyle();
                ofTranslate(kPhotoFbo.getWidth()/2, kPhotoFbo.getHeight()/2);
                ofSetRectMode(OF_RECTMODE_CENTER);

                ofEnableAlphaBlending();
                ofEnableSmoothing();

                drawKPhoto(0,0,animatedPhotoSize.val());
                drawKPhotoInfo(0,0,kPhotoFbo.getWidth(),kPhotoInfoAlpha);

                ofSetRectMode(OF_RECTMODE_CORNER);
                drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::white);

                ofDisableAlphaBlending();
                ofDisableSmoothing();


                ofPopStyle();
                ofPopMatrix();

            kPhotoFbo.end();



        break;
        case STATE_KCOLOR_IN:
            //show kyoto color;
            if(kPhotoInfoWaitCount<kColorDelay){
                kPhotoInfoWaitCount++;
            }else{
                if(!animatedPhotoPos.hasFinishedAnimating()){
                    float dt = 1.0f / 60.0f;
                    animatedPhotoPos.update( dt );

                }else{
                    animatedPhotoPos.animateFromTo(0,screenHeight);

                    resetKPhotoInfo();
                    kPhotoInfoWaitCount = 0;
                    kPhotoFbo.begin();
                    ofClear(255,255,255,0);
                    kPhotoFbo.end();
                    state = STATE_TCOLOR_IN;
                }
            }


        break;
        case STATE_TCOLOR_IN:
            //show taiwan color, pan in;
            if(kColorWaitCount<tColorDelay){
                kColorWaitCount++;
            }else{

                if(!animatedPhotoPos.hasFinishedAnimating()){
                    float dt = 1.0f / 60.0f;
                    animatedPhotoPos.update( dt );

                }else{
                    animatedPhotoPos.animateFromTo(0,screenHeight);
                    kColorWaitCount = 0;
                    state = STATE_TPHOTO_IN;
                }

            }

        break;
        case STATE_TPHOTO_IN:
            //show taiwan photo, mono tone, show text;
            if(tColorWaitCount<tPhotoDelay){
                tColorWaitCount++;
            }else{
                tPhotoWaitCount++;

                tPhotoFbo.begin();

                if(tPhotoWaitCount<tPhotoMonoDelay){ // hold tPhoto

                    tPhoto.draw(0,0,tPhotoFbo.getWidth(),tPhotoFbo.getHeight());

                }else if (tPhotoWaitCount<tPhotoMonoDelay+tPhotoMonoInfoDelay){ // hold tPhoto Mono

                    tPhotoMono.draw(0,0,tPhotoFbo.getWidth(),tPhotoFbo.getHeight());

                }else if (tPhotoWaitCount<tPhotoMonoDelay+tPhotoMonoInfoDelay+tPhotoColorFillDelay){// hold tPhoto Mono Info

                    tPhotoMonoInfo.draw(0,0,tPhotoFbo.getWidth(),tPhotoFbo.getHeight());

                    ofPushMatrix();
                    ofPushStyle();
                    ofEnableAlphaBlending();
                    ofEnableSmoothing();

                    ofTranslate(tPhotoFbo.getWidth()*980/1080,tPhotoFbo.getHeight()*270/1080);
                    // todo:: need to have an array hold the color circle's pos and color on all 80 photos

                    ofSetColor(ofColor::white);
                    ofSetCircleResolution(60);
                    ofDrawCircle(0,0,25+1);

                    ofSetColor(ofColor::fromHex(0xB87167));
                    ofDrawCircle(0,0,25);

                    ofDisableAlphaBlending();
                    ofDisableSmoothing();

                    ofPopStyle();
                    ofPopMatrix();


                }else {

                    tPhotoWaitCount = 0;
                    animatedCircleSize.animateFromTo(25, screenHeight*2);
                    state = STATE_TCOLOR_FILL;

                }

                tPhotoFbo.end();
            }


        break;
        case STATE_TCOLOR_FILL:
            //taiwan color fill;
            tPhotoFbo.begin();

            if(!animatedCircleSize.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedCircleSize.update( dt );

                ofPushMatrix();
                ofPushStyle();
                ofEnableAlphaBlending();
                ofEnableSmoothing();

                ofTranslate(tPhotoFbo.getWidth()*980/1080,tPhotoFbo.getHeight()*270/1080);
                // todo:: need to have an array hold the color circle's pos and color on all 80 photos

                ofSetCircleResolution(60);

                ofSetColor(ofColor::white);
                ofDrawCircle(0,0,animatedCircleSize.val()+1);

                ofSetColor(ofColor::fromHex(0xB87167));
                ofDrawCircle(0,0,animatedCircleSize.val());

                ofDisableAlphaBlending();
                ofDisableSmoothing();

                ofPopStyle();
                ofPopMatrix();

            }else{
                ofClear(255,255,255,0);
                state = STATE_TNAME_IN;
            }
            tPhotoFbo.end();

        break;
        case STATE_TNAME_IN:
            //show tCOLOR NAME;
            if(!animatedPhotoPos.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedPhotoPos.update( dt );

            }else{
                if(tColorNameWaitCount<photoBombDelay){
                    tColorNameWaitCount++;
                }else{
                    tColorNameWaitCount = 0;
                    animatedPhotoPos.animateFromTo(0,screenHeight);
                    state = STATE_PHOTO_BOMB;
                }
            }


        break;
        case STATE_PHOTO_BOMB:
            //show PHOTO BOMB;


            int photoBombCurrentPick;

            if(photoBombOrderIndex ==0){ //first time
                photoBombCurrentPick = *photoBombOrder.begin();
                ofLog()<<"photoBombCurrentPick: "<<photoBombCurrentPick;
                photoBombImg.load("tphotos/1-"+ofToString(photoBombCurrentPick)+"-mono.jpg");
                photoBombOrderIndex++;
                photoBombFbo.begin();
                photoBombImg.draw(ofRandom(-200,200),ofRandom(-200,200), photoBombFbo.getWidth(),photoBombFbo.getHeight());
                photoBombFbo.end();

            }

            if(photoBombSwitchWaitCount<photoBombSwitchDelay){
                ofLog()<<"photoBombSwitchWaitCount: "<<photoBombSwitchWaitCount;
                ofLog()<<"photoBombSwitchDelay: "<<photoBombSwitchDelay;

                photoBombSwitchWaitCount++;
            }else{
                ofLog()<<"new delay";

                //photoBombSwitchDelay =  pow((photoBombTotalNumb-photoBombOrderIndex),2)+10;   //new delay, shorter each time
                photoBombSwitchDelay = photoBombSwitchSpeed+(int)(pow(photoBombTotalNumb-photoBombOrderIndex,2)/photoBombSwitchAccel);
                photoBombSwitchWaitCount = 0; // reset counter

                if(photoBombOrderIndex<photoBombTotalNumb){
                    vector<int>::iterator it=photoBombOrder.begin()+photoBombOrderIndex;
                    photoBombCurrentPick = *it;
                    ofLog()<<"photoBombCurrentPick: "<<photoBombCurrentPick;
                    photoBombImg.load("tphotos/1-"+ofToString(photoBombCurrentPick)+"-mono.jpg");

                    photoBombOrderIndex++;
                }else{
                    photoBombSwitchWaitCount = 0;
                    photoBombOrderIndex = 0;
                    photoBombSwitchDelay = photoBombSwitchSpeed+(int)(pow(photoBombTotalNumb,2)/photoBombSwitchAccel);
                    random_shuffle ( photoBombOrder.begin(), photoBombOrder.end() );
                    
                    photoBombFbo.begin();
                    ofClear(255, 255, 255,0);
                    photoBombFbo.end();

                    state = STATE_PHOTO_BOMB_CLOSING;

                }

                photoBombFbo.begin();
                photoBombImg.draw(ofRandom(-200,200),ofRandom(-200,200), photoBombFbo.getWidth(),photoBombFbo.getHeight());
                photoBombFbo.end();

            }


        break;
        case STATE_PHOTO_BOMB_CLOSING:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            if(photoBombClosingWaitCount<mapDelay){
                photoBombClosingWaitCount++;
            }else{
                photoBombClosingWaitCount = 0;
                state = STATE_MAP;
            }
            
        break;
        case STATE_MAP:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            
        break;
        case STATE_ENDING:
            //show ending;

            playAndPreload();
            state = STATE_START;

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

            ofEnableAlphaBlending();
            ofEnableSmoothing();

            colorWheel.draw(0,-kPhotoFbo.getWidth()*0.045);
            drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::black);
            ofSetColor(ofColor::black);
            searchingFont.drawString("searching...", -searchingFont.stringWidth("searching...")/2, kPhotoFbo.getWidth()*0.04);

            ofDisableAlphaBlending();
            ofDisableSmoothing();

            ofPopStyle();
            ofPopMatrix();



        break;
        case STATE_KPHOTO_IN:
            //show kyoto photo, zoom, show text;

            ofEnableAlphaBlending();
            ofEnableSmoothing();

            kPhotoFbo.draw((screenWidth-screenHeight)/2,0);

            ofDisableAlphaBlending();
            ofDisableSmoothing();


        break;
        case STATE_KCOLOR_IN:
            //show kyoto color;
            kPhotoFbo.draw((screenWidth-screenHeight)/2-animatedPhotoPos.val(),0);
            kColor.draw((screenWidth+screenHeight)/2-animatedPhotoPos.val(),0,screenHeight,screenHeight);


        break;
        case STATE_TCOLOR_IN:
            //show taiwan color, pan in;
            kColor.draw((screenWidth-screenHeight)/2-animatedPhotoPos.val(),0,screenHeight,screenHeight);
            tColor.draw((screenWidth+screenHeight)/2-animatedPhotoPos.val(),0,screenHeight,screenHeight);


        break;
        case STATE_TPHOTO_IN:
            //show taiwan photo, mono tone, show text;
            tColor.draw((screenWidth-screenHeight)/2,0,screenHeight,screenHeight);
            tPhotoFbo.draw((screenWidth-screenHeight)/2,0);

        break;
        case STATE_TCOLOR_FILL:
            //show taiwan color;
            tPhotoFbo.draw((screenWidth-screenHeight)/2,0);


        break;
        case STATE_TNAME_IN:
            //show color name
            ofPushStyle();
            ofSetColor(ofColor::fromHex(0xB87167));
            // todo:: need to have an array hold the color circle's pos and color on all 80 photos
            ofDrawRectangle((screenWidth-screenHeight)/2,0,screenHeight,screenHeight);
            ofPopStyle();
            tName.draw((screenWidth+screenHeight)/2-animatedPhotoPos.val(),0,screenHeight,screenHeight);


        break;
        case STATE_PHOTO_BOMB:
            //show PHOTO BOMB;
            photoBombFbo.draw((screenWidth-screenHeight)/2,0);


        break;
        case STATE_PHOTO_BOMB_CLOSING:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            photoBombClosing.draw((screenWidth-screenHeight)/2,0,screenHeight,screenHeight);
            
            
        break;
        case STATE_MAP:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            
        break;
        case STATE_ENDING:
            //show ending;

        break;

        default:
        break;
    }

    ofPushStyle();
    ofSetColor(0);
    ofDrawRectangle(0, 0, (screenWidth-screenHeight)/2, screenHeight);
    ofDrawRectangle((screenWidth+screenHeight)/2, 0, (screenWidth-screenHeight)/2, screenHeight);

    ofPopStyle();

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
            tColor.load("tColors/1.jpg");
            tPhoto.load("tPhotos/1-1.jpg");
            tPhotoMono.load("tPhotos/1-1-mono.jpg");
            tPhotoMonoInfo.load("tPhotos/1-1-mono-info.jpg");
            tName.load("tNames/1.jpg");

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
    ofPushStyle();
        ofSetColor(255, 255, 255, _alpha);
        kPhotoInfo.draw(_x,_y,_size,_size);
    ofPopStyle();
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
