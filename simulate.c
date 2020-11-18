#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

int board_size = 5;
//場の状態(黒ゴマが－1、白ゴマが1)
int board[5][5] = {{0, 1, 0, 1, 0}, {0, 0, -1, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, -1, 0, -1, 0}};

//終了判定に使う
int move[4][2][2] = {{{1, 0}, {-1, 0}}, {{0, 1}, {0, -1}}, {{1, 1}, {-1, -1}}, {{1, -1}, {-1, 1}}};

//コマの動く方角8個
int komamove[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

int s[25][25][25] = {};         //盤面を表す1~2300と駒の位置をつなぐ(コマの位置から盤面の番号がわかる)
int t[2300][3] = {};            //0~3に、コマは3ついるが、それぞれの位置(0~24)を入れてる(盤面の番号からコマの位置がわかる)
int allbanmen[2300][2300] = {}; //次に第一引数側のプレイヤーが動かしたときにそのプレイヤーが勝つかどうか(勝ち:1,負け:0,未探索(最終的には引き分け):-1,(到達不可能盤面-100))
int banmen[2300][5][5] = {};    //盤面

//そのマスが移動できるか判定
int movable(int x, int y, int xx, int yy)
{
    if (x + xx >= 0 && x + xx <= 4 && y + yy >= 0 && y + yy <= 4 && board[x + xx][y + yy] == 0)
    {
        return 1;
    }
    return 0;
}

int agent1(int i, int j, int ii, int jj)
{
    if (board[i][j] != -1 || board[ii][jj] != 0) //移動前と後のマスがOKか
    {
        return 0;
    }
    else
    {
        int a = ii - i;
        int b = jj - j;
        int k = 0;

        // 移動する方角を調べる
        if (a > 0 && b == 0)
            k = 0;
        else if (a < 0 && b == 0)
            k = 1;
        else if (a == 0 && b > 0)
            k = 2;
        else if (a == 0 && b < 0)
            k = 3;
        else if (a > 0 && b > 0)
            k = 4;
        else if (a < 0 && b < 0)
            k = 5;
        else if (a > 0 && b < 0)
            k = 6;
        else if (a < 0 && b > 0)
            k = 7;

        int n = 0;
        while (movable(i, j, (n + 1) * komamove[k][0], (n + 1) * komamove[k][1])) //動かせるところまで動かす
        {
            n++;
        }
        //動かせるところまで動かしたマスが移動後のマスと一致しているか判定
        if (i + n * komamove[k][0] != ii || j + n * komamove[k][1] != jj)
        {
            return 0;
        }

        board[i + n * komamove[k][0]][j + n * komamove[k][1]] = board[i][j];
        board[i][j] = 0;
        return 1;
    }
}

int agent2(int i, int j, int ii, int jj)
{
    if (board[i][j] != 1 || board[ii][jj] != 0)
    {
        return 0;
    }
    else
    {
        int a = ii - i;
        int b = jj - j;
        int k = 0;

        if (a > 0 && b == 0)
            k = 0;
        else if (a < 0 && b == 0)
            k = 1;
        else if (a == 0 && b > 0)
            k = 2;
        else if (a == 0 && b < 0)
            k = 3;
        else if (a > 0 && b > 0)
            k = 4;
        else if (a < 0 && b < 0)
            k = 5;
        else if (a > 0 && b < 0)
            k = 6;
        else if (a < 0 && b > 0)
            k = 7;

        int n = 0;
        while (movable(i, j, (n + 1) * komamove[k][0], (n + 1) * komamove[k][1]))
        {
            n++;
        }

        if (i + n * komamove[k][0] != ii || j + n * komamove[k][1] != jj)
        {
            return 0;
        }

        board[i + n * komamove[k][0]][j + n * komamove[k][1]] = board[i][j];
        board[i][j] = 0;
        return 1;
    }
}

//終了判定に使うもの
int range_judge(int i, int j, int moveto[2][2])
{
    for (int ii = 0; ii < 2; ii++)
    {
        if (i + moveto[ii][0] < 0 || i + moveto[ii][0] > 4)
        {
            return 0;
        }
    }
    for (int ii = 0; ii < 2; ii++)
    {
        if (j + moveto[ii][1] < 0 || j + moveto[ii][1] > 4)
        {
            return 0;
        }
    }
    return 1;
}
void printboard()
{
    printf("%s", "\n +---+---+---+---+---+\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d", 5-i);
        printf("%s", "|");
        for (int j = 0; j < 5; j++)
        {
            if(board[i][j] == -1)
              {
                  printf("%s", " o ");
                  printf("%s", "|");
              }
            else if(board[i][j] == 1)
            {
              printf("%s", " x ");
              printf("%s", "|");
            }
            else
            {
              printf("%s", "   ");
              printf("%s", "|");
            }
        }
        printf("%s", "\n +---+---+---+---+---+\n");
    }
    printf("%s", "   A   B   C   D   E\n\n");
}

