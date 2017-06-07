
#include "stdafx.h"
/*
(c) Janusz Ganczarski
http://www.januszg.hg.pl
JanuszG@enter.net.pl
*/

#include <GL/glut.h>
#include <stdlib.h>
#include "colors.h"

// sta�e do obs�ugi menu podr�cznego

enum
{
	FULL_WINDOW = GL_ALWAYS + 100, // aspekt obrazu - ca�e okno
	ASPECT_1_1, // aspekt obrazu 1:1
	EXIT // wyj�cie
};

// aspekt obrazu

int aspect = FULL_WINDOW;

// rozmiary bry�y obcinania

const GLdouble left = -2.0;
const GLdouble right = 2.0;
const GLdouble bottom = -2.0;
const GLdouble top = 2.0;
const GLdouble near_ = 3.0;
const GLdouble far_ = 7.0;

// k�ty obrotu

GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;

// wska�nik naci�ni�cia lewego przycisku myszki

int button_state = GLUT_UP;

// po�o�enie kursora myszki

int button_x, button_y;

// rodzaj testu bufora g��boko�ci

GLenum depth_test = GL_LESS;

// funkcja generuj�ca scen� 3D

GLfloat vertex[11 * 3] =
{

	0.0f, 1.0f, 0.0f,
	0.0f, -1.0f, -1.0f,
	-0.6f, -1.0f, -0.80f,
	-1.0f, -1.0f, -0.2f,
	-0.9f, -1.0f, 0.5f,
	-0.3f, -1.0f, 0.9f,
	0.3f, -1.0f, 0.9f,
	0.9f, -1.0f, 0.5f,
	1.0f, -1.0f, -0.2f,
	0.6f, -1.0f, -0.8f,
	0.0, -1.0,  0.0,
};

int triangles[18 * 3] =
{
	1,2,0,
	2,3,0,
	3,4,0,
	4,5,0,
	5,6,0,
	6,7,0,
	7,8,0,
	8,9,0,
	9,1,0,
	2,1,10,
	3,2,10,
	4,3,10,
	5,4,10,
	6,5,10,
	7,6,10,
	8,7,10,
	9,8,10,
	1,9,10,
};

