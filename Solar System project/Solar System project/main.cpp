#define _CRT_SECURE_NO_DEPRECATE
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>

//Set the width and height of the Window
#define Windowwidth  1024
#define Windowheight 1024
// Here the Distance can be treated as a unit. For example, the distance between  
// mercurio and sun is 1*Distance, and between earth and sun is 3*Distance
#define Distance 100000000
//The radius of earth (expanded), can also be treated as unit to calcualte other planet's radius
#define Eradius 16000000
const GLfloat Pi = 3.1415926536f;
static int day = 0;
GLdouble Angle = 80.0;
GLdouble aix_x = 0.0, aix_y = 2000000000, aix_z = 2000000000;



//The ratio of radius0.4:1:1:0.5:11:9:4:3
//mercurios,venus,earth,mars,jupiter,saturn,uranus,neptune 
GLuint tbg,tsun,tearth,tmercu,tven,tmars,tjup,tsat,tura,tnep;

//Loading texture files, which can also be found at http://www.cppblog.com/doing5552/archive/2009/01/08/71532.aspx
int power_of_two(int n)
{
    if (n <= 0)
        return 0;
    return (n & (n - 1)) == 0;
}
GLuint LoadTexture(const char* filename)
{

    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint  texture_ID = 0;

    
    FILE* pFile = fopen(filename, "rb");
    if (pFile == 0)
        return 0;

    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, 54, SEEK_SET);

    
    {
        GLint line_bytes = width * 3;
        while (line_bytes % 4 != 0)
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    
    pixels = (GLubyte*)malloc(total_bytes);
    if (pixels == 0)
    {
        fclose(pFile);
        return 0;
    }

    
    if (fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    
    // The is for the compatibility with old version of opengl
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if (!power_of_two(width)
            || !power_of_two(height)
            || width > max
            || height > max)
        {
            const GLint new_width = 512;
            const GLint new_height = 512; // set the picture to 512*512
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            
            new_line_bytes = new_width * 3;
            while (new_line_bytes % 4 != 0)
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if (new_pixels == 0)
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            
            gluScaleImage(GL_RGB,
                width, height, GL_UNSIGNED_BYTE, pixels,
                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            
            free(pixels);
            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    
    glGenTextures(1, &texture_ID);
    if (texture_ID == 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
        GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
    free(pixels);
    return texture_ID;

}

//Initilize to load files
void init_LoadallTexture()
{
    tsun = LoadTexture("pictures/sol.bmp");
    tbg = LoadTexture("pictures/bg.bmp");
    tearth = LoadTexture("pictures/terra.bmp");
    tmercu = LoadTexture("pictures/mercurio.bmp");
    tven = LoadTexture("pictures/venus.bmp");
    tmars = LoadTexture("pictures/marte.bmp");
    tjup = LoadTexture("pictures/jupiter.bmp");
    tsat = LoadTexture("pictures/saturno.bmp");
    tura = LoadTexture("pictures/urano.bmp");
    tnep = LoadTexture("pictures/neptuno.bmp");
}

// Get every planet and background
//The transform of matrix firstly is set to achieve the Orbital revolution, then self-rotation 
//The parameters about lighting is specially set according to some references
//Quadratic surface to draw a sphere, which is convenient since the texture coordinate can directly used on sphere
void get_bg()
{

  
    //设置指针长度
    
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); //平行修剪空间
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tbg);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0); glVertex2d(-1.0, -1.0);
    glTexCoord2d(1.0, 0.0); glVertex2d(+1.0, -1.0);
    glTexCoord2d(1.0, 1.0); glVertex2d(+1.0, +1.0);
    glTexCoord2d(0.0, 1.0); glVertex2d(-1.0, +1.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}
void get_sun()
{
    glPushMatrix();
    glRotatef(day/25.0*360, 0.0, 0.0, -1.0);
    {
        GLfloat sun_light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat sun_light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat sun_light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat sun_light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, sun_light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, sun_light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, sun_light_specular);

        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        
    }
    {
        GLfloat sun_mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat sun_mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat sun_mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat sun_mat_emission[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat sun_mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, sun_mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, sun_mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);
    }


    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);
    glBindTexture(GL_TEXTURE_2D, tsun);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere,69600000, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}
