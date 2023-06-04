#include "application_ui.h"
#include "SDL2_gfxPrimitives.h"
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <ctime>
using namespace std;

#define EPSILON 0.0001f

// Définissez VORONOI en fonction de ce que vous voulez voir
// 0 : Triangles Delaunay
// 1 : Diagramme Voronoi avec segments
// 2 : Diagramme Voronoi avec polygones
#define VORONOI 2

struct Coords {
    int x, y;

    bool operator==(const Coords& other) {
        return x == other.x and y == other.y;
    }
};

struct Segment {
    Coords p1, p2;
};

struct Triangle {
    Coords p1, p2, p3;
    bool complet=false;
};

struct Polygon {
    Coords p1, p2, p3;
    int r, g, b;
};

struct Application {
    int width, height;
    Coords focus{100, 100};

    vector<Coords> points;
    vector<Coords> centers;
    vector<Segment> edges;
    vector<Triangle> triangles;
    vector<Polygon> polygons;
};


void swapTriangles(vector<Triangle>& triangles, int i, int j) {
    swap(triangles[i], triangles[j]);
}

bool compareCoords(Coords point1, Coords point2) {
    if (point1.y == point2.y)
        return point1.x < point2.x;
    return point1.y < point2.y;
}

void drawPoints(SDL_Renderer *renderer, const vector<Coords> &points, int r, int g, int b) {
    for (size_t i = 0; i < points.size(); i++) {
        filledCircleRGBA(renderer, points[i].x, points[i].y, 3, r, g, b, SDL_ALPHA_OPAQUE);
    }
}

void drawSegments(SDL_Renderer *renderer, const vector<Segment> &segments) {
    for (size_t i = 0; i < segments.size(); i++) {
        lineRGBA(
            renderer,
            segments[i].p1.x, segments[i].p1.y,
            segments[i].p2.x, segments[i].p2.y,
            240, 240, 20, SDL_ALPHA_OPAQUE);
    }
}

void drawEdges(SDL_Renderer *renderer, const vector<Segment> &edges) {
    for (size_t i = 0; i < edges.size(); i++)     {
        lineRGBA(
            renderer,
            edges[i].p1.x, edges[i].p1.y,
            edges[i].p2.x, edges[i].p2.y,
            240, 240, 20, SDL_ALPHA_OPAQUE);
    }
}

void drawTriangles(SDL_Renderer *renderer, const vector<Triangle> &triangles) {
    for (size_t i = 0; i < triangles.size(); i++) {
        const Triangle& t = triangles[i];
        trigonRGBA(
            renderer,
            t.p1.x, t.p1.y,
            t.p2.x, t.p2.y,
            t.p3.x, t.p3.y,
            0, 240, 160, SDL_ALPHA_OPAQUE
        );
    }
}

void drawPolygons(SDL_Renderer *renderer, const vector<Polygon> &polygons) {
    for (size_t i = 0; i < polygons.size(); i++) {
        const Polygon& p = polygons[i];
        filledTrigonRGBA(
            renderer,
            p.p1.x, p.p1.y,
            p.p2.x, p.p2.y,
            p.p3.x, p.p3.y,
            p.r, p.g, p.b, SDL_ALPHA_OPAQUE
        );
    }
}

void draw(SDL_Renderer *renderer, const Application &app) {
    /* Remplissez cette fonction pour faire l'affichage du jeu */
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    if (VORONOI == 0) {
        drawTriangles(renderer, app.triangles);
    }  
    else if (VORONOI == 1) {
        drawTriangles(renderer, app.triangles);
        drawPoints(renderer, app.centers, 255, 0, 0);
        drawEdges(renderer, app.edges);
    }
    else if (VORONOI == 2) {
        drawPolygons(renderer, app.polygons);
    }
    drawPoints(renderer, app.points, 240, 240, 23);
}

/*
   Détermine si un point se trouve dans un cercle définit par trois points
   Retourne, par les paramètres, le centre et le rayon
*/
bool CircumCircle(
    float pX, float pY,
    float x1, float y1, float x2, float y2, float x3, float y3,
    float *xc, float *yc, float *rsqr
)
{
    float m1, m2, mx1, mx2, my1, my2;
    float dx, dy, drsqr;
    float fabsy1y2 = fabs(y1 - y2);
    float fabsy2y3 = fabs(y2 - y3);

    /* Check for coincident points */
    if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON)
        return (false);

    if (fabsy1y2 < EPSILON)
    {
        m2 = -(x3 - x2) / (y3 - y2);
        mx2 = (x2 + x3) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (x2 + x1) / 2.0;
        *yc = m2 * (*xc - mx2) + my2;
    }
    else if (fabsy2y3 < EPSILON)
    {
        m1 = -(x2 - x1) / (y2 - y1);
        mx1 = (x1 + x2) / 2.0;
        my1 = (y1 + y2) / 2.0;
        *xc = (x3 + x2) / 2.0;
        *yc = m1 * (*xc - mx1) + my1;
    }
    else
    {
        m1 = -(x2 - x1) / (y2 - y1);
        m2 = -(x3 - x2) / (y3 - y2);
        mx1 = (x1 + x2) / 2.0;
        mx2 = (x2 + x3) / 2.0;
        my1 = (y1 + y2) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
        if (fabsy1y2 > fabsy2y3)
        {
            *yc = m1 * (*xc - mx1) + my1;
        }
        else
        {
            *yc = m2 * (*xc - mx2) + my2;
        }
    }

    dx = x2 - *xc;
    dy = y2 - *yc;
    *rsqr = dx * dx + dy * dy;

    dx = pX - *xc;
    dy = pY - *yc;
    drsqr = dx * dx + dy * dy;

    return ((drsqr - *rsqr) <= EPSILON ? true : false);
}

