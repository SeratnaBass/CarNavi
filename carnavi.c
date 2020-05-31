/*工夫点改良点*/
/*手動入力、再検索機能を追加*/
/*ダイクストラ法で経路探索できるようにした*/
/*探索した最短経路の通過地点を通過順にターミナルに表示した*/
/*次の地点までの距離の桁数を3桁に変更して見やすくした*/
/*目的地、現在地のマーカーの色を変更した*/
/*目的地、現在地、移動マーカーの形状を変更し、交差点の円と区別をつきやすくした*/
/*経路の色を他の道と変えてわかりやすくなるようにした*/
/*移動する車のマーカーの色を変更した*/
/*背景色を変更した*/
/*circle関数にz軸方向の引数を追加した*/
/*マーカーや現在地、目的地のマークをz軸方向にずらして表示することでより立体的な表現にした*/
/*入力される地名が日本語とローマ字のどちらにも対応できるようにした*/
/*座標を変換し、視点が車を中心に見るようにして、マーカーが地図の外に出ないようにした*/
/*地図を拡大してマーカーが通る地点が見やすくなるようにした*/
/*交差点名の文字を小さくして見やすくした*/
/*各地点の円を小さくして見やすくした*/
/*地図をx軸について52度回転させて斜めからの視点に変更した*/
/*z軸領域を拡張して、斜めからみたとき地形の高低差がわかるようにした*/
/*地図の大きさに合わせて車のマーカーを少し小さく調整した*/
/*比較的長さの短い、細かい道を移動している時は、地図を拡大し、長い道を移動している時は、地図を縮小し、車マーカーの位置がわかりやすくなるように場合分けをした*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <GL/glfw.h>
#include <FTGL/ftgl.h>

#define PathNumber 100
#define Radius_Marker 0.2
#define CrossingNumber 100
#define MaxName 50

double ORIGIN_X = 1.0;
double ORIGIN_Y = 1.0;
double REAL_SIZE_X = 3.4;
double REAL_SIZE_Y = 3.5;

typedef struct{
    double x,y;
} Position;
typedef struct{
    int id;
    Position pos;
    double wait;
    char jname[MaxName];
    char ename[MaxName];
    int points;
    int next[5];
    double distance;
} Crossing;

Crossing cross[CrossingNumber];
int path[PathNumber+1];
double cross_distance;
int cross_steps;

/*円を描画する関数,z軸方向にも引数を追加*/
void circle(double x,double y,double z,double r){
    int const N = 12;
    int i;
    glBegin(GL_LINE_LOOP);
    for (i=0; i<N; i++)
    glVertex3d(x+cos(2*M_PI*i/N)*r, y+sin(2*M_PI*i/N)*r,z);
    glEnd();
}

/*形状を変えた円を描くための関数, z軸方向にも引数を追加*/
void circle2(double x, double y, double z, double r){
    int const N = 12;
    int i;
    glBegin(GL_TRIANGLES);
    for (i=0; i<N; i++)
    glVertex3d(x+cos(2*M_PI*i/N)*r, y+sin(2*M_PI*i/N)*r,z);
    glEnd();
}

#ifndef FONT_FILENAME
#define FONT_FILENAME "/usr/share/fonts/truetype/takao-gothic/TakaoGothic.ttf"
#endif
FTGLfont *font;

void outtextxy(double x, double y, char const *text){
    double const scale = 0.0038;
    glPushMatrix();
    glTranslated(x,y,0.0);
    glScaled(scale, scale, scale);
    ftglRenderFont(font, text, FTGL_RENDER_ALL);
    glPopMatrix();
}

int map_read(char *filename){
    FILE *fp;
    int i,j;
    int crossing_number;
    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("File %s is not creatied\n", filename);
        return 0;
    }
    fscanf(fp, "%d", &crossing_number);
    for (i=0; i<crossing_number; i++){
        fscanf(fp,"%d,%lf,%lf,%lf,%[^,],%[^,],%d",
        &(cross[i].id),&(cross[i].pos.x),&(cross[i].pos.y),
        &(cross[i].wait),cross[i].jname,cross[i].ename,&(cross[i].points));
        for(j=0; j<cross[i].points; j++){
            fscanf(fp,",%d",&(cross[i].next[j]));
        }
    }
    fclose(fp);
    return crossing_number;
}

