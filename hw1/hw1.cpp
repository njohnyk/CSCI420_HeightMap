/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: Nikhil Johny K.
  Student ID: 2900797907
  Email: karuthed@usc.edu
*/

#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include "openGLHeader.h"
#include "glutHeader.h"
#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#ifdef WIN32
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2];                  // x,y coordinate of the mouse position
int leftMouseButton = 0;          // 1 if pressed, 0 if not 
int middleMouseButton = 0;        // 1 if pressed, 0 if not
int rightMouseButton = 0;         // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO* heightmapImage = NULL;

enum DisplayTypeEnum { MODE_1, MODE_2, MODE_3, MODE_4, MODE_EXTRA_CREDIT };
DisplayTypeEnum displayMode = MODE_1;

enum UserControlModeEnum { SHOWCASE, TRANSFORM };
UserControlModeEnum displayType = SHOWCASE;

BasicPipelineProgram * pipelineProgram;
OpenGLMatrix matrix;
float modelViewMatrix[16];
float projectionMatrix[16];
const float FOV = 45;

const int MAXIMUM_SCREENSHOTS = 301;
int g_screenshotCounter = 0;
bool g_takeScreenshots = false;

const float TRANSLATION_MODIFIER = 1.0f;
const float ROTATION_ANGLE_THETA = 0.05f;
const float SCALE_MODIFIER = 0.01f;

// Height Map class that generates height map data and build VBOs and VAOs
class HeightMap {
private:

    float heightScaleFactor = 0.025f;

    // Mode 1 members
    vector<GLfloat> mode1Vertices;
    vector<GLfloat> mode1Colors;

    // Mode 2 members
    vector<GLfloat> mode2Vertices;
    vector<GLfloat> mode2Colors;

    // Mode 3 members
    vector<GLfloat> mode3Vertices;
    vector<GLfloat> mode3Colors;

    // Mode 4 members
    vector<GLfloat> mode4Vertices;
    vector<GLfloat> mode4Colors;
    vector<GLfloat> pLeft;
    vector<GLfloat> pTop;
    vector<GLfloat> pRight;
    vector<GLfloat> pBottom;

    // Mode 1 VAO and VBOs
    GLuint mode1VAO;
    GLuint mode1VerticesVBO;
    GLuint mode1ColorVBO;

    // Mode 2 VAO and VBOs
    GLuint mode2VAO;
    GLuint mode2VerticesVBO;
    GLuint mode2ColorVBO;

    // Mode 3 VAO and VBOs
    GLuint mode3VAO;
    GLuint mode3VerticesVBO;
    GLuint mode3ColorVBO;

    // Mode 4 VAO and VBOs
    GLuint mode4VAO;
    GLuint mode4VerticesVBO;
    GLuint mode4ColorVBO;
    GLuint mode4VerticesLeftVBO;
    GLuint mode4VerticesRightVBO;
    GLuint mode4VerticesTopVBO;
    GLuint mode4VerticesBottomVBO;

public:
    void getHeightMapData(ImageIO* image);
    void generateBuffers();
    void mode1();
    void mode2();
    void mode3();
    void mode4();
    void modeExtraCredit();
} heightMap;

