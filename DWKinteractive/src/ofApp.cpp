#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    #ifdef TARGET_OPENGLES
        shader.load("shadersES2/shader");
    #else
        if(ofIsGLProgrammableRenderer()){
            shader.load("shadersGL3/shader");
        }else{
            shader.load("shadersGL2/shader");
        }
    #endif

    ofHideCursor();


    ofLog()<<"screen resolution: "<<ofGetWidth()<<" x "<<ofGetHeight();
    screenWidth = ofGetWidth();
    screenHeight = ofGetHeight();

    state = STATE_START;

    //for testing
//    loadAssets(6);
//    state = STATE_TPHOTO_MONO_IN_2;

    serial.listDevices();
  	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 9600;
  	serial.setup(0, baud); //open the first device

    bSendSerialMessage = false;

    detectedColor = 100;

    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);

    //square canvas
    movieFbo.allocate(screenHeight,screenHeight,GL_RGBA);
    colorFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample
    fadingFbo.allocate(1080, 1080,GL_RGBA, 8);
    fadeMaskFbo.allocate(1080, 1080,GL_RGBA, 8);
    mapSelectedFbo.allocate(1080, 1080,GL_RGBA, 8);
    photoInfoFbo.allocate(1080, 1080,GL_RGBA, 8);
    endingFbo.allocate(1080, 1080,GL_RGBA, 8);

    
    photoInfoFbo.begin();
    ofClear(255, 255, 255,0);
    photoInfoFbo.end();
    endingFbo.begin();
    ofClear(255, 255, 255,0);
    endingFbo.end();

    
    mapSelectedFbo.begin();
    ofClear(255, 255, 255,0);
    mapSelectedFbo.end();
    
    movieFbo.begin();
    ofClear(255, 255, 255,0);
    movieFbo.end();

    colorFbo.begin();
    ofClear(255, 255, 255,0);
    colorFbo.end();

    fadingFbo.begin();
    ofClear(0, 0, 0,0);
    fadingFbo.end();

    fadeMaskFbo.begin();
    ofClear(0,0,0,255);
    fadeMaskFbo.end();

    fadeMaskAlpha = 0;
    loadFadeImages(&colorCmykImg, &photoNameImg);


    searchingFont.load("Mrs Eaves OT Roman.otf", 17);
    searchingFont.setLetterSpacing(0.9);
    frameFont.load("Mrs Eaves OT Roman.otf", 24);

    crossFadeSpeed = 4;
    colorFillAlpha = 255;

    
    // timing variable
    frameCounter = 0;
    colorSmallHoldTime = 50;
    colorBigHoldTime = 50;
    photoHoldTime = 100;
    photoNameHoldTime = 250;
    photoInfoHoldTime = 550;
    mapSelectedHoldTime = 250;
    wrongColorHoldTime = 250;
    endingHoldTime = 250;
    endingAlpha = 0;



    endingImg.load("ending.jpg");
    wrongColorImg.load("wrong.jpg");

    animatedPosHorizontal.animateFromTo(0,1080);
    animatedPosHorizontal.setDuration(2);
    AnimCurve curve = (AnimCurve) (EASE_OUT);
    animatedPosHorizontal.setCurve( curve );

    animatedPosVertical.animateFromTo(0,1080);
    animatedPosVertical.setDuration(2);
    animatedPosVertical.setCurve( curve );

    
    animatedColorSize.animateFromTo(screenHeight*0.22, screenHeight);
    animatedColorSize.setDuration(1);
    animatedColorSize.setCurve( curve );

    colorPosYoffset.animateFromTo(-screenHeight*0.03, 0);
    colorPosYoffset.setDuration(1);
    colorPosYoffset.setCurve( curve );


    animatedCircleSize.animateFromTo(tPhotoGridFbo.getWidth()*0.01, tPhotoGridFbo.getWidth()*2);
    animatedCircleSize.setDuration(2);
    AnimCurve curve2 = (AnimCurve) (TANH);