void map_show(int crossing_number){
    int i,j,k;
    double x0, y0;
    double x1, y1;
    for (i=0; i<crossing_number; i++){
        x0 = cross[i].pos.x;
        y0 = cross[i].pos.y;
        glColor3d(1.0, 0.5, 0.5);
        circle(x0, y0, 0.0, 0.05);
        glColor3d(1.0, 1.0, 1.0);
        outtextxy(x0, y0, cross[i].jname);
        glColor3d(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        for (j=0; j<cross[i].points; j++){
            k = cross[i].next[j];
            x1 = cross[k].pos.x;
            y1 = cross[k].pos.y;
            glVertex2d(x0, y0);
            glVertex2d(x1, y1);
        }
        glEnd();
    }
}

/*手動入力の関数 再検索機能有り*/
int search_cross(char buff[256],int crossing_number){
    int f = 0;
    int ip_search;
    while(f == 0){
        scanf("%s",buff);
        for(int i=0;i<crossing_number;i++){
            if(strcmp(cross[i].jname,buff) == 0 || strcmp(cross[i].ename,buff) == 0){
                ip_search = i;
                f = 1;
            }
        }
        if(f == 0){
            printf("一致する検索結果が見つかりませんでした\n再検索してください\n");
        }
        if(f ==1){
            break; /*検索結果が一致するまで再検索可能*/
        }
    }
    return ip_search; /*検索された地点の交差点番号を返す*/
}

/*2地点間の距離を与える関数*/
double distance(int id1,int id2){
    return hypot(cross[id1].pos.x - cross[id2].pos.x, cross[id1].pos.y - cross[id2].pos.y);
}

/*ダイクストラ法で最小距離の確定をする関数*/
void dijkstra(int crossing_number,int target){
    int i,j,n;
    double d;
    double min_distance;
    int min_cross = 0;
    int done[CrossingNumber];
    for(i=0;i<crossing_number;i++){
        cross[i].distance=1e100;
        done[i]=0;
    }
    cross[target].distance=0;
    for(i=0;i<crossing_number;i++){
        min_distance=1e100;
        for(j=0;j<crossing_number;j++){
            if(done[j] == 0 && min_distance > cross[j].distance){
            min_distance = cross[j].distance;
            min_cross = j;
            }
        }
        done[min_cross] = 1;
        for(j=0;j<cross[min_cross].points;j++){
            n = cross[min_cross].next[j];
            d = distance(min_cross,n) + cross[min_cross].distance;
            if(cross[n].distance > d){
                cross[n].distance = d;
            }
        }
    }
}

/*経路探索関数*/
int pickup_path(int crossing_number,int start,int goal,int path[],int maxpath){
    int c=start;
    int i,j,n;
    double min_distance;
    int min_cross = 0;
    path[0] = start;
    i=1;
    c=start;
    while(c != goal){
        min_distance = 1e100;
        for(j=0;j<cross[c].points;j++){
            n = cross[c].next[j];
            if(cross[n].distance < min_distance){
                min_distance = cross[n].distance;
                min_cross = cross[c].next[j];
            }
        }
        c=min_cross;
        if((i>1) && (c==path[i-2])){ /*経路探索がうまくいっているかのチェック1*/
            printf("はまりました (%d-%d)。経路探索を断念します\n",
            c,path[i-1]);
            return -1;
        }
        path[i]=c;
        i++;
        if(i==maxpath-1){ /*経路の長さチェック2*/
            printf("経路が長くなり過ぎました。経路探索を断念します\n");
            return -2;
        }
    }
    path[i]=-1;
    return 0;
}

int main(void){
    int crossing_number;
    int ip;
    int js;
    double xv=0.0, yv=0.0;
    int width,height;
    double x0,x1,y0,y1;
    int k0,k1;
    double dist;
    int steps;
    char buff[256];
    int path[20];
    int i;
    int j=0;
    glfwInit();
    glfwOpenWindow(1000, 800, 0, 0, 0, 0, 0, 0, GLFW_WINDOW); /*ウィンドウを大きめにとった*/
    font = ftglCreateExtrudeFont(FONT_FILENAME);
    if(font == NULL){
        perror(FONT_FILENAME);
        fprintf(stderr, "could not load font\n");
        exit(1);
    }
    ftglSetFontFaceSize(font, 24, 24);
    ftglSetFontDepth(font, 0.01);
    ftglSetFontOutset(font, 0, 0.1);
    ftglSetFontCharMap(font, ft_encoding_unicode);
    crossing_number = map_read("map2.dat"); // 任意のdatファイルを読み込む
    if(crossing_number < 0){
        fprintf(stderr, "couldn't read map file\n");
        exit(1);
    }
    for(i=0;i<crossing_number;i++){
        cross[i].distance = 0;
    }
    printf("現在地を入力してください ->");
    int ip_from = search_cross(buff,crossing_number);
    if(ip_from<0){exit(1);}
    printf("目的地を入力してください ->");
    int ip_goal = search_cross(buff,crossing_number);
    if(ip_goal<0){exit(1);}
    dijkstra(crossing_number,ip_goal);
    if(pickup_path(crossing_number,ip_from,ip_goal,path,20) <0)
    return 1;
    printf("経路確定しました\n");
    i=0;
    while(path[i]>=0){ /*経路の交差点名を通る順に表示*/
        printf("%2d %5.3f %s\n", /*経路の各地点までの距離を少数第3位まで表示*/
        i+1,cross[path[i]].distance,cross[path[i]].jname);
        i++;
    }
    ip=ip_from;
    js=0;
    xv=cross[ip].pos.x;
    yv=cross[ip].pos.y;
    for(;;){
        REAL_SIZE_X =3.4;
        REAL_SIZE_Y =3.5;
        if(cross[ip].distance - cross[path[j]].distance <= 1.0){ /*地図の細かさによって拡大縮小を場合分け*/
            REAL_SIZE_X =2.0;
            REAL_SIZE_Y =2.0;
        }
        ORIGIN_X = xv; /*座標変換パラメータを逐次変換*/
        ORIGIN_Y = yv;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glRotated(52,1.0,0.0,0.0); /*空間の回転,斜めから見るように*/
        glOrtho(ORIGIN_X + REAL_SIZE_X * -0.5, ORIGIN_X + REAL_SIZE_X * 0.5, ORIGIN_Y + REAL_SIZE_Y * -0.5, ORIGIN_Y + REAL_SIZE_Y * 0.5, -10.0, 10.0); /*z軸領域の拡張, 3次元の表現*/
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        /*glfwWaitEvents();*/
        if (glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
        break;
        glfwGetWindowSize(&width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.2f, 0.17f, 0.0f); /*背景色の変更*/
        glClear(GL_COLOR_BUFFER_BIT);
        map_show(crossing_number);
        glColor3d(1.0,0.0,0.0);
        circle2(cross[ip_from].pos.x,cross[ip_from].pos.y,0.5,0.15); /*現在地の色,形状変え,z軸方向に少し浮かせる*/
        glColor3d(0.0,1.0,0.0);
        circle2(cross[ip_goal].pos.x,cross[ip_goal].pos.y,0.5,0.15); /*目的地の色,形状変え,z軸方向に少し浮かせる*/
        glColor3d(0.6,1.0,0.6); /*経路の色変え*/
        glBegin(GL_LINES);
        i=1;
        while(path[i]>=0){
            glVertex2d(cross[path[i-1]].pos.x, cross[path[i-1]].pos.y);
            glVertex2d(cross[path[i]].pos.x, cross[path[i]].pos.y);
            i++;
        }
        glEnd();
        if(ip!=ip_goal){
            k0=ip;
            k1=path[j];
            x0=cross[k0].pos.x;
            y0=cross[k0].pos.y;
            x1=cross[k1].pos.x;
            y1=cross[k1].pos.y;
            dist = hypot(x1-x0, y1-y0);
            steps = (int)(dist/0.1);
            js++;
            xv = x0+(x1-x0)/steps*js;
            yv = y0+(y1-y0)/steps*js;
            if(js>=steps){
                ip=k1;
                js=0;
                j++;
            }
        }
        glColor3d(0.0,0.16,1.0); /*車のマーカーの色を変更, z軸方向に少し浮かせる*/
        circle2(xv,yv,0.6,Radius_Marker * 0.5); /*マーカーの形状変更,地図の大きさに合わせてマーカーの大きさを調整*/
        glfwSwapBuffers();
        usleep(50*1000);
    }
    glfwTerminate();
    return 0;
}