// Calculate the height map given an image and setup vectors to store the height map data
void HeightMap::getHeightMapData(ImageIO* image) {
    int counter = 0;
    heightScaleFactor *= image->getHeight() / 50.0f;
    int width = image->getWidth();
    int height = image->getHeight();

    // Loop through all pixels and create and assign data for mode 1, 2, 3
    for (int i = -height / 2; i < height / 2 - 1; i++) {
        for (int j = -width / 2; j < width / 2 - 1; j++) {
            // Mode 1
            // Calculate bottom left, top left, top right and bottom right vertex positions
            GLfloat y = heightScaleFactor * image->getPixel(i + height / 2, j + width / 2, 0);
            GLfloat bl[3] = { static_cast<GLfloat>(i), y, static_cast<GLfloat>(j) };
            y = heightScaleFactor * image->getPixel(i + height / 2, j + width / 2 + 1, 0);
            GLfloat tl[3] = { static_cast<GLfloat>(i), y, static_cast<GLfloat>((j + 1)) };
            y = heightScaleFactor * image->getPixel(i + height / 2 + 1, j + width / 2 + 1, 0);
            GLfloat tr[3] = { static_cast<GLfloat>(i + 1), y, static_cast<GLfloat>((j + 1)) };
            y = heightScaleFactor * image->getPixel(i + height / 2 + 1, j + width / 2, 0);
            GLfloat br[3] = { static_cast<GLfloat>(i + 1), y, static_cast<GLfloat>(j) };

            // Mode 4
            int boundaryLeft = -width / 2;
            int boundaryRight = width / 2;
            int boundaryTop = height / 2;
            int boundaryBottom = -height / 2;

            GLfloat blLeft[3] = { static_cast<GLfloat>(bl[0] - 1), bl[1], static_cast<GLfloat>(bl[2]) };
            GLfloat blTop[3] = { static_cast<GLfloat>(bl[0]), bl[1], static_cast<GLfloat>(bl[2] + 1) };
            GLfloat blRight[3] = { static_cast<GLfloat>(bl[0] + 1), bl[1], static_cast<GLfloat>(bl[2]) };
            GLfloat blBottom[3] = { static_cast<GLfloat>(bl[0]), bl[1], static_cast<GLfloat>(bl[2] - 1) };

            GLfloat tlLeft[3] = { static_cast<GLfloat>(tl[0] - 1), tl[1], static_cast<GLfloat>(tl[2]) };
            GLfloat tlTop[3] = { static_cast<GLfloat>(tl[0]), tl[1], static_cast<GLfloat>(tl[2] + 1) };
            GLfloat tlRight[3] = { static_cast<GLfloat>(tl[0] + 1), tl[1], static_cast<GLfloat>(tl[2]) };
            GLfloat tlBottom[3] = { static_cast<GLfloat>(tl[0]), tl[1], static_cast<GLfloat>(tl[2] - 1) };

            GLfloat trLeft[3] = { static_cast<GLfloat>(tr[0] - 1), tr[1], static_cast<GLfloat>(tr[2]) };
            GLfloat trTop[3] = { static_cast<GLfloat>(tr[0]), tr[1], static_cast<GLfloat>(tr[2] + 1) };
            GLfloat trRight[3] = { static_cast<GLfloat>(tr[0] + 1), tr[1], static_cast<GLfloat>(tr[2]) };
            GLfloat trBottom[3] = { static_cast<GLfloat>(tr[0]), tr[1], static_cast<GLfloat>(tr[2] - 1) };

            GLfloat brLeft[3] = { static_cast<GLfloat>(br[0] - 1), br[1], static_cast<GLfloat>(br[2]) };
            GLfloat brTop[3] = { static_cast<GLfloat>(br[0]), br[1], static_cast<GLfloat>(br[2] + 1) };
            GLfloat brRight[3] = { static_cast<GLfloat>(br[0] + 1), br[1], static_cast<GLfloat>(br[2]) };
            GLfloat brBottom[3] = { static_cast<GLfloat>(br[0]), br[1], static_cast<GLfloat>(br[2] - 1) };

            if (blLeft[0] < boundaryLeft) blLeft[0] = i;
            if (blTop[2] >= boundaryTop) blTop[2] = j;
            if (blRight[0] >= boundaryRight) blRight[0] = i;
            if (blBottom[2] < boundaryBottom) blBottom[2] = j;

            if (tlLeft[0] < boundaryLeft) tlLeft[0] = i;
            if (tlTop[2] >= boundaryTop) tlTop[2] = j;
            if (tlRight[0] >= boundaryRight) tlRight[0] = i;
            if (tlBottom[2] < boundaryBottom) tlBottom[2] = j;

            if (trLeft[0] < boundaryLeft) trLeft[0] = i;
            if (trTop[2] >= boundaryTop) trTop[2] = j;
            if (trRight[0] >= boundaryRight) trRight[0] = i;
            if (trBottom[2] < boundaryBottom) trBottom[2] = j;

            if (brLeft[0] < boundaryLeft) brLeft[0] = i;
            if (brTop[2] >= boundaryTop) brTop[2] = j;
            if (brRight[0] >= boundaryRight) brRight[0] = i;
            if (brBottom[2] < boundaryBottom) brBottom[2] = j;

            y = heightScaleFactor * image->getPixel(brLeft[0] + height / 2, brLeft[2] + width / 2, 0);
            brLeft[1] = y;
            y = heightScaleFactor * image->getPixel(brTop[0] + height / 2, brTop[2] + width / 2, 0);
            brTop[1] = y;
            y = heightScaleFactor * image->getPixel(brRight[0] + height / 2, brRight[2] + width / 2, 0);
            brRight[1] = y;
            y = heightScaleFactor * image->getPixel(brBottom[0] + height / 2, brBottom[2] + width / 2, 0);
            brBottom[1] = y;

            y = heightScaleFactor * image->getPixel(blLeft[0] + height / 2, blLeft[2] + width / 2, 0);
            blLeft[1] = y;
            y = heightScaleFactor * image->getPixel(blTop[0] + height / 2, blTop[2] + width / 2, 0);
            blTop[1] = y;
            y = heightScaleFactor * image->getPixel(blRight[0] + height / 2, blRight[2] + width / 2, 0);
            blRight[1] = y;
            y = heightScaleFactor * image->getPixel(blBottom[0] + height / 2, blBottom[2] + width / 2, 0);
            blBottom[1] = y;

            y = heightScaleFactor * image->getPixel(tlLeft[0] + height / 2, tlLeft[2] + width / 2, 0);
            tlLeft[1] = y;
            y = heightScaleFactor * image->getPixel(tlTop[0] + height / 2, tlTop[2] + width / 2, 0);
            tlTop[1] = y;
            y = heightScaleFactor * image->getPixel(tlRight[0] + height / 2, tlRight[2] + width / 2, 0);
            tlRight[1] = y;
            y = heightScaleFactor * image->getPixel(tlBottom[0] + height / 2, tlBottom[2] + width / 2, 0);
            tlBottom[1] = y;

            y = heightScaleFactor * image->getPixel(trLeft[0] + height / 2, trLeft[2] + width / 2, 0);
            trLeft[1] = y;
            y = heightScaleFactor * image->getPixel(trTop[0] + height / 2, trTop[2] + width / 2, 0);
            trTop[1] = y;
            y = heightScaleFactor * image->getPixel(trRight[0] + height / 2, trRight[2] + width / 2, 0);
            trRight[1] = y;
            y = heightScaleFactor * image->getPixel(trBottom[0] + height / 2, trBottom[2] + width / 2, 0);
            trBottom[1] = y;

            pLeft.insert(pLeft.end(), tlLeft, tlLeft + 3);
            pLeft.insert(pLeft.end(), trLeft, trLeft + 3);
            pLeft.insert(pLeft.end(), blLeft, blLeft + 3);
            pLeft.insert(pLeft.end(), blLeft, blLeft + 3);
            pLeft.insert(pLeft.end(), trLeft, trLeft + 3);
            pLeft.insert(pLeft.end(), brLeft, brLeft + 3);

            pTop.insert(pTop.end(), tlTop, tlTop + 3);
            pTop.insert(pTop.end(), trTop, trTop + 3);
            pTop.insert(pTop.end(), blTop, blTop + 3);
            pTop.insert(pTop.end(), blTop, blTop + 3);
            pTop.insert(pTop.end(), trTop, trTop + 3);
            pTop.insert(pTop.end(), brTop, brTop + 3);

            pRight.insert(pRight.end(), tlRight, tlRight + 3);
            pRight.insert(pRight.end(), trRight, trRight + 3);
            pRight.insert(pRight.end(), blRight, blRight + 3);
            pRight.insert(pRight.end(), blRight, blRight + 3);
            pRight.insert(pRight.end(), trRight, trRight + 3);
            pRight.insert(pRight.end(), brRight, brRight + 3);

            pBottom.insert(pBottom.end(), tlBottom, tlBottom + 3);
            pBottom.insert(pBottom.end(), trBottom, trBottom + 3);
            pBottom.insert(pBottom.end(), blBottom, blBottom + 3);
            pBottom.insert(pBottom.end(), blBottom, blBottom + 3);
            pBottom.insert(pBottom.end(), trBottom, trBottom + 3);
            pBottom.insert(pBottom.end(), brBottom, brBottom + 3);


            // Arrange vertex position data for mode 1 rendering
            mode1Vertices.insert (mode1Vertices.end(), tl, tl + 3);
            mode1Vertices.insert (mode1Vertices.end(), tr, tr + 3);
            mode1Vertices.insert (mode1Vertices.end(), bl, bl + 3);
            mode1Vertices.insert (mode1Vertices.end(), br, br + 3);

            // Calculate color for mode 1
            GLfloat colorP[4] = { 0.76f, 0.65f, 0.29f, 1.0f };
            mode1Colors.insert (mode1Colors.end(), colorP, colorP + 4);
            mode1Colors.insert (mode1Colors.end(), colorP, colorP + 4);
            mode1Colors.insert (mode1Colors.end(), colorP, colorP + 4);
            mode1Colors.insert (mode1Colors.end(), colorP, colorP + 4);
            
            // Mode 2
            // Arrange vertex position data for mode 2 rendering
            mode2Vertices.insert (mode2Vertices.end(), tl, tl + 3);
            mode2Vertices.insert (mode2Vertices.end(), tr, tr + 3);
            mode2Vertices.insert (mode2Vertices.end(), bl, bl + 3);
            mode2Vertices.insert (mode2Vertices.end(), br, br + 3);
            mode2Vertices.insert (mode2Vertices.end(), bl, bl + 3);
            mode2Vertices.insert (mode2Vertices.end(), tl, tl + 3);
            mode2Vertices.insert (mode2Vertices.end(), br, br + 3);
            mode2Vertices.insert (mode2Vertices.end(), tr, tr + 3);

            // Calculate color for mode 2
            GLfloat colorWF[4] = { 1.0f, 0.0f, 0.5f, 1.0f }; 
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);
            mode2Colors.insert (mode2Colors.end(), colorWF, colorWF + 4);

            // Mode 3
            // Arrange vertex position data for mode 3 rendering
            mode3Vertices.insert (mode3Vertices.end(), tl, tl + 3);
            mode3Vertices.insert (mode3Vertices.end(), tr, tr + 3);
            mode3Vertices.insert (mode3Vertices.end(), bl, bl + 3);
            mode3Vertices.insert (mode3Vertices.end(), bl, bl + 3);
            mode3Vertices.insert (mode3Vertices.end(), tr, tr + 3);
            mode3Vertices.insert (mode3Vertices.end(), br, br + 3);

            // Calculate color for mode 3
            GLfloat color = (float)image->getPixel(i + height / 2, j + width / 2, 0) / (float)255.0f;
            GLfloat colorBL[4] = { color, color, color, 1.0f };
            color = (float)image->getPixel(i + height / 2, j + width / 2 + 1, 0) / (float)255.0f;
            GLfloat colorTL[4] = { color, color, color, 1.0f };
            color = (float)image->getPixel(i + height / 2 + 1, j + width / 2 + 1, 0) / (float)255.0f;
            GLfloat colorTR[4] = { color, color, color, 1.0f };
            color = (float)image->getPixel(i + height / 2 + 1, j + width / 2, 0) / (float)255.0f;
            GLfloat colorBR[4] = { color, color, color, 1.0f };
            
            mode3Colors.insert (mode3Colors.end(), colorTL, colorTL + 4);
            mode3Colors.insert (mode3Colors.end(), colorTR, colorTR + 4);
            mode3Colors.insert (mode3Colors.end(), colorBL, colorBL + 4);
            mode3Colors.insert (mode3Colors.end(), colorBL, colorBL + 4);
            mode3Colors.insert (mode3Colors.end(), colorTR, colorTR + 4);
            mode3Colors.insert (mode3Colors.end(), colorBR, colorBR + 4);


        }
    }

    // Reuse mode 3 vertex and color data for mode 4
    mode4Vertices = mode3Vertices;
    mode4Colors = mode3Colors;
}

