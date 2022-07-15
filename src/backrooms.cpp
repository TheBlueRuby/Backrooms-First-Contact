#include <GL/glut.h> 

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

#include <random>
#include <windows.h>
#include <mmsystem.h>
#include <fluidsynth.h>

#include "../Textures/TexAtlas.ppm"
#include "../Textures/sky.ppm"
#include "../Textures/Title.ppm"
#include "../Textures/lost.ppm"
#include "../Textures/won.ppm"
#include "../Textures/monsterTex.ppm"

#include "../Maps/map01.txt"

#ifndef M_PI
    #define M_PI 3.14159265358979325406
#endif

float frame1, frame2, fps;
int gameState=0, timer=0;
float fade=0;

int startTime = time(0);

//Keyrepeat On
typedef struct
{
	int w,a,s,d;
}ButtonKeys; ButtonKeys Keys;

//Midi player using FluidSynth
void playMidi()
{
	//FluidSynth definitions
    fluid_settings_t* settings;
    fluid_synth_t* synth;
	fluid_player_t* player;
	fluid_audio_driver_t* audioDriver;
    settings = new_fluid_settings();

    synth = new_fluid_synth(settings);
	player = new_fluid_player(synth);

	fluid_settings_setstr(settings, "audio.driver", "dsound");
	
	fluid_player_set_loop(player, -1);

	//Soundfont Loading
	
	int sfont_id = fluid_synth_sfload(synth, "./Sounds/soundFont.sf2", 1);
	if(sfont_id == FLUID_FAILED)
	{
		int sfontError = MessageBoxA(
						 NULL,
				(LPCSTR)"The soundfont couldn't be loaded!\nCheck if you have a file called \"soundFont.sf2\" in the Sounds folder.\nThe game will still run, but without any music. Are you sure you want to continue?\nClick No to quit the program.",
				(LPCSTR)"Backrooms First Contact ERROR",
						 MB_ICONERROR | MB_YESNO
				  );
    	switch (sfontError)
    	{
    	case IDYES:
    	    break;
    	case IDNO:
    	    exit;
    	}
	}

	//Midi file loading
	int midiLoad = fluid_player_add(player, "./Sounds/D_BKROOM.mid");

	if(midiLoad == FLUID_FAILED)
	{
		int midiError = MessageBoxA(
						 NULL,
				(LPCSTR)"The midi file couldn't be loaded!\nCheck if you have a file called \"D_BKROOM.mid\" in the Sounds folder.\nThe game will still run, but without any music. Are you sure you want to continue?\nClick No to quit the program.",
				(LPCSTR)"Backrooms First Contact ERROR",
						 MB_ICONERROR | MB_YESNO
				  );
    	switch (midiError)
    	{
    	case IDYES:
    	    break;
    	case IDNO:
    	    exit;
    	}
	}


	audioDriver = new_fluid_audio_driver(settings, synth);
	
	//Playback
	fluid_player_play(player);	
	if(gameState==3 || gameState==4)
	{
		fluid_player_stop(player);
	}
}

