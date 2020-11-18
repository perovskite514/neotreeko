#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#define N (2300 * 2300 * 3)

struct queue
{
    int data[N];
    int head;
    int tail;
};

struct queue check;

void initialize(struct queue *q)
{
    int i;

    q->head = 0;
    q->tail = 0;
    for (i = 0; i < N; ++i)
    {
        q->data[i] = 0;
    }
}

void enqueue(struct queue *q, int item)
{
    q->data[q->tail] = item;
    q->tail++;
}

int dequeue(struct queue *q)
{
    int tmp = q->data[q->head];
    q->head++;
    return tmp;
} 

int board_size = 5;
//場の状態(黒ゴマが－1、白ゴマが1)
int board[5][5] = {{0, 1, 0, 1, 0}, {0, 0, -1, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, -1, 0, -1, 0}};

//終了判定に使う
int move[4][2][2] = {{{1, 0}, {-1, 0}}, {{0, 1}, {0, -1}}, {{1, 1}, {-1, -1}}, {{1, -1}, {-1, 1}}};

//コマの動く方角8個
int komamove[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

//2300 = 25*24*23/3/2/1
int banmen[2300][5][5] = {};        //盤面
int s[25][25][25] = {};             //盤面を表す1~2300と駒の位置をつなぐ(コマの位置から盤面の番号がわかる)
int t[2300][3] = {};                //0~3に、コマは3ついるが、それぞれの位置(0~24)を入れてる(盤面の番号からコマの位置がわかる)
int shouhai[2300] = {};             //勝敗を入れる
int allbanmen[2300][2300] = {{-1}}; //次に第一引数側のプレイヤーが動かしたときにそのプレイヤーが勝つかどうか(勝ち:1,負け:0,未探索(最終的には引き分け):-1,(到達不可能盤面-100))
int p[3] = {};
int main()
{

    int d = 0;
    for (int a = 0; a < 25 - 2; a++)
    {
        for (int b = a + 1; b < 25 - 1; b++)
        {
            for (int c = b + 1; c < 25; c++)
            {

                banmen[d][a % 5][a / 5] = 1; //a, b, cでそれぞれの駒の位置
                banmen[d][b % 5][b / 5] = 1;
                banmen[d][c % 5][c / 5] = 1;
                if ((a % 5) + (c % 5) == 2 * (b % 5) && (a / 5) + (c / 5) == 2 * (b / 5))
                {
                    if ((a % 5) - (c % 5) <= 2 && (c % 5) - (a % 5) <= 2 && (a / 5) - (c / 5) <= 2 && (c / 5) - (a / 5) <= 2) //上のifと合わせて３こ揃っている条件(a<b<c)より必ずbが真ん中
                    {
                        shouhai[d] = 1; //1:勝利
                    }
                }
                s[a][b][c] = d;
                s[a][c][b] = d;
                s[b][a][c] = d;
                s[b][c][a] = d;
                s[c][a][b] = d;
                s[c][b][a] = d; //各コマの位置から盤面がわかるように
                t[d][0] = a;
                t[d][1] = b;
                t[d][2] = c; //盤面の番号からコマの位置がわかるように
                d++;
            }
        }
    }

    for (int a = 0; a < 2300; a++)
    {
        for (int b = 0; b < 2300; b++)
        {
            allbanmen[a][b] = -1;
        }
    }

    initialize(&check);
    for (int a = 0; a < 2300; a++)
    {
        for (int b = 0; b < 2300; b++)
        {
            if (shouhai[b] == 1 && shouhai[a] == 1)
            {
                allbanmen[a][b] = -100; //到達不可能
            }
            else if (banmen[a][t[b][0] % 5][t[b][0] / 5] == 1 || banmen[a][t[b][1] % 5][t[b][1] / 5] == 1 || banmen[a][t[b][2] % 5][t[b][2] / 5] == 1)
            {
                allbanmen[a][b] = -100; //重なってる場合アウト
            }
            else if (shouhai[a] == 1)
            {
                allbanmen[a][b] = -100; //(allbanmenの定義は「次に第一引数側のプレイヤーが動かしたときにそのプレイヤーが勝つかどうか」なのですでに勝った状態で次にaが動かすことはあり得ないため)
            }
            else if (shouhai[b] == 1)
            {
                allbanmen[a][b] = 0; //負け(allbanmenの定義は「次に第一引数側のプレイヤーが動かしたときにそのプレイヤーが勝つかどうか」なので次動かすはずのaは負けている)
                enqueue(&check, a);
                enqueue(&check, b);
                enqueue(&check, 0);
                //0:一手戻ったら勝ち盤面になる(今負け盤面),1:一手戻ったら負け盤面になる可能性がある(今勝ち盤面)
            }
        }
    }
    
    int v[30] = {};
    for(int j = 0; j <= 30; j++)
    {
        while ((check.tail - check.head) > 0)
        {
            int a = dequeue(&check);
            int b = dequeue(&check);
            int num = dequeue(&check);
            if (num == 0) //一手戻ったら勝ち盤面(bが揃っている)
            {
                for (int k = 0; k < 3; k++) //3つあるコマのうちどれを動かすか
                {
                    int d = t[b][k] % 5;
                    int e = t[b][k] / 5; //d,eがコマの位置
                    for (int f = -1; f < 2; f++)
                    {
                        for (int g = -1; g < 2; g++)
                        {                                                                                                                                                                                                                                                                                                                                                //f,gが方角
                            if (f * f + g * g > 0 && d + f >= 0 && d + f < 5 && e + g >= 0 && e + g < 5 && banmen[a][d + f][e + g] == 0 && banmen[b][d + f][e + g] == 0 && (d + (f * (-1)) < 0 || d + (f * (-1)) >= 5 || e + (g * (-1)) < 0 || e + (g * (-1)) >= 5 || banmen[a][d + (f * (-1))][e + (g * (-1))] == 1 || banmen[b][d + (f * (-1))][e + (g * (-1))] == 1)) //&& (!stopped))//まず一マス進めるか(戻れるか)最後のカッコは戻りたい方向の逆に壁があるかどうか(orは最初にTrueが出たら終わるのでIndex Errorは出ないはず)
                            {
                                int f1[5] = {f, 0, 0, 0, 0};
                                int g1[5] = {g, 0, 0, 0, 0};
                                int count = 0;
                                while (d + f1[count] + f >= 0 && d + f1[count] + f < 5 && e + g1[count] + g >= 0 && e + g1[count] + g < 5 && banmen[a][d + f1[count] + f][e + g1[count] + g] == 0 && banmen[b][d + f1[count] + f][e + g1[count] + g] == 0) //最長でどこまで戻れるか
                                {
                                    f1[count + 1] = f1[count] + f;
                                    g1[count + 1] = g1[count] + g;
                                    count++;
                                }
                                for (int index = 0; index < count; index++)
                                {
                                    for (int h = 0; h < 3; h++)
                                    {
                                        p[h] = t[b][h];
                                    }
                                    p[k] = d + f1[index] + 5 * (e + g1[index]);
                                    int hh = s[p[0]][p[1]][p[2]];                //戻った盤面の番号
                                    if(allbanmen[hh][a] == 1) continue;
                                    if(allbanmen[hh][a] == -100) continue;
                                    allbanmen[hh][a] = 1;
                                    enqueue(&check, hh);
                                    enqueue(&check, a);
                                    enqueue(&check, 1);
                                }
                            }
                        }
                    }
                }
            }
        //(一手戻って,そこからどう動かしても1にしか行けないなら0,それ自体が1ならスルー(1と上書きするでもOK),不明なところがあるならそのまま-1)
            else //一手戻ったら勝ち盤面(bが揃っている)
            {
                for (int k = 0; k < 3; k++) //3つあるコマのうちどれを動かすか
                {
                    int d = t[b][k] % 5;
                    int e = t[b][k] / 5; //d,eがコマの位置
                    for (int f = -1; f < 2; f++)
                    {
                        for (int g = -1; g < 2; g++)
                        {                                                                                                                                                                                                                                                                                                                                                //f,gが方角
                            if (f * f + g * g > 0 && d + f >= 0 && d + f < 5 && e + g >= 0 && e + g < 5 && banmen[a][d + f][e + g] == 0 && banmen[b][d + f][e + g] == 0 && (d + (f * (-1)) < 0 || d + (f * (-1)) >= 5 || e + (g * (-1)) < 0 || e + (g * (-1)) >= 5 || banmen[a][d + (f * (-1))][e + (g * (-1))] == 1 || banmen[b][d + (f * (-1))][e + (g * (-1))] == 1)) //&& (!stopped))//まず一マス進めるか(戻れるか)最後のカッコは戻りたい方向の逆に壁があるかどうか(orは最初にTrueが出たら終わるのでIndex Errorは出ないはず)
                            {
                                int f1[5] = {f, 0, 0, 0, 0};
                                int g1[5] = {g, 0, 0, 0, 0};
                                int count = 0;
                                while (d + f1[count] + f >= 0 && d + f1[count] + f < 5 && e + g1[count] + g >= 0 && e + g1[count] + g < 5 && banmen[a][d + f1[count] + f][e + g1[count] + g] == 0 && banmen[b][d + f1[count] + f][e + g1[count] + g] == 0) //最長でどこまで戻れるか
                                {
                                    f1[count + 1] = f1[count] + f;
                                    g1[count + 1] = g1[count] + g;
                                    count++;
                                }
                                for (int index = 0; index < count; index++)
                                {
                                    for (int h = 0; h < 3; h++)
                                    {
                                        p[h] = t[b][h];
                                    }
                                    p[k] = d + f1[index] + 5 * (e + g1[index]); //行き先
                                    int hh = s[p[0]][p[1]][p[2]];                //戻った盤面の番号

                                    if (allbanmen[hh][a] != -1) continue; //未探索だけ、その盤面からの順移動の遷移先を見ます。
                                    int fl = 0;   // フラグ変数
                                    int aa = a;
                                    int bb = hh; 
                                // 逆移動じゃなくて順移動を書く(逆側の壁を考えない＆＆最長移動だけ見る。)
                                    for (int kk = 0; kk < 3; kk++) //3つあるコマのうちどれを動かすか
                                    {
                                        int dd = t[bb][kk] % 5;
                                        int ee = t[bb][kk] / 5; //d,eがコマの位置
                                        for (int ff = -1; ff < 2; ff++)
                                        {
                                            for (int gg = -1; gg < 2; gg++)
                                            {
                                                if (ff * ff + gg * gg > 0 && dd + ff >= 0 && dd + ff < 5 && ee + gg >= 0 && ee + gg < 5 && banmen[aa][dd + ff][ee + gg] == 0 && banmen[bb][dd + ff][ee + gg] == 0)
                                                {
                                                    int f2[5] = {ff, 0, 0, 0, 0};
                                                    int g2[5] = {gg, 0, 0, 0, 0};
                                                    int co = 0;
                                                    while (dd + f2[co] + ff >= 0 && dd + f2[co] + ff < 5 && ee + g2[co] + gg >= 0 && ee + g2[co] + gg < 5 && banmen[aa][dd + f2[co] + ff][ee + g2[co] + gg] == 0 && banmen[bb][dd + f2[co] + ff][ee + g2[co] + gg] == 0) //最長でどこまで戻れるか
                                                    {
                                                        f2[co + 1] = f2[co] + ff;
                                                        g2[co + 1] = g2[co] + gg;
                                                        co++;
                                                    }

                                                    int pp[3] = {};
                                                    for (int h = 0; h < 3; h++)
                                                    {
                                                        pp[h] = t[bb][h];
                                                    }
                                                    pp[kk] = dd + f2[co] + 5 * (ee + g2[co]); //行き先
                                                    int hhh = s[pp[0]][pp[1]][pp[2]];
                                                    if (allbanmen[aa][hhh] == 0)
                                                    {
                                                        allbanmen[hh][a] = 1;
                                                        fl = 1;
                                                        goto end; // ループからの脱出にgoto文を用いる。
                                                    }

                                                    if (allbanmen[aa][hhh] == -1)
                                                    {
                                                        fl = 2;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    end:
                                    if (fl == 0) // fl = 0は遷移可能先が1だけだった状態なのでこの盤面は負け確
                                    {
                                        allbanmen[hh][a] = 0;
                                        enqueue(&check, hh);
                                        enqueue(&check, a);
                                        enqueue(&check, 0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
  
        for(int i = 0; i < 2300; i++)
        {
            for(int j = 0; j < 2300; j++)
            {
                if(allbanmen[i][j] != -1) continue;
                int a = i;
                int b = j;
                int fl = 0;
                int fla = 0;
                for (int kk = 0; kk < 3; kk++) //3つあるコマのうちどれを動かすか
                {
                    int dd = t[a][kk] % 5;
                    int ee = t[a][kk] / 5; //d,eがコマの位置 
                    for (int ff = -1; ff < 2; ff++)
                    {
                        for (int gg = -1; gg < 2; gg++)
                        {                                                                                                                                                                                                                                                                                                                                          
                            if (ff * ff + gg * gg > 0 && dd + ff >= 0 && dd + ff < 5 && ee + gg >= 0 && ee + gg < 5 && banmen[a][dd + ff][ee + gg] == 0 && banmen[b][dd + ff][ee + gg] == 0) 
                            {
                                int f2[5] = {ff, 0, 0, 0, 0};
                                int g2[5] = {gg, 0, 0, 0, 0};
                                int co = 0;
                                while (dd + f2[co] + ff >= 0 && dd + f2[co] + ff < 5 && ee + g2[co] + gg >= 0 && ee + g2[co] + gg < 5 && banmen[a][dd + f2[co] + ff][ee + g2[co] + gg] == 0 && banmen[b][dd + f2[co] + ff][ee + g2[co] + gg] == 0) //最長でどこまで戻れるか
                                {
                                    f2[co + 1] = f2[co] + ff;
                                    g2[co + 1] = g2[co] + gg;
                                    co++;
                                }

                                int pp[3]={};
                                for (int h1 = 0; h1 < 3; h1++)
                                {
                                    pp[h1] = t[a][h1];
                                }
                                pp[kk] = dd + f2[co] + 5 * (ee + g2[co]); //行き先
                                int hh = s[pp[0]][pp[1]][pp[2]];
                                if(allbanmen[b][hh] == 0) fl++;
                                if(allbanmen[b][hh] == -1) fla++;
                            }
                        }
                    }
                }
                if(fl != 0)  // fl = 0は遷移可能先が1だけだった状態なのでこの盤面は負け確
                {
                    allbanmen[a][b] = 1;
                    enqueue(&check, a);
                    enqueue(&check, b);
                    enqueue(&check, 1);
                }

                if(fla == 0 && fl == 0)
                {
                    allbanmen[a][b] = 0;
                    enqueue(&check, a);
                    enqueue(&check, b);
                    enqueue(&check, 0);
                }
            }
        }

        int p1 = 0;

        for(int i = 0; i < 2300; i++)
        {
            for(int j = 0; j < 2300; j++)
            {
                if(allbanmen[i][j] == -1) p1++;
            }
        }

        v[j] = p1;
        if(j == 0) continue;
        if(v[j] == v[j-1]) break;
    }
  
    FILE *outputfile; 
    outputfile = fopen("data.txt", "w"); 
    if (outputfile == NULL) exit(1);

    for (int i = 0; i < 2300; i++)
    {
        for (int j = 0; j < 2300; j++)
        {
            fprintf(outputfile, "%d\n", allbanmen[i][j]);
        }
    }

    for (int i = 0; i < 2300; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            fprintf(outputfile, "%d\n", t[i][j]); 
        }
    }

    for (int i = 0; i < 25; i++)
    {
        for (int j = 0; j < 25; j++)
        {
            for (int k = 0; k < 25; k++)
            {
                fprintf(outputfile, "%d\n", s[i][j][k]); 
            }
        }
    }

    for (int i = 0; i < 2300; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            for (int k = 0; k < 5; k++)
            {
                fprintf(outputfile, "%d\n", banmen[i][j][k]); 
            }
        }
    }
    
    fclose(outputfile); 
    return 0;
}