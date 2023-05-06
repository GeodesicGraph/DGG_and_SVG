#include "stdafx.h"
#include "Shlwapi.h"
#include <random>
#include "wxn/wxn_dijstra.h"
#include "arcball.h"
#include "ICH/BaseModel.h"
#include "ICH/RichModel.h"
#include "ich/ICHWithFurtherPriorityQueue.h"
#include "wxn/wxnTime.h"
#include "wxn/wxnMath.h"
#include "RgbImage.h"
#include "wxn/wxnMath.h"
#include "ConfigLib/configfile.h"
#include "ConfigLib/configitem.h"
#include "RMS_LIB/MeshObject.h"
#include "RMS_LIB/Frame.h"
#include "RMS_LIB/ExtendedWmlCamera.h"
#define TW_STATIC
#include "AntTweakBar/AntTweakBar.h"

using namespace configlib;

CPoint3D hit_point(0,0,0);
int g_hit_vertex_id = -1;
vector<int> g_hit_vertices(1);
MeshObject g_mesh;
CRichModel g_model("");
CRichModel* g_model_ptr = NULL;
SparseGraph<float>* g_svg_graph = NULL;
enum GRAPH_TYPE{
	GRAPH_DGG, GRAPH_SVG
};
int g_graph_type = GRAPH_SVG;//0 for svg , 1 for dgg
double g_eps_vg=0.01;
int g_svg_k = 50;
string g_exe_dir = "";

enum DISTANCE_FIELD_SELECT_POINT_STATUS{
  DISTANCE_FIELD_UNINITIAL, DISTANCE_FIELD_COMPUTED, DISTANCE_FIELD_CHANGED
} g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
enum MouseClickMode{
  NONE,
  SELECT_POINT,
  ADD_POINT
};
MouseClickMode mouse_click_mode(NONE);
enum MethodMode{
  METHOD_DISTANCE_FIELD
};
MethodMode g_method_mode(METHOD_DISTANCE_FIELD);

Wml::ExtendedWmlCamera * g_pCamera = NULL;
Wml::Vector2f g_vLastMousePos = Wml::Vector2f::ZERO;

float g_Zoom;


/* window width and height */
int win_width, win_height;
int gButton;
int startx,starty;
int shadeFlag      = 0;

//glui
int   main_window;
int obj_shade_mode = 0;
int projection_mode = 0;

bool file_just_import = false;

struct HitObject{
  double z1 , z2;
  int name;
  HitObject(){}
  HitObject(double _z1,double _z2,int _name):z1(_z1),z2(_z2),name(_name){}
};
HitObject hitFace(0,0,-1);
vector<float> distance_field;

GLuint texture;


void printButtonMessage(const string& message);
void printButtonMessage(const string& message1, const string& message2);
string ExePath();
string toRelativePath(const string& input_filename);
string toFullPath(const string& input_filename);


void drawText(const char *text, int length, int x, int y){
  glMatrixMode(GL_PROJECTION); // change the current matrix to PROJECTION
  double matrix[16]; // 16 doubles in stack memory
  glGetDoublev(GL_PROJECTION_MATRIX, matrix); // get the values from PROJECTION matrix to local variable
  glLoadIdentity(); // reset PROJECTION matrix to identity matrix
  glOrtho(0, 800, 0, 600, -5, 5); // orthographic perspective
  glMatrixMode(GL_MODELVIEW); // change current matrix to MODELVIEW matrix again
  glLoadIdentity(); // reset it to identity matrix
  glPushMatrix(); // push current state of MODELVIEW matrix to stack
  glLoadIdentity(); // reset it again. (may not be required, but it my convention)
  glRasterPos2i(x, y); // raster position in 2D
  for(int i=0; i<length; i++){
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)text[i]); // generation of characters in our text with 9 by 15 GLU font
  }
  glPopMatrix(); // get MODELVIEW matrix value from stack
  glMatrixMode(GL_PROJECTION); // change current matrix mode to PROJECTION
  glLoadMatrixd(matrix); // reset
  glMatrixMode(GL_MODELVIEW); // change current matrix mode to MODELVIEW
}

void display()
{
  const vector<float>* distance_field = NULL;
  const vector<CPoint3D>* sampled_points = NULL;
  vector<float>* sampled_distance_field = NULL;
  vector<CPoint3D>* tangent_field_ptr = NULL;

  if (g_method_mode == METHOD_DISTANCE_FIELD) {
    if (g_distance_field_status == DISTANCE_FIELD_CHANGED) {
      if (mouse_click_mode == SELECT_POINT) {
        if (g_svg_graph != NULL) {
          ElapasedTime t;
          g_svg_graph->findShortestDistance(g_hit_vertex_id, true);
          distance_field = g_svg_graph->distanceField();    
          char buf_message1[1024];
          sprintf(buf_message1,"Select vertex %d" , g_hit_vertex_id);
          char buf_message2[1024];
          sprintf(buf_message2,"%lf seconds for geodesic distance field ", t.getTime());
          printButtonMessage(buf_message1,buf_message2);
          //t.printTime("t");
          //printf("distance_field computed");
          g_distance_field_status = DISTANCE_FIELD_COMPUTED;
        }
      } else if (mouse_click_mode == ADD_POINT) {
        if (g_svg_graph != NULL) {
          ElapasedTime t;
          g_svg_graph->findShortestDistance(g_hit_vertices, true);
          distance_field = g_svg_graph->distanceField();                       
          g_distance_field_status = DISTANCE_FIELD_COMPUTED;
          char buf_message1[1024];
          sprintf(buf_message1,"Add vertex %d" , g_hit_vertices.back());
          char buf_message2[1024];
          sprintf(buf_message2,"%lf seconds for geodesic distance field ", t.getTime());
          printButtonMessage(buf_message1,buf_message2);
        }
      }
    } else if (g_distance_field_status == DISTANCE_FIELD_COMPUTED) {
      //if (g_hit_vertex_id >= 0) {
      if (g_svg_graph != NULL) {
        distance_field = g_svg_graph->distanceField();
      }
      //}
    }
  } 
  /* clear frame buffer */
  //glClearColor(30.0/255.0,144.0/255.0,255.0/255.0, 0.0f); 
  glClearColor(220.0/255.0,234.0/255.0, 248.0/255.0,1.0f);
  //glClearColor(234.0 / 255.0 , 234.0 / 255.0 , 234.0 / 255.0 , 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //setupLight();

  //setupEye();
  /* transform from the eye coordinate system to the world system */
  //glPushMatrix();
  /* transform from the world to the ojbect coordinate system */
  //setupObject();
  /* draw the mesh */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  g_pCamera->Update();

  if (g_model_ptr != NULL) {
    color_white.SetColor();
    if (obj_shade_mode == 0) {
      if (g_method_mode ==METHOD_DISTANCE_FIELD) {
        if (g_distance_field_status == DISTANCE_FIELD_COMPUTED) {
          glEnable(GL_TEXTURE_2D);
          g_model_ptr->Render(GL_RENDER, distance_field);
          glDisable(GL_TEXTURE_2D);
        } else {
          g_model_ptr->Render(GL_RENDER);
        }
      } 
    }else if(obj_shade_mode == 1) {
      glShadeModel(GL_FLAT);
      g_model_ptr->Render(GL_RENDER);
    }else if(obj_shade_mode == 2) {
      glShadeModel(GL_SMOOTH);
      g_model_ptr->Render(GL_RENDER);
      g_model_ptr->DrawWireframe();
    }else if(obj_shade_mode == 3) {
      g_model_ptr->DrawPoint();
    }
  }

  TwDraw();

  glutSwapBuffers();
}

