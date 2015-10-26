#ifndef _OF_APP
#define _OF_APP

#include "ofMain.h"
#include "ofxOpenCv.h"
//#include "ofxCvMain.h"
#include "ofxFidMain.h"
#include "ofxGui.h"
#include "ofxGLWarper.h"
#include "ofxSecondWindow.h"

//stupid ball class
class card{
    
public:
    card(){
        
    }
    
    void setup(int _id){
        
        ident = _id;
    
        
        
        if (ident == 0) image.loadImage("Frame1.png");
        else if (ident == 1) image.loadImage("Frame2.png");
        else if (ident == 2) image.loadImage("Round1.png");
        else if (ident == 3) image.loadImage("Round2.png");
        else if (ident == 4) image.loadImage("Text1.png");
        else if (ident == 5) image.loadImage("Text2.png");
        else if (ident == 6) image.loadImage("Frame2.png");
        else if (ident == 7) image.loadImage("Frame2.png");
        else if (ident == 8) image.loadImage("Frame2.png");
        else if (ident == 9) image.loadImage("Frame2.png");
        
        size.x = 120;       // Tama–o tarjeta grande ancho: 12 cm
        size.y = 80;       // Tama–o tarjeta grande alto: 8 cm
        
        time = ofGetSystemTimeMicros();
        timeCheck = time;
        timeOrigin = time;
        realTime = time;
        timer = 2000000;
        
        
    }
    
    void update(int width, int height){
        
        time = ofGetSystemTimeMicros();
        
        if (time - timeCheck > timer) {
            
            timeOrigin = time;
            
        }
        
        timeCheck = time;
        realTime = time - timeOrigin;
        
        size.x = 120 * 640 / width;
        size.y = 80 * 480 / height;
        
    }
    
    void draw(){
        
        
        image.draw(0,0,size.x,size.y);
        
    }
    
    ofPoint size;
    int ident;
    float deg;
    float time;
    float timeOrigin;
    float timeCheck;
    float realTime;
    float timer;
    
    ofVideoPlayer video;
    ofImage image;
    string texto;
    
};


class ofApp : public ofSimpleApp{
	
	public:
	
		ofVideoGrabber 		vidGrabber;
		ofxCvColorImage		colorImg;
        ofxCvColorImage     warpedImg;
        ofxCvGrayscaleImage grayImage;
	
		ofxFiducialTracker	fidfinder;
	
		bool				bLearnBakground;
		bool				backgroundSubOn;
        int                 camWidth;
        int                 camHeight;
        int                 canvasWidth;
        int                 canvasHeight;
        int                 outputWidth;
        int                 outputHeight;
        int                 projWidth;
        int                 projHeight;
        ofPoint             guiPos;
        ofPoint             canvas1Pos;
        ofPoint             canvas2Pos;
        ofPoint             outputPos;
        ofxPanel            gui;
        ofxToggle           selection;
        ofxToggle           warping;
        ofxIntSlider        threshold;
        ofxIntSlider        tableWidth;
        ofxIntSlider        tableHeight;

        ofPoint             roiCorners[4];
        ofPoint             destCorners[4];
        bool                activeCorners[4];
        ofFbo               canvas1;
        ofFbo               canvas2;
        ofFbo               canvasWarp;
        ofFbo               output;
        ofFbo               warpedOutput;
        ofPixels            pixels;
        ofMesh              quadRoi;
        ofxGLWarper         warperGui;
        ofxGLWarper         warperOutput;
        ofxSecondWindow     secondWindow;
        card                cards[10];

        int whichDisplay;
        int mainDisplayWidth, mainDisplayHeight;
        int secondDisplayWidth, secondDisplayHeight;
    
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased();
		
		
};

#endif
	