// Generate and bind VAO and VBOs
void HeightMap::generateBuffers () {
    // Mode 1
    // VAO setup
    glGenVertexArrays(1, &mode1VAO);
    glBindVertexArray(mode1VAO);
    // Vertex data
    glGenBuffers(1, &mode1VerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode1VerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, mode1Vertices.size() * sizeof(GLfloat), &mode1Vertices[0], GL_STATIC_DRAW);
    // Color data
    glGenBuffers(1, &mode1ColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode1ColorVBO);
    glBufferData(GL_ARRAY_BUFFER, mode1Colors.size() * sizeof(GLfloat), &mode1Colors[0], GL_STATIC_DRAW);
   
    // Mode 2
    // VAO setup
    glGenVertexArrays(1, &mode2VAO);
    glBindVertexArray(mode2VAO);
    // Vertex data
    glGenBuffers(1, &mode2VerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode2VerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, mode2Vertices.size() * sizeof(GLfloat), &mode2Vertices[0], GL_STATIC_DRAW);
    // Color data
    glGenBuffers(1, &mode2ColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode2ColorVBO);
    glBufferData(GL_ARRAY_BUFFER, mode2Colors.size() * sizeof(GLfloat), &mode2Colors[0], GL_STATIC_DRAW);

    // Mode 3
    // VAO setup
    glGenVertexArrays(1, &mode3VAO);
    glBindVertexArray(mode3VAO);
    // Vertex data
    glGenBuffers(1, &mode3VerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode3VerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, mode3Vertices.size() * sizeof(GLfloat), &mode3Vertices[0], GL_STATIC_DRAW);
    // Color data
    glGenBuffers(1, &mode3ColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode3ColorVBO);
    glBufferData(GL_ARRAY_BUFFER, mode3Colors.size() * sizeof(GLfloat), &mode3Colors[0], GL_STATIC_DRAW);

    // Mode 4
    // VAO setup
    glGenVertexArrays(1, &mode4VAO);
    glBindVertexArray(mode4VAO);
    // Vertex data
    glGenBuffers(1, &mode4VerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, mode4Vertices.size() * sizeof(GLfloat), &mode4Vertices[0], GL_STATIC_DRAW);
    // Color data
    glGenBuffers(1, &mode4ColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode4ColorVBO);
    glBufferData(GL_ARRAY_BUFFER, mode4Colors.size() * sizeof(GLfloat), &mode4Colors[0], GL_STATIC_DRAW);
    // Vertex data for the left, top, right and bottom neighbours
    glGenBuffers(1, &mode4VerticesLeftVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesLeftVBO);
    glBufferData(GL_ARRAY_BUFFER, pLeft.size() * sizeof(GLfloat), &pLeft[0], GL_STATIC_DRAW);
    glGenBuffers(1, &mode4VerticesTopVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesTopVBO);
    glBufferData(GL_ARRAY_BUFFER, pTop.size() * sizeof(GLfloat), &pTop[0], GL_STATIC_DRAW);
    glGenBuffers(1, &mode4VerticesRightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesRightVBO);
    glBufferData(GL_ARRAY_BUFFER, pRight.size() * sizeof(GLfloat), &pRight[0], GL_STATIC_DRAW);
    glGenBuffers(1, &mode4VerticesBottomVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesBottomVBO);
    glBufferData(GL_ARRAY_BUFFER, pBottom.size() * sizeof(GLfloat), &pBottom[0], GL_STATIC_DRAW);
}