void reshape(int w, int h)
{
  // set viewport and frustum, and update camera
  g_pCamera->SetViewPort(0, 0, w, h);
  g_pCamera->SetFrustum( 40.0f, float(w)/float(h), 0.01f, 100.0);
  g_pCamera->Update();
  // Send the new window size to AntTweakBar
  TwWindowSize(w, h);
}

void setupGLstate(){

  g_pCamera = new Wml::ExtendedWmlCamera(1.0f,1.0f);
  g_pCamera->SetTargetFrame( Wml::Vector3f(0.0, 0.0, 4.0f), 
    Wml::Vector3f(-1.0f, 0.0, 0.0),
    Wml::Vector3f(0.0, 1.0f, 0.0), 
    Wml::Vector3f(0.0, 0.0, 0.0));


  GLfloat lightOneColor[] = {0.6, 0.6, 0.6, 1};
  GLfloat globalAmb[] = {.1, .1, .1, 1};
  //GLfloat lightOnePosition[] = {.0,  .2, 1, 0.0};
  GLfloat lightOnePosition[] = {.0,  .0, 1, 0.0};

  glEnable(GL_CULL_FACE);
  //glFrontFace(GL_CCW);      
  glEnable(GL_DEPTH_TEST);
  //glClearColor(0,0,0,0);
  glShadeModel(GL_SMOOTH);


  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);

  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
  //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);

  glEnable (GL_LINE_SMOOTH);
  glHint (GL_LINE_SMOOTH, GL_NICEST);




}

bool find_hit(float x , float y , rms::Frame3f& v_hit_frame)
{
  // find hit point
  const unsigned int * pViewport = g_pCamera->GetViewport();
  Wml::Ray3f ray;
  g_pCamera->GetPickRay( x, y, 
    pViewport[2], pViewport[3], ray.Origin, ray.Direction );
  return g_mesh.FindHitFrame(ray , v_hit_frame);
}

bool find_hit(float x , float y , rms::Frame3f& v_hit_frame, int& hit_tri_id)
{
  // find hit point
  const unsigned int * pViewport = g_pCamera->GetViewport();
  Wml::Ray3f ray;
  g_pCamera->GetPickRay( x, y, 
    pViewport[2], pViewport[3], ray.Origin, ray.Direction );
  return g_mesh.FindHitFrame(ray , v_hit_frame, hit_tri_id);
}

void  mouseClick(int button , int state, int x, int y) {

  if (!TwEventMouseButtonGLUT(button , state , x , y)) {

    rms::Frame3f v_hit_frame;
    int hit_tri_id = -1;
    bool bHit = find_hit(x, y, v_hit_frame, hit_tri_id);

    hit_point = CPoint3D(v_hit_frame.Origin().X(),
      v_hit_frame.Origin().Y(),
      v_hit_frame.Origin().Z());
    g_vLastMousePos = Wml::Vector2f(x,y);

    /* set up an arcball around the Eye's center
    switch y coordinates to right handed system  */

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN  ) 
    { 
      gButton = GLUT_LEFT_BUTTON;  
      if( mouse_click_mode == SELECT_POINT) {
        //printf("select\n");
        if (hit_tri_id >= 0) {
          //printf("hit_tri_id %d\n" , hit_tri_id);
          int pos = 0;
          double min_dis = 1e10;
          for (int j = 0; j < 3; ++j) {
            double dis = (g_model_ptr->Vert(g_model_ptr->Face(hit_tri_id)[j]) - hit_point).Len();
            if (dis <min_dis){
              min_dis = dis;
              pos = j;
            }
          }
          g_hit_vertex_id = g_model_ptr->Face(hit_tri_id)[pos];
          g_hit_vertices[0] = g_hit_vertex_id;
          if (g_method_mode == METHOD_DISTANCE_FIELD) {
            g_distance_field_status = DISTANCE_FIELD_CHANGED;
          }
          glutPostRedisplay();
        }
      } else if(mouse_click_mode == ADD_POINT) {
        if (hit_tri_id >= 0) {
          int pos = 0;
          double min_dis = 1e10;
          for (int j = 0; j < 3; ++j) {
            double dis = (g_model_ptr->Vert(g_model_ptr->Face(hit_tri_id)[j]) - hit_point).Len();
            if (dis <min_dis){
              min_dis = dis;
              pos = j;
            }
          }
          int vert_id = g_model_ptr->Face(hit_tri_id)[pos];
          g_hit_vertices.push_back(vert_id);
          if (g_method_mode == METHOD_DISTANCE_FIELD) {
            g_distance_field_status = DISTANCE_FIELD_CHANGED;
          }
          glutPostRedisplay();
        }
      }


    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {

    }


    if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
      startx = x;
      starty = y;
      gButton = GLUT_MIDDLE_BUTTON;
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
      startx = x;
      starty = y;
      gButton = GLUT_RIGHT_BUTTON;
    }
  }
  return ;
}

