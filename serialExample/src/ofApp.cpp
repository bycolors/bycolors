#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
	
	ofBackground(255);
	ofSetLogLevel(OF_LOG_NOTICE);
	
	font.load("DIN.otf", 64);
	
	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	
	// this should be set to whatever com port your serial device is connected to.
	// (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
	// arduino users check in arduino app....
	int baud = 9600;
	serial.setup(8, baud); //open the first device
//	serial.setup("/dev/cu.wch ch341 USB=>RS232 1440", baud); // mac osx example
	
	nTimesRead = 0;
	nBytesRead = 0;
	readTime = 0;
   ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	
		// (2) read
		// now we try to read 3 bytes
		// since we might not get them all the time 3 - but sometimes 0, 6, or something else,
		// we will try to read three bytes, as much as we can
		// otherwise, we may have a "lag" if we don't read fast enough
		// or just read three every time. now, we will be sure to 
		// read as much as we can in groups of three...
		
		nTimesRead = 0;
		nBytesRead = 0;
		int nRead  = 0;  // a temp variable to keep count per read
		
		
		memset(bytesReturned, 0, 21);
		
		while( (nRead = serial.readBytes( bytesReturned, 21)) > 0){
			nTimesRead++;	
			nBytesRead = nRead;
		};
		
    if (bytesReturned[0] == 'T') {
        
        temp = (bytesReturned[1]<<8) + bytesReturned[2];
    }
    
    if(bytesReturned[3] == 'V') {
        
            vValue = (bytesReturned[4]<<8) + bytesReturned[5];
    }
    if(bytesReturned[6] == 'B') {
    
        bValue = (bytesReturned[7]<<8) + bytesReturned[8];
    
    }
    if(bytesReturned[9] == 'G') {
    
        gValue = (bytesReturned[10]<<8) + bytesReturned[11];
    
    }
    if(bytesReturned[12] == 'Y') {
    
        yValue = (bytesReturned[13]<<8) + bytesReturned[14];
    
    }
    if(bytesReturned[15] == 'O') {
    
        oValue = (bytesReturned[16]<<8) + bytesReturned[17];
    
    }
    if(bytesReturned[18] == 'R') {
    
        rValue = (bytesReturned[19]<<8) + bytesReturned[20];
    
    }

//		memcpy(bytesReadString, bytesReturned, 3);
		
		readTime = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofApp::draw(){
//	if (nBytesRead > 0 && ((ofGetElapsedTimef() - readTime) < 0.5f)){
//		ofSetColor(0);
//	} else {
//		ofSetColor(220);
//	}
//	string msg;
//	msg += "nBytes read " + ofToString(nBytesRead) + "\n";
//	msg += "nTimes read " + ofToString(nTimesRead) + "\n";
//	msg += "read temp: " + ofToString(temp) + "\n";
//	msg += "read vValue: " + ofToString(vValue) + "\n";
//	msg += "read bValue: " + ofToString(bValue) + "\n";
//	msg += "read gValue: " + ofToString(gValue) + "\n";
//	msg += "read yValue: " + ofToString(yValue) + "\n";
//	msg += "read oValue: " + ofToString(oValue) + "\n";
//	msg += "read rValue: " + ofToString(rValue) + "\n";
//
//	msg += "(at time " + ofToString(readTime, 3) + ")";
//	font.drawString(msg, 50, 100);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){ 
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
	
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
void ofApp::mouseEnterValue(int x, int y){

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