void construitVoronoi(Application &app) {
    srand((unsigned) time(NULL));
    sort(app.points.begin(), app.points.end(), compareCoords);

    // Vider la liste existante de triangles
    app.triangles.clear();

    // Créer un très grand triangle et le rajouter à la liste de triangles déjà créés
    Triangle grandTriangle = {{-1000, -1000}, {500, 3000}, {1500, -1000}};
    app.triangles.push_back(grandTriangle);

    for(size_t i = 0; i < app.points.size(); i++){

        // ******************** //
        // *TRIANGLES DELAUNAY* //
        // ******************** //

        // Créer une liste de segments LS
        vector<Segment> LS;

        // Pour chaque triangle T déjà créé :
        for(size_t j = 0; j < app.triangles.size(); j++){
            // Tester si le cercle circonscrit contient le point P
            float posxCentre, posyCentre, rayon;          
            if (CircumCircle(app.points[i].x, app.points[i].y, 
                            app.triangles[j].p1.x, app.triangles[j].p1.y, 
                            app.triangles[j].p2.x, app.triangles[j].p2.y, 
                            app.triangles[j].p3.x, app.triangles[j].p3.y,
                            &posxCentre, &posyCentre, &rayon)) {
                // Récupérer les différents segments de ce triangle dans LS
                Segment s1 = {{app.triangles[j].p1.x, app.triangles[j].p1.y},{app.triangles[j].p2.x,app.triangles[j].p2.y}};
                Segment s2 = {{app.triangles[j].p2.x, app.triangles[j].p2.y},{app.triangles[j].p3.x,app.triangles[j].p3.y}};
                Segment s3 = {{app.triangles[j].p3.x, app.triangles[j].p3.y},{app.triangles[j].p1.x,app.triangles[j].p1.y}};
                LS.push_back(s1);
                LS.push_back(s2);
                LS.push_back(s3);

                // Enlever le triangle T de la liste 
                app.triangles.erase(app.triangles.begin() + j);
                j--;
            }
        }

        // Pour chaque segment S de la liste LS
        for (size_t k = 0; k < LS.size(); k++) {
            for (size_t j = 0; j < LS.size(); j++) {
                // Si un segment partage ses points avec un autre segment, les virer
                if(k == j) break;
                    
                if (LS[k].p1 == LS[j].p2 && LS[k].p2 == LS[j].p1) {
                   LS.erase(LS.begin() + j);
                   j--;
                   k--;
                   LS.erase(LS.begin() + k);
                } 
            }
        }

        // Pour chaque segment S de la liste LS
        for(size_t l = 0; l < LS.size(); l++) {
            // Créer un nouveau triangle composé du segment S et du point P
            Triangle newTriangle = {LS[l].p1, LS[l].p2, app.points[i]};
            app.triangles.push_back(newTriangle);
        }  

        // Vider la liste existante de centres
        app.centers.clear();

        // Pour chaque triangle T déjà créé :
        for(size_t m = 0; m < app.triangles.size(); m++) {
            // On récupère les coordonnées du centre de son cercle circonscrit
            float posxCentre, posyCentre, rayon;
            CircumCircle(app.points[i].x, app.points[i].y,
                        app.triangles[m].p1.x, app.triangles[m].p1.y, 
                        app.triangles[m].p2.x, app.triangles[m].p2.y, 
                        app.triangles[m].p3.x, app.triangles[m].p3.y,
                        &posxCentre, &posyCentre, &rayon);

            Coords centre = {posxCentre, posyCentre};
            app.centers.push_back(centre);
        }

        // ******************************* //
        // *DIAGRAMME VORONOI AVEC LIGNES* //
        // ******************************* //
        
        if (VORONOI == 1) {
            // Vider la liste existante de edges
            app.edges.clear();

            // Pour chaque triangle T déjà créé :
            for(size_t n = 0; n < app.triangles.size(); n++) {
                // Vérifiez quels sont les triangles adjacents à celui-ci
                for(size_t o = 0; o < app.triangles.size(); o++) {
                    if (n == o) break;

                    int sharedPoints = 0;

                    if (app.triangles[n].p1 == app.triangles[o].p1 || 
                        app.triangles[n].p1 == app.triangles[o].p2 || 
                        app.triangles[n].p1 == app.triangles[o].p3)
                        sharedPoints++;

                    if (app.triangles[n].p2 == app.triangles[o].p1 || 
                        app.triangles[n].p2 == app.triangles[o].p2 || 
                        app.triangles[n].p2 == app.triangles[o].p3) 
                        sharedPoints++;

                    if (app.triangles[n].p3 == app.triangles[o].p1 || 
                        app.triangles[n].p3 == app.triangles[o].p2 || 
                        app.triangles[n].p3 == app.triangles[o].p3) 
                        sharedPoints++;

                    // Si les triangles sont adjacents alors on relie leurs centres
                    if (sharedPoints == 2) {
                        app.edges.push_back({{app.centers[n].x, app.centers[n].y}, {app.centers[o].x, app.centers[o].y}});
                    }
                }  
            } 
        }

        // ********************************** //
        // *DIAGRAMME VORONOI AVEC POLYGONES* //
        // ********************************** //

        else if (VORONOI == 2) {
            // Liste des centres des cercles circonscrits
            vector<Coords> LC;

            // Parcourir tous les triangles
            for (size_t p = 0; p < app.triangles.size(); p++) {
                // Vérifier si le point app.points[i] est présent dans le triangle
                if (app.triangles[p].p1 == app.points[i] || app.triangles[p].p2 == app.points[i] || app.triangles[p].p3 == app.points[i]) {
                    // Vérifier si le triangle est adjacent à d'autres triangles
                    bool isAdjacent = false;
                    
                    // Parcourir à nouveau tous les triangles pour les comparer
                    for (size_t q = 0; q < app.triangles.size(); q++) {
                        // Ignorer le triangle courant
                        if (&app.triangles[p] == &app.triangles[q]) {
                            continue;
                        }
                        
                        // Vérifier si le point app.points[i] est présent dans l'autre triangle
                        if (app.triangles[q].p1 == app.points[i] || app.triangles[q].p2 == app.points[i] || app.triangles[q].p3 == app.points[i]) {
                            // Vérifier si les triangles ont des arêtes communes
                            if ((app.triangles[p].p1 == app.triangles[q].p2 || app.triangles[p].p1 == app.triangles[q].p3 ||
                                app.triangles[p].p2 == app.triangles[q].p1 || app.triangles[p].p2 == app.triangles[q].p3 ||
                                app.triangles[p].p3 == app.triangles[q].p1 || app.triangles[p].p3 == app.triangles[q].p2)) {
                                isAdjacent = true;
                                break;
                            }
                        }
                    }
                    
                    // Si le triangle est adjacent, l'ajouter à la liste
                    if (isAdjacent) {
                        LC.push_back(app.centers[p]);
                    }
                }
            }

            // Couleur aléatoire
            int r = rand() % 256;
            int g = rand() % 256;
            int b = rand() % 256;

            // Parcourir les centres
            for (size_t j = 0; j < LC.size(); j++) {
                Coords& center1 = LC[j];

                // Parcourir les autres centres
                for (size_t k = 0; k < LC.size(); k++) {
                    if (j != k) {
                        Coords& center2 = LC[k];

                        // Créer un triangle avec les centres et le point
                        Polygon polygon = {app.points[i], center1, center2, r, g, b};
                        
                        // Ajouter le triangle à la liste des polygons
                        app.polygons.push_back(polygon);
                    }
                }
            } 
        }
    }
}