void mouseMove(int x, int y)
{
  if( !TwEventMouseMotionGLUT(x, y) )  // send event to AntTweakBar
  { // event has not been handled by AntTweakBar
    // your code here to handle the event
    // ...

    static const float fPanScale = 0.01f;
    static const float fZoomScale = 0.01f;
    static const float fOrbitScale = 0.01f;
    static const float fExpMapScale = 0.005f;

    if( file_just_import ){
      file_just_import = false;
      return;
    }
    rms::Frame3f vLastHitFrame;
    bool bLastHit = find_hit(g_vLastMousePos.X(), g_vLastMousePos.Y(), vLastHitFrame);
    CPoint3D last_hit_point = CPoint3D(vLastHitFrame.Origin().X(),
      vLastHitFrame.Origin().Y(),
      vLastHitFrame.Origin().Z());
    Wml::Vector2f vCur(x,y);
    Wml::Vector2f vDelta(vCur - g_vLastMousePos);
    g_vLastMousePos = vCur;
    rms::Frame3f vHitFrame;
    int hit_tri_id = -1;
    bool bHit = find_hit(x, y, vHitFrame, hit_tri_id);

    hit_point = CPoint3D(vHitFrame.Origin().X(),
      vHitFrame.Origin().Y(),
      vHitFrame.Origin().Z());

    /* rotation, call arcball */
    if (gButton == GLUT_LEFT_BUTTON ) 
    {
      if(mouse_click_mode == NONE){
        g_pCamera->OrbitLateral(-vDelta.X() * fOrbitScale);
        g_pCamera->OrbitVertical(vDelta.Y() * fOrbitScale);
        glutPostRedisplay();
      }else if ( mouse_click_mode == SELECT_POINT) {
        if (mouse_click_mode == SELECT_POINT) {
          if (hit_tri_id >= 0) {
            int pos = 0;
            double min_dis = 1e10;
            for (int j = 0; j < 3; ++j) {
              double dis = (g_model_ptr->Vert(g_model_ptr->Face(hit_tri_id)[j]) - hit_point).Len();
              if (dis < min_dis){
                min_dis = dis;
                pos = j;
              }
            }
            g_hit_vertex_id = g_model_ptr->Face(hit_tri_id)[pos];
            g_hit_vertices[0] = g_hit_vertex_id;
            if (g_method_mode == METHOD_DISTANCE_FIELD) {
              g_distance_field_status = DISTANCE_FIELD_CHANGED;
            } 
            glutPostRedisplay();
          }
        } 
      }
    }

    /*xy translation */
    if (gButton == GLUT_MIDDLE_BUTTON) 
    {
      g_pCamera->PanLateral(vDelta.X() * fPanScale);
      g_pCamera->PanVertical(vDelta.Y() * fPanScale);
      glutPostRedisplay();
    }

    /* zoom in and out */
    if (gButton == GLUT_RIGHT_BUTTON ) {
      g_pCamera->DollyZoom((-vDelta.Y() + vDelta.X()) * fZoomScale);
      glutPostRedisplay();
    }
  }

}

void keyBoard(unsigned char key, int x, int y) 
{
  if( !TwEventKeyboardGLUT(key , x , y)){

    switch( key )
    {
    case 'p':
      mouse_click_mode = SELECT_POINT;
      break;
    case 'n':
      mouse_click_mode = NONE;
      break;
    case 'd':
      {
        string dijstra_dis_file_name = "_dij_dis.txt";
        FILE* dijstra_dis_file = fopen(dijstra_dis_file_name.c_str() , "w");
        if (dijstra_dis_file == NULL) {
          printf("error, %s cannot open!\n");
          exit(1);
        }
        for (int i = 0; i < g_svg_graph->node_number_;++i) {
          fprintf(dijstra_dis_file, "%lf\n" , g_svg_graph->distanceToSource(i));
        }
        fclose(dijstra_dis_file);
      }
      break;
    case 27:
      exit(0);
      break;
    }
    glutPostRedisplay();  
  }
}

void glutIdle(void)
{


  /* According to the GLUT specification, the current window is 
  undefined during an idle callback.  So we need to explicitly change
  it if necessary */
  //if ( glutGetWindow() != main_window ) 
  //    glutSetWindow(main_window);  

  /*  GLUI_Master.sync_live_all();  -- not needed - nothing to sync in this
  application  */

  glutPostRedisplay();
}

void printLinesToObj(const vector<vector<CPoint3D>>& paths , const string& file_name)
{
  FILE* path_file = fopen(file_name.c_str() , "w");
  int cnt_v = 1;
  for (int i = 0; i < paths.size(); ++i) {
    fprintf(path_file , "g %s_%d\n" , file_name.substr(0,file_name.length() - 4).c_str() , i);
    for (int j = 0; j < paths[i].size(); ++j) {
      fprintf(path_file , "v %lf %lf %lf\n" , paths[i][j].x , paths[i][j].y , paths[i][j].z);
    }
    int j = 0;
    fprintf(path_file , "l %d %d\n" , cnt_v + j , cnt_v + j + 1);
    cnt_v += paths[i].size();
  }
  fclose(path_file);
}

