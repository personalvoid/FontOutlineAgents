#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup()
{
    bRunAgents = false ;


    //FontPool::Instance()->addFont( "Batang.ttf" , "BATANG" ) ;
    //FontPool::Instance()->addFont( "CooperBlack.ttf" , "COOPER_BLACK" ) ;

    ofSetLogLevel( OF_LOG_VERBOSE ) ;
    ofBackground( 0 ) ;
    ofSetFrameRate( 60 ) ;
    ofSetVerticalSync( true ) ;

    Tweenzor::init( ) ;
 //   colorPicker.setColorRadius( 1.0 );

    //This is just an easy way to grab a color from a color palette
    //colorPool.addColor( ofColor( 242 , 202 , 82 ) ) ;
    //colorPool.addColor( ofColor( 242 , 163 , 65 ) ) ;
    //colorPool.addColor( ofColor( 217 , 99 , 30 ) ) ;
    //colorPool.addColor( ofColor( 89 , 25 , 2 ) ) ;

    //Add our quote in line by line
    quote.setup( ) ;

    //void addWordBlock ( string word , ofPoint position , int fontSize ) ;
    float padding = 50 ;
   // quote.addWordBlock( "WORD1", ofPoint( ofRandom( ofGetWidth() ) , ofRandom( padding , ofGetHeight() - padding ) )  , quote.fontSize ) ;
   // quote.addWordBlock( "WORD2", ofPoint( ofRandom( ofGetWidth() ) , ofRandom( padding , ofGetHeight() - padding ) )  , quote.fontSize ) ;
    //quote.addWordBlock( "WORD3", ofPoint( ofRandom( ofGetWidth() ) , ofRandom( padding , ofGetHeight() - padding ) )  , quote.fontSize ) ;
    setupGUI( ) ;

    canvasAlpha = 255.0f ;

    ofPoint center = ofPoint ( ofGetWidth() / 2 , ofGetHeight() / 2 ) ;



    //agentTypes.push_back("ribbon" ) ;
    //agentTypes.push_back("circles" ) ;
    //agentTypes.push_back("triangle" ) ;

    ofEnableAlphaBlending() ;
    bDebugDraw = false ;
    inspector.setup( ) ;

    ofAddListener( Events::Instance()->ADD_NEW_COLOR , this , &testApp::newColorHandler ) ;
    ofAddListener( Events::Instance()->REMOVE_LAST_COLOR , this , &testApp::removeLastColorHandler ) ;
    ofAddListener( Events::Instance()->FONT_UPDATED , this , &testApp::fontUpdatedHandler ) ;

    trailFbo.allocate( ofGetWidth() , ofGetHeight() ) ;
    trailFbo.begin() ;
    ofClear( 0 , 0 , 0, 1 ) ;
    trailFbo.end() ;
}

void testApp::newColorHandler( ofColor &color )
{
    cout << "newColor HANDLER! R : " << color.r << " G: " << color.g << " B: " << color.b << endl ;
    colorPool.setColors( inspector.colors ) ;
    resetAgents( ) ;
}

void testApp::removeLastColorHandler( int &args )
{
    colorPool.setColors( inspector.colors ) ;
    resetAgents( ) ;
}

void testApp::createNewWordBlock()
{
    string batangPath = "Batang.ttf"  ;
    quote.addWordBlock( "" , inspector.getSelectedFontPath() , ofPoint( ofGetWidth() / 2 , ofGetHeight() / 2 ) , newFontSize , true ) ;
}