void playEndSFX()
{
	PlaySound(NULL, NULL, 0); 
	PlaySound(TEXT("..\\Sounds\\first-contact.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

//------------------------Player Setup and unused 2D rendering------------------------------------------------
float degToRad(float a) { return a*M_PI/180.0;}
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}

float px,py,pdx,pdy,pa;

void drawPlayer2D()
{
 glColor3f(1,1,0);   glPointSize(8);    glLineWidth(4);
 glBegin(GL_POINTS); glVertex2i(px,py); glEnd();
 glBegin(GL_LINES);  glVertex2i(px,py); glVertex2i(px+pdx*20,py+pdy*20); glEnd();
}

//-----------------------------------------------------------------------------

/*------Enemy logic & rendering------*/
typedef struct
{
	int type;			//static, key, enemy
	int state;			//on or off
	int map;			//texture to display
	float x,y,z;		//position
}sprite; sprite sp[4];
int depth[120];

void drawSprite()
{
	int x,y,s;
	
	//enemy logic
	if(px<sp[0].x+30 && px>sp[0].x-30 && py<sp[0].y+30 && py>sp[0].y-30) {gameState=4; timer=0;}
	//attack
	int spx=(int)sp[0].x>>6,		  spy=(int)sp[0].y>>6; 			//Position on grid
	int spx_add=((int)sp[0].x+15)>>6, spy_add=((int)sp[0].y+15)>>6; //Position on grid added movement offset
	int spx_sub=((int)sp[0].x-15)>>6, spy_sub=((int)sp[0].y-15)>>6; //Position on grid subtract movement offset

	if(sp[0].x>px && mapWallArray[spy*10+spx_sub]==0){	sp[0].x-=0.03*fps;}
	if(sp[0].x<px && mapWallArray[spy*10+spx_add]==0){	sp[0].x+=0.03*fps;}
	if(sp[0].y>py && mapWallArray[spy_sub*10+spx]==0){	sp[0].y-=0.03*fps;}
	if(sp[0].y<py && mapWallArray[spy_add*10+spx]==0){	sp[0].y+=0.03*fps;}

	//rendering
	for(s=0;s<4;s++)
	{
		//temp float vars
		float sx=sp[0].x-px;
		float sy=sp[0].y-py;
		float sz=sp[0].z;

		//Billboard Effect
		float CS=cos(degToRad(pa)), SN=sin(degToRad(pa));
		float a=sy*CS+sx*SN;
		float b=sx*CS-sy*SN;
		sx=a; sy=b;

		//Convert to Screenspace
		sx=(sx*108.0/sy)+(120/2);
		sy=(sz*108.0/sy)+( 80/2);

		//Render sprite
		int scale=32*80/b;
		if(scale<0){scale=0;} if(scale>120){scale=120;}

		//Sprite Renderer
		float t_x=0, t_y=31, t_x_step=31.5/(float)scale, t_y_step=32.0/(float)scale;

		for (x=sx-scale/2;x<sx+scale/2;x++)
		{
			t_y=31;
			for (y=0;y<scale;y++)
			{
				if (sx>0 && sx<120 && b<depth[(int)sx])
				{
					int pixel=((int)t_y*32+(int)t_x)*3+(sp[s].map*32*32*3);
					int red  =monsterTex[pixel+0];
					int green=monsterTex[pixel+1];
					int blue =monsterTex[pixel+2];
					
					if(!(red==255 && blue==255 && green==0)) //Exclude Purple
					{
						glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,sy*8-y*8); glEnd();
					}
					t_y-=t_y_step; if(t_y<0) {t_y=0;}
				}
			}
			t_x+=t_x_step;
		}
	}
}

//-----------------------------------------------------

void drawMap2D()
{
 int x,y,xo,yo;
 for(y=0;y<mapY;y++)
 {
  for(x=0;x<mapX;x++)
  {
   if(mapWallArray[y*mapX+x]>0){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}
   xo=x*mapS; yo=y*mapS;
   glBegin(GL_QUADS); 
   glVertex2i( 0   +xo+1, 0   +yo+1); 
   glVertex2i( 0   +xo+1, mapS+yo-1); 
   glVertex2i( mapS+xo-1, mapS+yo-1);  
   glVertex2i( mapS+xo-1, 0   +yo+1); 
   glEnd();
  } 
 } 
}//-----------------------------------------------------------------------------

//---------------------------Draw Rays and Walls--------------------------------
//Reason: Causes compiler to flag line, unneccesary.  float distance(ax,ay,bx,by,ang){ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);}

void drawRays2D()
{
	
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 ra=FixAng(pa+30);                                                              //ray set back 30 degrees
 
 for(r=0;r<120;r++)
 {
  int vmt=0, hmt=0; //Map Texture Numbers
  //---Vertical--- 
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//looking left
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//looking right
  else { rx=px; ry=py; dof=8;}                                                  //looking up or down. no hit  

  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && mapWallArray[mp]>0){ vmt=mapWallArray[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
  else{ rx=px; ry=py; dof=8;}                                                   //looking straight left or right
 
  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && mapWallArray[mp]>0){ hmt=mapWallArray[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                                  //check next horizontal
  } 
  
  float shade=1;
  glColor3f(1,0.87,0.3);
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(1,0.85,0.);}        //horizontal hit first
  //glLineWidth(2); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd();		//draw ray in 2D map
    
  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
  int lineH = (mapS*640)/(disH);                 								//line height
  float ty_step=32.0/(float)lineH;
  float ty_off=0;
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}    
  int lineOff = 320 - (lineH>>1);                                               //line offset
  
  depth[r]=disH; //save line depth
  //Draw Textures on wall
  int y;
  float ty=ty_off*ty_step/*+hmt*32*/;

  float tx;
  if (shade==1) {tx=(int)(rx/2.0)%32; if (ra>180){ tx=31-tx;}} 
  else			{tx=(int)(ry/2.0)%32; if (ra>90 && ra<270){ tx=31-tx;}}			//Align texture based on wall direction
  
  for (y=0; y<lineH; y++)
  {
	int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
	int red  =TexAtlas[pixel+0]*shade;
	int green=TexAtlas[pixel+1]*shade;
	int blue =TexAtlas[pixel+2]*shade;
  	
  	glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,y+lineOff);glEnd();//draw vertical wall 
  	ty+=ty_step;
  }
 
  for(y=lineOff+lineH;y<640;y++)
  {
	  //Floor Textures
	  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
	  tx=px/2 + cos(deg)*318*32/dy/raFix;
	  ty=py/2 - sin(deg)*318*32/dy/raFix;
	  int mp=mapFloorArray[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;

	  int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
	  int red  =TexAtlas[pixel+0]*0.7;
	  int green=TexAtlas[pixel+1]*0.7;
	  int blue =TexAtlas[pixel+2]*0.7;
  	  glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,y);glEnd();

	  //Ceiling Textures
	  pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
	  red  =TexAtlas[pixel+0];
	  green=TexAtlas[pixel+1];
	  blue =TexAtlas[pixel+2];
  	  glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,640-y);glEnd();

  }

  ra=FixAng(ra-0.5);                                                              //go to next ray
 }
}//-----------------------------------------------------------------------------