void terminatexx(void)
{

  TwTerminate();
}

int fileExists(const TCHAR * file)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE handle = FindFirstFile(file, &FindFileData) ;
  int found = handle != INVALID_HANDLE_VALUE;
  if(found) 
  {
    //FindClose(&handle); this will crash
    FindClose(handle);
  }
  return found;
}


void readTexture(string filename)
{
  RgbImage theTexMap( filename.c_str() );

  // Pixel alignment: each row is word aligned (aligned to a 4 byte boundary)
  //    Therefore, no need to call glPixelStore( GL_UNPACK_ALIGNMENT, ... );

  // allocate a texture name
  glGenTextures( 1, &texture );
  glBindTexture( GL_TEXTURE_2D, texture );
  // select modulate to mix texture with color for shading
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  // when texture area is small, bilinear filter the closest mipmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    GL_LINEAR_MIPMAP_NEAREST );
  // when texture area is large, bilinear filter the first mipmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  // if wrap is true, the texture wraps over at the edges (repeat)
  //       ... false, the texture ends at the edges (clamp)
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
    GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
    GL_CLAMP );
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3,theTexMap.GetNumCols(), theTexMap.GetNumRows(),
    GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData() );

  //gluBuild2DMipmaps(GL_TEXTURE_2D,3,ImageWidth,ImageHeight,GL_RGB,GL_UNSIGNED_BYTE,&Image[0][0][0]);
  //glEnable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_2D);

}

void printButtonMessage(const string& message) { 
  string log_message1 = (string)"Panel/buttonMessage1  label = '" + message + (string)"'";
  TwDefine(log_message1.c_str());
  string log_message2 = (string)"Panel/buttonMessage2  label = '" + (string)" " + (string)"'";
  TwDefine(log_message2.c_str());
}

void printButtonMessage(const string& message1,const string& message2) { 

  string log_message1 = "Panel/buttonMessage1  label = '" + message1 + "'";
  TwDefine(log_message1.c_str());

  string log_message2 = "Panel/buttonMessage2  label = '" + message2 + "'";
  TwDefine(log_message2.c_str());
}


//void TW_CALL CB_ComputeError(void* clientData)
//{
//  string input_obj_name = g_model_ptr->GetFullPathAndFileName();
//  if (g_hit_vertex_id < 0)
//    return;
//  if (g_model_ptr == NULL) 
//    return;
//  if (g_svg_graph == NULL) 
//    return;
//  if (g_distance_field_status != DISTANCE_FIELD_COMPUTED)
//    return;
//  int source_vert = g_hit_vertex_id;
//
//
//
//  
//    vector<int> sources;
//    if (mouse_click_mode == SELECT_POINT) {
//      sources.push_back(source_vert);
//    }else{
//      sources = g_hit_vertices;
//    }
//    CICHWithFurtherPriorityQueue ich(*g_model_ptr,sources);
//    set<int> dest;
//    ich.ExecuteLocally_Dis(1e5,dest);
//
//
//    vector<double> correct_dis(g_svg_graph->NodeNum());
//    for (int i = 0; i < g_svg_graph->NodeNum(); ++i) {
//      correct_dis[i] = ich.m_InfoAtVertices[i].disUptodate;
//    }
//
//
//    int cnt_error_dis = 0;
//    double ave_error = 0;
//    for (int i = 0; i < correct_dis.size(); ++i) {
//      if (fabs(correct_dis[i]) < 1e-6 ) continue;
//      double error = fabs(g_svg_graph->distanceToSource(i) * g_svg_graph->maxDis() - correct_dis[i])/correct_dis[i];
//      ave_error += error;
//    }
//    ave_error /= correct_dis.size();
//
//  
//  printf("ave_error is %lf\n" , ave_error);
//
//
//
//  char buf_message[1024];
//  sprintf(buf_message,"Mean Err : %.3lf%%",ave_error*100.0);
//  printButtonMessage(buf_message);
//}

void TW_CALL CB_ComputeError(void* clientData)
{
  string input_obj_name = g_model_ptr->GetFullPathAndFileName();
  if (g_hit_vertex_id < 0)
    return;
  if (g_model_ptr == NULL) 
    return;
  if (g_svg_graph == NULL) 
    return;
  if (g_distance_field_status != DISTANCE_FIELD_COMPUTED)
    return;
  int source_vert = g_hit_vertex_id;



  
    vector<int> sources;
    if (mouse_click_mode == SELECT_POINT) {
      sources.push_back(source_vert);
    }else{
      sources = g_hit_vertices;
    }
    CICHWithFurtherPriorityQueue ich(*g_model_ptr,sources);
    set<int> dest;
    ich.ExecuteLocally_Dis(1e5,dest);


    vector<double> correct_dis(g_svg_graph->NodeNum());
    for (int i = 0; i < g_svg_graph->NodeNum(); ++i) {
      correct_dis[i] = ich.m_InfoAtVertices[i].disUptodate;
    }


    int cnt_error_dis = 0;
    double ave_error = 0;
    for (int i = 0; i < correct_dis.size(); ++i) {
      if (fabs(correct_dis[i]) < 1e-6 ) continue;
      double error = fabs(g_svg_graph->distanceToSource(i) * g_svg_graph->maxDis() - correct_dis[i])/correct_dis[i];
      ave_error += error;
    }
    ave_error /= correct_dis.size();

  
  printf("Relative mean error  %lf\n" , ave_error);



  char buf_message[1024];
  sprintf(buf_message,"Mean Err : %.3lf%%",ave_error*100.0);
  printButtonMessage(buf_message);
}

