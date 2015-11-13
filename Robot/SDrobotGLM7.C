
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <stack> 
#include <math.h> 

using namespace std; 

#ifdef __APPLE__
#include <GL/glew.h> 
#include <GLUT/glut.h> 
#include <OpenGL/gl.h> 
#else 
#include <GL/glew.h> 
#include <GL/glut.h> 
#include <GL/gl.h>
#define M_PI 3.1415926
#endif

#include<glm/glm.hpp> 
#include<glm/gtx/transform.hpp>

GLuint programObject;
GLuint SetupGLSL(char*); 

typedef struct 
{
  float location[4]; 
  float normal[4]; 
  float color[4]; 
} Vertex; 

int nindices; 
Vertex *cyverts;   // cylinder vertices
GLuint *cindices; 


float vertices[] = {-0.5, -0.5, -0.5, 1.0,
		    0,0.6,0, 1, 
		    0.5, -0.5, -0.5, 1.0,
		    0.6,0.6,0, 1, 
		    0.5, 0.5, -0.5, 1.0,
		    0,0.6,0, 1, 
		    -0.5, 0.5, -0.5,1.0,
		    0,0.6,0, 1, 
		    -0.5, -0.5, 0.5, 1.0,
		    0,0.6,0, 1, 
		    0.5, -0.5, 0.5, 1.0,
		    0,0.6,0, 1, 
		    0.5, 0.5, 0.5, 1.0,
		    0,0, 0.6, 1, 
		    -0.5, 0.5, 0.5, 1.0

}; 

GLubyte tindices[36];  

GLuint vboHandle[2];   // a VBO that contains interleaved positions and colors 
GLuint indexVBO[2]; 

float angle1=0, angle2=0;
float angle3=0, angle4=0;

glm::mat4 modelM = glm::mat4(1.0f); 

stack<glm::mat4> mat_stack;

/////////////////////////////////
// Define Light Properties -  Ia, Id, Is, and light position 
//////////////////////////////////

GLfloat light_ambient[4] = {0.8,0.8,0.8,1};  //Ia 
GLfloat light_diffuse[4] = {0.8,0.8,0.8,1};  //Id
GLfloat light_specular[4] = {1,1,1,1};  //Is
GLfloat light_pos [4] = {10,10, 10, 1};

/////////////////////////////////
// Define Default Material Properties -  Ka, Kd, Ks, Shininess 
//////////////////////////////////

GLfloat mat_ambient[4] = {0.3,0.3,0.3,1};  //Ka 
GLfloat mat_diffuse[4] = {0.8,0.8,0,1};  //Kd
GLfloat mat_specular[4] = {1,1,1,1};  //Ks
GLfloat mat_shine[1] = {20}; 


/////////////////////////////////
// glut mouse control 
// 
int xform_mode = 0; 
#define XFORM_NONE    0 
#define XFORM_ROTATE  1
#define XFORM_SCALE 2 

int press_x, press_y; 
int release_x, release_y; 
float z_angle = 0.0;
float x_angle = 0.0; 
float scale_size = 1;

bool WIRE_FRAME =false; 

////////////////////////////////////////////////////////////////////////////////////