// Draw mode 1
void HeightMap::mode1() {
    // Setup vertex shader pipeline
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode1VerticesVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode1ColorVBO);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    // Draw mode 1
    glDrawArrays(GL_POINTS, 0, mode1Vertices.size() / 3);
}

// Draw mode 2
void HeightMap::mode2() {
    // Setup vertex shader pipeline
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode2VerticesVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode2ColorVBO);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    // Draw mode 2
    glDrawArrays(GL_LINES, 0, mode2Vertices.size() / 3);
}

// Draw mode 3
void HeightMap::mode3() {
    // Setup vertex shader pipeline
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode3VerticesVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode3ColorVBO);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    // Draw mode 3
    glDrawArrays(GL_TRIANGLES, 0, mode3Vertices.size() / 3);

}

// Draw mode 4
void HeightMap::mode4() {
    // Setup vertex shader pipeline
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode4ColorVBO);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "leftVertices");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesLeftVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "topVertices");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesTopVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "rightVertices");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesRightVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "bottomVertices");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode4VerticesBottomVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    // Draw mode 4
    glDrawArrays(GL_TRIANGLES, 0, mode4Vertices.size() / 3);
}

// Draw mode 5 - Extra credit
void HeightMap::modeExtraCredit() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);   
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0, -1.0);

    // Setup vertex shader pipeline
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode2VerticesVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode2ColorVBO);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    // Draw wireframe 
    glDrawArrays(GL_LINES, 0, mode2Vertices.size() / 3);

    glDisable(GL_POLYGON_OFFSET_LINE);   
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode3VerticesVBO);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glBindBuffer(GL_ARRAY_BUFFER, mode3ColorVBO);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, mode3Vertices.size() / 3);
}