void Normal(GLfloat *n, int i)
{
	GLfloat v1[3], v2[3];
	// obliczenie wektor�w na podstawie wsp�rz�dnych wierzcho�k�w tr�jk�t�w
	v1[0] = vertex[3 * triangles[3 * i + 1] + 0] - vertex[3 * triangles[3 * i + 0] + 0];
	v1[1] = vertex[3 * triangles[3 * i + 1] + 1] - vertex[3 * triangles[3 * i + 0] + 1];
	v1[2] = vertex[3 * triangles[3 * i + 1] + 2] - vertex[3 * triangles[3 * i + 0] + 2];
	v2[0] = vertex[3 * triangles[3 * i + 2] + 0] - vertex[3 * triangles[3 * i + 1] + 0];
	v2[1] = vertex[3 * triangles[3 * i + 2] + 1] - vertex[3 * triangles[3 * i + 1] + 1];
	v2[2] = vertex[3 * triangles[3 * i + 2] + 2] - vertex[3 * triangles[3 * i + 1] + 2];
	// obliczenie waktora normalnego przy pomocy iloczynu wektorowego
	n[0] = v1[1] * v2[2] - v1[2] * v2[1];
	n[1] = v1[2] * v2[0] - v1[0] * v2[2];
	n[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

// wska�nik przesuni�cia warto�ci g��bi
bool polygon_offset = true;

// k�t obrotu kuli
GLfloat angle = 0.0;

// wska�nik rysowania p�aszczyzna przekroju
bool cutting_plane = true;


void Display()
{
	// kolor t�a - zawarto�� bufora koloru
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// czyszczenie bufora koloru i bufora g��boko�ci
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// wyb�r macierzy modelowania
	glMatrixMode(GL_MODELVIEW);

	// macierz modelowania = macierz jednostkowa
	glLoadIdentity();

	// przesuni�cie uk�adu wsp�rz�dnych sze�cianu do �rodka bry�y odcinania
	glTranslatef(0, 0, -(near_ + far_) / 2);

	// obroty sze�cianu
	glRotatef(rotatex, 1.0, 0, 0);
	glRotatef(rotatey, 0, 1.0, 0);

	// niewielkie powi�kszenie sze�cianu
	glScalef(1.15, 1.15, 1.15);

	// w��czenie testu bufora g��boko�ci
	glEnable(GL_DEPTH_TEST);

	// wyb�r funkcji do testu bufora g��boko�ci
	glDepthFunc(depth_test);

	// najpierw rysujemy kul� obracaj�c� si� wewn�trz sze�cianu;
	// z uwagi na celowy brak efekt�w o�wietlenia, obr�t kuli
	// podkre�la druga kula w wersji "szkieletowej"
	glPushMatrix();
	angle += 0.1;
	glRotatef(angle, 0.5, 0.5, 0.0);
	glColor3fv(Red);
	if (polygon_offset)
		glEnable(GL_POLYGON_OFFSET_FILL);

	glPolygonOffset(0.5, 0.5);
	glutSolidSphere(0.3, 10, 10);
	glColor3fv(Black);
	glutWireSphere(0.3, 10, 10);
	if (polygon_offset)
		glDisable(GL_POLYGON_OFFSET_FILL);

	glPopMatrix();

	// rysowanie p�aszczyzny otworu w sze�cianie

	if (cutting_plane)

	{
		// wy��czenie rysowania w buforze kolor�w
		glDrawBuffer(GL_NONE);
		// rysowanie kwadratu cz�ciowo ods�aniaj�cego wn�trze sze�cianu
		// (kwadrat jest po�o�ony o 0,001 jednostki nad bokiem sze�cianu)
		glBegin(GL_QUADS);
		glVertex3f(-0.5, -1.001f, -0.5);
		glVertex3f(0.5, -1.001f, -0.5);
		glVertex3f(0.5, -1.001f, 0.5);
		glVertex3f(-0.5, -1.001f, 0.5);
		glEnd();


		// w��czenie rysowania w buforze kolor�w

		glDrawBuffer(GL_BACK);

	}


	glBegin(GL_TRIANGLES);
	for (int i = 0; i < 18; i++)
	{
		GLfloat n[3];
		Normal(n, i);
		// u�redniony wektor normalny jest normalizowany tylko, gdy biblioteka
		// obs�uguje automatyczne skalowania jednostkowych wektor�w normalnych
		glNormal3fv(n);
		glColor3fv(Lime);
		glVertex3fv(&vertex[3 * triangles[3 * i + 0]]);
		glColor3fv(Yellow);
		glVertex3fv(&vertex[3 * triangles[3 * i + 1]]);
		glColor3fv(Blue);
		glVertex3fv(&vertex[3 * triangles[3 * i + 2]]);
	}

	// koniec definicji sze�cianu RGB
	glEnd();

	// skierowanie polece� do wykonania
	glFlush();

	// zamiana bufor�w koloru
	glutSwapBuffers();
}

// zmiana wielko�ci okna

void Reshape(int width, int height)
{
	// obszar renderingu - ca�e okno
	glViewport(0, 0, width, height);

	// wyb�r macierzy rzutowania
	glMatrixMode(GL_PROJECTION);

	// macierz rzutowania = macierz jednostkowa
	glLoadIdentity();

	// parametry bry�y obcinania
	if (aspect == ASPECT_1_1)
	{
		// wysoko�� okna wi�ksza od wysoko�ci okna
		if (width < height && width > 0)
			glFrustum(left, right, bottom * height / width, top * height / width, near_, far_);
		else

			// szeroko�� okna wi�ksza lub r�wna wysoko�ci okna
			if (width >= height && height > 0)
				glFrustum(left * width / height, right * width / height, bottom, top, near_, far_);

	}
	else
		glFrustum(left, right, bottom, top, near_, far_);

	// generowanie sceny 3D
	Display();
}

// obs�uga klawiszy funkcyjnych i klawiszy kursora

void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
		// kursor w lewo
	case GLUT_KEY_LEFT:
		rotatey -= 1;
		break;

		// kursor w g�r�
	case GLUT_KEY_UP:
		rotatex -= 1;
		break;

		// kursor w prawo
	case GLUT_KEY_RIGHT:
		rotatey += 1;
		break;

		// kursor w d�
	case GLUT_KEY_DOWN:
		rotatex += 1;
		break;
	}

	// odrysowanie okna
	Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// obs�uga przycisk�w myszki

void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		// zapami�tanie stanu lewego przycisku myszki
		button_state = state;

		// zapami�tanie po�o�enia kursora myszki
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
}

