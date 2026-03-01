#include<stdio.h>
#include<stdlib.h>

#define D -1

const int point_4[8][2] = {{-1,0},{0,1},{1,0},{0,-1},{-1,0},{0,1},{1,0},{0,-1}};
const int corner_4[8][2] = {{0,0},{1,0},{1,1},{0,1},{0,0},{1,0},{1,1},{0,1}};
const int point_8[16][2]={{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1}};

static void change(const int **x, int **ans, int a, int b,int **check_table,int i,int j,int s,int t,int *error);
typedef struct pointlist PointList;

typedef struct pointlist{
    int x;
    int y;   //二次元配列a[x][y]的な感じで文字の出現順に軸をとる(面倒なので)
    int status;
    PointList *next;   //線形リスト
}PointList;

typedef struct header{
    PointList *begin;
    PointList *end;
}Header;


int **loadinfo(char *filename, int *a, int *b){
    FILE *fp = fopen(filename,"r");
    int c = 0;
    int **result;
    if(fp == NULL){
        fprintf(stderr, "error: cannot open %s\n", filename);
        exit(1);
    }
    c = fscanf(fp,"%d,%d\n",a,b);
    if(c != 2){
        fprintf(stderr, "error: unexpected input from %s\n", filename);
        exit(1);
    }
    result = (int **)malloc(((*a) + 2)*sizeof(int*));
    int *l = (int *)malloc(((*a) + 2)*((*b) + 2)*sizeof(int));
    for(int j = 0; j < (*a) + 2 ; j++){
        result[j] = l + j*((*b)+2);
    }
    for(int j = 0; j < (*b) + 2 ; j++){
        result[0][j] = D;
        result[(*a)+1][j] = D;
    }
    for(int i = 1 ; i <= *a ; i++){
        result[i][0] = -1;
        result[i][(*b) + 1] = -1;
        for(int j = 1; j <= *b ; j++){
            char d = fgetc(fp);
            if(d == ','){
                if(j < *b){
                    result[i][j] = -1;
                }else{
                    fprintf(stderr, "error: the length of line is more than %d\n",*b);
                    exit(1);
                }
            }else if('0' <= d && d <= '3'){
                result[i][j] = d - '0';
                d = fgetc(fp);
                if(d != ',' && d != '\n'&& d != '\r'){
                    if((d == EOF) && (i == *a) && (j == *b)){
                        continue;
                    }
                    fprintf(stderr, "error: unexpected input from %s\n", filename);
                    exit(1);
                }else if(d == '\r'){
                    d = fgetc(fp);
                    if(d!='\n'){
                        fprintf(stderr, "error: unexpected input from %s\n", filename);
                        exit(1);
                    }
                }
            }else if(d == '\n'||d == '\r'){
                if(d == '\r'){
                    d = fgetc(fp);
                    if(d!='\n'){
                        fprintf(stderr, "error: unexpected input from %s\n", filename);
                        exit(1);
                    }
                }
                result[i][j] = -1;
                if(j != *b){
                    fprintf(stderr, "error: the length of line is less than %d\n",*b);
                    exit(1);
                }
            }else if(d == EOF){
                if(i*j != (*a)*(*b)){
                    fprintf(stderr, "error: the field is less than %d,%d\n",*a,*b);
                    exit(1);
                }
                result[i][j] = -1;
            }else{
                fprintf(stderr, "error: %c cannot be used in Slitherlink \n",d);
                exit(1);
            }

        }
    }
    return result;
    
}

void print_answer(int **table,const int **x,int a,int b){
    for(int  i= 1 ; i <= a ; i++){
        for(int j = 1 ; j <= b ; j++){
            if(table[i][j]){
                printf("\x1b[47m");
                if(x[i][j] == -1){
                    printf(" ");
                }else{
                    printf("\x1b[37m");
                    printf("%d",x[i][j]);
                    printf("\x1b[39m");
                }
                printf("\x1b[49m");
            }else{
                printf("\x1b[40m");
                if(x[i][j] == -1){
                    printf(" ");
                }else{
                    printf("\x1b[37m");
                    printf("%d",x[i][j]);
                    printf("\x1b[39m");
                }
                printf("\x1b[49m");
            }
        }
        printf("\n");
    }   
}

// void print_D(int **table,int a,int b){
//     int notD = 0;
//     for(int  i= 1 ; i <= a ; i++){
//         for(int j = 1 ; j <= b ; j++){
//             if(table[i][j] == D){
//                 printf("#");
//             }else{
//                 printf(" ");
//                 notD++;
//             }
//         }
//         printf("\n");
//     }   
//     printf("%d\n",notD);
// }

void copy_table(int **ans,int **tmp ,int a, int b){
    for(int i = 0; i < a + 2; i++){
        for(int j = 0 ; j < b + 2; j++){
            tmp[i][j] = ans[i][j];
        }
    }
}

void clean_check_table(int a, int b,int **check_table){
    for(int i = 0; i < a; i++){
        for(int j = 0; j < b; j++){
            if(check_table[i+1][j+1] != D) check_table[i+1][j+1] = 0;
        }
    }
}

Header get_list_of_s(int **ans,int **check_table, int a, int b,int s){
    Header head = {.begin = NULL,.end = NULL};
    for(int i = 1; i < a+ 1;i++){
        for(int j = 1; j < b + 1;j++){
            if(check_table[i][j] == s){
                PointList *point = (PointList *)malloc(sizeof(PointList));
                *point = (PointList){.next = NULL,.status = ans[i][j],.x = i,.y = j};
                if(head.end == NULL){
                    head.begin = point;
                    head.end = point;
                }else{
                    head.end->next = point;
                    head.end = point;
                }
                point = NULL;
            }
        }
    }
    return head;
}