//    AnimCurve curve2 = (AnimCurve) (CUBIC_EASE_IN);
    animatedCircleSize.setCurve( curve2 );

    resetGridAnimation();

    tPhotoScanBarPos = 0;

    colorWheel.setup(screenHeight*0.051,screenHeight*0.037,2.625); //outter radius, inner radius, rotation speed

    totalMovies = 3;
    nowPlayer = 0;
    loadingMovieId = 0;
    playAndPreload();

    showInfo = false;

    if (XML.load("mySettings.xml")) {
        ofLog()<<"mySettings.xml loaded!";
        canvasOffsetX=XML.getValue("position:canvasOffsetX", 0);
        canvasOffsetY=XML.getValue("position:canvasOffsetY", 0);

    }else{
        XML.setValue("position:canvasOffsetX", 0);
        XML.setValue("position:canvasOffsetY", 0);
        XML.saveFile("mySettings.xml"); //puts settings.xml file in the bin/data folder
    }

}

//--------------------------------------------------------------
void ofApp::update(){
    switch (state) {
        case STATE_START:
            //play movie loop;
            checkSerial();
            updateMovie();
        break;

        //--------------------------------------------------------------

        case STATE_DETECTED:
            //object detected! searching;
            if(movies[nowPlayer].isPlaying()){
                switchMovie();
                movies[nowPlayer].stop();
                movieFbo.begin();
                ofClear(255, 255, 255,0);
                movieFbo.end();
            }
            if(colorWheel.times<70){
                colorWheel.update();
            }else{
                colorWheel.resetTimes();
                state = STATE_COLOR_ZOOM;
                //prepare for next scene
                colorFbo.begin();
                ofClear(255, 255, 255,0);
                colorFbo.end();


            }
        break;

        //--------------------------------------------------------------

        case STATE_COLOR_ZOOM:
            //show selected color, zoom, show text;
            if(frameCounter<colorSmallHoldTime){
                frameCounter++;
            }else{
                if(!animatedColorSize.hasFinishedAnimating()){
                    float dt = 1.0f / 60.0f;
                    animatedColorSize.update( dt );
                    colorPosYoffset.update( dt );
                }else{
                    frameCounter = 0;
                    state = STATE_COLOR_CMYK_IN;
                    //prepare for next scene
                    fadingFbo.begin();
                    colorCmykImg.draw(0, 0, fadingFbo.getWidth(),fadingFbo.getHeight());
                    fadingFbo.end();
                }
            }

            colorFbo.begin();
                ofPushMatrix();
                ofPushStyle();

                //clean the fbo
                ofSetColor(0);
                ofDrawRectangle(0, 0, colorFbo.getWidth(), colorFbo.getHeight());

                ofTranslate(colorFbo.getWidth()/2, colorFbo.getHeight()/2);
                ofSetRectMode(OF_RECTMODE_CENTER);

                ofEnableAlphaBlending();
                ofEnableSmoothing();

                ofSetColor(allColorPickers[detectedColor-1]);
                ofDrawRectangle(0,colorPosYoffset.val(),animatedColorSize.val(),animatedColorSize.val());


                ofSetRectMode(OF_RECTMODE_CORNER);
                drawFrame(-colorFbo.getWidth()*0.11,-colorFbo.getWidth()*0.15, colorFbo.getWidth()*0.22, colorFbo.getWidth()*0.305,ofColor::white);

                ofDisableAlphaBlending();
                ofDisableSmoothing();


                ofPopStyle();
                ofPopMatrix();

            colorFbo.end();
        break;

        //--------------------------------------------------------------

        case STATE_COLOR_CMYK_IN:
            if(frameCounter<colorBigHoldTime){
                //HOLD full big color
                frameCounter++;
            }else{

                if(colorFillAlpha>0){
                    //FADE out color fill to show colorCmykImg
                    colorFillAlpha-=3;
                    colorFbo.begin();
                    ofColor cc = allColorPickers[detectedColor-1];
                    cc.a = colorFillAlpha;
                    ofSetColor(cc);
                    ofDrawRectangle(0, 0, colorFbo.getWidth(), colorFbo.getHeight());
                    colorFbo.end();

                }else{
                    if(frameCounter<colorBigHoldTime+50){
                        frameCounter++;
                    }else{
                        colorFillAlpha=255;
                        frameCounter = 0;

                        animatedColorSize.animateFromTo(screenHeight*0.22, screenHeight);
                        colorPosYoffset.animateFromTo(-screenHeight*0.03, 0);
                        state = STATE_PHOTO_IN;
                        loadFadeImages(&colorCmykImg, &photoImg);

                    }

                }
            }

        break;

        //--------------------------------------------------------------
            
        case STATE_PHOTO_IN:
            PhotoCrossFade();
            
            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<photoHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    frameCounter = 0;
                    state = STATE_PHOTO_NAME_IN;
                    loadFadeImages(&photoImg, &photoNameImg);
                }
                
            }
            break;
            

        //--------------------------------------------------------------

        case STATE_PHOTO_NAME_IN:
            PhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<photoNameHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    loadFadeImages(&colorCmykImg, &photoNameImg);

                    state = STATE_PHOTO_INFO_IN;
                    frameCounter = 0;

                    photoInfoFbo.begin();
                    photoNameImg.draw(0, 0, photoInfoFbo.getWidth(),photoInfoFbo.getHeight());
                    photoInfoFbo.end();

                }

            }
        break;

        //--------------------------------------------------------------

        case STATE_PHOTO_INFO_IN:

            if(!animatedPosHorizontal.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedPosHorizontal.update( dt );
                animatedPosHorizontal.update( dt );
                photoInfoFbo.begin();
                photoNameImg.draw(animatedPosHorizontal, 0, photoInfoFbo.getWidth(),photoInfoFbo.getHeight());
                photoInfoImg.draw(float(animatedPosHorizontal)-photoInfoFbo.getWidth(), 0, photoInfoFbo.getWidth(),photoInfoFbo.getHeight());
                photoInfoFbo.end();
            }else{
                if(frameCounter<photoInfoHoldTime) {
                    frameCounter++;
                }
                else{
                    state = STATE_MAP_SELECTED_IN;
                    frameCounter = 0;
                    animatedPosHorizontal.animateFromTo(0,1080);

                    photoInfoFbo.begin();
                    ofClear(255, 255, 255,0);
                    photoInfoFbo.end();

                    
                    mapSelectedFbo.begin();
                    photoInfoImg.draw(0, 0, mapSelectedFbo.getWidth(),mapSelectedFbo.getHeight());
                    mapSelectedFbo.end();

                }

            }
        break;

        //--------------------------------------------------------------

        case STATE_MAP_SELECTED_IN:
            //show SELECTED MAP, fade in;

            if(!animatedPosHorizontal.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedPosHorizontal.update( dt );
                animatedPosHorizontal.update( dt );
                mapSelectedFbo.begin();
                photoInfoImg.draw(animatedPosHorizontal, 0, mapSelectedFbo.getWidth(),mapSelectedFbo.getHeight());
                mapSelectedImg.draw(float(animatedPosHorizontal)-mapSelectedFbo.getWidth(), 0, mapSelectedFbo.getWidth(),mapSelectedFbo.getHeight());
                mapSelectedFbo.end();

            }else{
                if(frameCounter<mapSelectedHoldTime) {
                    frameCounter++;
                }
                else{
                    animatedPosHorizontal.animateFromTo(0,1080);
                    state = STATE_ENDING;
                    frameCounter = 0;
                    loadFadeImages(&mapSelectedImg, &endingImg);
                    
                    mapSelectedFbo.begin();
                    ofClear(255, 255, 255,0);
                    mapSelectedFbo.end();

                    
                    endingFbo.begin();
                    mapSelectedImg.draw(0, 0, endingFbo.getWidth(),endingFbo.getHeight());
                    endingFbo.end();

                }

            }

        break;
        
        //--------------------------------------------------------------

        case STATE_ENDING:
            //show ending;
            PhotoCrossFade();
            
            if(!animatedPosVertical.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedPosVertical.update( dt );
                animatedPosVertical.update( dt );
                endingFbo.begin();
                mapSelectedImg.draw(0, animatedPosVertical, endingFbo.getWidth(),endingFbo.getHeight());
                endingImg.draw(0, float(animatedPosVertical)-endingFbo.getHeight() ,endingFbo.getWidth(),endingFbo.getHeight());
                endingFbo.end();
            }else{

                if(frameCounter<endingHoldTime){
                    frameCounter++;
                }else{
                    if(endingAlpha<255){
                        endingAlpha+=15;
                        if(endingAlpha>255)endingAlpha=255;
                    }else{
                        endingAlpha = 0;
                        frameCounter = 0;
                        serial.flush();

                        state = STATE_START;
                        frameCounter = 0;

                        animatedPosVertical.animateFromTo(0,1080);

                        endingFbo.begin();
                        ofClear(255,255,255,0);
                        endingFbo.end();
                        

                        ofLog()<<"starting over";
                    }
                }
            }
        break;
        //--------------------------------------------------------------

        case STATE_WRONG_COLOR:
            if(frameCounter<wrongColorHoldTime){
                frameCounter++;
                checkSerial();

            }else{
                serial.flush();
                frameCounter = 0;
                state = STATE_START;
                ofLog()<<"starting over";
            }

        break;

        default:
          break;
    }

    //show fps on title bar
    std::stringstream strm;
	strm << "fps: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());


}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0, 0, 0);

    switch (state) {
        case STATE_START:
            //play movie loop;

            movieFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY);
        break;
        //--------------------------------------------------------------
        case STATE_DETECTED:
            //object detected! searching;

            ofPushMatrix();
            ofPushStyle();
            ofTranslate(screenWidth/2+canvasOffsetX, screenHeight/2+canvasOffsetY);

            ofEnableAlphaBlending();
            ofEnableSmoothing();

            ofSetColor(0);
            ofDrawRectangle(-screenHeight/2,-screenHeight/2,screenHeight,screenHeight);

            colorWheel.draw(0,-colorFbo.getWidth()*0.045);
            drawFrame(-colorFbo.getWidth()*0.11,-colorFbo.getWidth()*0.15, colorFbo.getWidth()*0.22, colorFbo.getWidth()*0.305,ofColor::white);
            ofSetColor(ofColor::white);
            searchingFont.drawString("searching...", -searchingFont.stringWidth("searching...")/2, colorFbo.getWidth()*0.04);
            ofSetRectMode(OF_RECTMODE_CENTER);
           // searchingImg.draw(0,-kPhotoFbo.getHeight()*0.2,kPhotoFbo.getWidth()*0.15,kPhotoFbo.getWidth()*0.15*32/146);

            ofDisableAlphaBlending();
            ofDisableSmoothing();

            ofPopStyle();
            ofPopMatrix();

        break;
        //--------------------------------------------------------------

        case STATE_COLOR_ZOOM:
            //show kyoto photo, zoom, show text;
            colorFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY);
        break;

        //--------------------------------------------------------------

        case STATE_COLOR_CMYK_IN:
            //fade from color fill to color cmyk
            colorCmykImg.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            colorFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY);
        break;
        
        //--------------------------------------------------------------
            
        case STATE_PHOTO_IN:
            //fade from color cmyk to photo name
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            break;
            
        //--------------------------------------------------------------

        case STATE_PHOTO_NAME_IN:
            //fade from color cmyk to photo name
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            break;

        //--------------------------------------------------------------

        case STATE_PHOTO_INFO_IN:
            photoInfoFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            break;

        //--------------------------------------------------------------

        case STATE_MAP_SELECTED_IN:
            mapSelectedFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

        break;

        //--------------------------------------------------------------
        case STATE_ENDING:
            //show ending;
            endingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            ofPushStyle();
            ofEnableAlphaBlending();
            ofSetColor(0, 0, 0, endingAlpha);
            ofDrawRectangle((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            ofDisableAlphaBlending();
            ofPopStyle();
        break;
        //--------------------------------------------------------------
        case STATE_WRONG_COLOR:
            wrongColorImg.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

        break;

        default:
        break;
    }

    //draw mask on left and right
    ofPushStyle();
    ofSetColor(0);
    ofDrawRectangle(canvasOffsetX,canvasOffsetY, (screenWidth-screenHeight)/2, screenHeight);
    ofDrawRectangle((screenWidth+screenHeight)/2+canvasOffsetX,canvasOffsetY,(screenWidth-screenHeight)/2, screenHeight);

    ofPopStyle();

    if (showInfo) {
        ofDrawBitmapString(ofGetTimestampString("%c"),50,50);
        ofDrawBitmapString("Next Movie ID: "+ofToString(loadingMovieId), 50, 90);
        ofDrawBitmapString("Total Frames: "+ofToString(movies[nowPlayer].getTotalNumFrames()), 50, 120);
        ofDrawBitmapString("Current Frame: "+ofToString(movies[nowPlayer].getCurrentFrame()), 50, 150);
        ofDrawBitmapString("Detected color: "+ofToString(detectedColor), 50, 180);
        ofDrawBitmapString("selected TPhoto: "+ofToString(selectedTPhoto), 50, 210);
        ofDrawBitmapString("selected grid index: "+ofToString(selectedGridIndex), 50, 240);

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
            canvasOffsetY--;
        break;
        case OF_KEY_DOWN:
            canvasOffsetY++;
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
            detectedColor = 1;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '2':
            detectedColor = 2;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '3':
            detectedColor = 3;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '4':
            detectedColor = 4;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '5':
            detectedColor = 5;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '6':
            detectedColor = 6;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '7':
            detectedColor = 7;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '8':
            detectedColor = 8;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;

        case '9':
            detectedColor = 9;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case 'q':
            detectedColor = 10;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 'w':
            detectedColor = 11;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 'e':
            detectedColor = 12;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 'a':
            detectedColor = 13;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 's':
            detectedColor = 14;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 'x':
            state = STATE_WRONG_COLOR;
            break;


        default:
            break;
    }

    XML.setValue("position:canvasOffsetX", canvasOffsetX);
    XML.setValue("position:canvasOffsetY", canvasOffsetY);

    XML.saveFile("mySettings.xml"); //puts settings.xml file in the bin/data folder
}