// obs�uga ruchu kursora myszki

void MouseMotion(int x, int y)
{
	if (button_state == GLUT_DOWN)
	{
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) *(x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) *(button_y - y);
		button_y = y;
		glutPostRedisplay();
	}
}

// obs�uga menu podr�cznego

void Menu(int value)
{
	switch (value)
	{
		// obszar renderingu - ca�e okno
	case FULL_WINDOW:
		aspect = FULL_WINDOW;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// obszar renderingu - aspekt 1:1
	case ASPECT_1_1:
		aspect = ASPECT_1_1;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// test bufora g��boko�ci: GL_NEVER
	case GL_NEVER:
		depth_test = GL_NEVER;
		Display();
		break;

		// test bufora g��boko�ci: GL_LESS
	case GL_LESS:
		depth_test = GL_LESS;
		Display();
		break;

		// test bufora g��boko�ci: GL_EQUAL
	case GL_EQUAL:
		depth_test = GL_EQUAL;
		Display();
		break;

		// test bufora g��boko�ci: GL_LEQUAL
	case GL_LEQUAL:
		depth_test = GL_LEQUAL;
		Display();
		break;

		// test bufora g��boko�ci: GL_GREATER
	case GL_GREATER:
		depth_test = GL_GREATER;
		Display();
		break;

		// test bufora g��boko�ci: GL_NOTEQUAL
	case GL_NOTEQUAL:
		depth_test = GL_NOTEQUAL;
		Display();
		break;

		// test bufora g��boko�ci: GL_GEQUAL
	case GL_GEQUAL:
		depth_test = GL_GEQUAL;
		Display();
		break;

		// test bufora g��boko�ci: GL_ALWAYS
	case GL_ALWAYS:
		depth_test = GL_ALWAYS;
		Display();
		break;

		// wyj�cie
	case EXIT:
		exit(0);
	}
}

int main(int argc, char * argv[])
{
	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);

	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// rozmiary g��wnego okna programu
	glutInitWindowSize(500, 500);

	// utworzenie g��wnego okna programu
#ifdef WIN32

	glutCreateWindow("Sze�cian 5");
#else

	glutCreateWindow("Szescian 5");
#endif

	// do��czenie funkcji generuj�cej scen� 3D
	glutDisplayFunc(Display);

	// do��czenie funkcji wywo�ywanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);

	// do��czenie funkcji obs�ugi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);

	// obs�uga przycisk�w myszki
	glutMouseFunc(MouseButton);

	// obs�uga ruchu kursora myszki
	glutMotionFunc(MouseMotion);

	// utworzenie menu podr�cznego
	glutCreateMenu(Menu);

	// utworzenie podmenu - aspekt obrazu
	int MenuAspect = glutCreateMenu(Menu);
#ifdef WIN32

	glutAddMenuEntry("Aspekt obrazu - ca�e okno", FULL_WINDOW);
#else

	glutAddMenuEntry("Aspekt obrazu - cale okno", FULL_WINDOW);
#endif

	glutAddMenuEntry("Aspekt obrazu 1:1", ASPECT_1_1);

	// utworzenie podmenu - Test z-bufora
	int MenuZbuffer = glutCreateMenu(Menu);
	glutAddMenuEntry("GL_NEVER", GL_NEVER);
	glutAddMenuEntry("GL_LESS", GL_LESS);
	glutAddMenuEntry("GL_EQUAL", GL_EQUAL);
	glutAddMenuEntry("GL_LEQUAL", GL_LEQUAL);
	glutAddMenuEntry("GL_GREATER", GL_GREATER);
	glutAddMenuEntry("GL_NOTEQUAL", GL_NOTEQUAL);
	glutAddMenuEntry("GL_GEQUAL", GL_GEQUAL);
	glutAddMenuEntry("GL_ALWAYS", GL_ALWAYS);

	// menu g��wne
	glutCreateMenu(Menu);
	glutAddSubMenu("Aspekt obrazu", MenuAspect);
	glutAddSubMenu("Test z-bufora", MenuZbuffer);
#ifdef WIN32

	glutAddMenuEntry("Wyj�cie", EXIT);
#else

	glutAddMenuEntry("Wyjscie", EXIT);
#endif

	// okre�lenie przycisku myszki obs�uguj�cej menu podr�czne
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// wprowadzenie programu do obs�ugi p�tli komunikat�w
	glutMainLoop();
	return 0;
}
