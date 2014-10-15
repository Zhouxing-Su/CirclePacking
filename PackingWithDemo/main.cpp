/*

instance format:

NumberOfObjects RadiusOfContainer
RadiusOfObject1 RadiusOfObject2 ...

*/

#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>



using namespace cv;
using namespace std;



void init();
void readInstance( const string &testCaseName );
void randomInit();
void greedyinit();
double embedDepthBetweenObjs( int i, int j );
double embedDepthToContainer( int i );
void findDmax();
void findDmax_SingleForce();
void findDmax_ResultantForce();
void perturb();
void perturb_randomDrop();
void perturb_switchPair();
void solve();
void printResult( int waitTime = 1 );
double dist( double x1, double y1, double x2, double y2 );




// graph painting
const string WINDOW_NAME = "Display";
const int WINDOW_MARGIN = 128;
const int WINDOW_SIZE = 512;
const int ORIGIN_SHIFT = WINDOW_MARGIN + WINDOW_SIZE / 2;
int COORD_EXPAND;
double ACCURACY;
double MERELY_CHANGE;
// paint panel
Mat image( 2 * WINDOW_MARGIN + WINDOW_SIZE, 2 * WINDOW_MARGIN + WINDOW_SIZE, CV_8UC3, Scalar( 255, 255, 255 ) );
Mat emptyImage( 2 * WINDOW_MARGIN + WINDOW_SIZE, 2 * WINDOW_MARGIN + WINDOW_SIZE, CV_8UC3, Scalar( 255, 255, 255 ) );

// instance
const int MAX_OBJECT_NUMBER = 100;
int ObjectNum;
double ContainerRadius;
double r[MAX_OBJECT_NUMBER];
// solution
double x[MAX_OBJECT_NUMBER];
double y[MAX_OBJECT_NUMBER];
// objective
double Dmax;
double DmaxNoTabu;
double deltaDmax;

// local search
int iterCount = 1;
int tabuTable[MAX_OBJECT_NUMBER] = { 0 };
int obj1;
int obj2;  // -1 for container
int DmaxNoChange = 0;
const int MAX_NO_IMPROVE_STEP = 512;




int main( int argc, char** argv )
{
    init();
    printResult( 0 );

    clock_t start = clock();

    solve();

    clock_t end = clock();
    cout << end - start << "ms" << endl;

    printResult( 0 );

    return 0;
}




void init()
{
    readInstance( "testcase.txt" );
    randomInit();

    namedWindow( WINDOW_NAME, WINDOW_AUTOSIZE );
    COORD_EXPAND = WINDOW_SIZE / (2 * ContainerRadius);
    ACCURACY = ContainerRadius / WINDOW_SIZE / 2;
    MERELY_CHANGE = 4 * ACCURACY;
}

void readInstance( const string &testCaseName )
{
    ifstream fin( testCaseName );

    fin >> ObjectNum >> ContainerRadius;
    for (int i = 0; i < ObjectNum; i++) {
        fin >> r[i];
    }

    fin.close();
}

void randomInit()
{
    random_device rd;
    mt19937 gen( rd() );
    uniform_real_distribution<> rand( -ContainerRadius, ContainerRadius );

    for (int i = 0; i < ObjectNum; i++) {
        double radius;
        do {
            x[i] = rand( gen );
            y[i] = rand( gen );
            radius = sqrt( x[i] * x[i] + y[i] * y[i] );
        } while (radius >= ContainerRadius);
    }
}

void greedyinit()
{

}

double embedDepthBetweenObjs( int i, int j )
{
    double dx = x[i] - x[j];
    double dy = y[i] - y[j];
    double embedDepth = r[i] + r[j] - (sqrt( dx*dx + dy*dy ));
    return embedDepth > 0 ? embedDepth : 0;
}

double embedDepthToContainer( int i )
{
    double embedDepth = (sqrt( x[i] * x[i] + y[i] * y[i] ) + r[i]) - ContainerRadius;
    return embedDepth > 0 ? embedDepth : 0;
}

void findDmax()
{
    Dmax = 0;
    DmaxNoTabu = 0;
    obj1 = 0;
    obj2 = -1;

    findDmax_SingleForce();
}

void findDmax_SingleForce()
{
    double originDmax = DmaxNoTabu;

    for (int i = 0; i < ObjectNum; i++) {
        double d = embedDepthToContainer( i );
        if (d > Dmax) {
            DmaxNoTabu = d;
            if (iterCount > tabuTable[i]) {
                Dmax = d;
                obj1 = i;
                obj2 = -1;
            }
        }
        for (int j = i + 1; j < ObjectNum; j++) {
            d = embedDepthBetweenObjs( i, j );
            if (d > Dmax) {
                DmaxNoTabu = d;
                if (iterCount > tabuTable[i] && iterCount > tabuTable[j]) {
                    Dmax = d;
                    obj1 = i;
                    obj2 = j;
                }
            }
        }
    }

    deltaDmax = originDmax - DmaxNoTabu;
}

