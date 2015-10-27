#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    camWidth = 1024;
    camHeight = 768;
    projWidth = 1024;
    projHeight = 768;
    canvasHeight = ofGetHeight() / 3 - 10;
    canvasWidth = canvasHeight * 4 / 3;
    outputHeight = ofGetHeight() * 2 / 3 - 10;
    outputWidth = outputHeight * 4 / 3;
    guiPos = ofPoint(10,10);
    canvas1Pos = ofPoint(ofGetWidth()/3 + 5, 5);
    canvas2Pos = ofPoint(ofGetWidth()*2/3 + 5, 5);
    outputPos = ofPoint(ofGetWidth()/3 + 5, ofGetHeight()/3+5);
    
    ofSetWindowTitle("FaceMapper - OTU CINEMA SL");
    
    CGDisplayCount displayCount;
    CGDirectDisplayID displays[32];
    
    // grab the active displays
    CGGetActiveDisplayList(32, displays, &displayCount);
    
    cout<<"displayCount "<<displayCount<<endl;
    
	vidGrabber.setVerbose(true);
	vidGrabber.initGrabber(camWidth, camHeight);
	colorImg.allocate(camWidth, camHeight);
	grayImage.allocate(canvasWidth, canvasHeight);
    warpedImg.allocate(canvasWidth, canvasHeight);
    
    if (displayCount == 1) {
        
        whichDisplay = 0;
        mainDisplayWidth = CGDisplayPixelsWide ( displays[whichDisplay] );
        mainDisplayWidth = CGDisplayPixelsHigh ( displays[whichDisplay] );
        
    }
    
    else {
        
        whichDisplay = 1;
        mainDisplayWidth = CGDisplayPixelsWide ( displays[0] );
        mainDisplayHeight = CGDisplayPixelsHigh ( displays[0] );
        secondDisplayWidth = CGDisplayPixelsWide ( displays[whichDisplay] );
        secondDisplayHeight = CGDisplayPixelsHigh ( displays[whichDisplay] );
        
    }
    
    
    // the arguments for the second window are its initial x and y position,
    // and its width and height. the last argument is whether the window
    // should be undecorated, i.e. title bar is visible. setting it to
    // true removes the title bar.
    
    
    if (displayCount == 1) secondWindow.setup("Output", 0, 0, 640, 480, false);
    else secondWindow.setup("Output", mainDisplayWidth, 0, secondDisplayWidth, secondDisplayHeight, true);
    
    
	threshold = 80;
	bLearnBakground = true;
	backgroundSubOn = false;
	
	//detect finger is off by default
	fidfinder.detectFinger		= false;
	//fidfinder.maxFingerSize		= 25;
	//fidfinder.minFingerSize		= 5;
	//fidfinder.fingerSensitivity	= 0.05f; //from 0 to 2.0f
        
    gui.setup("Control Panel"); // most of the time you don't need a name
    gui.add(selection.setup("Image Selection", false));
    gui.add(warping.setup("Warping Mode", false));
    gui.add(threshold.setup("Threshold", 160, 0, 255));
    gui.add(tableWidth.setup("Largo Mesa (cm)", 200, 10, 500));
    gui.add(tableHeight.setup("Ancho Mesa (cm)", 100, 10, 500));
    
    // Definimos los puntos de recorte de ROI (warping inverso de imagen de entrada)
    roiCorners[0] = ofPoint(10,10);
    roiCorners[1] = ofPoint(canvasWidth-10, 10);
    roiCorners[2] = ofPoint(canvasWidth - 10, canvasHeight - 10);
    roiCorners[3] = ofPoint(10,canvasHeight - 10);
    destCorners[0] = ofPoint(0,0);
    destCorners[1] = ofPoint(canvasWidth, 0);
    destCorners[2] = ofPoint(canvasWidth, canvasHeight);
    destCorners[3] = ofPoint(0,canvasHeight);
    
    for (int i = 0; i < 4; i++) {
        
        activeCorners[i] = false;
        
    }
    
    // Inicializamos los canvas
    
    canvas1.allocate(canvasWidth, canvasHeight, GL_RGB);
    canvas2.allocate(canvasWidth, canvasHeight, GL_RGB);
    canvasWarp.allocate(canvasWidth, canvasHeight, GL_RGB);
    output.allocate(outputWidth, outputHeight, GL_RGB);
    warpedOutput.allocate(outputWidth, outputHeight, GL_RGB);
    
    
    pixels.allocate(canvasWidth, canvasHeight, GL_RGB);

    // Inicializamos el quad
    quadRoi.setMode(OF_PRIMITIVE_LINE_LOOP);
    
    for(int i = 0; i < 4; i++) {
        
        quadRoi.addVertex(roiCorners[i]);

    }
    
    // Inicializamos el warper
    
    
    warperGui.setup(outputPos.x, outputPos.y, outputWidth, outputHeight);
    warperGui.activate();
    
    warperOutput.setup(mainDisplayWidth, 0, secondDisplayWidth, secondDisplayHeight);; //initializates ofxGLWarper
    
    // Inicializamos el objeto "Bola"
    //lets setup some stupid particles
    for(int i = 0; i < 10; i++){
        
        cards[i].setup(i);

    }
}