void drawSky()
{int x, y;
	for (y=0;y<320; y++)
	{
		for (x=0;x<960; x++)
		{
			int xo=(int)pa*2-x; if(xo<0){ xo+=960;} xo=xo % 960;
		 	int pixel=(y*120+x)*3;
	  		int red  =sky[pixel+0];
	 		int green=sky[pixel+1];
	  		int blue =sky[pixel+2];
  	  		glPointSize(1);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(x,y);glEnd();
		}
	}
}

void screen(int v)
{int x, y;
 int *T;
 if(v==1){ T=title;};
 if(v==2){ T=won;};
 if(v==3){ T=lost;};
	for (y=0;y<640; y++)
	{
		for (x=0;x<960; x++)
		{
		 	int pixel=(y*960+x)*3;
	  		int red  =T[pixel+0]*fade;
	 		int green=T[pixel+1]*fade;
	  		int blue =T[pixel+2]*fade;
  	  		glPointSize(1);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(x,y);glEnd();
		}
	}
	if(fade<1){fade+=0.001*fps;}
	if(fade>1){fade=1;}
}


void init()
{
 glClearColor(0.3,0.3,0.3,0);
 px=150; py=400; pa=90;
 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa)); 
 mapWallArray[44] = 4;
 mapWallArray[74] = 4;
 playMidi();

 sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=5.5*64; sp[0].y=5*64; sp[0].z=20; //sprite 1
}