void TW_CALL CB_AverageError(void* clientData)
{
  string input_obj_name = g_model_ptr->GetFullPathAndFileName();
  if (g_model_ptr == NULL) 
    return;
  if (g_svg_graph == NULL) 
    return;
  //if (g_distance_field_status != DISTANCE_FIELD_COMPUTED)
  //  return;
  //int source_vert = g_hit_vertex_id;
  double total_ave = 0;
  for (int i = 0; i < 10; ++i) 
  {
    int source = rand() % g_model_ptr->GetNumOfVerts();
    g_svg_graph->findShortestDistance(source,true);
    vector<int> sources;
    sources.push_back(source);
    CICHWithFurtherPriorityQueue ich(*g_model_ptr,sources);
    set<int> dest;
    ich.ExecuteLocally_Dis(1e5,dest);

    vector<double> correct_dis(g_svg_graph->NodeNum());
    for (int i = 0; i < g_svg_graph->NodeNum(); ++i) {
      correct_dis[i] = ich.m_InfoAtVertices[i].disUptodate;
    }


    int cnt_error_dis = 0;
    double ave_error = 0;
    for (int i = 0; i < correct_dis.size(); ++i) {
      if (fabs(correct_dis[i]) < 1e-6 ) continue;
      double error = fabs(g_svg_graph->distanceToSource(i) * g_svg_graph->maxDis() - correct_dis[i])/correct_dis[i];
      ave_error += error;
    }
    ave_error /= correct_dis.size();
    total_ave += ave_error;
  }
  total_ave /= 10.0;
  printf("total average_error is %lf\n" , total_ave );



  char buf_message[1024];
  sprintf(buf_message,"Mean Err : %.3lf%%",total_ave*100.0);
  printButtonMessage(buf_message);
}

void TW_CALL CB_EdgeLen(void* clientData)
{
	if (g_model_ptr == NULL)
		return;
	if (g_svg_graph == NULL)
		return;
	//edge length hist
	string input_obj_name = g_model_ptr->GetFullPathAndFileName();
	string prefix = input_obj_name.substr(0, input_obj_name.length() - 4);
	string edge_len_filename;
	if (g_graph_type == GRAPH_DGG) {
		edge_len_filename = prefix + "_dgg_edge_len.txt";
	}
	else if (g_graph_type == GRAPH_SVG) {
		edge_len_filename = prefix + "_svg_edge_len.txt";
	}
	FILE* edge_len_file = fopen(edge_len_filename.c_str(), "w");
	//fprintf(edge_len_file, "edge_len = [");
	for (int i = 0; i < g_model_ptr->GetNumOfVerts(); ++i) {
		for (int j = 0; j < g_svg_graph->graph_neighbor_dis[i].size(); ++j) {
			fprintf(edge_len_file, "%lf\n", g_svg_graph->graph_neighbor_dis[i][j]);
		}
	}
	//fprintf(edge_len_file, "]\n");


	fclose(edge_len_file);

}

void TW_CALL CB_DegreeModel(void* clientData)
{
  if (g_model_ptr == NULL) 
    return;
  if (g_svg_graph == NULL) 
    return;
  string input_obj_name = g_model_ptr->GetFullPathAndFileName();
	string prefix = input_obj_name.substr(0, input_obj_name.length() - 4);

  double max_degree = 0;
  double ave_degree = 0;
	vector<pair<double,double>> degrees(g_svg_graph->graph_neighbor.size());

  for (int i = 0; i < g_svg_graph->graph_neighbor.size(); ++i) {
    double degree = (double)g_svg_graph->graph_neighbor[i].size();
		degrees[i] = make_pair(degree,degree);
		max_degree = max(degree, max_degree);
    ave_degree += degree;
  }
	printf("max_degree %lf\n" , max_degree);
  ave_degree /= g_svg_graph->graph_neighbor.size();

	double min_degree = 200;
	max_degree = 500;
	for (auto& d:degrees) {
		d.first = (d.first - min_degree) / (max_degree - min_degree);
		d.second = (d.second - min_degree) / (max_degree - min_degree);
	}
	string textured_model_filename;
	if (g_graph_type == GRAPH_DGG) {
		textured_model_filename = prefix + "_dgg_degrees.obj";
	} else if (g_graph_type == GRAPH_SVG) {
		textured_model_filename = prefix + "_svg_degrees.obj";
	}
	g_model_ptr->FastSaveObjFile(textured_model_filename, degrees);


	//degree_saddle
	string saddle_degree_hist_filename;
	if (g_graph_type == GRAPH_DGG) {
		saddle_degree_hist_filename = prefix + "_dgg_saddle_degrees.txt";
	} else if (g_graph_type == GRAPH_SVG) {
		saddle_degree_hist_filename = prefix + "_svg_saddle_degrees.txt";
	}
	FILE* saddle_degree_file = fopen(saddle_degree_hist_filename.c_str(), "w");
	//saddle degree
	//fprintf(degree_file, "saddle_degrees = [");
	for (int i = 0; i < g_model_ptr->GetNumOfVerts(); ++i) {
		if (g_model_ptr->AngleSum(i) > 2.0 * M_PI) {
			fprintf(saddle_degree_file, "%.2lf " , (double)g_svg_graph->graph_neighbor[i].size());
		}
	}
	fclose(saddle_degree_file);
	//fprintf(degree_file, "]\n");
	//non-saddle degree
	string non_saddle_degree_hist_filename;
	if (g_graph_type == GRAPH_DGG) {
		non_saddle_degree_hist_filename = prefix + "_dgg_non_saddle_degrees.txt";
	} else if (g_graph_type == GRAPH_SVG) {
		non_saddle_degree_hist_filename = prefix + "_svg_non_saddle_degrees.txt";
	}
	FILE* non_saddle_degree_file = fopen(non_saddle_degree_hist_filename.c_str(), "w");
	for (int i = 0; i < g_model_ptr->GetNumOfVerts(); ++i) {
		if (g_model_ptr->AngleSum(i) <= 2.0 * M_PI) {
			fprintf(non_saddle_degree_file, "%.2lf " , (double)g_svg_graph->graph_neighbor[i].size());
		}
	}
	//fprintf(degree_file, "]\n");
	fclose(non_saddle_degree_file);

	//edge length hist
	string edge_len_filename;
	if (g_graph_type == GRAPH_DGG) {
		edge_len_filename = prefix + "_dgg_edge_len.txt";
	} else if (g_graph_type == GRAPH_SVG) {
		edge_len_filename = prefix + "_svg_edge_len.txt";
	}
	FILE* edge_len_file = fopen(edge_len_filename.c_str(), "w");
	//fprintf(edge_len_file, "edge_len = [");
	for (int i = 0; i < g_model_ptr->GetNumOfVerts(); ++i) {
		for (int j = 0; j < g_svg_graph->graph_neighbor_dis[i].size(); ++j) {
			fprintf(edge_len_file, "%lf\n" , g_svg_graph->graph_neighbor_dis[i][j]);
		}
	}
	//fprintf(edge_len_file, "]\n");
	
	
	fclose(edge_len_file);




	//Degree

  //FILE* 
//	g_model_ptr->ReadFile

  
}