bool handleEvent(Application &app) {
    /* Remplissez cette fonction pour gérer les inputs utilisateurs */
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return false;
        else if (e.type == SDL_WINDOWEVENT_RESIZED) {
            app.width = e.window.data1;
            app.height = e.window.data1;
        }
        else if (e.type == SDL_MOUSEWHEEL) {
        }
        else if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_RIGHT) {
                app.focus.x = e.button.x;
                app.focus.y = e.button.y;
                app.points.clear();
            }
            else if (e.button.button == SDL_BUTTON_LEFT) {
                app.focus.y = 0;
                app.points.push_back(Coords{e.button.x, e.button.y});
                construitVoronoi(app);
            }
        }
    }
    return true;
}

int main(int argc, char **argv) {
    SDL_Window *gWindow;
    SDL_Renderer *renderer;
    Application app{720, 720, Coords{0, 0}};
    bool is_running = true;

    // Creation de la fenetre
    gWindow = init("Awesome Voronoi", 720, 720);

    if (!gWindow) {
        SDL_Log("Failed to initialize!\n");
        exit(1);
    }

    renderer = SDL_CreateRenderer(gWindow, -1, 0); // SDL_RENDERER_PRESENTVSYNC

    /*  GAME LOOP  */
    while (true) {
        // INPUTS
        is_running = handleEvent(app);
        if (!is_running)
            break;

        // EFFACAGE FRAME
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // DESSIN
        draw(renderer, app);

        // VALIDATION FRAME
        SDL_RenderPresent(renderer);

        // PAUSE en ms
        SDL_Delay(1000 / 30);
    }

    // Free resources and close SDL
    close(gWindow, renderer);

    return 0;
}