void InitCylinder(int nslices, int nstacks, float r, float g, float b) 
{
  int nvertices = nslices * nstacks; 
  cyverts = new Vertex[nvertices]; 

  printf(" M PI = %f\n", M_PI); 
  float Dangle = 2*M_PI/(float)(nslices-1); 

  for (int j =0; j<nstacks; j++)
    for (int i=0; i<nslices; i++) {
      int idx = j*nslices + i; // mesh[j][i] 
      float angle = Dangle * i; 
      cyverts[idx].location[0] = cyverts[idx].normal[0] = cos(angle); 
      cyverts[idx].location[1] = cyverts[idx].normal[1] = sin(angle); 
      cyverts[idx].location[2] = j*1.0/(float)(nstacks-1); 
      cyverts[idx].normal[2] = 0.0; 
      cyverts[idx].location[3] = 1.0;  cyverts[idx].normal[3] = 0.0; 
      if (i %2 ==0) {cyverts[idx].color[0] = r; cyverts[idx].color[1] = g; cyverts[idx].color[2] = b; }
      else {cyverts[idx].color[0] = 0.0; cyverts[idx].color[1] = 0.0; cyverts[idx].color[2] = .8; } 
      cyverts[idx].color[3] = 1.0; 
    }
  // now create the index array 

  nindices = (nstacks-1)*2*(nslices+1); 
  cindices = new GLuint[nindices]; 
  int n = 0; 
  for (int j =0; j<nstacks-1; j++)
    for (int i=0; i<=nslices; i++) {
      int mi = i % nslices;  
      int idx = j*nslices + mi; // mesh[j][mi] 
      int idx2 = (j+1) * nslices + mi; 
      cindices[n++] = idx; 
      cindices[n++] = idx2; 
    }
}

//////////////////////////////////////////////////////////////////

void InitCylinder_VBO(int nslices, int nstacks) 
{

  InitCylinder(nslices, nstacks, 1.0, 1.0, 0.0);
  
  int nvertices = nslices * nstacks; 
  nindices = (nstacks-1)*2*(nslices+1); 

  glGenBuffers(1, &vboHandle[1]);   // create an interleaved VBO object
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]);   // bind the first handle 

  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*nvertices, cyverts, GL_STATIC_DRAW); // allocate space and copy the position data over
  glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 

  glGenBuffers(1, &indexVBO[1]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*nindices, cindices, GL_STATIC_DRAW);  // load the index data 

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 

  // by now, we moved the position and color data over to the graphics card. There will be no redundant data copy at drawing time 
}

//////////////////////////////////////////////////////////////////////////////////
//
// create VBO objects and send the triangle vertices/colors to the graphics card
// 
void InitCube_VBO() 
{
  glGenBuffers(1, &vboHandle[0]);   // create an interleaved VBO object
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);   // bind the first handle 

  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*64, vertices, GL_STATIC_DRAW); // allocate space and copy the position data over
  glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 

  tindices[0] = 0;   tindices[1] = 1;   tindices[2] = 2; 
  tindices[3] = 0;   tindices[4] = 2;   tindices[5] = 3;
  
  tindices[6] = 4;   tindices[7] = 5;   tindices[8] = 6; 
  tindices[9] = 4;   tindices[10] = 6;   tindices[11] = 7; 
					   
  tindices[12] = 0;   tindices[13] = 3;   tindices[14] = 7; 
  tindices[15] = 0;   tindices[16] = 7;   tindices[17] = 4; 
					    
  tindices[18] = 1;   tindices[19] = 2;   tindices[20] = 6; 
  tindices[21] = 1;   tindices[22] = 6;   tindices[23] = 5;

  tindices[24] = 0;   tindices[25] = 1;   tindices[26] = 5; 
  tindices[27] = 0;   tindices[28] = 5;   tindices[29] = 4;

  tindices[30] = 3;   tindices[31] = 2;   tindices[32] = 6; 
  tindices[33] = 3;   tindices[34] = 6;   tindices[35] = 7; 

  glGenBuffers(1, &indexVBO[0]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[0]); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*36, tindices, GL_STATIC_DRAW);  // load the index data 

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 

} 

//////////////////////////////////////////////////////////////////////////////////////////

void draw_cube(float* m, float color[3], GLuint c0,GLuint c1, GLuint m1 ) {

  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[0]);

  glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, 32,(char*) NULL+0);
  glVertexAttribPointer(c1,4,GL_FLOAT, GL_FALSE, 32,(char*) NULL+16); 

  glUniformMatrix4fv(m1, 1, GL_FALSE, m); 

  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (char*)NULL+0); 
}