void DGG_initial(const string& dgg_file_name)
{
	if (g_svg_graph != NULL) {
		delete g_svg_graph;
		g_svg_graph = NULL;
	}
	g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
	g_graph_type = GRAPH_DGG;
	g_svg_graph = new LC_HY<float>();
	g_svg_graph->read_svg_file_with_angle(dgg_file_name);

}

void TW_CALL CB_DGGPreprocess(void* clientData)
{
  //string cmd = "c:\util\DGG_precompute.exe bunny_nf10k.obj 0.001 d 15
  if (g_model_ptr != NULL) {
    char buf[1024];
    double eps_vg = g_eps_vg;
    double theta = asin(sqrt(eps_vg)) / M_PI * 180;
    double const_for_theta = 30.0 / theta;
    //double const_for_theta = 5.0;
		string input_obj_name = g_model_ptr->GetFullPathAndFileName();
    sprintf(buf,"%s\\lib\\DGG_precompute.exe %s %lf p %.0lf", g_exe_dir.c_str(),  input_obj_name.c_str(), eps_vg, const_for_theta);
    printf("%s\n",buf);
    system(buf);
    sprintf(buf,"%s_DGG%lf_c%.0lf_pruning.binary", input_obj_name.substr(0,input_obj_name.length() - 4 ).c_str(), eps_vg, const_for_theta);

    string dgg_file_name = string(buf);
    //cout << dgg_file_name << "\n";

    string prefix = input_obj_name.substr(0, input_obj_name.length() - 4);
    string config_filename = prefix + "_DGG_" + to_string(eps_vg) + ".config";
    configfile config_file(config_filename.c_str());
    configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
    conf_obj_file_name = toRelativePath(input_obj_name);
    configitem<double> conf_eps_vg (config_file, "main", "eps_vg","s=",g_eps_vg);
    conf_eps_vg = g_eps_vg;
    configitem<std::string> conf_dgg_file_name    (config_file , "main" , "dgg_file_name"     , "s=","");
    conf_dgg_file_name = toRelativePath(dgg_file_name);
    configitem<std::string> conf_texture_distance_file_name    (config_file , "main" , "texture_distance_file_name"     , "t=","");

    conf_texture_distance_file_name = "data\\pic\\texture_distance_map.bmp";
    config_file.write();

		DGG_initial(dgg_file_name);

    char buf_message[1024];
    sprintf(buf_message,"DGG Preprocess completed");
    printButtonMessage(buf_message);

  }
}

void TW_CALL CB_OpenDGGConfigFile(void* clientData)
{
  OPENFILENAME ofn ;
  char szFile[MAX_PATH]  = "\0";
  ZeroMemory(&ofn , sizeof(ofn));
  ofn.lStructSize			= sizeof(ofn);
  //ofn.hwndOwner			=  FindWindow(NULL, "Mesh-Viewer");	//find main window to freeze it
  ofn.lpstrFile			= szFile ;
  //ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile			= sizeof(szFile);
  ofn.lpstrFilter			= "Model file (.config)\0*.config\0All\0*.*\0";
  ofn.nFilterIndex		= 1;
  ofn.lpstrFileTitle		= NULL ;
  ofn.nMaxFileTitle		= 0 ;
  string dir_name = ExePath();
  ofn.lpstrInitialDir = dir_name.c_str();
  ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  GetOpenFileName(&ofn);    
  LPSTR config_filename = ofn.lpstrFile;   

  if (config_filename && config_filename[0] != '\0')
  {
    ElapasedTime timer;
    timer.start();

    configfile config_file(config_filename);

    configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
    configitem<std::string> conf_dgg_file_name    (config_file , "main" , "dgg_file_name"     , "s=","");
    configitem<double> conf_eps_vg (config_file, "main", "eps_vg","s=",0.01);
    configitem<std::string> conf_texture_filename    (config_file , "main" , "texture_distance_file_name"     , "t=","");

    config_file.read();
    //cout << string(conf_obj_file_name) <<" __\n";
    string obj_filename = toFullPath(conf_obj_file_name);
    printf("obj_filename %s\n" , obj_filename.c_str());
    g_eps_vg = conf_eps_vg;
    string dgg_filename = toFullPath(conf_dgg_file_name);
    string texture_filename = toFullPath(conf_texture_filename);
    readTexture(texture_filename);

    CRichModel model(obj_filename);
    g_model = model;
    g_model_ptr = &g_model;
    g_model_ptr->Preprocess();
		g_mesh.ReadCRichModel(g_model);

		DGG_initial(dgg_filename);

    string log_message1 = (string)"Panel/buttonMessage1  label = '" + config_filename + (string)"loaded" + (string)"'";
    TwDefine(log_message1.c_str());

  }


}