//--------------------------------------------------------------
void testApp::update()
{
    ofSetWindowTitle(  "FPS: " + ofToString( ofGetFrameRate()) ) ;
//    ofSetWindowTitle( "FPS: " + ofToString( ofGetFrameRate() ) ;
    Tweenzor::update( ofGetElapsedTimeMillis() ) ;
    inspector.update( ) ;


    if ( !bRunAgents || quote.bReadyToStart == false )
    {
        //cout << "not ready to run!" << endl ;
        return ;
    }


    int count = 0 ;
    vector<Agent*>::iterator a ;
    for ( a = agents.begin() ; a != agents.end() ; a++ )
    {
        (*a)->update() ;
        (*a)->update() ;
        (*a)->update() ;


        //TODO: this feels so hacky make this actually use an event
        //Pass the index ? or a reference to the agent itseld ?
        //Flag if the target needs to be changed
        if ( (*a)->bTarget == true )
        {
            ofPoint _position = quote.getNextTarget( count ) ;
            QuotePath * qp = quote.getQuotePathAt( count ) ;
           // if ( qp
            if ( qp->bTeleportFlag == true )
            {
                (*a)->position = _position ;
                qp->bTeleportFlag = false ;
                (*a)->startNewPath() ;
            }

            if ( qp->bFinished == true )
            {

                (*a)->bFinished = true ;
            }

            (*a)->bTarget = false ;
            (*a)->setTarget ( _position ) ;
        }
        count++ ;
    }

    cout << "num of Agents : " << agents.size() << endl ; 
}



//--------------------------------------------------------------
void testApp::draw()
{
    ofSetColor( 215 , 215 , 215 ) ;
    ofRect( 0 , 0 , ofGetWidth() , ofGetHeight() ) ;



    //newProjectBook.draw( ) ;
    quote.drawWordBlocks( ) ;

    trailFbo.begin() ;

    //Call the agent draw() ! Nice and simple
    for ( int a = 0 ; a < agents.size() ; a++ )
    {
        ofPushMatrix() ;
            int wordIndex = quote.getQuotePathAt( a )->curWord ;
            agents[a]->draw ( false ) ;
        ofPopMatrix() ;
    }

    trailFbo.end() ;

    ofSetColor( 255 , 255 , 255 ) ;
    trailFbo.draw( 0 , 0 ) ;


    inspector.draw( ) ;

}


void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();

	if(name == "MAX SPEED")
		a_maxSpeed = ((ofxUISlider *) e.widget)->getScaledValue();

    if(name == "MAX FORCE")
		a_maxForce = ((ofxUISlider *) e.widget)->getScaledValue();

    if(name == "BUFFER DIST")
		a_targetBuffer = ((ofxUISlider *) e.widget)->getScaledValue();

    if(name ==  "PATH SAMPLING" )
		a_pathSampling =(int) ((ofxUISlider *) e.widget)->getScaledValue();

    if ( name == "NUM AGENTS" )
    {
        a_numAgents =(int) ((ofxUISlider *) e.widget)->getScaledValue();
        resetAgents( ) ;
    }

    if ( name == "MAX SPEED R OFFSET" )
        a_rOffsetMaxSpeed = ((ofxUISlider *) e.widget)->getScaledValue();

    if ( name == "MAX FORCE R OFFSET"  )
        a_rOffsetMaxTurn = ((ofxUISlider *) e.widget)->getScaledValue();

    if ( name == "TRAIL TYPES" )
    {
       // ((ofxUIRadio * ) e.widget)->ge
    }

    if ( name == "NEW FONT SIZE" )
    {
        newFontSize = ((ofxUISlider *) e.widget)->getScaledValue() ;

         WordBlock * wb = quote.getEditableBlock( ) ;
         if ( wb != NULL )
         {
             updateNewWordBlock( wb->word , newFontSize ) ;
             resetAgents() ;
         }
    }
    //newFontSize
    /*
     vector<string> hnames; hnames.push_back("LINES"); hnames.push_back("CIRCLES"); hnames.push_back("RECTANGLES");
     gui->addWidgetDown(new ofxUIRadio( radioSize , radioSize , "TRAIL TYPES", hnames, OFX_UI_ORIENTATION_HORIZONTAL));
     */

    gui->saveSettings("GUI/settings.xml") ;

    vector<Agent*>::iterator a ;
    for ( a = agents.begin() ; a != agents.end() ; a++ )
    {
        (*a)->maxVelocity = a_maxSpeed + ofRandom( -a_rOffsetMaxSpeed , a_rOffsetMaxSpeed ) ;
        (*a)->maxForce = a_maxForce  + ofRandom( -a_rOffsetMaxTurn , a_rOffsetMaxTurn ) ;
        (*a)->targetBufferDist = a_targetBuffer ;
        (*a)->pathSampling = a_pathSampling ;
    }
}