Header get_2common(Header h1, Header h2){
    Header common ={.begin = NULL,.end =NULL};
    PointList *p1;
    PointList *p2;
    if(h1.begin == NULL || h2.begin == NULL){
        return common;
    }
    p1 = h1.begin;
    p2 = h2.begin;
    while(p1 != h1.end && p2!= h2.end){
        if(p1->x > p2->x){
            p2 = p2->next;
        }else if(p2->x > p1->x){
            p1 = p1->next;
        }else{
            if(p1->y > p2->y){
                p2 = p2->next;
            }else if (p2->y > p1->y){
                p1 = p1->next;
            }else{
                if(p1->status == p2->status){
                    PointList *point = (PointList *)malloc(sizeof(PointList));
                    *point = (PointList){.next = NULL,.status = p1->status,.x = p1->x,.y = p1->y};
                    if(common.end == NULL){
                        common.begin = point;
                        common.end = point;
                    }else{
                        common.end->next = point;
                        common.end = point;
                    }
                    point = NULL;
                }
                p1 = p1->next;
                p2 = p2->next;
            }
        }
    }
    return common;
}

Header get_3common(Header h1, Header h2 ,Header h3){
    Header common ={.begin = NULL,.end =NULL};
    PointList *p1;
    PointList *p2;
    PointList *p3;
    if(h1.begin == NULL || h2.begin == NULL || h3.begin == NULL){
        return common;
    }
    p1 = h1.begin;
    p2 = h2.begin;
    p3 = h3.begin;
    while(p1 != h1.end && p2!= h2.end && p3!=h3.end){
        if(p1->x > p2->x){
            p2 = p2->next;
            if(p1->x > p3->x){
                p3 = p3->next;
            }else if(p3->x > p1->x){
                p1 = p1->next;
            }
        }else if(p2->x > p1->x){
            p1 = p1->next;
            if(p2->x > p3->x){
                p3 = p3->next;
            }else if(p3->x > p2->x){
                p2 = p2->next;
            }
        }else if(p3->x != p1->x){
            if(p3->x > p1->x){
                p1 = p1->next;
                p2 = p2->next;
            }else{
                p3 = p3->next;
            }
        }else{
            if(p1->y > p2->y){
                p2 = p2->next;
                if(p1->y > p3->y){
                    p3 = p3->next;
                }else if(p3->y > p1->y){
                    p1 = p1->next;
                }
            }else if (p2->y > p1->y){
                p1 = p1->next;
                if(p2->y > p3->y){
                    p3 = p3->next;
                }else if(p3->y > p2->y){
                    p2 = p2->next;
                }
            }else if(p3->y != p1->y){
                if(p3->y > p1->y){
                    p1 = p1->next;
                    p2 = p2->next;
                }else{
                    p3 = p3->next;
                }
            }else{
                if(p1->status == p2->status && p1->status == p3->status){
                    PointList *point = (PointList *)malloc(sizeof(PointList));
                    *point = (PointList){.next = NULL,.status = p1->status,.x = p1->x,.y = p1->y};
                    if(common.end == NULL){
                        common.begin = point;
                        common.end = point;
                    }else{
                        common.end->next = point;
                        common.end = point;
                    }
                    point = NULL;
                }
                p1 = p1->next;
                p2 = p2->next;
                p3 = p3->next;
            }
        }
    }
    return common;
}

void free_PointList(Header *h){
    if(h->begin == NULL)return;
    for(PointList *p = h->begin;p != NULL;){
        PointList *tmp = p->next;
        free(p);
        p = tmp;
        // if(p == h->end){
        //     free(p);
        //     break;
        // }
    }
    h->begin = NULL;
    h->end = NULL;
}

void fill(int a ,int b,int **z,int x,int y,int t){
    z[x][y] = t;
    if(x < a + 1){
        if(z[x + 1][y] != t){
            fill(a,b,z,x + 1,y,t);
        }
    }
    if(x > 0){
        if(z[x - 1][y] != t){
            fill(a,b,z,x - 1,y,t);
        }
    }
    if(y > 0){
        if(z[x][y - 1] != t){
            fill(a,b,z,x,y - 1,t);
        }
    }
    if(y < b + 1){
        if(z[x][y + 1] != t){
            fill(a,b,z,x,y + 1,t);
        }
    }
}

void fill2(int **check_table,int a ,int b,int **z,int **ans, int x,int y,int t,int s,int *flag){
    z[x][y] = t;
    if(x < a + 1){
        if(z[x + 1][y] != t && (check_table[x + 1][y] == D ||check_table[x + 1][y] == s)){
            fill2(check_table,a,b,z,ans,x + 1,y,t,s,flag);
        }else if(check_table[x + 1][y] != D && check_table[x + 1][y] != s){
            (*flag)++;
        }
    }
    if(x > 0){
        if(z[x - 1][y] != t && (check_table[x - 1][y] == D || check_table[x - 1][y] == s)){
            fill2(check_table,a,b,z,ans,x - 1,y,t,s,flag);
        }else if(check_table[x - 1][y] != D && check_table[x - 1][y] != s){
            (*flag)++;
        }
    }
    if(y > 0){
        if(z[x][y - 1] != t && (check_table[x][y - 1] == D || check_table[x][y - 1] == s)){
            fill2(check_table,a,b,z,ans,x,y - 1,t,s,flag);
        }else if(check_table[x][y - 1] != D && check_table[x][y - 1] != s){
            (*flag)++;
        }
    }
    if(y < b + 1){
        if(z[x][y + 1] != t && (check_table[x][y + 1] == D || check_table[x][y + 1] == s)){
            fill2(check_table,a,b,z,ans,x,y + 1,t,s,flag);
        }else if(check_table[x][y + 1] != D && check_table[x][y + 1] != s){
            (*flag)++;
        }
    }
}


int isIsolate(int **check_table,int a ,int b,int **ans,int s){
    int flag = 0;
    int *z[a+2];
    int z0[(a+2)*(b+2)];
    for(int i = 0; i < a + 2;i++){
        z[i] = &z0[(b+2)*i];
    }
    copy_table(ans,z,a,b);
    fill2(check_table,a,b,z,ans,0,0,1,s,&flag);
    for(int i = 0; i < a + 2;i++){
        for(int j = 0; j < b + 2; j++){
            if((check_table[i][j] == D || check_table[i][j] == s) && z[i][j] == 0){
                flag = 0;
                fill2(check_table,a,b,z,ans,i,j,1,s,&flag);
                if(flag) continue;
                return 1;
            }
        }
    }
    flag = 0;
    copy_table(ans,z,a,b);
    for(int i = 0; i < a + 2; i++){
        for(int j = 0; j < b + 2; j++){
            if(z[i][j] == 1 && check_table[i][j] == D){
                if(flag > 0){
                    flag = 0;
                    fill2(check_table,a,b,z,ans,i,j,0,s,&flag);
                    if(!flag){
                        return 1;
                    }
                }else if(flag == 0){
                    fill2(check_table,a,b,z,ans,i,j,0,s,&flag);
                    if(!flag) flag =-1;
                }else{
                    return 1;
                }
            }
        }
    }
    return 0;

}