//void draw_cylinder(float* local2clip, float* local2eye, float color[3], GLuint c0, GLuint c1, GLuint c2, GLuint m1, GLuint m2, GLuint m3) {
void draw_cylinder(glm::mat4 local2clip, glm::mat4 local2eye, float color[3], GLuint c0, GLuint c1, GLuint c2, GLuint m1, GLuint m2, GLuint m3) {
    
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]);

  glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, sizeof(Vertex),(char*) NULL+0);  // position 
  glVertexAttribPointer(c1,4,GL_FLOAT, GL_FALSE, sizeof(Vertex),(char*) NULL+32); // color
  glVertexAttribPointer(c2,4,GL_FLOAT, GL_FALSE, sizeof(Vertex),(char*) NULL+16); // normal

  glm::mat4 normal_matrix = glm::inverse(local2eye);
  normal_matrix = glm::transpose(normal_matrix);

  glUniformMatrix4fv(m1, 1, GL_FALSE, (float*) &local2clip[0][0]);   // pass the local2clip matrix
  glUniformMatrix4fv(m2, 1, GL_FALSE, (float*) &local2eye[0][0]);   // pass the local2eye matrix
  glUniformMatrix4fv(m3, 1, GL_FALSE, (float*) &normal_matrix[0][0]);   // pass the local2eye matrix 

  glDrawElements(GL_TRIANGLE_STRIP, nindices, GL_UNSIGNED_INT, (char*) NULL+0); 

}


//////////////////////////////////////////////////////////////

#define SetMaterialColor(d, r, g, b, a)  glUniform4f(d, r, g, b, a); 