void testApp::openFontDialogue( )
{
    //
    cout << "open Font Dialogue!" << endl ;
    ofFileDialogResult fontResult = ofSystemLoadDialog(  "Open New .otf or .ttf font file" , false ) ;


    string path = fontResult.getPath() ; //
    cout << "path : " << path << endl ;
    inspector.addFont( path ) ;

}

void testApp::updateNewWordBlock ( string _word , float _fontSize )
{
    WordBlock * wb = quote.getEditableBlock() ;
    if ( wb == NULL )
    {
        cout << "word block is NULL " << endl ;
        return ;
    }

    wb->word = _word ;
    wb->fontSize = _fontSize ;
    //wb->set
    wb->updateWord( ) ;
    wb->updateFont(  inspector.getSelectedFontPath() , wb->fontSize ) ;
    //wb->font.loadFont( inspector.getSelectedFontPath() , wb->fontSize , true, true, true ) ;
}


void testApp::fontUpdatedHandler( int &args )
{
    WordBlock * wb = quote.getEditableBlock() ;
    if ( wb != NULL )
    {
       wb->updateFont( inspector.getSelectedFontPath() , wb->fontSize  ) ;
    }

}
void testApp::createNewAgent()
{
    int index = agents.size() ;
    //Create our agent
    Agent* agent = new RibbonAgent() ;
    agent->colorPool.addColor( colorPool.getRandomColor() ) ;
    agent->colorPool.addColor( colorPool.getRandomColor() ) ;
    agent->colorPool.addColor( colorPool.getRandomColor() ) ;

    // ( x , y ) , speed, maxTurn
    agent->setup( ofVec2f ( ofGetWidth()/2 , ofGetHeight()/2 ) , ofVec2f( 0.3f , 0 ) , 20.0f ) ;
    agent->color = agent->colorPool.getRandomColor() ;

    quote.createQuotePath( ) ;
   // cout << "making agent : " << i << endl ;
    ofPoint p1 = quote.getPointByChar ( 0 , index ) ;
    agent->startNewPath() ;
    agent->position = p1 ;
    agent->target = p1 ;

    agent->maxVelocity = a_maxSpeed ;
    agent->maxForce = a_maxForce ;
    agent->targetBufferDist = a_targetBuffer ;
    agent->pathSampling = a_pathSampling ;


    agents.push_back( agent ) ;

}

void testApp::exportPDF( )
{
    ofFileDialogResult result = ofSystemSaveDialog("quote", "Save your font layers" ) ;


    //ofBackground( 0 , 0, 0 ) ;
    for ( int i = 0 ; i < agents.size() ; i++ )
    {
        string _fileName = result.getPath() +"_"+ ofToString( i ) + ".pdf" ;
        ofBeginSaveScreenAsPDF( _fileName ) ;
        ofSetColor( 255 , 255 , 255 , 0 ) ;
        ofRect( 0 , 0, ofGetWidth() , ofGetHeight() ) ;
        //ofTranslate( quote.charTranslateOffset.x , quote.charTranslateOffset.y ) ;
     //   int wordIndex = quote.getQuotePathAt( i )->curLine ;
     //   ofTranslate( quote.wordBlocks[ wordIndex ]->translate.x , quote.wordBlocks[ wordIndex ]->translate.y , 0 ) ;
        //ofSetColor( 0 , 0 , 0 ) ;
        //ofRect( 0 , 0, ofGetWidth() , ofGetHeight() ) ;
        agents[i]->draw( false ) ;
        agents[i]->drawEntirePath( ) ;
        ofEndSaveScreenAsPDF() ;
    }


    string combinedName = result.getPath() +"_combined.pdf" ;
    ofBeginSaveScreenAsPDF( combinedName ) ;
    ofSetColor( 255 , 255 , 255 , 0 ) ;
    ofRect( 0 , 0, ofGetWidth() , ofGetHeight() ) ;


    //ofTranslate( quote.charTranslateOffset.x , quote.charTranslateOffset.y ) ;

    for ( int i = 0 ; i < agents.size() ; i++ )
    {
        //string _fileName = result.getPath() +"_"+ ofToString( i ) + ".pdf" ;
               //ofSetColor( 0 , 0 , 0 ) ;
        //ofRect( 0 , 0, ofGetWidth() , ofGetHeight() ) ;
        agents[i]->draw( false ) ;

    }
    ofEndSaveScreenAsPDF() ;


}