int count_holes(int **x ,int a,int b){
    int *y[a+2];
    int y0[(a+2)*(b+2)];
    for(int i = 0; i < a + 2;i++){
        y[i] = &y0[(b+2)*i];
    }
    int f = 0;
    for(int i = 0; i < a + 2; i++){
        for(int j = 0; j < b + 2; j++){
            if(i == 0 || j == 0|| i == a + 1||j == b + 1){
                y[i][j] = 0;
            }else{
                y[i][j] = x[i][j];
            }
        }
    }
    fill(a,b,y,0,0,1);
    for(int i = 0; i < a ; i++){
        for(int j = 0; j < b ; j++){
            if(!y[i + 1][j + 1])f++;
        }
    }
    return f;

}

int count_island(int **x ,int a,int b){
    int *y[a+2];
    int y0[(a+2)*(b+2)];
    for(int i = 0; i < a + 2;i++){
        y[i] = &y0[(b+2)*i];
    }
    int f = -1;
    for(int i = 0; i < a + 2; i++){
        for(int j = 0; j < b + 2; j++){
            if(i == 0 || j == 0|| i == a + 1||j == b + 1){
                y[i][j] = 0;
            }else{
                y[i][j] = x[i][j];
            }
        }
    }
    for(int i = 0; i < a + 2; i++){
        for(int j = 0; j < b + 2; j++){
            if(y[i][j]){
                fill(a,b,y,i,j,0);
                f++;
            }
        }
    }
    return f;
}

int isAnswer(const int **x,int a ,int b,int **ans){
    if(count_holes(ans,a,b)){
        return 0;
    }
    if(count_island(ans,a,b)){
        return 0;
    }
    for(int i = 1; i < a + 1; i++){
        for(int j = 1 ; j < b + 1; j++){
            if(x[i][j] != -1){
                int edge = ans[i - 1][j] + ans[i + 1][j] + ans[i][j - 1] + ans[i][j + 1];
                if((ans[i][j] == 1 && x[i][j] != 4 - edge) || (ans[i][j] == 0 && x[i][j] != edge)){
                    return 0;
                }
            }
        }
    }
    return 1;
}


static int **random_table(int a, int b){
    int ** result = (int **)malloc((a + 2)*sizeof(int*));
    int *l = (int *)malloc((b + 2)*(a + 2)*sizeof(int));
    for(int i = 0 ; i < a + 2 ; i++){
        result[i] = &l[(b+2)*i];
    }
    for(int i = 0 ; i < a + 2 ; i++){
        for(int j = 0; j < b + 2 ; j++){
            l[j] = rand()%2;
            if(i == 0 || j == 0 ||i == a + 1 || j == b + 1){
                l[j] = 0;
            }
        }
    }
    return result;
}