void TW_CALL CB_SVGPreprocess(void* clientData)
{
  //string cmd = "c:\util\DGG_precompute.exe bunny_nf10k.obj 0.001 d 15
  if (g_model_ptr != NULL) {
    char buf[1024];
    string input_obj_name = g_model_ptr->GetFullPathAndFileName();
    string prefix = input_obj_name.substr(0,input_obj_name.length()-4);

    sprintf(buf,"%s\\lib\\DGG_precompute.exe %s %d n", g_exe_dir.c_str(), input_obj_name.c_str(), g_svg_k);
    printf("%s\n",buf);
    system(buf);

    string svg_file_name = prefix + "_svg_k" + to_string(g_svg_k) +  ".binary";
    //cout << svg_file_name << "\n";
    string config_filename = prefix + "_svg_k_" + to_string(g_svg_k) + ".config";

    configfile config_file(config_filename.c_str());
    configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
    conf_obj_file_name = toRelativePath(input_obj_name);
    configitem<int> conf_svg_k (config_file, "main", "svg_k","s=",50);
    conf_svg_k = g_svg_k;
    configitem<std::string> conf_svg_file_name    (config_file , "main" , "svg_file_name"     , "s=","");
    conf_svg_file_name = toRelativePath(svg_file_name);
    configitem<std::string> conf_texture_distance_file_name    (config_file , "main" , "texture_distance_file_name"     , "t=","");

    conf_texture_distance_file_name = "data\\pic\\texture_distance_map.bmp";
    config_file.write();

    if (g_svg_graph != NULL) {
      delete g_svg_graph;
      g_svg_graph = NULL;
    }
		g_graph_type = GRAPH_SVG;
    g_svg_graph = new LC_LLL<float>(); 
    g_svg_graph->read_svg_file_binary(svg_file_name);
    g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
    string log_message1 = (string)"Panel/buttonMessage1  label = '" + (string)"SVG Preprocess completed" + (string)"'";
    TwDefine(log_message1.c_str());

  }

}

void TW_CALL CB_OpenSVGConfigFile(void* clientData)
{
  OPENFILENAME ofn ;
  char szFile[MAX_PATH]  = "\0";
  ZeroMemory(&ofn , sizeof(ofn));
  ofn.lStructSize			= sizeof(ofn);
  //ofn.hwndOwner			=  FindWindow(NULL, "Mesh-Viewer");	//find main window to freeze it
  ofn.lpstrFile			= szFile ;
  //ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile			= sizeof(szFile);
  ofn.lpstrFilter			= "Model file (.config)\0*.config\0All\0*.*\0";
  ofn.nFilterIndex		= 1;
  ofn.lpstrFileTitle		= NULL ;
  ofn.nMaxFileTitle		= 0 ;
  string dir_name = ExePath();
  ofn.lpstrInitialDir = dir_name.c_str();
  ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  GetOpenFileName(&ofn);    
  LPSTR config_filename = ofn.lpstrFile;   

  if (config_filename && config_filename[0] != '\0')
  {
    ElapasedTime timer;
    timer.start();
    configfile config_file(config_filename);

    configitem<std::string> conf_obj_file_name    (config_file , "main" , "obj_file_name"     , "o=","");
    configitem<std::string> conf_svg_file_name    (config_file , "main" , "svg_file_name"     , "s=","");
    configitem<int> conf_svg_k (config_file, "main", "svg_k","s=",50);

    configitem<std::string> conf_texture_filename    (config_file , "main" , "texture_distance_file_name"     , "t=","");

    config_file.read();
    //cout << string(conf_obj_file_name) <<" __\n";
    string obj_filename = toFullPath(conf_obj_file_name);
    printf("obj_filename %s\n" , obj_filename.c_str());
    g_svg_k = conf_svg_k;
    string svg_filename = toFullPath(conf_svg_file_name);
    string texture_filename = toFullPath(conf_texture_filename);
    CRichModel model(obj_filename);
    g_model = model;
    g_model_ptr = &g_model;
    g_model_ptr->Preprocess();

    g_mesh.ReadCRichModel(g_model);

    readTexture(texture_filename);
    if (g_svg_graph != NULL) {
      delete g_svg_graph;
      g_svg_graph = NULL;
    }
		g_graph_type = GRAPH_SVG;
    g_svg_graph = new LC_LLL<float>(); 
    g_svg_graph->read_svg_file_binary(svg_filename);
    g_distance_field_status = DISTANCE_FIELD_UNINITIAL;
    string log_message1 = (string)"Panel/buttonMessage1  label = '" + config_filename + (string)"loaded" + (string)"'";
    TwDefine(log_message1.c_str());

  }


}


string ExePath() {
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}

string toRelativePath(const string& input_filename) {
  char buf[MAX_PATH];
    PathRelativePathTo(buf,
      g_exe_dir.c_str(),
      FILE_ATTRIBUTE_DIRECTORY,
      input_filename.c_str(),
      FILE_ATTRIBUTE_NORMAL);
  if (buf[0] == '.' && buf[1] == '\\') {
    return string(buf+2);
  }else{
    return string(buf);
  }
}

string toFullPath(const string& input_filename)
{

  /*DWORD WINAPI GetFullPathName(
  _In_  LPCTSTR lpFileName,
  _In_  DWORD   nBufferLength,
  _Out_ LPTSTR  lpBuffer,
  _Out_ LPTSTR  *lpFilePart
);
*/
  //char full[MAX_PATH];

  ////// GetFullPathName(input_filename.c_str(), MAX_PATH, full,NULL);

  // if( _fullpath( full, input_filename.c_str(), _MAX_PATH ) != NULL )
  // {
  //   return string(full);
  // }
  // else{
  //   assert(false);
  //    return "";
  // }
  return g_exe_dir + "\\" + input_filename;
}