void testApp::setupGUI ( )
{
    float canvasHeight = 125 ;
    float sliderLength = 100 ;
    float padding = 15 ;
    gui = new ofxUICanvas( 0 , ofGetHeight() - canvasHeight , ofGetWidth() , canvasHeight );
    //gui->addWidgetDown(new ofxUILabel("SLIDERS", OFX_UI_FONT_LARGE));
    gui->addWidgetRight(new ofxUILabel("SPACE - play/pause , S - save project", OFX_UI_FONT_SMALL ));
    gui->addWidgetRight(new ofxUILabel("R - reset , F - Load Font , N - New Word Box , E - Edit Word , ENTER - Finish Edit , BACKSPACE - Delete Word , P - PDF" , OFX_UI_FONT_SMALL ));

    gui->addWidgetDown(new ofxUISlider( sliderLength , 15 , 0.0, 12.0f, a_maxSpeed, "MAX SPEED"));
    gui->addWidgetRight(new ofxUISlider( sliderLength , 15 , 0.0f, 4.0f, a_rOffsetMaxSpeed , "MAX SPEED R OFFSET" )) ;
    gui->addWidgetRight(new ofxUISlider( sliderLength , 15 , 0.0, 5.0f, a_maxForce, "MAX FORCE"));
    gui->addWidgetRight(new ofxUISlider( sliderLength , 15 , 0.0f, 4.0f, a_rOffsetMaxTurn , "MAX FORCE R OFFSET" )) ;

    gui->addWidgetDown(new ofxUISlider( sliderLength , 15 , 0.0, 50.0f, a_targetBuffer, "BUFFER DIST"));
    gui->addWidgetRight(new ofxUISlider( sliderLength , 15 , 1 , 14 , a_pathSampling, "PATH SAMPLING"));
    gui->addWidgetRight(new ofxUISlider( sliderLength , 15 , 1 , 125 , a_numAgents, "NUM AGENTS"));
    gui->addWidgetRight(new ofxUISlider( sliderLength * 2 , 15 , 1 , 125 , newFontSize, "NEW FONT SIZE" ));

    //newFontSize
    float radioSize = 45 ;
    vector<string> hnames; hnames.push_back("LINES"); hnames.push_back("CIRCLES"); hnames.push_back("RECTANGLES");
	//gui->addWidgetDown(new ofxUIRadio( radioSize , radioSize , "TRAIL TYPES", hnames, OFX_UI_ORIENTATION_HORIZONTAL));
    //vector<string> vnames;
    /*
    vnames.push_back("ROCKS");
    vnames.push_back("MY");
    vnames.push_back("SOCKS");

    ofxUIRadio *radio = (ofxUIRadio *) gui->addWidgetDown(new ofxUIRadio(sliderLength, sliderLength, "VERTICAL RADIO", vnames, OFX_UI_ORIENTATION_VERTICAL));
    radio->activateToggle("SOCKS"); */

    ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);
    gui->loadSettings( "GUI/settings.xml" ) ;
   // gui->toggleVisible() ;
}

void testApp::resetAgents()
{
    if ( !quote.bReadyToStart )
        return ;

    trailFbo.begin() ;
    ofClear( 0 , 0 , 0, 1 ) ;
    trailFbo.end() ;

    quote.resetQuotePaths( ) ;
    agents.clear() ;

    for ( int i = 0 ; i < a_numAgents ; i++ )
    {
        createNewAgent( ) ;
    }

    bRunAgents = false ;
}