//--------------------------------------------------------------
void ofApp::update(){
	
	ofBackground(100,100,100);
	vidGrabber.update();
    
    if ((warping) && (!warperGui.isActive())) {
        
        warperGui.activate();
        warperOutput.activate();
    }
    
    else if (warping) {

        // Actualizamos posiciones
        
        ofPoint dif = ofPoint(outputPos.x, outputPos.y);
        
        float multWidth = (float) secondDisplayWidth / (float) outputWidth;
        float multHeight = (float) secondDisplayHeight / (float) outputHeight;
        ofPoint mult = ofPoint(multWidth, multHeight);
        
        
        
        
        ofPoint tl = warperGui.getCorner(ofxGLWarper::TOP_LEFT);
        ofPoint tr = warperGui.getCorner(ofxGLWarper::TOP_RIGHT);
        ofPoint br = warperGui.getCorner(ofxGLWarper::BOTTOM_RIGHT);
        ofPoint bl = warperGui.getCorner(ofxGLWarper::BOTTOM_LEFT);
        
        warperOutput.setCorner(ofxGLWarper::TOP_LEFT, (tl - dif) * mult);
        warperOutput.setCorner(ofxGLWarper::TOP_RIGHT, (tr - dif) * mult);
        warperOutput.setCorner(ofxGLWarper::BOTTOM_RIGHT, (br - dif) * mult);
        warperOutput.setCorner(ofxGLWarper::BOTTOM_LEFT, (bl - dif) * mult);
        
    }
    
    else if ((!warping) && (warperGui.isActive())) {
        
        warperGui.deactivate();
        warperOutput.deactivate();
        
    }
	
	if (vidGrabber.isFrameNew()){
		
		colorImg.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);
		
        // ACTUALIZAMOS CANVAS 1
        canvas1.begin();
        ofNoFill();
        ofSetColor(255);
        colorImg.draw(0, 0, canvasWidth, canvasHeight);
        
        if (selection) {

        ofSetColor(255,255,0);
        quadRoi.draw();

            ofSetColor(255,255,0);
        
            for (int i = 0; i < 4; i++) {
                
                if (activeCorners[i]) ofSetColor(255,0,0);
                else ofSetColor(255,255,0);
            
                ofRect(roiCorners[i].x - 5, roiCorners[i].y - 5, 10, 10);

            }
            
        }
        
        ofSetColor(255);
        
        canvas1.end();
        // ------------------------

        
        // ACTUALIZAMOS CANVASWARP
        canvasWarp.begin();
        colorImg.getTextureReference().bind();
        
        glBegin(GL_QUADS);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        
        for (int i = 0; i < 4; i++) {
            glTexCoord2f(roiCorners[i].x *(float)camWidth/(float)canvasWidth,
                         roiCorners[i].y *(float)camWidth/(float)canvasWidth);
            glVertex2f(destCorners[i].x, destCorners[i].y);
            
        }
        
        glEnd();
        colorImg.getTextureReference().unbind();
        
        canvasWarp.readToPixels(pixels);
        canvasWarp.end();
        // ------------------------
        
        
        // GUARDAMOS WARPEDIMAGE Y PASAMOS A BN Y FILTRAMOS
        warpedImg.setFromPixels(pixels);
        grayImage.setFromColorImage(warpedImg);
        grayImage.threshold(threshold);
        // ------------------------

        
        // ACTUALIZAMOS CANVAS2
        canvas2.begin();
        grayImage.draw(0,0);
        canvas2.end();
        // ------------------------
        
        
        // Pasamos la imagen al fiducial finder
        fidfinder.findFiducials( grayImage );
        
        // Dibujamos el canvas de salida
        output.begin();
        
        ofBackground(0);
        
        if (warping) ofSetColor(255,255,0);
        else ofSetColor(255);
        
        ofNoFill();
        ofRect(1,1,outputWidth-1,outputHeight-1);
        
        ofSetColor(255);
        
        //use this method for the FiducialTracker
        //to get fiducial info you can use the fiducial getter methods
        for (list<ofxFiducial>::iterator fiducial = fidfinder.fiducialsList.begin(); fiducial != fidfinder.fiducialsList.end(); fiducial++) {
            
            int fiducialX = (fiducial -> getX()) * (float) outputWidth / (float) canvasWidth;
            int fiducialY = fiducial -> getY() * (float) outputHeight / (float) canvasHeight;
            float fiducialAngle = fiducial -> getAngleDeg() - 90;
            
            ofRect(0,0,outputWidth, outputHeight);
            ofPushMatrix();
            
            ofTranslate(fiducialX, fiducialY);
            ofRotate(fiducialAngle);
            
            cards[fiducial -> getId()].update(tableWidth, tableHeight);
            cards[fiducial -> getId()].draw();
            ofNoFill();
            ofSetColor(255);
            ofEllipse(0, 0, 20, 20);
            
            string text = "Fid: " + ofToString( fiducial->getId() );
            ofDrawBitmapString( text, 5, 5 );
            ofSetColor(255);
            
            ofPopMatrix();
            
            cout << "fiducial " << fiducial->getId() << " found at ( " << fiducial->getX() << "," << fiducial->getY() << " )" << endl;

        }
        
        output.end();
        
	}
    
    // Actualizamos el quad del ROI de la imagen de entrada
    
    for(int i = 0; i < 4; i++) {
        quadRoi.setVertex(i, roiCorners[i]);

    }
    
	
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // FONDO NEGRO Y COLOR BLANCO
    ofBackground(0);
    ofSetColor(255);
	
    // DIBUJAMOS IMAGEN CÁMARA EN CANVAS1
    canvas1.draw(canvas1Pos.x, canvas1Pos.y, canvasWidth, canvasHeight);
    
    // DIBUJAMOS IMAGEN BN FILTRADA EN CANVAS2
    canvas2.draw(canvas2Pos.x, canvas2Pos.y, canvasWidth, canvasHeight);
    
    warperGui.begin();
    
    // DIBUJAMOS OUTPUT EN OUTPUT CANVAS
    output.draw(outputPos.x, outputPos.y, outputWidth, outputHeight);
    
    warperGui.end();
    
    secondWindow.begin();
    ofBackground(0);
    warperOutput.begin();
    output.draw(mainDisplayWidth, 0, secondDisplayWidth, secondDisplayHeight);
    warperOutput.end();
    warperOutput.draw();
    secondWindow.end();

    gui.draw();
	
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	if( key == ' ' ) {
        bLearnBakground = true;
    } else if( key == '-' ) {
        threshold = MAX( 0, threshold-1 );
    } else if( key == '+' || key == '=' ) {
        threshold = MIN( 255, threshold+1 );
    } else if( key == 'b' ) {
		backgroundSubOn = false;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){

    
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
    if ( (selection) && (x > canvas1Pos.x) && ( x < canvas1Pos.x + canvasWidth) &&
        ( y > canvas1Pos.y ) && ( y < canvas1Pos.y + canvasHeight)) {
        
        ofPoint mouse = ofPoint(x - canvas1Pos.x, y - canvas1Pos.y);
        
        for (int i = 0; i < 4; i++) {
            
            if (activeCorners[i] == true) {
                
                roiCorners[i] = ofPoint(mouse.x, mouse.y);
                
            }
            
            else activeCorners[i] = false;
        }
        
    }
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
    if ( (selection) && (x > canvas1Pos.x) && ( x < canvas1Pos.x + canvasWidth) &&
        ( y > canvas1Pos.y ) && ( y < canvas1Pos.y + canvasHeight)) {
    
        ofPoint mouse = ofPoint(x - canvas1Pos.x, y - canvas1Pos.y);
        
        for (int i = 0; i < 4; i++) {
            
            if (mouse.distance(roiCorners[i]) < 10) {
                
                activeCorners[i] = true;
                
            }
            
            else activeCorners[i] = false;
        }
        
    }
    
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(){

    for (int i = 0; i < 4; i++) {
        
        activeCorners[i] = false;
            

    }
}