//--------------------------------------------------------------
void ofApp::playAndPreload(){
    // PLAY the current movie

    string timeOfDay;

    if (ofGetHours()<11) {
        timeOfDay = "morning";
    }else if(ofGetHours()<17){
        timeOfDay = "afternoon";
    }else{
        timeOfDay = "night";
    }

    // need to load the first movie : should only happen once
    if( !movies[nowPlayer].isLoaded() ){

        movies[nowPlayer].load( "movies/"+timeOfDay+"-"+ofToString(loadingMovieId) + ".mp4" );
        //todo:: check time and load morning, afternoon, night movies
    }

    movies[nowPlayer].setLoopState(OF_LOOP_NORMAL);
    movies[nowPlayer].play();

    ofLog()<<"playing: "<<timeOfDay<<"-"<<loadingMovieId<<".mp4";

    // PRELOAD the next movie

    // get a unique random ID
    int preloadId = rand()%totalMovies; //0~totalmovies-1(=2)

    while (preloadId == loadingMovieId ) {
        preloadId = rand()%totalMovies;
    }

    // set the loading movie to the new ID
    loadingMovieId = preloadId;

    // load movie in the player that isn't 'nowPlayer'
    int nextPlayer = nowPlayer == 0 ? 1 : 0;
    movies[nextPlayer].load( "movies/"+timeOfDay+"-"+ofToString(loadingMovieId) + ".mp4" );
    ofLog()<<"playing: "<<timeOfDay<<"-"<<loadingMovieId<<".mp4";

    movies[nowPlayer].setVolume(0);
    movies[nextPlayer].setVolume(0);


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
void ofApp::updateMovie(){
    if(!movies[nowPlayer].isPlaying()){
        movies[nowPlayer].play();
    }
  movies[nowPlayer].update();

  int totalFrames = movies[nowPlayer].getTotalNumFrames();
  int currentFrame = movies[nowPlayer].getCurrentFrame();
  if((currentFrame>0) && (totalFrames-currentFrame <20)){
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

        serial.writeByte('a');

        if(serial.available()){
            int myByte = 0;
            myByte=serial.readByte();

            if ( myByte == OF_SERIAL_NO_DATA ){
                //ofLog()<<"no data";
            }else if ( myByte == OF_SERIAL_ERROR ){
                ofLog()<<"error";
            }else if ( myByte > 14){
//                state = STATE_WRONG_COLOR;
            }else{
                ofLog()<<"detected color: "<< myByte;
                detectedColor = myByte;

                //      kPhoto.load("kPhotos/1.jpg");
                //      kPhotoMonoInfo.load("kPhotos/1-info.jpg");
                //      kColor.load("kColors/1.jpg");

                loadAssets(detectedColor);
                state = STATE_DETECTED;

            }
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

void ofApp::PhotoCrossFade(){

    fadingFbo.begin();
    shader.begin();
    // here is where the fbo is passed to the shader
    shader.setUniformTexture("maskTex", fadeMaskFbo.getTexture(), 1 );
    shader.setUniformTexture("foregroundTex", foregroundImage->getTexture(), 2 );

    backgroundImage->draw(0, 0);

    shader.end();
    fadingFbo.end();
}

//--------------------------------------------------------------

void ofApp::resetkPhotoMonoInfo(){
    frameCounter = 0;
}

//--------------------------------------------------------------


void ofApp::loadAssets(float _numOfColor){

    string colorNumber = ofToString(_numOfColor);

    colorCmykImg.load("colorCMYK/colorCMYK-c"+colorNumber+".jpg");
    photoImg.load("photo/photo-c"+colorNumber+".jpg");
    photoNameImg.load("photoName/photoName-c"+colorNumber+".jpg");
    photoInfoImg.load("photoInfo/photoInfo-c"+colorNumber+".jpg");
    mapSelectedImg.load("mapSelected/mapSelected-c"+colorNumber+".jpg");

}

//--------------------------------------------------------------
void ofApp::updateFadeMask(int _fadeSpeed){
    fadeMaskAlpha+=_fadeSpeed;
    if(fadeMaskAlpha>255) fadeMaskAlpha =255;
    if(fadeMaskAlpha<0)fadeMaskAlpha =0;
    fadeMaskFbo.begin();
    ofPushStyle();
    ofSetColor(fadeMaskAlpha);
    ofDrawRectangle(0, 0, fadeMaskFbo.getWidth(), fadeMaskFbo.getHeight());
    ofPopStyle();
    fadeMaskFbo.end();
}
//--------------------------------------------------------------
void ofApp::resetFadeMask(){
    fadeMaskAlpha = 0;
    fadeMaskFbo.begin();
    ofClear(0,0,0,255);
    fadeMaskFbo.end();
}


//--------------------------------------------------------------

void ofApp::resetGridAnimation(){
    gridSize = 3892;
    photoSize = 1080;

    tGridPos.setPosition(ofPoint((-allGridLocators[selectedGridIndex].x),(-allGridLocators[selectedGridIndex].y)));
    tGridPos.animateTo(ofPoint(0,0));

    tPhotoGridSizeScale.animateFromTo(1, 1080/gridSize);

    tGridPos.setDuration(2);
    tPhotoGridSizeScale.setDuration(2);

    AnimCurve curve3 = (AnimCurve) (EASE_IN_EASE_OUT);

    tGridPos.setCurve( curve3 );
    tPhotoGridSizeScale.setCurve( curve3 );


    for(int i =0; i<8; i++){
        ofPoint loc = ofPoint(gridSize*cos(i*45),gridSize*sin(i*45));
        tGridPhotoPos[i].setPosition(loc);
        tGridPhotoPos[i].setDuration(2);
        tGridPhotoPos[i].setCurve(curve3);
        tGridPhotoPos[i].animateTo(ofPoint(allGridLocators[i].x,allGridLocators[i].y));
    }
}

void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::loadFadeImages(ofImage* _foreground, ofImage* _background){

    foregroundImage=_foreground;
    backgroundImage=_background;


    if(foregroundImage->getWidth() != fadeMaskFbo.getWidth()){
        foregroundImage->resize(fadeMaskFbo.getWidth(), fadeMaskFbo.getHeight());
    }

    if(backgroundImage->getWidth() != fadeMaskFbo.getWidth()){
        backgroundImage->resize(fadeMaskFbo.getWidth(), fadeMaskFbo.getHeight());
    }


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