//終了判定
int fin_judge(int board[5][5])
{
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (board[i][j] != 0)
            {
                for (int k = 0; k < 4; k++)
                {
                    if (range_judge(i, j, move[k]))
                    {
                        if ((board[i][j] + board[i + move[k][0][0]][j + move[k][0][1]] + board[i + move[k][1][0]][j + move[k][1][1]]) * (board[i][j] + board[i + move[k][0][0]][j + move[k][0][1]] + board[i + move[k][1][0]][j + move[k][1][1]]) == 9)
                        {
                            printf("%s%d%d\n", "fin", i, j + 'A');
                            printboard();
                            return (board[i][j]) * (board[i][j]);
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int main()
{
    FILE *fp;
    fp = fopen("data.txt", "r");

    if (fp == NULL)
        exit(1);
    else
    {

        for (int i = 0; i < 2300; i++)
        {
            for (int j = 0; j < 2300; j++)
            {
                fscanf(fp, "%d", &allbanmen[i][j]);
            }
        }

        for (int i = 0; i < 2300; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                fscanf(fp, "%d", &t[i][j]);
            }
        }

        for (int i = 0; i < 25; i++)
        {
            for (int j = 0; j < 25; j++)
            {
                for (int k = 0; k < 25; k++)
                {
                    fscanf(fp, "%d", &s[i][j][k]);
                }
            }
        }

        for (int i = 0; i < 2300; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                for (int k = 0; k < 5; k++)
                {
                    fscanf(fp, "%d", &banmen[i][j][k]);
                }
            }
        }
    }

    fclose(fp);

    int teban;
    scanf("%d", &teban);

    int ca = 0; // 手数のカウンター、300を超えたら終わり。

    printf("%s", "gamestart\n");

    if (teban == 0) //AI戦では人間先攻
    {
        while (1)
        {
            // 300超え引き分け判定
            if (ca == 300)
            {
                printf("HIKIWAKE\n");
                return 0;
            }

            printboard();
            int i, j, ii, jj;
            char a, b;
            scanf("%d%c%d%c", &i, &a, &ii, &b);
            i = 5 - i;
            ii = 5 - ii;
            j = a - 'A';
            jj = b - 'A';
            //ここら辺は入出力のあれこれです。5-iの変換はルールを見るとわかる。盤面のマスの呼び方に合わせて補正をかけています。

            int f3 = agent1(i, j, ii, jj); //返り値を持たせて勝敗判定も兼ねさせています。

            if (f3 == 0)
            {
                printf("You Lose\n");
                return 0;
            }

            if (fin_judge(board))
            {
                printf("You Win\n");
                return 0;
            }
            ca++;

            int p[3] = {};
            int q[3] = {};
            int po = 0, qo = 0;

            for (int i = 0; i < 5; i++)
            {
                for (int j = 0; j < 5; j++)
                {
                    if (board[i][j] == 1)
                    {
                        p[po] = j * 5 + i;
                        po++;
                    }

                    else if (board[i][j] == -1)
                    {
                        q[qo] = j * 5 + i;
                        qo++;
                    }
                }
            }

            //白ゴマを移動
            int d0 = s[p[0]][p[1]][p[2]];
            int d1 = s[q[0]][q[1]][q[2]];
            int kati[100][4] = {};
            int make[100][4] = {};
            int hikiwake[100][4] = {};
            int kpo = 0, mpo = 0, hpo = 0;
            int fin = 0;

            for (int kk = 0; kk < 3; kk++) //3つあるコマのうちどれを動かすか
            {
                int dd = t[d0][kk] % 5;
                int ee = t[d0][kk] / 5; //d,eがコマの位置
                for (int ff = -1; ff < 2; ff++)
                {
                    for (int gg = -1; gg < 2; gg++)
                    {
                        if (ff * ff + gg * gg > 0 && dd + ff >= 0 && dd + ff < 5 && ee + gg >= 0 && ee + gg < 5 && banmen[d0][dd + ff][ee + gg] == 0 && banmen[d1][dd + ff][ee + gg] == 0)
                        {
                            int f2[5] = {ff, 0, 0, 0, 0};
                            int g2[5] = {gg, 0, 0, 0, 0};
                            int co = 0;
                            while (dd + f2[co] + ff >= 0 && dd + f2[co] + ff < 5 && ee + g2[co] + gg >= 0 && ee + g2[co] + gg < 5 && banmen[d0][dd + f2[co] + ff][ee + g2[co] + gg] == 0 && banmen[d1][dd + f2[co] + ff][ee + g2[co] + gg] == 0) //最長でどこまで戻れるか
                            {
                                f2[co + 1] = f2[co] + ff;
                                g2[co + 1] = g2[co] + gg;
                                co++;
                            }

                            int pp[3] = {};

                            for (int h = 0; h < 3; h++)
                            {
                                pp[h] = t[d0][h];
                            }

                            pp[kk] = dd + f2[co] + 5 * (ee + g2[co]); //行き先
                            int hh = s[pp[0]][pp[1]][pp[2]];
                            if (allbanmen[d1][hh] == 0 && fin == 0)
                            {
                                board[dd][ee] = 0;
                                board[dd+f2[co]][ee+g2[co]] = 1;
                                if(fin_judge(board))
                                {
                                    fin = 1;
                                    kati[0][0] = dd;
                                    kati[0][1] = ee;
                                    kati[0][2] = dd + f2[co];
                                    kati[0][3] = ee + g2[co];
                                    kpo = 1;
                                }
                                else
                                {
                                    kati[kpo][0] = dd;
                                    kati[kpo][1] = ee;
                                    kati[kpo][2] = dd + f2[co];
                                    kati[kpo][3] = ee + g2[co];
                                    kpo++;
                                }

                                board[dd][ee] = 1;
                                board[dd+f2[co]][ee+g2[co]] = 0;
                            }

                            if (allbanmen[d1][hh] == 1)
                            {
                                make[mpo][0] = dd;
                                make[mpo][1] = ee;
                                make[mpo][2] = dd + f2[co];
                                make[mpo][3] = ee + g2[co];
                                mpo++;
                            }

                            if (allbanmen[d1][hh] == -1)
                            {
                                hikiwake[hpo][0] = dd;
                                hikiwake[hpo][1] = ee;
                                hikiwake[hpo][2] = dd + f2[co];
                                hikiwake[hpo][3] = ee + g2[co];
                                hpo++;
                            }
                        }
                    }
                }
            }

            int l, k, ll, kk;
            int ran;
            if(kpo != 0)
            {
                if(kpo == 1)
                {
                    l = kati[0][0];
                    k = kati[0][1];
                    ll = kati[0][2];
                    kk = kati[0][3];
                }
                else
                {
                ran = rand() % kpo;
                l = kati[ran][0];
                k = kati[ran][1];
                ll = kati[ran][2];
                kk = kati[ran][3];
                }
            }


            else if (hpo != 0)
            {
                ran = rand() % hpo;
                l = hikiwake[ran][0];
                k = hikiwake[ran][1];
                ll = hikiwake[ran][2];
                kk = hikiwake[ran][3];
            }

            else
            {
                ran = 0;
                l = make[ran][0];
                k = make[ran][1];
                ll = make[ran][2];
                kk = make[ran][3];
            }

            printboard();
            int f4 = agent2(l, k, ll, kk);
            printf("AI ");
            char c,d;
            l = 5 - l;
            ll = 5 - ll;
            c = k + 'A';
            d = kk + 'A';
            printf("%d%c%d%c\n", l, c, ll, d);

            if (f4 == 0)
            {
                printf("You Win\n");
                return 0;
            }
            if (fin_judge(board))
            {
                printf("You Lose\n");
                return 0;
            }
            ca++;
        }
    }

    else //後手、他はほぼ一緒
    {
        while (1)
        {
            // 300超え引き分け判定
            if (ca == 300)
            {
                printf("HIKIWAKE\n");
                return 0;
            }

            int p[3] = {};
            int q[3] = {};
            int po = 0, qo = 0;
            for (int i = 0; i < 5; i++)
            {
                for (int j = 0; j < 5; j++)
                {
                    if (board[i][j] == -1)
                    {
                        p[po] = j * 5 + i;
                        po++;
                    }

                    else if (board[i][j] == 1)
                    {
                        q[qo] = j * 5 + i;
                        qo++;
                    }
                }
            }

            //黒ゴマを移動
            int d0 = s[p[0]][p[1]][p[2]];
            int d1 = s[q[0]][q[1]][q[2]];
            int kati[100][4] = {};
            int make[100][4] = {};
            int hikiwake[100][4] = {};
            int kpo = 0, mpo = 0, hpo = 0;
            int fin = 0;
            for (int kk = 0; kk < 3; kk++) //3つあるコマのうちどれを動かすか
            {
                int dd = t[d0][kk] % 5;
                int ee = t[d0][kk] / 5; //d,eがコマの位置
                for (int ff = -1; ff < 2; ff++)
                {
                    for (int gg = -1; gg < 2; gg++)
                    {
                        if (ff * ff + gg * gg > 0 && dd + ff >= 0 && dd + ff < 5 && ee + gg >= 0 && ee + gg < 5 && banmen[d0][dd + ff][ee + gg] == 0 && banmen[d1][dd + ff][ee + gg] == 0)
                        {
                            int f2[5] = {ff, 0, 0, 0, 0};
                            int g2[5] = {gg, 0, 0, 0, 0};
                            int co = 0;
                            while (dd + f2[co] + ff >= 0 && dd + f2[co] + ff < 5 && ee + g2[co] + gg >= 0 && ee + g2[co] + gg < 5 && banmen[d0][dd + f2[co] + ff][ee + g2[co] + gg] == 0 && banmen[d1][dd + f2[co] + ff][ee + g2[co] + gg] == 0) //最長でどこまで戻れるか
                            {
                                f2[co + 1] = f2[co] + ff;
                                g2[co + 1] = g2[co] + gg;
                                co++;
                            }

                            int pp[3] = {};
                            for (int h = 0; h < 3; h++)
                            {
                                pp[h] = t[d0][h];
                            }

                            pp[kk] = dd + f2[co] + 5 * (ee + g2[co]); //行き先
                            int hh = s[pp[0]][pp[1]][pp[2]];

                            if (allbanmen[d1][hh] == 0 && fin == 0)
                            {
                                board[dd][ee] = 0;
                                board[dd+f2[co]][ee+g2[co]] = -1;
                                if(fin_judge(board))
                                {
                                    fin = 1;
                                    kati[0][0] = dd;
                                    kati[0][1] = ee;
                                    kati[0][2] = dd + f2[co];
                                    kati[0][3] = ee + g2[co];
                                    kpo = 1;
                                }
                                else
                                {
                                    kati[kpo][0] = dd;
                                    kati[kpo][1] = ee;
                                    kati[kpo][2] = dd + f2[co];
                                    kati[kpo][3] = ee + g2[co];
                                    kpo++;
                                }

                                board[dd][ee] = -1;
                                board[dd+f2[co]][ee+g2[co]] = 0;
                            }

                            if (allbanmen[d1][hh] == 1)
                            {
                                make[mpo][0] = dd;
                                make[mpo][1] = ee;
                                make[mpo][2] = dd + f2[co];
                                make[mpo][3] = ee + g2[co];
                                mpo++;
                            }

                            if (allbanmen[d1][hh] == -1)
                            {
                                hikiwake[hpo][0] = dd;
                                hikiwake[hpo][1] = ee;
                                hikiwake[hpo][2] = dd + f2[co];
                                hikiwake[hpo][3] = ee + g2[co];
                                hpo++;
                            }
                        }
                    }
                }
            }

            int l, k, ll, kk;
            int ran;
            if(kpo != 0)
            {
                if(kpo == 1)
                {
                    l = kati[0][0];
                    k = kati[0][1];
                    ll = kati[0][2];
                    kk = kati[0][3];
                }
                else
                {
                ran = rand() % kpo;
                l = kati[ran][0];
                k = kati[ran][1];
                ll = kati[ran][2];
                kk = kati[ran][3];
                }
            }

            else if (hpo != 0)
            {
                ran = rand() % hpo;
                l = hikiwake[ran][0];
                k = hikiwake[ran][1];
                ll = hikiwake[ran][2];
                kk = hikiwake[ran][3];
            }

            else
            {
                ran = 0;
                l = make[ran][0];
                k = make[ran][1];
                ll = make[ran][2];
                kk = make[ran][3];
            }

            printboard();
            printf("AI ");
            int f3 = agent1(l, k, ll, kk);
            char c,d;
            l = 5 - l;
            ll = 5 - ll;
            c = k + 'A';
            d = kk + 'A';
            printf("%d%c%d%c\n", l, c, ll, d);

            if (f3 == 0)
            {
                printf("You Win\n");
                return 0;
            }
            if (fin_judge(board))
            {
                printf("You Lose\n");
                return 0;
            }

            ca++;

            printboard();
            int i, j, ii, jj;
            char a, b;
            scanf("%d%c%d%c", &i, &a, &ii, &b);
            i = 5 - i;
            ii = 5 - ii;
            j = a - 'A';
            jj = b - 'A';

            int f4 = agent2(i, j, ii, jj);
            printboard();

            if (f4 == 0)
            {
                printf("You Lose\n");
                return 0;
            }

            if (fin_judge(board))
            {
                printf("You Win\n");
                return 0;
            }

            ca++;
        }
    }
}