void testApp::saveProjectFile( )
{
    projectXml.clear( ) ;
    ofFileDialogResult saveResult = ofSystemSaveDialog( "myProject" , "Project Name?" ) ;
    string path = saveResult.getPath() ; //

    int index = path.find( ".xml" ) ;
    if ( index > 0 )
        cout << "user added .xml" << endl ;
    else
        path += ".xml" ;

//    projectXml.setValue( "fontPath"  , quote.fontPath ) ;
//    projectXml.setValue( "fontSize" , quote.fontSize ) ;

    for ( int i = 0 ; i < quote.wordBlocks.size() ; i++ )
    {
        int tagNum = projectXml.addTag( "wordBlock" ) ;
        WordBlock * wb = quote.wordBlocks[i] ;
        projectXml.pushTag( "wordBlock" , tagNum ) ;
        projectXml.setValue( "text" , wb->word ) ;
        projectXml.setValue( "fontSize" , wb->fontSize ) ;
        string rawFontPath = wb->fontPath ;
        cout << "rawPath : " << rawFontPath << endl ;
        string shortFontPath = rawFontPath.substr ( 3 , rawFontPath.size() - 3 ) ;
        cout << "shortened fontPath: " << shortFontPath << endl ;
        projectXml.setValue( "fontPath" , shortFontPath ) ;
        projectXml.setValue( "translateX" , wb->translate.x ) ;
        projectXml.setValue( "translateY" , wb->translate.y ) ;
        //projectXml.setValue( "wordBlock" , quote.wordBlocks[i]->word , i ) ;
        projectXml.popTag( ) ;
    }


    for ( int c = 0 ; c < inspector.colors.size() ; c++ )
    {
        //projectXml.addTag( "color" ) ;
        //projectXml.pushTag( "color" , c ) ;
        //int hexColor = colorPool.pool[c].toHex() ;
            //projectXml.setValue( "color" , hexColor ) ;
            projectXml.setValue( "r" , colorPool.pool[c].r ) ;
            projectXml.setValue( "g" , colorPool.pool[c].g ) ;
            projectXml.setValue( "b" , colorPool.pool[c].b ) ;
       // projectXml.popTag( ) ;

    }

    for ( int f = 0 ; f < inspector.fonts.size() ; f++ )
    {
        projectXml.setValue( "fontPath" , inspector.fonts[f].filePath , f ) ;
    }

    projectXml.setValue ( "MAX SPEED" , a_maxSpeed ) ;
    projectXml.setValue ( "MAX SPEED R OFFSET" , a_rOffsetMaxSpeed ) ;
    projectXml.setValue ( "MAX FORCE" , a_maxForce ) ;
    projectXml.setValue ( "MAX FORCE R OFFSET" , a_rOffsetMaxTurn ) ;
    projectXml.setValue ( "BUFFER DIST" , a_targetBuffer ) ;
    projectXml.setValue ( "PATH SAMPLING" , a_pathSampling ) ;
    projectXml.setValue ( "NUM AGENTS" , a_numAgents ) ;
    projectXml.saveFile( path ) ;

}

void testApp::openProjectFile( )
{

    cout << "open Dialgoue!" << endl ;
    ofFileDialogResult loadResult = ofSystemLoadDialog(  "Open Project XML" ) ;


    string path = loadResult.getPath() ; //
    cout << "end result XML path!" << endl ;

    projectXml.loadFile( path ) ;

    int numWordBlocks = projectXml.getNumTags( "wordBlock" ) ;

    quote.clearWordBlocks() ;
    quote.setup( ) ;

    for ( int i = 0 ; i < numWordBlocks ; i++ )
    {
        projectXml.pushTag( "wordBlock" , i ) ;
        ofPoint translate = ofPoint( projectXml.getValue( "translateX" , 0 ) , projectXml.getValue( "translateY" , 0 ) ) ;
        string text =   projectXml.getValue( "text" , "noText" ) ;
        string fontPath = "../" + projectXml.getValue( "fontPath" , "noFont" ) ;
        float fontSize = projectXml.getValue ( "fontSize" , 18 ) ;

        quote.addWordBlock( text , fontPath , translate , fontSize ) ;
        projectXml.popTag( ) ;
    }


    int numColorTags = projectXml.getNumTags( "color" ) ;
    cout << "numColorTags : " << numColorTags << endl ;
    colorPool.clear() ;
    inspector.colors.clear() ;
    for ( int c = 0 ; c < numColorTags ; c++ )
    {
        projectXml.pushTag( "color" , c ) ;

            ofColor color ;
            color.r = projectXml.getValue( "r" , 125 ) ;
            color.g = projectXml.getValue( "g" , 125 ) ;
            color.b = projectXml.getValue( "b" , 125 ) ;
            //cout << "r : " << color.r << " g : " << color.g << " b : " << color.b << endl ;
            colorPool.addColor( color ) ;
        projectXml.popTag( ) ;

        inspector.colors.push_back( color ) ;

    }
    a_maxSpeed = projectXml.getValue ( "MAX SPEED" , a_maxSpeed , a_maxSpeed ) ;
    a_rOffsetMaxSpeed = projectXml.getValue ( "MAX SPEED R OFFSET" , a_rOffsetMaxSpeed , a_rOffsetMaxSpeed ) ;
    a_maxForce = projectXml.getValue ( "MAX FORCE" , a_maxForce , a_maxForce ) ;
    a_rOffsetMaxTurn = projectXml.getValue ( "MAX FORCE R OFFSET" , a_rOffsetMaxTurn , a_rOffsetMaxTurn ) ;
    a_targetBuffer = projectXml.getValue ( "BUFFER DIST" , a_targetBuffer , a_targetBuffer ) ;
    a_pathSampling = projectXml.getValue ( "PATH SAMPLING" , a_pathSampling , a_pathSampling ) ;
    a_numAgents = projectXml.getValue ( "NUM AGENTS" , a_numAgents , a_numAgents ) ;

    resetAgents( ) ;
    quote.bReadyToStart = true ;
}