void TW_CALL CB_OpenMeshFile(void* clientData)
{
  OPENFILENAME ofn;
  char szFile[MAX_PATH]  = "\0";
  ZeroMemory(&ofn , sizeof(ofn));
  ofn.lStructSize			= sizeof(ofn);
  //ofn.hwndOwner			=  FindWindow(NULL, "Mesh-Viewer");	//find main window to freeze it
  ofn.lpstrFile			= szFile ;
  //ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile			= sizeof(szFile);
  ofn.lpstrFilter			= "Model file (.m, .obj)\0*.m;*.obj\0All\0*.*\0";
  ofn.nFilterIndex		= 1;
  ofn.lpstrFileTitle		= NULL;
  ofn.nMaxFileTitle		= 0 ;
  //char buffer[MAX_PATH];
  //GetCurrentDirectory(MAX_PATH,buffer);
  string dir_name = g_exe_dir + "\\data";
  ofn.lpstrInitialDir = dir_name.c_str();
  //ofn.lpstrInitialDir		= NULL ;
  ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  GetOpenFileName(&ofn);    
  LPSTR local_filename = ofn.lpstrFile;   

  if (local_filename && local_filename[0] != '\0')
  {
    ElapasedTime timer;
    timer.start();


    string obj_file_name =local_filename;

    string texture_distance_file_name = toFullPath("data\\pic\\texture_distance_map.bmp");
    
    CRichModel model(obj_file_name);
    g_model = model;
    g_model_ptr = &g_model;
    g_model_ptr->Preprocess();

    g_mesh.ReadCRichModel(g_model);

    readTexture(texture_distance_file_name);
  }
}

int main(int argc , char** argv)
{
  //if (argc < 2) {
  //  printf("error! xx.exe xx.conf\n");
  //}
  
  g_exe_dir = ExePath();
  TwBar *bar; // Pointer to the tweak bar
  TwInit(TW_OPENGL, NULL);
  // Create a tweak bar
  bar = TwNewBar("Panel");
	TwAddButton(bar, "buttonOpenMeshFile", CB_OpenMeshFile, NULL, "label = 'Open Mesh file'");
  TwAddSeparator(bar, "Method SVG", NULL);
	TwAddButton(bar, "buttonOpengSVGConfigFile", CB_OpenSVGConfigFile, NULL, "label = 'Open SVG Config file'");
	TwAddButton(bar, "buttonSVGPreprocess", CB_SVGPreprocess, NULL, "label = 'SVG Prerpocess'");
  TwAddVarRW( bar, "varRWSVG_K", TW_TYPE_INT32, &g_svg_k, " label='k' min=0 step=1");
  TwAddSeparator(bar, "Method DGG", NULL);
	TwAddButton(bar, "buttonOpengDGGConfigFile", CB_OpenDGGConfigFile, NULL, "label = 'Open DGG Config file'");
	TwAddButton(bar, "buttonDGGPreprocess", CB_DGGPreprocess, NULL, "label = 'DGG Prerpocess'");
  TwAddVarRW( bar, "varRWEPS_VG", TW_TYPE_DOUBLE, &g_eps_vg, " label='eps' min=0 max=1 step=0.0001");
  TwAddSeparator(bar, NULL, NULL);
	TwAddButton(bar, "buttonComputeError", CB_ComputeError, NULL, "label = 'Compute RMS'");
//	TwAddButton(bar, "buttonDispError", CB_DispError, NULL, "label = 'Compute Error'");
	//TwAddButton(bar, "buttonAverageError", CB_AverageError, NULL, "label = 'Average Error'");
	//TwAddButton(bar, "buttonDegreeModel", CB_DegreeModel, NULL, "label = 'Dump Degree Model'");
	//TwAddButton(bar, "buttonDegreeModel", CB_EdgeLen, NULL, "label = 'Dump Degree Model'");
	
  TwAddSeparator(bar, NULL, NULL);

  {
    // ShapeEV associates Shape enum values with labels that will be displayed instead of enum values
    TwEnumVal shapeEV[3] = {    {NONE, "Rotate Model"},
                                {SELECT_POINT, "Select Source"} ,
                                {ADD_POINT, "Add Source"}
                           };
    // Create a type for the enum shapeEV
    TwType shapeType = TwDefineEnum("SelectMode", shapeEV, 3);
    // add 'g_CurrentShape' to 'bar': this is a variable of type ShapeType. Its key shortcuts are [<] and [>].
    TwAddVarRW(bar, "Mouse Status", shapeType, &mouse_click_mode, " keyIncr='<' keyDecr='>' help='change select mode' ");
  }

  {
    // ShapeEV associates Shape enum values with labels that will be displayed instead of enum values
    TwEnumVal shapeEV[1] = { {METHOD_DISTANCE_FIELD, "Computing Distance Field"}};
    // Create a type for the enum shapeEV
    TwType shapeType = TwDefineEnum("MethodMode", shapeEV, 1);
    // add 'g_CurrentShape' to 'bar': this is a variable of type ShapeType. Its key shortcuts are [<] and [>].
    //TwAddVarRW(bar, "Method", shapeType, &g_method_mode, " keyIncr='<' keyDecr='>' help='change select mode' ");
  }


  TwAddSeparator(bar, NULL, NULL);
  TwAddButton(bar, "buttonMessage1" , NULL, NULL, "label = ' '");
  TwAddButton(bar, "buttonMessage2" , NULL, NULL, "label = ' '");



  glutInit(&argc, argv);                /* Initialize GLUT */
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition(200,200);
  glutInitWindowSize(1200,800);
  main_window = glutCreateWindow("DGG");        /* Create window with given title */

  glViewport(0,0,1200,800);



  glutDisplayFunc(display);             /* Set-up callback functions */

  glutIdleFunc(glutIdle);

  glutReshapeFunc(reshape);

  glutMouseFunc(mouseClick);

  glutMotionFunc(mouseMove);
  glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);

  glutKeyboardFunc(keyBoard);

  TwGLUTModifiersFunc(glutGetModifiers);

  setupGLstate();




  glutMainLoop();                       /* Start GLUT event-processing loop */
  return 0;
}