// Save a screenshot
void saveScreenshot(const char * filename) {
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

// Display the respective mode
void displayFunc() {
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLuint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");
  switch (displayMode) {
    case MODE_1:
      glUniform1i(loc, 0);
      heightMap.mode1();
      break;

    case MODE_2:
      glUniform1i(loc, 0);
      heightMap.mode2();
      break;

    case MODE_3:
      glUniform1i(loc, 0);
      heightMap.mode3();
      break;

    case MODE_4:
      glUniform1i(loc, 1);
      heightMap.mode4();
      break;

    case MODE_EXTRA_CREDIT:
      glUniform1i(loc, 0);
      heightMap.modeExtraCredit();
      break;
  }
  glutSwapBuffers();
}

// Set up pipeline and create the showcase display mode where the height map rotates continuously
void idleFunc() {
  if (g_screenshotCounter < MAXIMUM_SCREENSHOTS && g_takeScreenshots) {
    std::stringstream ss;
    ss << "anim/" << g_screenshotCounter << ".jpg";
    std::string name = ss.str();
    char const *pchar = name.c_str();
    saveScreenshot (pchar);
    g_screenshotCounter++;
  }
  if (displayType == SHOWCASE && heightmapImage != NULL) {
    matrix.SetMatrixMode (OpenGLMatrix::ModelView);
    matrix.Rotate(ROTATION_ANGLE_THETA, 0.0f, 1.0f, 0.0f);
    matrix.GetMatrix (modelViewMatrix);

    pipelineProgram->SetModelViewMatrix (modelViewMatrix);
  }
  glutPostRedisplay();
}

void reshapeFunc(int w, int h) {
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode (OpenGLMatrix::Projection);
  matrix.LoadIdentity ();
  matrix.Perspective (FOV, (float)windowWidth / (float)windowHeight, 0.01, 1000.0);
  matrix.GetMatrix (projectionMatrix);

  pipelineProgram->SetProjectionMatrix (projectionMatrix);
}

// Detect mouse for translation, rotation and scaling
void mouseMotionDragFunc(int x, int y) {

  if (displayType == TRANSFORM) {

    int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };
    switch (controlState) {
      case TRANSLATE:
        if (leftMouseButton) {
          landTranslate[0] += mousePosDelta[0] * 0.01f;
          landTranslate[1] -= mousePosDelta[1] * 0.01f;
        }

        if (middleMouseButton) {
          landTranslate[2] += mousePosDelta[1] * 0.01f;
        }

        // Translate
        matrix.SetMatrixMode (OpenGLMatrix::ModelView);
        matrix.Translate(landTranslate[0] * TRANSLATION_MODIFIER, landTranslate[1] * TRANSLATION_MODIFIER, landTranslate[2] * TRANSLATION_MODIFIER);
        matrix.GetMatrix (modelViewMatrix);

        pipelineProgram->SetModelViewMatrix (modelViewMatrix);
        break;

      case ROTATE:
        if (leftMouseButton) {
          landRotate[0] += mousePosDelta[1];
          landRotate[1] += mousePosDelta[0];
        }

        if (middleMouseButton) {
          landRotate[2] += mousePosDelta[1];
        }

        // Rotate
        matrix.SetMatrixMode (OpenGLMatrix::ModelView);
        matrix.Rotate(ROTATION_ANGLE_THETA, landRotate[0], landRotate[1], landRotate[2]);
        matrix.GetMatrix (modelViewMatrix);

        pipelineProgram->SetModelViewMatrix (modelViewMatrix);
        break;

      case SCALE:
        if (leftMouseButton) {
          landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f * SCALE_MODIFIER;
          landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f * SCALE_MODIFIER;
        }

        if (middleMouseButton) {
          landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f * SCALE_MODIFIER;
        }

        // Scale
        matrix.SetMatrixMode (OpenGLMatrix::ModelView);
        matrix.Scale(landScale[0], landScale[1], landScale[2]);
        matrix.GetMatrix (modelViewMatrix);

        pipelineProgram->SetModelViewMatrix (modelViewMatrix);

        break;
    }

    mousePos[0] = x;
    mousePos[1] = y;
  }
}