void testApp::mousePressed ( int x , int y , int button )
{
    //newProjectBook.input( x , y ) ;
    quote.inputDown ( x , y ) ;
    inspector.input( x , y ) ;

}

void testApp::initProject ( )
{
   // quote.clearQuotes() ;
   // newTextLine = "BLANK" ;
}

void testApp::mouseDragged ( int x , int y , int button )
{
    quote.inputMove ( x , y ) ;
}

void testApp::mouseReleased( int x , int y , int button )
{
    quote.inputUp ( x , y ) ;
}

void testApp::keyPressed( int key )
{
    cout << "key : " << key << endl ;
 //   if ( canvasAlpha > 0.0f )
 //   {
        WordBlock * wb = quote.getEditableBlock( ) ;
        if ( wb == NULL )
        {
            switch ( key )
            {
                case 'p':
                case 'P':
                    exportPDF() ;
                    break ;

                case 'r':
                case 'R':
                    resetAgents() ;
                    break ;

                case 's':
                case 'S':
                    saveProjectFile( ) ;
                    break ;

                case 'o':
                case 'O':
                    openProjectFile( ) ;
                    break ;

                case ' ':
                    bRunAgents = !bRunAgents ;
                    break ;

                case 'd':
                case 'D':
                    bDebugDraw = !bDebugDraw ;
                    break ;

                case 'n':
                case 'N':
                    cout << "new WordBlock" << endl ;
                    createNewWordBlock() ;
                    break ;

                case 13 :
                    cout << "end typing wordBlock" << endl ;
                    break ;

                case 'e':
                case 'E':
                    quote.editWordBlockAt ( mouseX , mouseY ) ;
                    break ;

                case 'f':
                case 'F':
                    openFontDialogue() ;
                    break ;
                case 127 :
                case 8 :
                    quote.removeWordBlockAt ( mouseX , mouseY ) ;
                    break ;

            }
            cout << "no editable path!" << endl;
        }
        else
        {
            if ( key == 13 )
            {
                wb->bEditable = false ;
                return ;
            }

            //Backspace
            else if ( key == 127|| key == 8 )
            {
                string word = wb->word ;
                if ( word.size() > 0 )
                {
                    string word1 = word.substr( 0 , word.size()-1 ) ;
                    word = word1 ;
                }
                wb->word = word ;

                updateNewWordBlock( wb->word , newFontSize ) ;
                //updateNewWordBlock( wb->word , newFontSize ) ;
                return ;
            }

            wb->word += key ;
            cout << "updating word blcok with : " << wb->word << endl ;
            //void testApp::updateNewWordBlock ( string _word , float _fontSize )
           //   WordBlock * wb = quote.getEditableBlock( ) ;
            updateNewWordBlock( wb->word , newFontSize ) ;

        }
  //  }

}