void display()
{   
	//Frame Rate Time Scaling
	frame2=glutGet(GLUT_ELAPSED_TIME);
	fps=(frame2-frame1);
	frame1=glutGet(GLUT_ELAPSED_TIME);

	//Gamestate Manager
	if (gameState==0) { init(); fade=0; timer=0; gameState=1;} 											 //Game Initialization
	if (gameState==1) { screen(1); timer+=1*fps; if(timer>3000) {timer=0; gameState=2;}}				 //Title Screen
	if (gameState==2)																					 //Main Game Loop
	{
		//Button Turn Handler
		if(Keys.a){ pa+=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
		if(Keys.d){ pa-=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 

		//Collision Handler
		int xo=0; if(pdx<0) { xo=-20;} else { xo=20;}
		int yo=0; if(pdy<0) { yo=-20;} else { yo=20;}
		int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;
		int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipx_sub_yo=(py-yo)/64.0;

		//Button Movement Handler
		if(Keys.w)
		{ 
			if(mapWallArray[ipy*mapX		 + ipx_add_xo]==0) {px+=pdx*0.2*fps;}
			if(mapWallArray[ipy_add_yo*mapX  + ipx		 ]==0) {py+=pdy*0.2*fps;}
		}			

		if(Keys.s)
		{ 
			if(mapWallArray[ipy*mapX		 + ipx_sub_xo]==0) {px-=pdx*0.2*fps;}
			if(mapWallArray[ipx_sub_yo*mapX  + ipx	     ]==0) {py-=pdy*0.2*fps;}
		}

		//Redraw Graphics
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		//drawSky();
		drawRays2D();
		drawSprite();

		timer+=1*fps;

		//play monster audio
		if(timer>29000 && timer<30000){playEndSFX();}
		//Check win condition
		if(timer>30000){fade=0; timer=0; gameState=3;}

	}

	if(gameState==3) { 	screen(2); timer+=1*fps; if(timer>3000				 ) { fade=0; timer=0; gameState=0;}	} //Win Screen
	if(gameState==4) { 	screen(3); timer+=1*fps; if(timer>3000 && timer<30000) { fade=0; timer=0; gameState=0;} } //Lose Screen

	glutPostRedisplay();
	glutSwapBuffers();  
}

void openDoor()
{
		int xo=0; if(pdx<0) { xo=-25;} else { xo=25;}
		int yo=0; if(pdy<0) { yo=-25;} else { yo=25;}
		int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
		int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;
		if (mapWallArray[ipy_add_yo*mapX+ipx_add_xo]==4 || mapWallArray[ipy_add_yo*mapX+ipx_add_xo]==5)
			{
				mapWallArray[ipy_add_yo*mapX+ipx_add_xo]=0;
			}
}

void ButtonDown(unsigned char key, int x, int y)
{
	if (key=='w') {Keys.w=1;}
	if (key=='a') {Keys.a=1;}
	if (key=='s') {Keys.s=1;}
	if (key=='d') {Keys.d=1;}
	if (key=='e')							//Open Door
	{openDoor();}
	if (key==' ')
	{openDoor();}
	glutPostRedisplay();
}

void ButtonUp(unsigned char key, int x, int y)
{
	if (key=='w') {Keys.w=0;}
	if (key=='a') {Keys.a=0;}
	if (key=='s') {Keys.s=0;}
	if (key=='d') {Keys.d=0;}
	glutPostRedisplay();
}

void specialButtonDown(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			Keys.w=1;
			break;
		case GLUT_KEY_DOWN:
			Keys.s=1;
			break;
		case GLUT_KEY_LEFT:
			Keys.a=1;
			break;
		case GLUT_KEY_RIGHT:
			Keys.d=1;
			break;
	}
}

void specialButtonUp(int key, int x, int y) 
{
 	switch (key)
 	{
		case GLUT_KEY_UP:
			Keys.w=0;
			break;
		case GLUT_KEY_DOWN:
			Keys.s=0;
			break;
		case GLUT_KEY_LEFT:
			Keys.a=0;
			break;
		case GLUT_KEY_RIGHT:
			Keys.d=0;
			break;
	 }
}

void resize (int w,int h)
{
	glutReshapeWindow(960,640);
}

int main(int argc, char* argv[])
{ 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(960,640);
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2-960/2,glutGet(GLUT_SCREEN_HEIGHT)/2-640/2);
	glutCreateWindow("Backrooms: First Contact Demo 0.2 by Mite Productions");
	gluOrtho2D(0,960,640,0);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(ButtonDown);
	glutKeyboardUpFunc(ButtonUp);
	glutSpecialFunc(specialButtonDown);
	glutSpecialUpFunc(specialButtonUp);
	glutMainLoop();
}