void findDmax_ResultantForce()
{
    for (int i = 0; i < ObjectNum; i++) {
        double d = embedDepthToContainer( i );


        //get dx dy


        for (int j = 0; j < ObjectNum; j++) {
            if (i != j) {
                d += embedDepthBetweenObjs( i, j );

                //get dx dy

            }
        }
        if (d > Dmax) {
            DmaxNoTabu = d;
            if (iterCount > tabuTable[i]) {
                Dmax = d;
                obj1 = i;
            }
        }
    }
}

void perturb()
{
    perturb_randomDrop();
}

void perturb_randomDrop()
{
    random_device rd;
    mt19937 gen( rd() );
    uniform_int_distribution<> randNode( 0, ObjectNum - 1 );
    uniform_real_distribution<> randCoord( -ContainerRadius, ContainerRadius );

    double radius;
    while (true) {
        int i = randNode( gen );

        double nx = randCoord( gen );
        double ny = randCoord( gen );
        radius = sqrt( nx * nx + ny * ny );
        for (int j = 0; j < ObjectNum; j++) {
            if (i != j) {
                if (dist( nx, ny, x[j], y[j] ) < (r[j] + r[i] / 16)) {
                    radius = ContainerRadius + ACCURACY;
                }
            }
        }

        if (radius <= ContainerRadius) {
            x[i] = nx;
            y[i] = ny;
            break;
        }
    }
}

void perturb_switchPair()
{

}

void solve()
{
    random_device rd;
    mt19937 gen( rd() );
    uniform_int_distribution<> tabuRand( -ObjectNum / 8, ObjectNum / 8 );
    uniform_int_distribution<> enforceRand( 0, MAX_NO_IMPROVE_STEP );


    for (; true; iterCount++) {
        findDmax();

        double u = ((enforceRand( gen ) < DmaxNoChange) ? 1 : 1) * Dmax + ACCURACY;
        if (obj2 == -1) {   // embed in container
            x[obj1] += (x[obj1] > 0 ? -1 : 1) * u / sqrt( 1 + abs( y[obj1] / x[obj1] ) );
            y[obj1] += (y[obj1] > 0 ? -1 : 1) * u / sqrt( 1 + abs( x[obj1] / y[obj1] ) );
        } else {    // embed in other object
            double dx = x[obj1] - x[obj2];
            double dy = y[obj1] - y[obj2];
            double rate = r[obj1] / (r[obj1] + r[obj2]);
            x[obj1] += (dx > 0 ? 1 : -1) * u / sqrt( 1 + abs( dy / dx ) ) * rate;
            y[obj1] += (dy > 0 ? 1 : -1) * u / sqrt( 1 + abs( dx / dy ) ) * rate;
            x[obj2] += (dx > 0 ? -1 : 1) * u / sqrt( 1 + abs( dy / dx ) ) * (1 - rate);
            y[obj2] += (dy > 0 ? -1 : 1) * u / sqrt( 1 + abs( dx / dy ) ) * (1 - rate);
        }
        tabuTable[obj1] = iterCount + ObjectNum / 4 + tabuRand( gen );

        printResult();
        if (DmaxNoTabu < ACCURACY) {
            break;
        } else if (deltaDmax < MERELY_CHANGE) {
            DmaxNoChange++;
        } else {
            DmaxNoChange = 0;
        }

        if (DmaxNoChange > MAX_NO_IMPROVE_STEP) {
            DmaxNoChange = 0;
            perturb();
            continue;
        }
    }
}

void printResult( int waitTime )
{
    emptyImage.copyTo( image );
    circle( image, Point( ORIGIN_SHIFT, ORIGIN_SHIFT ), WINDOW_SIZE / 2, Scalar( 0, 0, 0 ), 1, CV_AA );
    for (int i = 0; i < ObjectNum; i++) {
        circle( image, Point( COORD_EXPAND * x[i] + ORIGIN_SHIFT, COORD_EXPAND * y[i] + ORIGIN_SHIFT ),
            r[i] * COORD_EXPAND, Scalar( 0, 0, 0 ), 1, CV_AA );
    }

    imshow( WINDOW_NAME, image );
    waitKey( waitTime );
}

double dist( double x1, double y1, double x2, double y2 )
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt( dx * dx + dy * dy );
}