static void change(const int **x, int **ans, int a, int b,int **check_table,int i,int j,int s,int t,int *error){
    int f = 0;
    if(*error)return; //保留
    if(check_table[i][j] == s||check_table[i][j]==D){
        if(ans[i][j] != t){
            (*error)++;
        }
        return;
    }
    check_table[i][j] = s;
    ans[i][j] = t;
    for(int n = 0; n < 4 ; n++){
        f = -1;
        for(int m = 0; m < 4 ; m++){
            int u = check_table[i - 1 + corner_4[n][0] + corner_4[m][0]][j - 1 + corner_4[n][1] + corner_4[m][1]];
            if(u != s && u != D){
                if(f == -1){
                    f = m;
                } else{
                    f = -2;
                }
            }
        }
        if(f >= 0){
            if(ans[i - 1 + corner_4[n][0] + corner_4[f + 1][0]][j - 1 + corner_4[n][1] + corner_4[f + 1][1]] == ans[i - 1 + corner_4[n][0] + corner_4[f + 3][0]][j - 1 + corner_4[n][1] + corner_4[f + 3][1]] && ans[i - 1 + corner_4[n][0] + corner_4[f + 3][0]][j - 1 + corner_4[n][1] + corner_4[f + 3][1]] == 1 - ans[i - 1 + corner_4[n][0] + corner_4[f + 2][0]][j - 1 + corner_4[n][1] + corner_4[f + 2][1]]){
                change(x,ans,a,b,check_table,i - 1 + corner_4[n][0] + corner_4[f][0],j - 1 + corner_4[n][1] + corner_4[f][1],s,ans[i - 1 + corner_4[n][0] + corner_4[f + 1][0]][j - 1 + corner_4[n][1] + corner_4[f + 1][1]],error);
            }
        }
    }
    f = 0;
    for(int n = 0; n < 8;n++){
        if(f == 2)f = 0;
        if(f){
            if(x[i + point_8[n][0]][j + point_8[n][1]] == 0){
                if(check_table[i + point_8[n][0]][j + point_8[n][1]] == s || check_table[i + point_8[n][0]][j + point_8[n][1]] == D){
                    if(ans[i + point_8[n][0]][j + point_8[n][1]] != ans[i][j]) (*error)++;
                }else{
                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 1][0],j + point_8[n + 1][1],s,t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 7][0],j + point_8[n + 7][1],s,t,error);
                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_8[n][0],j + point_8[n][1] + point_8[n][1],s,t,error);
                }
            }else if(x[i + point_8[n][0]][j + point_8[n][1]] == 1){
                for(int m = 0; m < 4 ;m++){
                    if(check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] == s||check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]==D){
                        int m0 = m;
                        int t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                        if(check_table[i + point_8[n][0]][j + point_8[n][1]] == s||check_table[i + point_8[n][0]][j + point_8[n][1]]==D){
                            if(ans[i + point_8[n][0]][j + point_8[n][1]] != ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 1][0],j + point_8[n][1] + point_4[m + 1][1],s,1-t0,error);
                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 2][0],j + point_8[n][1] + point_4[m + 2][1],s,1-t0,error);
                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 3][0],j + point_8[n][1] + point_4[m + 3][1],s,1-t0,error);
                                m = 4;
                                break;
                            }
                        }
                        t0 = -1;
                        m++;
                        int m1 = -1;
                        for(;m < 4 ; m++){
                            if(check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] == s||check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]==D){
                                if(ans[i + point_8[n][0]][j + point_8[n][1]] != ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] && check_table[i + point_8[n][0]][j + point_8[n][1]] == D){
                                    t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 1][0],j + point_8[n][1] + point_4[m + 1][1],s,1-t0,error);
                                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 2][0],j + point_8[n][1] + point_4[m + 2][1],s,1-t0,error);
                                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 3][0],j + point_8[n][1] + point_4[m + 3][1],s,1-t0,error);
                                    m = 4;
                                    break;
                                }
                                if(ans[i + point_8[n][0] + point_4[m0][0]][j + point_8[n][1] + point_4[m0][1]] == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                    int tmp = t0;
                                    t0 = ans[i + point_8[n][0] + point_4[m0][0]][j + point_8[n][1] + point_4[m0][1]];
                                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,t0,error);
                                    t0 = tmp;
                                }
                                if(t0 == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,t0,error);
                                    if(t0 == ans[i + point_8[n][0] + point_4[m0][0]][j + point_8[n][1] + point_4[m0][1]]){
                                        for(int k = 0; k < 4 ;k++){
                                            if(m != k && m0 != k && m1 != k){
                                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[k][0],j + point_8[n][1] + point_4[k][1],s,1-t0,error);
                                            }
                                        }
                                    }
                                }
                                t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                                m1 = m;
                            }
                        }
                    }
                }
            }else if(x[i + point_8[n][0]][j + point_8[n][1]] == 2){
                for(int m = 0; m < 4 ;m++){
                    if(check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] == s||check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]==D){
                        int m0 = m;
                        int m1 = m;
                        int t0 = -1;
                        m++;
                        for(;m < 4 ; m++){
                            if(check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] == s||check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]==D){
                                if(ans[i + point_8[n][0] + point_4[m0][0]][j + point_8[n][1] + point_4[m0][1]] == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                    t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                                    for(int k = 0; k < 4; k++){
                                        if(k == m || k == m0){
                                            continue;
                                        }else{
                                            change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[k][0],j + point_8[n][1] + point_4[k][1],s,1-t0,error);
                                        }
                                    }
                                    m = 4;
                                    break;
                                }
                                if(t0 == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                    for(int k = 0; k < 4; k++){
                                        if(k == m || k == m1){
                                            continue;
                                        }else{
                                            change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[k][0],j + point_8[n][1] + point_4[k][1],s,1-t0,error);
                                        }
                                    }
                                    m = 4;
                                    break;
                                }
                                m1 = m;
                                t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                            }
                        }
                    }
                }
            }
            if(x[i + point_8[n][0]][j + point_8[n][1]] == 3){
                if(x[i + point_8[n][0] + point_8[n][0]][j + point_8[n][1] + point_8[n][1]] == 3){
                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,1-t,error);
                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_8[n][0],j + point_8[n][1] + point_8[n][1],s,t,error);
                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_8[n][0] + point_8[n][0],j + point_8[n][1] + point_8[n][1] + point_8[n][1],s,1-t,error);
                }else if(x[i + point_8[n][0] + point_8[n][0]][j + point_8[n][1] + point_8[n][1]] == 0){
                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_8[n][0],j + point_8[n][1] + point_8[n][1],s,1-t,error);//0の処理による
                }
                if(x[i + point_8[n][0] + point_8[n + 1][0]][j + point_8[n][1] + point_8[n + 1][1]] == 3){
                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,1-t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 6][0],j + point_8[n + 6][1],s,t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 7][0],j + point_8[n + 7][1],s,t,error);
                }
                if(x[i + point_8[n][0] + point_8[n + 7][0]][j + point_8[n][1] + point_8[n + 7][1]] == 3){
                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,1-t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 1][0],j + point_8[n + 1][1],s,t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 2][0],j + point_8[n + 2][1],s,t,error);
                }
                if(x[i + point_8[n + 1][0]][j + point_8[n + 1][1]] == 0){
                    change(x,ans,a,b,check_table,i + point_8[n + 1][0],j + point_8[n + 1][1],s,1-t,error);
                }
                if(x[i + point_8[n + 7][0]][j + point_8[n + 7][1]] == 0){
                    change(x,ans,a,b,check_table,i + point_8[n + 7][0],j + point_8[n + 7][1],s,1-t,error);
                }
                for(int m = 0; m < 4 ;m++){
                    if(check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] == s||check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]==D){
                        int m0 = m;
                        int t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                        if(check_table[i + point_8[n][0]][j + point_8[n][1]] == s||check_table[i + point_8[n][0]][j + point_8[n][1]]==D){
                            if(ans[i + point_8[n][0]][j + point_8[n][1]] == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 1][0],j + point_8[n][1] + point_4[m + 1][1],s,1-t0,error);
                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 2][0],j + point_8[n][1] + point_4[m + 2][1],s,1-t0,error);
                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 3][0],j + point_8[n][1] + point_4[m + 3][1],s,1-t0,error);
                                m = 4;
                                break;
                            }
                        }
                        t0 = -1;
                        m++;
                        int m1 = -1;
                        for(;m < 4 ; m++){
                            if(check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] == s||check_table[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]==D){
                                if(ans[i + point_8[n][0]][j + point_8[n][1]] == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]] && check_table[i + point_8[n][0]][j + point_8[n][1]] == D){
                                    t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 1][0],j + point_8[n][1] + point_4[m + 1][1],s,1-t0,error);
                                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 2][0],j + point_8[n][1] + point_4[m + 2][1],s,1-t0,error);
                                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[m + 3][0],j + point_8[n][1] + point_4[m + 3][1],s,1-t0,error);
                                    m = 4;
                                    break;
                                }
                                if(ans[i + point_8[n][0] + point_4[m0][0]][j + point_8[n][1] + point_4[m0][1]] == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                    int tmp =t0;
                                    t0 = ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]];
                                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,1-t0,error);
                                    t0 = tmp;
                                }
                                if(t0 == ans[i + point_8[n][0] + point_4[m][0]][j + point_8[n][1] + point_4[m][1]]){
                                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,1-t0,error);
                                    if(t0 == ans[i + point_8[n][0] + point_4[m0][0]][j + point_8[n][1] + point_4[m0][1]]){
                                        for(int k = 0; k < 4 ;k++){
                                            if(m != k && m0 != k && m1 != k){
                                                change(x,ans,a,b,check_table,i + point_8[n][0] + point_4[k][0],j + point_8[n][1] + point_4[k][1],s,1-t0,error);
                                            }
                                        }
                                    }
                                }
                                m1 = m;
                                t0 = ans[i + point_8[n][0] + point_4[m1][0]][j + point_8[n][1] + point_4[m1][1]];
                            }
                        }
                    }
                }
            }
        }else{
            if(x[i + point_8[n][0]][j + point_8[n][1]] == 3){
                if(x[i + point_8[n][0] + point_8[n][0]][j + point_8[n][1] + point_8[n][1]] == 3){
                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,1-t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 1][0],j + point_8[n + 1][1],s,t,error);
                    change(x,ans,a,b,check_table,i + point_8[n + 7][0],j + point_8[n + 7][1],s,t,error);
                }
                if(x[i + point_8[n][0] + point_8[n + 1][0]][j + point_8[n][1] + point_8[n + 1][1]] == 0){
                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_8[n + 1][0],j + point_8[n][1] + point_8[n + 1][1],s,1-t,error);
                }
                if(x[i + point_8[n][0] + point_8[n + 7][0]][j + point_8[n][1] + point_8[n + 7][1]] == 0){
                    change(x,ans,a,b,check_table,i + point_8[n][0] + point_8[n + 7][0],j + point_8[n][1] + point_8[n + 7][1],s,1-t,error);
                }
            }
        }
        if(x[i + point_8[n][0]][j + point_8[n][1]] == 3 && x[i + point_8[n + 1][0]][j + point_8[n + 1][1]] == 3){
            if(f){
                change(x,ans,a,b,check_table,i + point_8[n + 2][0],j + point_8[n + 2][1],s,t,error);
                change(x,ans,a,b,check_table,i + point_8[n][0] + point_8[n][0],j + point_8[n][1] + point_8[n][1],s,1-t,error);
                change(x,ans,a,b,check_table,i + point_8[n + 1][0] + point_8[n][0],j + point_8[n + 1][1] + point_8[n][1],s,1-t,error);
            }else{
                change(x,ans,a,b,check_table,i + point_8[n + 7][0],j + point_8[n + 7][1],s,t,error);
                change(x,ans,a,b,check_table,i + point_8[n + 1][0] + point_8[n + 1][0],j + point_8[n + 1][1] + point_8[n + 1][1],s,1-t,error);
                change(x,ans,a,b,check_table,i + point_8[n + 1][0] + point_8[n][0],j + point_8[n + 1][1] + point_8[n][1],s,1-t,error);
            }
        }
        f++;
    }
    if(x[i][j] == 0){
        for(int n = 0; n < 4;n++){
            change(x,ans,a,b,check_table,i + point_4[n][0],j + point_4[n][1],s,t,error);
        }
    }else if(x[i][j] == 1){
        int counter = 0;
        f = 0;
        for(int n = 0; n < 8;n++){
            if(f == 2)f = 0;
            if(!f){
                if(x[i + point_8[n][0]][j + point_8[n][1]] == 0){
                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,t,error);
                }
            }else{
                if(check_table[i + point_8[n][0]][j + point_8[n][1]] == D || check_table[i + point_8[n][0]][j + point_8[n][1]] == s){
                    if(ans[i + point_8[n][0]][j + point_8[n][1]] != t){
                        change(x,ans,a,b,check_table,i + point_8[n + 2][0],j + point_8[n + 2][1],s,t,error);
                        change(x,ans,a,b,check_table,i + point_8[n + 4][0],j + point_8[n + 4][1],s,t,error);
                        change(x,ans,a,b,check_table,i + point_8[n + 6][0],j + point_8[n + 6][1],s,t,error);
                        break;
                    }
                }else{
                    if(!counter)counter = n;
                    else counter = -1;
                }
                if(n == 7){
                    if(counter!= -1 && counter!= 0){
                        change(x,ans,a,b,check_table,i + point_8[counter][0],j + point_8[counter][1],s,1-t,error);
                    }
                }
            }
            f++;
        }
    }else if(x[i][j] == 2){
        int n0 = -1;
        int n1 = -1;
        for(int n = 0; n < 4;n++){
            if(check_table[i + point_4[n][0]][j + point_4[n][1]] == D || check_table[i + point_4[n][0]][j + point_4[n][1]] == s){
                if(n0 == -1)n0 = n;
                else if(n1 == -1){
                    n1 = n;
                    if(ans[i + point_4[n0][0]][j + point_4[n0][1]] == ans[i + point_4[n1][0]][j + point_4[n1][1]]){
                        for(int m = 0; m < 4; m++){
                            if(m == n0 || m == n1){
                                continue;
                            }else{
                                change(x,ans,a,b,check_table,i + point_4[m][0],j + point_4[m][1],s,1-ans[i + point_4[n1][0]][j + point_4[n1][1]],error);
                            }
                        }
                        break;
                    }
                }else{
                    if(ans[i + point_4[n][0]][j + point_4[n][1]] == ans[i + point_4[n0][0]][j + point_4[n0][1]]){
                        for(int m = 0; m < 4; m++){
                            if(m == n || m == n0){
                                continue;
                            }else{
                                change(x,ans,a,b,check_table,i + point_4[m][0],j + point_4[m][1],s,1-ans[i + point_4[n0][0]][j + point_4[n0][1]],error);
                            }
                        }
                    }else{
                        for(int m = 0; m < 4; m++){
                            if(m == n ||m == n1){
                                continue;
                            }else{
                                change(x,ans,a,b,check_table,i + point_4[m][0],j + point_4[m][1],s,1-ans[i + point_4[n1][0]][j + point_4[n1][1]],error);
                            }
                        }
                    }
                }
            }
        }
    }else if(x[i][j] == 3){
        int counter = 0;
        f = 0;
        for(int n = 0; n < 8;n++){
            if(f == 2)f = 0;
            if(!f){
                if(x[i + point_8[n][0]][j + point_8[n][1]] == 0){
                    change(x,ans,a,b,check_table,i + point_8[n][0],j + point_8[n][1],s,1-t,error);
                }else if(x[i + point_8[n][0]][j + point_8[n][1]] == 3){
                    change(x,ans,a,b,check_table,i - point_8[n][0],j - point_8[n][1],s,1-t,error);
                }
            }else{
                if(x[i + point_8[n][0]][j + point_8[n][1]] == 3){
                    change(x,ans,a,b,check_table,i - point_8[n][0],j - point_8[n][1],s,1-t,error);
                }
                if(check_table[i + point_8[n][0]][j + point_8[n][1]] == D || check_table[i + point_8[n][0]][j + point_8[n][1]] == s){
                    if(ans[i + point_8[n][0]][j + point_8[n][1]] == t){
                        change(x,ans,a,b,check_table,i + point_8[n + 2][0],j + point_8[n + 2][1],s,1-t,error);
                        change(x,ans,a,b,check_table,i + point_8[n + 4][0],j + point_8[n + 4][1],s,1-t,error);
                        change(x,ans,a,b,check_table,i + point_8[n + 6][0],j + point_8[n + 6][1],s,1-t,error);
                        break;
                    }
                }else{
                    if(!counter)counter = n;
                    else counter = -1;
                }
                if(n == 7){
                    if(counter!= -1 && counter!= 0){
                        change(x,ans,a,b,check_table,i + point_8[counter][0],j + point_8[counter][1],s,t,error);
                    }
                }
            }
            f++;
        }
    }
}

