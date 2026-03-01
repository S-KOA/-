#include<stdio.h>
#include<stdlib.h>
#include <string.h>

#define D -1

int a;
int b;
int flag = 0;

int **loadinfo(char *filename){
    FILE *fp = fopen(filename,"r");
    int c = 0;
    int **result;
    if(fp == NULL){
        fprintf(stderr, "error: cannot open %s\n", filename);
        exit(1);
    }
    c = fscanf(fp,"%d,%d\n",&a,&b);
    a += 2;
    b += 2; 
    if(c != 2){
        fprintf(stderr, "error: unexpected input from %s\n", filename);
        exit(1);
    }
    result = (int **)malloc(a*sizeof(int*));
    int *l = (int *)malloc(a*b*sizeof(int));
    for(int j = 0; j < a ; j++){
        result[j] = l + j*b;
    }
    for(int j = 0; j < b ; j++){
        result[0][j] = D;
        result[a-1][j] = D;
    }
    for(int i = 1 ; i < a-1 ; i++){
        result[i][0] = D;
        result[i][b-1] = D;
        for(int j = 1; j < b-1 ; j++){
            char d = fgetc(fp);
            if(d == ','){
                if(j < b-2){
                    result[i][j] = -1;
                }else{
                    fprintf(stderr, "error: the length of line is more than %d\n",b-2);
                    exit(1);
                }
            }else if('0' <= d && d <= '3'){
                result[i][j] = d - '0';
                d = fgetc(fp);
                if(d != ',' && d != '\n'&& d != '\r'){
                    if((d == EOF) && (i == a-2) && (j == b-2)){
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
                if(j != b-2){
                    fprintf(stderr, "error: the length of line is less than %d\n",b-2);
                    exit(1);
                }
            }else if(d == EOF){
                if(i*j != (a-2)*(b-2)){
                    fprintf(stderr, "error: the field is less than %d,%d\n",a-2,b-2);
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

int isAnswer(int**ans){
    for(int  i= 1 ; i < a-1 ; i++){
        for(int j = 1 ; j < b-1 ; j++){
            if(abs(ans[i][j])!= 1) return 0;
        }
    }
    return 1;
}
void print_table(int**table){
    for(int  i= 1 ; i < a-1 ; i++){
        for(int j = 1 ; j < b-1 ; j++){
            printf("%d ",table[i][j]);
        }
        printf("\n");
    }
}
void print_answer(int **table,const int **x){
    for(int  i= 1 ; i < a-1 ; i++){
        for(int j = 1 ; j < b-1 ; j++){
            if(table[i][j]<0){
                printf("\x1b[47m");
                if(x[i][j] == D){
                    printf(" ");
                }else{
                    printf("\x1b[37m");
                    printf("%d",x[i][j]);
                    printf("\x1b[39m");
                }
                printf("\x1b[49m");
            }else{
                printf("\x1b[40m");
                if(x[i][j] == D){
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

int sign(int x){
    return (x>0)-(x<0);
}

int** copy_board(int** src) {
    int **dst = (int**)malloc(a * sizeof(int*));
    int *l = (int*)malloc(a * b * sizeof(int));
    for (int i = 0; i < a; i++) {
        dst[i] = &l[b * i];
    }
    memcpy(l, src[0], a * b * sizeof(int));
    return dst;
}

void free_board(int** board) {
    free(board[0]);
    free(board);
}

int root(int**ans,int s){
    if(abs(s)==1) return 1;
    int t = abs(s);
    if(ans[t/b][t%b] == t) return s;
    else return ans[t/b][t%b] =  sign(ans[t/b][t%b])*root(ans,abs(ans[t/b][t%b]));
}

void fill(int **ans,int s){
    ans[s/b][s%b] = 1;
    int s0 = s+1;
    if(ans[s0/b][s0%b] != 1)fill(ans,s0);
    s0 = s+b;
    if(ans[s0/b][s0%b] != 1)fill(ans,s0);
    s0 = s-b;
    if(ans[s0/b][s0%b] != 1)fill(ans,s0);
    s0 = s-1;
    if(ans[s0/b][s0%b] != 1)fill(ans,s0);
    return;
}

int isSeparate(int**ans){
    int count = 0;
    int **copy_ans= (int**)malloc(a*sizeof(int*));
    int *copy_l = (int*)malloc(a*b*sizeof(int));
    for(int i = 0 ; i < a ; i++){
        copy_ans[i] = &copy_l[b*i];
    }
    memcpy(copy_l,ans[0],a*b*sizeof(int));
    int s = 0;
    while(s < a*b){
        if(copy_l[s] !=1){
            fill(copy_ans,s);
            break;
        }
        s++;
    }
    s=0;
    while(s < a*b){
        if(copy_l[s] !=1){
            free(copy_ans);
            free(copy_l);
            return 1;
        }
        s++;
    }
    free(copy_ans);
    free(copy_l);
    return 0;

}

void union_(int**ans,int t0,int t1,int pm){
    if(flag)return;
    if(ans[t1/b][t1%b] == pm*ans[t0/b][t0%b])return;
    if(abs(root(ans,t0)) > abs(root(ans,t1))){
        int tmp = t0;
        t0 = t1;
        t1 = tmp;
    }
    if((ans[t1/b][t1%b] == ans[t0/b][t0%b] && pm == -1)||(ans[t1/b][t1%b] == -ans[t0/b][t0%b] && pm == 1)){
        flag++;
        return;
    }


    int s2 = ans[t1/b][t1%b];
    if(t1!=abs(s2)){
        ans[t1/b][t1%b] = pm*ans[t0/b][t0%b];
        union_(ans,t0,abs(s2),pm*sign(s2));
    }else{
        ans[t1/b][t1%b] = pm*ans[t0/b][t0%b];
    }
}

void check(const int** x,int** ans,int i,int j){
    if(flag)return;
    if(abs(ans[i][j])== 1&&abs(ans[i-1][j])== 1&&abs(ans[i][j-1])== 1&&abs(ans[i][j+1])== 1&&abs(ans[i+1][j])== 1)return;
    int s = i*b+j;
    int r = root(ans,s);

    int r1 = root(ans,s-b);
    int r2 = root(ans,s-1);
    int r3 = root(ans,s+1);
    int r4 = root(ans,s+b); 
    if(r1 == r2 && r1 == -root(ans,s-b-1) && r1 != r){union_(ans,s,s-1,1);check(x,ans,i,j-1);} 
    if(r1 == r3 && r1 == -root(ans,s-b+1) && r1 != r){union_(ans,s,s-b,1);check(x,ans,i-1,j);} 
    if(r4 == r2 && r4 == -root(ans,s+b-1) && r4 != r){union_(ans,s,s+b,1);check(x,ans,i+1,j);} 
    if(r4 == r3 && r4 == -root(ans,s+b+1) && r4 != r){union_(ans,s,s+1,1);check(x,ans,i,j+1);} 

    if(r1 == r2 && r2 == r3 && r3 == r4)union_(ans,s,s+1,1);
    else if(x[i][j]==0){
        union_(ans,s-b,s,1);
        union_(ans,s-1,s,1); 
        union_(ans,s,s+1,1);
        union_(ans,s,s+b,1);
        if(x[i+1][j+1] == 3){
            union_(ans, s+b+1, s+1, -1);
            union_(ans, s+b+1, s+b, -1);
        }
        if(x[i+1][j-1] == 3){
            union_(ans, s+b-1, s-1, -1);
            union_(ans, s+b-1, s+b, -1);
        }
        if(x[i-1][j+1] == 3){
            union_(ans, s-b+1, s+1, -1);
            union_(ans, s-b+1, s-b, -1);
        }
        if(x[i-1][j-1] == 3){
            union_(ans, s-b-1, s-1, -1);
            union_(ans, s-b-1, s-b, -1);
        }
    }
    else if(x[i][j]==1){
        if(r == -r1){union_(ans,s-1,s,1);union_(ans,s,s+1,1);union_(ans,s,s+b,1);}
        if(r == -r2){union_(ans,s-b,s,1);union_(ans,s,s+1,1);union_(ans,s,s+b,1);}
        if(r == -r3){union_(ans,s-b,s,1);union_(ans,s,s-1,1);union_(ans,s,s+b,1);}
        if(r == -r4){union_(ans,s-b,s,1);union_(ans,s,s-1,1);union_(ans,s,s+1,1);}
        if(r1 == r2){union_(ans,s-b,s,1);union_(ans,s+1,s+b,-1);}
        if(r2 == r3){union_(ans,s-1,s,1);union_(ans,s-b,s+b,-1);}
        if(r3 == r4){union_(ans,s,s+1,1);union_(ans,s-b,s-1,-1);}
        if(r4 == r1){union_(ans,s,s+b,1);union_(ans,s-1,s+1,-1);}
        if(r1 == r3){union_(ans,s-b,s,1);union_(ans,s-1,s+b,-1);}
        if(r2 == r4){union_(ans,s-1,s,1);union_(ans,s-b,s+1,-1);}
        
    }
    else if(x[i][j]==2){
        if(r1 == r2){union_(ans,s+1,s+b,1);union_(ans,s-1,s+b,-1);}
        if(r2 == r3){union_(ans,s-b,s+b,1);union_(ans,s-1,s+b,-1);}
        if(r3 == r4){union_(ans,s-1,s-b,1);union_(ans,s-b,s+1,-1);}
        if(r4 == r1){union_(ans,s-1,s+1,1);union_(ans,s-b,s+1,-1);}
        if(r1 == r3){union_(ans,s-1,s+b,1);union_(ans,s-b,s-1,-1);}
        if(r2 == r4){union_(ans,s-b,s+1,1);union_(ans,s+1,s+b,-1);}
    }
    else if(x[i][j]==3){
        if(r == r1){union_(ans,s-1,s,-1);union_(ans,s,s+1,-1);union_(ans,s,s+b,-1);}
        if(r == r2){union_(ans,s-b,s,-1);union_(ans,s,s+1,-1);union_(ans,s,s+b,-1);}
        if(r == r3){union_(ans,s-b,s,-1);union_(ans,s,s-1,-1);union_(ans,s,s+b,-1);}
        if(r == r4){union_(ans,s-b,s,-1);union_(ans,s,s-1,-1);union_(ans,s,s+1,-1);}
        if(r1 == r2){union_(ans,s-b,s,-1);union_(ans,s+1,s+b,-1);}
        if(r2 == r3){union_(ans,s-1,s,-1);union_(ans,s-b,s+b,-1);}
        if(r3 == r4){union_(ans,s,s+1,-1);union_(ans,s-b,s-1,-1);}
        if(r4 == r1){union_(ans,s,s+b,-1);union_(ans,s-1,s+1,-1);}
        if(r1 == r3){union_(ans,s-b,s,-1);union_(ans,s-1,s+b,-1);}
        if(r2 == r4){union_(ans,s-1,s,-1);union_(ans,s-b,s+1,-1);}
        if(x[i][j+1] == 3){
            union_(ans,s-1,s,-1);
            union_(ans,s,s+1,-1);
            union_(ans,s+1,s+2,-1);

            union_(ans,s-b,s-b+1,1);
            union_(ans,s+b,s+b+1,1);
        }
        if(x[i+1][j] == 3){
            union_(ans,s-b,s,-1);
            union_(ans,s,s+b,-1);
            union_(ans,s+b,s+b*2,-1);

            union_(ans,s+1,s+b+1,1);
            union_(ans,s-1,s+b-1,1);
        }
        if(x[i+1][j+1] == 3){
            union_(ans,s-b,s,-1);
            union_(ans,s-1,s,-1);
            union_(ans,s-b-1,s,-1);

            union_(ans,s+b+1,s+b+2,-1);
            union_(ans,s+b+1,s+b*2+1,-1);
            union_(ans,s+b+1,s+b*2+2,-1);

        }
        if(x[i+1][j-1] == 3){
            union_(ans, s, s+1, -1);
            union_(ans, s, s-b, -1);
            union_(ans, s, s-b+1, -1);

            union_(ans, s+b-1, s+b-2, -1);
            union_(ans, s+b-1, s+b*2-1, -1);
            union_(ans, s+b-1, s+b*2-2, -1);
        }
    }

}

void checks(const int** x,int** ans,int** copy_ans,int i,int j){
    while(memcmp(ans[0],copy_ans[0],a*b*sizeof(int))){
            memcpy(copy_ans[0],ans[0],a*b*sizeof(int));
            for(int i = 1; i < a-1 ;i++){
                for(int j = 1; j < b-1 ;j++){
                    check(x,ans,i,j);
                    if(flag)break;
                }
                if(flag)break;
            }
        }
}

void assume(const int** x,int** ans,int i,int j){
    int count = 0;
    int **copy_ans= (int**)malloc(a*sizeof(int*));
    int *copy_l = (int*)malloc(a*b*sizeof(int));
    int **copy_ans2= (int**)malloc(a*sizeof(int*));
    int *copy_l2 = (int*)malloc(a*b*sizeof(int));
    for(int i = 0 ; i < a ; i++){
        copy_ans[i] = &copy_l[b*i];
        copy_ans2[i] = &copy_l2[b*i];
    }
    if(x[i][j] == 1){
        flag = 0;
        int ss[4] = {-b,-1,1,b};
        int *ls[4];
        for(int n = 0; n < 4; n++){
            ls[n] =  (int*)malloc(a*b*sizeof(int));
            for(int m = 0;m < a*b ;m++){
                ls[n][m] =0;
            }
        }
        int notconflict=0;
        for(int n = 0;n<4;n++){
            memcpy(copy_l,ans[0],a*b*sizeof(int));
            memcpy(copy_l2,ans[0],a*b*sizeof(int));
            flag = 0;
            union_(copy_ans,i*b+j,i*b+j+ss[n],-1);
            checks(x,copy_ans,copy_ans2,i,j);
            if((!isSeparate(copy_ans))&& (!flag)){
                if(notconflict == 0){
                    notconflict = n+1;
                }
                else{
                    notconflict = -1;
                }
                for(int m=0; m<a*b;m++){
                    if(copy_l[m]!=ans[0][m]){
                        ls[n][m] = copy_l[m];
                    }
                }
            }
            else{
                ls[n][0]=-100;
            }
        }
        if(notconflict>0)union_(ans,i*b+j,i*b+j+ss[notconflict-1],-1);
        else if(notconflict==-1){
            int *same = (int*)malloc(a*b*sizeof(int));
            int counter=0;
            for(int n = 0;n<4;n++){
                if(ls[n][0]==-100)continue;
                if(counter == 0){
                    memcpy(same,ls[n],a*b*sizeof(int));
                }else{
                    for(int m = 0; m < a*b;m++){
                        if(same[m]!=ls[n][m])same[m] =0;
                    }
                }
                counter++;
            }
            for (int n = 0; n < a*b; n++)
            {
                if(same[n])union_(ans,abs(same[n]),n,sign(same[n]));
            }
            free(same);            
        }
        for(int n = 0; n < 4; n++){
            free(ls[n]);
        }
    }
    if(x[i][j] == 2){
        flag = 0;
        int ss1[6] = {-b,-1,1,b,-b,-1};
        int ss2[6] = {-1,1,b,-b,1,b};
        int *ls[6];
        for(int n = 0; n < 6; n++){
            ls[n] =  (int*)malloc(a*b*sizeof(int));
            for(int m = 0;m < a*b ;m++){
                ls[n][m] =0;
            }
        }

        int notconflict=0;
        for(int n = 0;n<6;n++){
            memcpy(copy_l,ans[0],a*b*sizeof(int));
            memcpy(copy_l2,ans[0],a*b*sizeof(int));
            flag = 0;
            union_(copy_ans,i*b+j+ss1[n],i*b+j+ss2[n],1);
            checks(x,copy_ans,copy_ans2,i,j);
            if((!isSeparate(copy_ans))&& (!flag)){
                if(notconflict == 0){
                    notconflict = n+1;
                }
                else{
                    notconflict = -1;
                }                
                for(int m=0; m<a*b;m++){
                    if(copy_l[m]!=ans[0][m]){
                        ls[n][m] = copy_l[m];
                    }
                }
            }
            else{
                ls[n][0]=-100;
            }
            
        }
        if(notconflict>0)union_(ans,i*b+j+ss2[notconflict-1],i*b+j+ss2[notconflict-1],1);
        else if(notconflict==-1){
            int *same = (int*)malloc(a*b*sizeof(int));
            int counter=0;
            for(int n = 0;n<6;n++){
                if(ls[n][0]==-100)continue;
                if(counter == 0){
                    memcpy(same,ls[n],a*b*sizeof(int));
                }else{
                    for(int m = 0; m < a*b;m++){
                        if(same[m]!=ls[n][m])same[m] =0;
                    }
                }
                counter++;
            }
            for (int n = 0; n < a*b; n++){
                if(same[n])union_(ans,abs(same[n]),n,sign(same[n]));
            }
            for(int n = 0; n < 6; n++){
                free(ls[n]);
            }
            
        }
    }
    if(x[i][j] == 3){
        flag = 0;
        int ss[4] = {-b,-1,1,b};
        int *ls[4];
        for(int n = 0; n < 4; n++){
            ls[n] =  (int*)malloc(a*b*sizeof(int));
            for(int m = 0;m < a*b ;m++){
                ls[n][m] =0;
            }
        }

        int notconflict=0;
        for(int n = 0;n<4;n++){
            memcpy(copy_l,ans[0],a*b*sizeof(int));
            memcpy(copy_l2,ans[0],a*b*sizeof(int));
            flag = 0;
            union_(copy_ans,i*b+j,i*b+j+ss[n],1);
            checks(x,copy_ans,copy_ans2,i,j);
            if((!isSeparate(copy_ans))&& (!flag)){
                if(notconflict == 0){
                    notconflict = n+1;
                }
                else{
                    notconflict = -1;
                }
                for(int m=0; m<a*b;m++){
                    if(copy_l[m]!=ans[0][m]){
                        ls[n][m] = copy_l[m];
                    }
                }
            }
        }
        if(notconflict>0)union_(ans,i*b+j,i*b+j+ss[notconflict-1],1);
        else if(notconflict==-1){
            int *same = (int*)malloc(a*b*sizeof(int));
            int counter=0;
            for(int n = 0;n<4;n++){
                if(ls[n][0]==-100)continue;
                if(counter == 0){
                    memcpy(same,ls[n],a*b*sizeof(int));
                }else{
                    for(int m = 0; m < a*b;m++){
                        if(same[m]!=ls[n][m])same[m] =0;
                    }
                }
                counter++;
            }
            for (int n = 0; n < a*b; n++)
            {
                if(same[n])union_(ans,abs(same[n]),n,sign(same[n]));
            }
            free(same);            
        }
    }
    int *ls[2];
    for(int n = 0; n < 2; n++){
        ls[n] =  (int*)malloc(a*b*sizeof(int));
        for(int m = 0;m < a*b ;m++){
            ls[n][m] =0;
        }
    }
    {memcpy(copy_l,ans[0],a*b*sizeof(int));
    memcpy(copy_l2,ans[0],a*b*sizeof(int));
    flag = 0;
    union_(copy_ans,1,i*b+j,1);
    checks(x,copy_ans,copy_ans2,i,j);

    if(isSeparate(copy_ans)||flag){
        free(copy_ans);
        free(copy_ans2);
        free(copy_l);
        free(copy_l2);
        free(ls[0]);
        free(ls[1]);
        flag = 0;
        union_(ans,1,i*b+j,-1);
        return;
        
    }else{
        for(int m=0; m<a*b;m++){
            if(copy_l[m]!=ans[0][m]){
                ls[0][m] = copy_l[m];
            }
        }
    }}
    {memcpy(copy_l,ans[0],a*b*sizeof(int));
    memcpy(copy_l2,ans[0],a*b*sizeof(int));
    flag = 0;
    union_(copy_ans,1,i*b+j,-1);
    checks(x,copy_ans,copy_ans2,i,j);

    if(isSeparate(copy_ans)||flag){
        free(copy_ans);
        free(copy_ans2);
        free(copy_l);
        free(copy_l2);
        free(ls[0]);
        free(ls[1]);
        flag=0;
        union_(ans,1,i*b+j,1);
        return;

    }else{
        for(int m=0; m<a*b;m++){
            if(copy_l[m]!=ans[0][m]){
                ls[0][m] = copy_l[m];
            }
        }
    }}
    for(int m = 0; m < a*b;m++){
        if(ls[0][m] != 0 && ls[0][m]==ls[1][m])union_(ans,abs(ls[0][m]),m,sign(ls[0][m]));
    }
    free(copy_ans);
    free(copy_ans2);
    free(copy_l);
    free(copy_l2);
    free(ls[0]);
    free(ls[1]);
    flag = 0;
    return;
}


int** search_recursive(const int **x, int **ans) {
    int **copy_ans = copy_board(ans);

    while(1) {
        flag = 0;
        
        for(int i = 1; i < a-1 ;i++){
            for(int j = 1; j < b-1 ;j++){
                check(x, ans, i, j);
                if(flag) break;
            }
            if(flag) break;
        }
        
        if (flag || isSeparate(ans)) {
            
            free_board(copy_ans);
            return NULL;
        }

        if (isAnswer(ans)) {

            free_board(copy_ans);
            return copy_board(ans); 
        }


        if(!memcmp(copy_ans[0], ans[0], a * b * sizeof(int))) {
            for(int i = 1; i < a-1 ;i++){
                for(int j = 1; j < b-1 ;j++){
                    if(abs(ans[i][j]) != 1){
                        assume(x, ans, i, j);
                        if(flag) break; 
                    }
                }
                if(flag) break;
            }
            
            if (flag || isSeparate(ans)) {
                free_board(copy_ans);
                return NULL;
            }
            
            if (!memcmp(copy_ans[0], ans[0], a * b * sizeof(int))) {
                break; 
            }
        }

        memcpy(copy_ans[0], ans[0], a * b * sizeof(int));
    }
    
    free_board(copy_ans);

    int target_i = -1, target_j = -1;
    for(int i = 1; i < a-1 ;i++){
        for(int j = 1; j < b-1 ;j++){
            if(abs(ans[i][j]) != 1){
                target_i = i;
                target_j = j;
                break;
            }
        }
        if(target_i != -1) break;
    }

    if (target_i == -1) return NULL; 


    int **next_ans_1 = copy_board(ans);
    flag = 0;
    union_(next_ans_1, 1, target_i * b + target_j, 1);
    int **res_1 = search_recursive(x, next_ans_1);
    free_board(next_ans_1);
    
    if (res_1 != NULL) {
        return res_1; 
    }

    int **next_ans_2 = copy_board(ans);
    flag = 0;
    union_(next_ans_2, 1, target_i * b + target_j, -1);
    int **res_2 = search_recursive(x, next_ans_2);
    free_board(next_ans_2);

    return res_2; 
}

int **search(const int **x){
    int **ans = (int**)malloc(a*sizeof(int*));
    int *l = (int*)malloc(a*b*sizeof(int));
    for(int i = 0 ; i < a ; i++){
        ans[i] = &l[b*i];
        if(i == 0 || i == a-1){
            for(int j = 0; j < b ;j++){
                ans[i][j] = 1;
            }
        }
        else{
            for(int j = 1; j < b-1 ;j++){
                ans[i][j] = i*b+j;
            }
            ans[i][0]= 1;
            ans[i][b-1]=1;
        }
    }

    int **final_ans = search_recursive(x, ans);
    
    if (final_ans == NULL) {
        fprintf(stderr, "error: Unsolvable or invalid puzzle.\n");
        return ans; 
    }

    free(ans[0]);
    free(ans);
    
    return final_ans; 
}

int main(int argc,char**argv){
    if(argc == 1){
        fprintf(stderr, "Usage: %s <puzzle file> \n", argv[0]);
        exit(1);
    }
    int **x;
    x = loadinfo(argv[1]);
    int **answer = search((const int **)x);
    print_answer(answer,(const int **)x);
    free(answer[0]);
    free(answer);
    free(x[0]);
    free(x);
    return 0;
}