void mouseMotionFunc(int x, int y) {
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y) {
  switch (button) {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  mousePos[0] = x;
  mousePos[1] = y;
}

// Assign keyboard keys to modes and transformations
void keyboardFunc(unsigned char key, int x, int y) {

  switch (key) {
    case 27: // ESC key
      exit(0); // exit the program
      break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
      break;

    case '1':
      displayMode = MODE_1;
      break;

    case '2':
      displayMode = MODE_2;
      break;

    case '3':
      displayMode = MODE_3;
      break;
    case '4':
      displayMode = MODE_4;
      break;
    case '5':
      displayMode = MODE_EXTRA_CREDIT;
      break;

    case 'r':
        controlState = ROTATE;
        break;
    case 's':
        controlState = SCALE;
        break;
    case 't':
        controlState = TRANSLATE;
        break;

    case 9:
        if (displayType == SHOWCASE) {
        displayType = TRANSFORM;
        std::cout << "Transfromation mode." << std::endl;
        }

        else {
        displayType = SHOWCASE;
        std::cout << "Showcase mode." << std::endl;
        }
        break;

    case 'q':
        g_takeScreenshots = true;
        std::cout << "Starting animation!." << std::endl;
        break;
  }
}

// Initialize the scene and create the height map and buffers
void initScene(int argc, char *argv[]) {
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK) {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  pipelineProgram = new BasicPipelineProgram;
  int ret = pipelineProgram->Init(shaderBasePath);
  if (ret != 0) abort();

  pipelineProgram->Bind();

  matrix.SetMatrixMode (OpenGLMatrix::ModelView);
  matrix.LoadIdentity ();
  matrix.LookAt (
    256, 128, 256,
    0, 0, 0,
    0, 1, 0
  );
  matrix.GetMatrix (modelViewMatrix);

  pipelineProgram->SetModelViewMatrix (modelViewMatrix);

  heightMap.getHeightMapData(heightmapImage);
  heightMap.generateBuffers();

  glEnable(GL_DEPTH_TEST);
  std::cout << "GL error: " << glGetError() << std::endl;
  glDepthFunc(GL_LESS);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0); 
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK) {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