static void change3(const int **x, int **ans, int a, int b,int **check_table,int i,int j,int *s,int t,int *changed){
    int notconfilct = 0;
    int n0 = -1;
    int m0 = -1;
    int t0 = -1;
    int error0 = 0;
    int *error = &error0;
    int *tmp[a+2];
    int l1[(a+2)*(b+2)];
    for(int n = 0; n < a + 2; n++){
        tmp[n] = l1 + n*(b+2);
    }
    for(int n = 0; n < 4;n++){
        if(x[i + point_4[n][0]][j + point_4[n][1]] == 1){
            Header headlist[8];
            for(int m = 0; m < 8; m++){
                headlist[m] = (Header){.begin = NULL,.end = NULL};
            }
            for(int m = 0; m < 4; m++){
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,1-t,error);
                if(*error){
                    *error = 0;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m;
                            t0 = t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,1-t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,t,error);
                if(*error){
                    *error = 0;
                    continue;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m;
                            t0 = 1-t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
            }
            if(notconfilct == 1){
                n = 4;
                free_PointList(headlist);
                break;
            }else if(notconfilct){
                while(notconfilct >2){
                    Header h = get_3common(headlist[notconfilct - 3],headlist[notconfilct - 2],headlist[notconfilct - 1]);
                    free_PointList(&headlist[notconfilct - 3]);
                    free_PointList(&headlist[notconfilct - 2]);
                    free_PointList(&headlist[notconfilct - 1]);
                    headlist[notconfilct - 3] = h;
                    notconfilct -= 2;
                }
                if(notconfilct == 2){
                    Header h = get_2common(headlist[0],headlist[1]);
                    free_PointList(headlist);
                    free_PointList(&headlist[1]);
                    headlist[0] =h;
                }
                if(headlist[0].begin != NULL){
                    for(PointList *p = headlist[0].begin ; p != NULL; p= p->next){
                        change(x,ans,a,b,check_table,p->x,p->y,D,p->status,error);
                        (*changed)++;
                    }
                    free_PointList(headlist);
                }
                notconfilct = 0;
            }
        }else if(x[i + point_4[n][0]][j + point_4[n][1]] == 2){
            Header headlist[12];
            for(int m = 0; m < 12; m++){
                headlist[m] = (Header){.begin = NULL,.end = NULL};
            }
            for(int m = 0; m < 3; m++){
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[3][0],j + point_4[n][1] + point_4[3][1],*s,t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,t,error);
                if(*error){
                    *error = 0;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[3][0]][j + point_4[n][1] + point_4[3][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m;
                            t0 = t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,1-t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[3][0],j + point_4[n][1] + point_4[3][1],*s,t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,t,error);
                if(*error){
                    *error = 0;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[3][0]][j + point_4[n][1] + point_4[3][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m + 3;
                            t0 = t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,1-t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[3][0],j + point_4[n][1] + point_4[3][1],*s,1-t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,1-t,error);
                if(*error){
                    *error = 0;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[3][0]][j + point_4[n][1] + point_4[3][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m;
                            t0 = 1-t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[3][0],j + point_4[n][1] + point_4[3][1],*s,1-t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,1-t,error);
                if(*error){
                    *error = 0;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[3][0]][j + point_4[n][1] + point_4[3][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m + 3;
                            t0 = 1-t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
            }
            if(notconfilct == 1){
                n = 4;
                free_PointList(headlist);
                break;
            }else if(notconfilct){
                while(notconfilct >2){
                    Header h = get_3common(headlist[notconfilct - 3],headlist[notconfilct - 2],headlist[notconfilct - 1]);
                    free_PointList(&headlist[notconfilct - 3]);
                    free_PointList(&headlist[notconfilct - 2]);
                    free_PointList(&headlist[notconfilct - 1]);
                    headlist[notconfilct - 3] = h;
                    notconfilct -= 2;
                }
                if(notconfilct == 2){
                    Header h = get_2common(headlist[0],headlist[1]);
                    free_PointList(headlist);
                    free_PointList(&headlist[1]);
                    headlist[0] =h;
                }
                if(headlist[0].begin != NULL){
                    for(PointList *p = headlist[0].begin ; p != NULL; p= p->next){
                        change(x,ans,a,b,check_table,p->x,p->y,D,p->status,error);
                        (*changed)++;
                    }
                    free_PointList(headlist);
                }
                notconfilct = 0;
            }
        }else if(x[i + point_4[n][0]][j + point_4[n][1]] == 3){
            Header headlist[8];
            for(int m = 0; m < 8; m++){
                headlist[m] = (Header){.begin = NULL,.end = NULL};
            }
            for(int m = 0; m < 4; m++){
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,t,error);
                if(*error){
                    *error = 0;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m;
                            t0 = t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
                (*s)++;
                copy_table(ans,tmp,a,b);
                change(x,tmp,a,b,check_table,i + point_4[n][0],j + point_4[n][1],*s,1-t,error);
                change(x,tmp,a,b,check_table,i + point_4[n][0] + point_4[m][0],j + point_4[n][1] + point_4[m][1],*s,1-t,error);
                if(*error){
                    *error = 0;
                    continue;
                }else{
                    if(check_table[i + point_4[n][0]][j + point_4[n][1]] != D || check_table[i + point_4[n][0] + point_4[m][0]][j + point_4[n][1] + point_4[m][1]] != D){
                        if(!isIsolate(check_table,a,b,tmp,*s)){
                            n0 = n;
                            m0 = m;
                            t0 = 1-t;
                            headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                            notconfilct++;
                        }
                    }
                }
            }
            if(notconfilct == 1){
                n = 4;
                free_PointList(headlist);
                break;
            }else if(notconfilct){
                while(notconfilct >2){
                    Header h = get_3common(headlist[notconfilct - 3],headlist[notconfilct - 2],headlist[notconfilct - 1]);
                    free_PointList(&headlist[notconfilct - 3]);
                    free_PointList(&headlist[notconfilct - 2]);
                    free_PointList(&headlist[notconfilct - 1]);
                    headlist[notconfilct - 3] = h;
                    notconfilct -= 2;
                }
                if(notconfilct == 2){
                    Header h = get_2common(headlist[0],headlist[1]);
                    free_PointList(headlist);
                    free_PointList(&headlist[1]);
                    headlist[0] =h;
                }
                if(headlist[0].begin != NULL){
                    for(PointList *p = headlist[0].begin ; p != NULL; p= p->next){
                        change(x,ans,a,b,check_table,p->x,p->y,D,p->status,error);
                        (*changed)++;
                    }
                    free_PointList(headlist);
                }
                notconfilct = 0;
            }
        }
    }
    if(notconfilct == 1){
        if(n0 != -1){
            if(x[i + point_4[n0][0]][j + point_4[n0][1]] == 1){
                (*s)++;
                if(check_table[i + point_4[n0][0]][j + point_4[n0][1]] != D || check_table[i + point_4[n0][0] + point_4[m0][0]][j + point_4[n0][1] + point_4[m0][1]] != D){
                    change(x,ans,a,b,check_table,i + point_4[n0][0],j + point_4[n0][1],D,t0,error);
                    change(x,ans,a,b,check_table,i + point_4[n0][0] + point_4[m0][0],j + point_4[n0][1] + point_4[m0][1],D,1-t0,error);
                    (*changed)++;
                }
            }else if(x[i + point_4[n0][0]][j + point_4[n0][1]] == 2){
                if(m0 < 3){
                    if(check_table[i + point_4[n0][0]][j + point_4[n0][1]] != D || check_table[i + point_4[n0][0] + point_4[3][0]][j + point_4[n0][1] + point_4[3][1]] != D || check_table[i + point_4[n0][0] + point_4[m0][0]][j + point_4[n0][1] + point_4[m0][1]] != D){
                        (*s)++;
                        change(x,ans,a,b,check_table,i + point_4[n0][0],j + point_4[n0][1],D,t0,error);
                        change(x,ans,a,b,check_table,i + point_4[n0][0] + point_4[3][0],j + point_4[n0][1] + point_4[3][1],D,t0,error);
                        change(x,ans,a,b,check_table,i + point_4[n0][0] + point_4[m0][0],j + point_4[n0][1] + point_4[m0][1],D,t0,error);
                        (*changed)++;
                    }
                }else{
                    m0 -= 3;
                    if(check_table[i + point_4[n0][0]][j + point_4[n0][1]] != D || check_table[i + point_4[n0][0] + point_4[3][0]][j + point_4[n0][1] + point_4[3][1]] != D || check_table[i + point_4[n0][0] + point_4[m0][0]][j + point_4[n0][1] + point_4[m0][1]] != D){
                        (*s)++;
                        change(x,ans,a,b,check_table,i + point_4[n0][0],j + point_4[n0][1],D,1-t0,error);
                        change(x,ans,a,b,check_table,i + point_4[n0][0] + point_4[3][0],j + point_4[n0][1] + point_4[3][1],D,t0,error);
                        change(x,ans,a,b,check_table,i + point_4[n0][0] + point_4[m0][0],j + point_4[n0][1] + point_4[m0][1],D,t0,error);
                        (*changed)++;
                    }
                }
            }else if(x[i + point_4[n0][0]][j + point_4[n0][1]] == 3){
                (*s)++;
                if(check_table[i + point_4[n0][0]][j + point_4[n0][1]] != D || check_table[i + point_4[n0][0] + point_4[m0][0]][j + point_4[n0][1] + point_4[m0][1]] != D){
                    change(x,ans,a,b,check_table,i + point_4[n0][0],j + point_4[n0][1],D,t0,error);
                    change(x,ans,a,b,check_table,i + point_4[n0][0] + point_4[m0][0],j + point_4[n0][1] + point_4[m0][1],D,t0,error);
                    (*changed)++;
                }
            }
        }
    }
    if(check_table[i][j] != D){
        Header headlist[2];
        headlist[0] = (Header){.begin = NULL,.end = NULL};
        headlist[1] = (Header){.begin = NULL,.end = NULL};
        (*s)++;
        copy_table(ans,tmp,a,b);
        change(x,tmp,a,b,check_table,i,j,*s,t,error);
        if(*error){
            *error = 0;
        }else{
            if(!isIsolate(check_table,a,b,tmp,*s)){
                t0 = t;
                headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                notconfilct++;
            }
        }
        (*s)++;
        copy_table(ans,tmp,a,b);
        change(x,tmp,a,b,check_table,i,j,*s,1-t,error);
        if(*error){
            *error = 0;
        }else{
            if(!isIsolate(check_table,a,b,tmp,*s)){
                t0 = 1-t;
                headlist[notconfilct] = get_list_of_s(tmp,check_table,a,b,*s);
                notconfilct++;
            }
        }
        if(check_table[i][j] != D){
            if(notconfilct == 1){
                (*s)++;
                (*changed)++;
                change(x,ans,a,b,check_table,i,j,D,t0,error);
                free_PointList(headlist);
            }else{
                Header h = get_2common(headlist[0],headlist[1]);
                free_PointList(headlist);
                free_PointList(&headlist[1]);
                if(h.begin != NULL){
                    for(PointList *p = h.begin ; p != NULL; p = p->next){
                        change(x,ans,a,b,check_table,p->x,p->y,D,p->status,error);
                        (*changed)++;
                    }
                    free_PointList(&h);
                }
            }
        }

    }
}

static void brute_force_attack(const int **x, int **ans, int a, int b,int **check_table){
    clean_check_table(a,b,check_table);
    int error = 0;
    int index = 0;
    char list[1000] = {0};
    int *check_table2[a+2];
    int *tmp[a+2];
    int l0[(a+2)*(b+2)];
    int l1[(a+2)*(b+2)];
    for(int i = 0; i < a + 2; i++){
        check_table2[i] = l0 + i*(b+2);
        tmp[i] = l1 + i*(b+2);
    }
    copy_table(ans,tmp,a,b);
    copy_table(check_table,check_table2,a,b);
    while(1){
        for(int i = 1; i < a + 1; i++){
            for(int j = 1; j < b + 1; j++){
                if(check_table2[i][j] == D)continue;
                change(x,tmp,a,b,check_table2,i,j,D,list[index],&error);
                if(error || isIsolate(check_table2,a,b,tmp,D)){
                    error = 0;
                    i = a+1;
                    j = b+1;
                    break;
                }
                int s = 0;
                int flag = 1;
                while(flag){
                    flag = 0;
                    s = 0;
                    for(int n = 1; n < a + 1; n++){
                        for(int m = 1 ; m < b + 1; m++){
                            int t = 1-tmp[n][m];
                            if(check_table2[n - 1][m] != D ||check_table2[n + 1][m] != D||check_table2[n][m - 1] != D||check_table2[n][m + 1] != D||check_table2[n][m] !=D){
                                change3(x,tmp,a,b,check_table2,n,m,&s,t,&flag);
                            }
                            s++;
                        }
                    }
                }
                index++;
            }
        }
        if(isAnswer(x,a,b,tmp)){
            copy_table(tmp,ans,a,b);
            return;
        }
        int n = 0;
        list[n] += 1;
        while(n < 1000){
            if(list[n] == 2){
                list[n] = 0;
                n++;
                list[n] += 1;
            }else{
                break;
            }
        }
        copy_table(ans,tmp,a,b);
        copy_table(check_table,check_table2,a,b);
        index = 0;
    }

}

static void init_check_table(const int **x, int **ans, int a, int b,int **check_table){
    int error = 0;
    int *check_table2[a+4];
    int *ans2[a+4];
    int *x2[a+4];
    int l0[(a+4)*(b+4)];
    int l1[(a+4)*(b+4)];
    int l2[(a+4)*(b+4)];
    for(int i = 0; i < a + 4; i++){
        check_table2[i] = l0 + i*(b+4);
        ans2[i] = l1 + i*(b+4);
        x2[i] = l2 + i*(b+4);
    }
    for(int i = 0; i < a + 4; i++){
        for(int j = 0 ; j < b + 4; j++){
            x2[i][j] = -1;
            check_table2[i][j] = 0;
        }
    }
    for(int i = 0; i < a + 2; i++){
        for(int j = 0 ; j < b + 2; j++){
            x2[i + 1][j + 1] = x[i][j];
            ans2[i + 1][j + 1] = ans[i][j];
        }
    }
    for(int i = 0; i < a + 2 ; i++){
        change((const int **)x2,ans2,a + 2,b + 2,check_table2,i + 1,1,D,0,&error);
        change((const int **)x2,ans2,a + 2,b + 2,check_table2,i + 1,b + 2,D,0,&error);
        if(error){
            fprintf(stderr, "error: this can't be solved\n");
            exit(1);
        }
    }
    for(int j = 0; j < b ; j++){
        change((const int **)x2,ans2,a + 2,b + 2,check_table2,1,j + 2,D,0,&error);
        change((const int **)x2,ans2,a + 2,b + 2,check_table2,a + 2,j + 2,D,0,&error);
        if(error){
            fprintf(stderr, "error: this can't be solved\n");
            exit(1);
        }
    }
    for(int i = 0; i < a + 2; i++){
        for(int j = 0 ; j < b + 2; j++){
            check_table[i][j] = check_table2[i + 1][j + 1];
            ans[i][j] = ans2[i + 1][j + 1];
        }
    }

    int s = 1;
    int flag = 1;
    while(flag){
        flag = 0;
        s = 0;
        for(int i = 1; i < a + 1; i++){
            for(int j = 1 ; j < b + 1; j++){
                int t = 1-ans[i][j];
                if(check_table[i - 1][j] != D ||check_table[i + 1][j] != D||check_table[i][j - 1] != D||check_table[i][j + 1] != D||check_table[i][j] !=D){
                    change3(x,ans,a,b,check_table,i,j,&s,t,&flag);
                }
                s++;
            }
        }
    }
    clean_check_table(a,b,check_table);
}

int **search(const int **x ,int a,int b){
    int **ans = random_table(a,b);
    int *check_table[a+2];
    int l[(a+2)*(b+2)];
    for(int i = 0 ; i < a + 2 ; i++){
        check_table[i] = &l[(b+2)*i];
    }
    srand(4);
    init_check_table(x,ans,a,b,check_table);
    // print_D(check_table,a,b);
    brute_force_attack(x,ans,a,b,check_table);
    return ans;
}

int main(int argc,char**argv){
    if(argc == 1){
        fprintf(stderr, "Usage: %s <puzzle file> \n", argv[0]);
        exit(1);
    }
    int a,b,**x;
    x = loadinfo(argv[1],&a,&b);
    int **answer = search((const int **)x,a,b);
    print_answer(answer,(const int **)x,a,b);
    free(answer[0]);
    free(answer);
    free(x[0]);
    free(x);
    return 0;
}