/////////////////////////////////////////////////////////////
void display() 
{ 
  glClearColor(0,0,0,1); 
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  if (WIRE_FRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  glUseProgram(programObject);

  GLuint c0 = glGetAttribLocation(programObject, "position");
  GLuint c1 = glGetAttribLocation(programObject, "color");
  GLuint c2 = glGetAttribLocation(programObject, "normal");
  GLuint m1 = glGetUniformLocation(programObject, "local2clip");
  GLuint m2 = glGetUniformLocation(programObject, "local2eye");
  GLuint m3 = glGetUniformLocation(programObject, "normal_matrix");

  GLuint Ia = glGetUniformLocation(programObject, "light_ambient");
  GLuint Id = glGetUniformLocation(programObject, "light_diffuse");
  GLuint Is = glGetUniformLocation(programObject, "light_specular");
  GLuint Lpos = glGetUniformLocation(programObject, "light_pos");

  GLuint Ka = glGetUniformLocation(programObject, "mat_ambient");
  GLuint Kd = glGetUniformLocation(programObject, "mat_diffuse");
  GLuint Ks = glGetUniformLocation(programObject, "mat_specular");
  GLuint Shine = glGetUniformLocation(programObject, "mat_shine"); 

  glUniform4f(Ia, light_ambient[0], light_ambient[1], light_ambient[2], light_ambient[3]);
  glUniform4f(Id, light_diffuse[0], light_diffuse[1], light_diffuse[2], light_diffuse[3]);
  glUniform4f(Is, light_specular[0], light_specular[1], light_specular[2], light_specular[3]);
  glUniform4f(Lpos, light_pos[0], light_pos[1], light_pos[2], light_pos[3]);

  glUniform4f(Ka, mat_ambient[0], mat_ambient[1], mat_ambient[2], mat_ambient[3]);
  glUniform4f(Kd, mat_diffuse[0], mat_diffuse[1], mat_diffuse[2], mat_diffuse[3]);
  glUniform4f(Ks, mat_specular[0], mat_specular[1], mat_specular[2], mat_specular[3]);
  glUniform1f(Shine, mat_shine[0]); 
  
  glEnableVertexAttribArray(c0);
  glEnableVertexAttribArray(c1);
  glEnableVertexAttribArray(c2);

  // define/get the viewing matrix 
  glm::mat4 view = glm::lookAt(glm::vec3(5, 5, .2), 
                               glm::vec3(0.0, 0.0, 0.0), 
                               glm::vec3(0.0, 0.0, 1.0));
  
  // define/get the projection matrix 
  glm::mat4 projection = glm::perspective(60.0f,1.0f,.01f,100.0f); 

  // define the modeling matrix 
  glm::mat4 model = glm::mat4(1.0f); 
  model = glm::rotate(model, z_angle, glm::vec3(0.0f, 0.0f, 1.0f)); 
  model = glm::rotate(model, x_angle, glm::vec3(1.0f, 0.0f, 0.0f)); 
  model = glm::scale(model, glm::vec3(scale_size, scale_size, scale_size));

  float color[3];

  glm::mat4 mvp; // model-view-projection matrix
  glm::mat4 mv;  // model-view matrix 

  mvp = projection*view*model;
  mv = view * model;
  
  color[0] = 1.0;   color[1] = 1.0;   color[2] = 0.0;
  SetMaterialColor(Kd, 0.7, 0, 0.7, 1); 
  //  draw_cylinder(&mvp[0][0], &mv[0][0], color, c0, c1, c2, m1, m2, m3);
  draw_cylinder(mvp, mv, color, c0, c1, c2, m1, m2, m3); 
  

  ///////////////////////////////
  //      draw a green floor
  //////////////////////////////
  color[0] = 0.2; color[1] = 0.4; color[2] = 0.2; 
  glm::mat4 floorM = glm::translate(model, glm::vec3(0.0, 0.0, -.8)); 
  floorM = glm::scale(floorM, glm::vec3(15.0, 15.0, 0.1));

  mvp = projection*view*floorM;

  SetMaterialColor(Kd, 0, 0.7, 0, 1); 
  draw_cube(&mvp[0][0], color, c0, c1, m1);
  
    
  //////////////////////////////
  //    draw robot
  //////////////////////////////
  
  color[0] = 0; color[1] = 0; color[2] = 1;

  mat_stack.push(modelM);
  modelM = glm::scale(modelM, glm::vec3(4.0f, 1.0f, 1.0f));
  mvp = projection*view*model*modelM;
  mv = view*model*modelM;
  
  //draw_cube(&mvp, color, c0);
  SetMaterialColor(Kd, 0.7, 0, 0, 1); 
  //  draw_cylinder(&mvp[0][0], &mv[0][0], color, c0, c1, c2, m1, m2, m3);
  draw_cylinder(mvp, mv, color, c0, c1, c2, m1, m2, m3); 
  
  modelM = mat_stack.top();  mat_stack.pop();

  mat_stack.push(modelM);

  color[0] = 1; color[1] = 0; color[2] = 0; 
  modelM = glm::translate(modelM, glm::vec3(1.0f, 0.0f, 0.0f));
  modelM = glm::rotate(modelM, angle1, glm::vec3(0.0f, 0.0f, 1.0f));
  modelM = glm::translate(modelM, glm::vec3(2.0f, 0.0f, 0.0f));  
  
  mat_stack.push(modelM);  
  modelM = glm::scale(modelM, glm::vec3(2.0f, 1.0f, 1.0f));
  mvp = projection*view*model*modelM;
  mv = view*model*modelM;
  
  //  draw_cube(&mvp, color, c0);
  SetMaterialColor(Kd, 0, 0, 0.7, 1); 
  //  draw_cylinder(&mvp[0][0], &mv[0][0], color, c0, c1, c2, m1, m2, m3);
  draw_cylinder(mvp, mv, color, c0, c1, c2, m1, m2, m3); 
  
  modelM = mat_stack.top();  mat_stack.pop(); 

  color[0] = 0; color[1] = 1; color[2] = 0; 
  modelM = glm::translate(modelM, glm::vec3(1.0f, 0.0f, 0.0f));
  modelM = glm::rotate(modelM, angle2, glm::vec3(0.0f, 0.0f, 1.0f));
  modelM = glm::translate(modelM, glm::vec3(1.0f, 0.0f, 0.0f));  
  
  modelM = glm::scale(modelM, glm::vec3(2.0f, 1.0f, 1.0f));
  mvp = projection*view*model*modelM;
  mv = view*model*modelM;
  
  //  draw_cube(&mvp, color, c0);
  SetMaterialColor(Kd, 0.7, 0.7, 0.7, 1);
  //draw_cylinder(&mvp[0][0], &mv[0][0], color, c0, c1, c2, m1, m2, m3);
  draw_cylinder(mvp, mv, color, c0, c1, c2, m1, m2, m3); 
  
  modelM = mat_stack.top();  mat_stack.pop();

  glDisableClientState(GL_VERTEX_ARRAY); 

  glutSwapBuffers(); 

} 

///////////////////////////////////////////////////////////////

void mymotion(int x, int y)
{
    if (xform_mode==XFORM_ROTATE) {
      z_angle += (x - press_x)/5.0; 
      if (z_angle > 180) z_angle -= 360; 
      else if (z_angle <-180) z_angle += 360; 
      press_x = x; 
           
      x_angle -= (y - press_y)/5.0; 
      if (x_angle > 180) x_angle -= 360; 
      else if (x_angle <-180) x_angle += 360; 
      press_y = y; 
    }
        else if (xform_mode == XFORM_SCALE){
      float old_size = scale_size;
      scale_size *= (1+ (y - press_y)/60.0); 
      if (scale_size <0) scale_size = old_size; 
      press_y = y; 
    }
    glutPostRedisplay(); 
}
void mymouse(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {
    press_x = x; press_y = y; 
    if (button == GLUT_LEFT_BUTTON)
      xform_mode = XFORM_ROTATE; 
         else if (button == GLUT_RIGHT_BUTTON) 
      xform_mode = XFORM_SCALE; 
  }
  else if (state == GLUT_UP) {
          xform_mode = XFORM_NONE; 
  }
}



///////////////////////////////////////////////////////////////

void mykey(unsigned char key, int x, int y)
{
        float d_angle = 10; 
	if (key == 'q') exit(1); 
	if (key == 'R') 
	  modelM = glm::rotate(modelM, d_angle, glm::vec3(0.0f, 0.0f, 1.0f)); 
	//  modelM =  rotatez(modelM, d_angle);
	if (key == 'r') 
	  modelM = glm::translate(modelM, glm::vec3(0.1f, 0.0f, 0.0f)); 
	//  modelM =  translate(modelM, .1,0,0);
	if (key == 'l') 
	  modelM = glm::translate(modelM, glm::vec3(-0.1f, 0.0f, 0.0f)); 
	//modelM =  translate(modelM, -.1,0,0);
	if (key == 'f') 
	  modelM = glm::translate(modelM, glm::vec3(0.0f, 0.1f, 0.0f)); 
	//modelM =  translate(modelM, 0,.1,0);
	if (key == 'b') 
	  modelM = glm::translate(modelM, glm::vec3(0.0f, -0.1f, 0.0f)); 
	//modelM =  translate(modelM, 0,-.1,0);
	if (key == 'c') {
	  modelM =  glm::mat4(1.0f);
	  angle1 = angle2 = angle3 = angle4 = 0; 
	}
	if (key == 's') {
	  WIRE_FRAME = !WIRE_FRAME;
	}

	if (key == '1') {
          angle1 += 5; 
          printf(" hello!\n"); 
        }
        if (key == '2') 
          angle2 += 5;

	
	glutPostRedisplay(); 
}

int main(int argc, char** argv) 
{ 

  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH); 
  glutInitWindowSize(600,600); 

  glutCreateWindow("fixed function pipeline: simple"); 
  glutDisplayFunc(display); 
  glutMouseFunc(mymouse); 
  glutKeyboardFunc(mykey);
  glutMotionFunc(mymotion);

  glewInit(); 

  InitCube_VBO();

  InitCylinder_VBO(10,10);

  programObject = SetupGLSL("robot7");  //create shaders - assume the shaders are do_nothing.vert and do_nothing.frag

  glutMainLoop(); 

} 