void get_mercu()
{

    glPushMatrix();
    glTranslatef(2, 0.0f, 0.0f);
    glRotatef(2, 0.0f, 0.0f, -1.0f);
    glPopMatrix();
    glPushMatrix();
    glRotatef(day / 87.7 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef( Distance, 0.0f, 0.0f);
    {
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);

    glBindTexture(GL_TEXTURE_2D, tmercu);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 0.4*Eradius, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}
void get_ven()
{

    glPushMatrix();
    glTranslatef(2, 0.0f, 0.0f);
    glRotatef(1.5, 0.0f, 0.0f, -1.0f);
    glPopMatrix();
    glPushMatrix();
    glRotatef(day / 224.7 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef(2 * Distance, 0.0f, 0.0f);
    {
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);

    glBindTexture(GL_TEXTURE_2D, tven);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere,  Eradius, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}
void get_earth()
{

    glPushMatrix();
    glRotatef(day / 360.0 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef(3 * Distance, 0.0f, 0.0f);
    
    {
        GLfloat earth_mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat earth_mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat earth_mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat earth_mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat earth_mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, earth_mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, earth_mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, earth_mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, earth_mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, earth_mat_shininess);
        /*glutSolidSphere(15945000, 20, 20);*/

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);
    
    glBindTexture(GL_TEXTURE_2D, tearth);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 15945000, 100, 100);
    glDisable(GL_TEXTURE_2D);
    //glDisable(GL_LIGHT0);
    //glDisable(GL_LIGHTING);
    glPopMatrix();
}
void get_mars()
{

    glPushMatrix();
    glTranslatef(2, 0.0f, 0.0f);
    glRotatef(1.5, 0.0f, 0.0f, -1.0f);
    glPopMatrix();
    glPushMatrix();
    glRotatef(day / 687.0 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef(4 * Distance, 0.0f, 0.0f);
    {
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);

    glBindTexture(GL_TEXTURE_2D, tmars);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 0.5*Eradius, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}
void get_jup()
{
    glPushMatrix();
    glTranslatef(2, 0.0f, 0.0f);
    glRotatef(180, 0.0f, 0.0f, -1.0f);
    glPopMatrix();
    glPushMatrix();
    glRotatef(day / 4337.0 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef(6.5 * Distance, 0.0f, 0.0f);

    {
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);

    glBindTexture(GL_TEXTURE_2D, tjup);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 11*Eradius, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}
void get_sat()
{
    glPushMatrix();
    glTranslatef(2, 0.0f, 0.0f);
    glRotatef(180, 0.0f, 0.0f, -1.0f);
    glPopMatrix();
    glPushMatrix();
    glRotatef(day / 10000.0 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef(10.5 * Distance, 0.0f, 0.0f);

    {
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);

    glBindTexture(GL_TEXTURE_2D, tsat);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 9*Eradius, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}
void get_ura()
{
    glPushMatrix();
    glTranslatef(2, 0.0f, 0.0f);
    glRotatef(180, 0.0f, 0.0f, -1.0f);
    glPopMatrix();
    glPushMatrix();
    glRotatef(day / 20000.0 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef(13 * Distance, 0.0f, 0.0f);

    {
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);

    glBindTexture(GL_TEXTURE_2D, tura);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 4*Eradius, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}
void get_nep()
{
    glPushMatrix();
    glTranslatef(2, 0.0f, 0.0f);
    glRotatef(120, 0.0f, 0.0f, -1.0f);
    glPopMatrix();
    glPushMatrix();
    glRotatef(day / 164.0 * 360, 0.0f, 0.0f, -1.0f);
    glTranslatef(15 * Distance, 0.0f, 0.0f);

    {
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess = 0.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

    }
    GLUquadricObj* sphere = NULL;
    sphere = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    gluQuadricDrawStyle(sphere, GLU_FILL);

    glBindTexture(GL_TEXTURE_2D, tnep);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 3*Eradius, 100, 100);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}

//Display Function
void myDisplay(void)

{
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    get_bg();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(Angle, 1, 1, 10000000000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(aix_x, aix_y, aix_z, 0, 0, 0, 0, 0, 1);
    get_sun();
    get_mercu();
    get_ven();
    get_earth();
    get_mars();
    get_jup();
    get_sat();
    get_ura();
    get_nep();
    
    glFlush();   
    glutSwapBuffers();
    
}

//Here the timing function is used to control the speed of frame rate
void timerProc(int id)
{
    ++day;
    glutPostRedisplay();
    glutTimerFunc(50, timerProc, 1);//The first parameter is depend on your own device
}

//Control the view position and angle
void mykeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'W':
    case 'w':
        aix_y += 100000000;
        aix_z -= 100000000;
        break;
    case 'S':
    case 's':
        aix_y -= 100000000;
        aix_z += 100000000;
        break;
    case 'A':
    case 'a':
        aix_x -= 100000000;
        aix_z += 100000000;
        break;
    case 'D':
    case 'd':
        aix_x += 100000000;
        aix_z -= 100000000;
        break;


    case 'J':
    case 'j':
        Angle -= 5.0;
        break;
    case 'L':
    case 'l':
        Angle += 5.0;
        break;
    }
    glutPostRedisplay();

}
int main(int argc, char* argv[])

{

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(Windowwidth, Windowheight);
    
    glutCreateWindow("Solar System");
    init_LoadallTexture();
    glutDisplayFunc(&myDisplay);
    glutKeyboardFunc(mykeyboard);
    glutTimerFunc(50, timerProc, 1);
    glutMainLoop();

    return 0;


}

