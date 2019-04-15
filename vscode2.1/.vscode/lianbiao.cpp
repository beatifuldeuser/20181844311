#include <stdio.h>
#include <time.h>
#include <conio.h>
#include <string.h>

#define     MAXNN   100
#define     MAXN    10

typedef struct square
{
    int i[MAXNN];
    int remain[MAXNN];
    int totalCount, resultCount, partResultCount;
    int t_start, t_current;
    int stdsum;
    int rsum[MAXN],csum[MAXN];
    int working;
    int threshold_max[MAXN], threshold_min[MAXN];
} SQUARE;

typedef struct squarelist {
    int     i[MAXNN];
    struct squarelist  *next;
} SQUARENODE;

int     exam(SQUARE *ps);
void    print(SQUARE s, int direct);
void    msquare( SQUARE *ps, int no);

void    initSquare( SQUARE  *ps);
void    beginWorker(SQUARE *ps);

SQUARENODE *sl_append(SQUARE *ps); 
void    sl_free( SQUARENODE head);

int     NN;
int     N, interval;
SQUARENODE  head, *tail;

int main(int argc, char *argv[])
{
    SQUARE s;

    printf("input a number(3-5):");
    scanf("%d,%d", &N, &interval);
    if (interval<=0)
        interval = 1000;
    NN = N*N;

    head.next = NULL;
    tail = &head;

    initSquare(&s);

    
    msquare(&s, 0);
    memcpy( s.i, tail->i, NN*sizeof(int));
    print( s, true);

    char    filename[256];
    sprintf(filename, "%dx%d result.txt", N,N);
    
    sl_free(head);
    _getch();
}

void    beginWorker(SQUARE *ps) 
{
    ps->working = true;
    msquare( ps, 0);
}

void    initSquare( SQUARE  *ps) 
{

    ps->t_start = clock();
    ps->resultCount = ps->totalCount = ps->partResultCount = 0;

    int     i, temp;

    ps->working = false;
    ps->t_start = clock();
    temp = 0;    
    for(i=1;i<=NN;i++) {
        ps->i[i-1] = 0;
        ps->remain[i] = 1;
        temp += i;
    }
    ps->stdsum = temp / N;
    ps->t_current = 0;

    ps->threshold_max[N-1] = 0;
    temp = NN;
    for(i=1;i<N;i++) {
        ps->threshold_max[N-1-i] = temp+ps->threshold_max[N-1-i+1];
        temp--;
    }

    ps->threshold_min[N-1] = 0;
    temp = 1;
    for(i=1;i<N;i++) {
        ps->threshold_min[N-1-i] = temp+ps->threshold_min[N-1-i+1];
        temp++;
    }
        
}

SQUARENODE *sl_append(SQUARE *ps) 
{
    SQUARENODE  *newnode;

    newnode = new SQUARENODE;

    memcpy( newnode->i, ps->i, NN*sizeof(int) );
    newnode->next = tail->next;
    tail->next = newnode;
    tail = newnode;

    return newnode;
}

void    sl_free( SQUARENODE head)
{
    SQUARENODE  *node;

    while( head.next!=NULL) {
        node = head.next;
        head.next = node->next;
        delete node;
    }
}

void    calcThreshold( SQUARE *ps) 
{
    int     i,count = 0;

    ps->threshold_max[N-1] = 0;
    for(i=NN-1;i>=0;i--) {
        if (ps->remain[i]) {
            count ++;
            ps->threshold_max[N-1-count] = ps->threshold_max[N-1-count -1];
            if (count==N-1)
                break;
        }

    }

}

void    msquare( SQUARE *ps, int no)
{
    int     i;
    int     r, c, rsum, csum, temprsum, tempcsum;

    r = no / N;
    c = no % N;

    if (r==0) {
        ps->csum[c] = 0;
        if (c==0) {
            for(i=0;i<N;i++)
                ps->rsum[i]=0;
        }
    }
    rsum = ps->rsum[r];
    csum = ps->csum[c];

    if ( c>1 ) {
        if ( (rsum+ ps->threshold_max[c-1]<ps->stdsum) || rsum+ps->threshold_min[c-1]>ps->stdsum )
            return;
    }
    if ( r>1 ) {
        if ( (csum+ ps->threshold_max[r-1]<ps->stdsum) || csum+ps->threshold_min[r-1]>ps->stdsum )
            return;
    }

    for(i=1;i<=NN;i++) {
        if ( ps->remain[i]==0 )
            continue;

        ps->totalCount ++;
        tempcsum = csum + i;
        temprsum = rsum + i;
        if (temprsum>ps->stdsum || tempcsum>ps->stdsum)
            break;

        if ( c==N-1 && temprsum!=ps->stdsum)
            continue;
        if ( r==N-1 && tempcsum!=ps->stdsum)
            continue;
            
        
        ps->i[no] = i;
        ps->remain[i] = 0;
        ps->rsum[r] = temprsum;
        ps->csum[c] = tempcsum;
        if (no==NN-1) {
            ps->partResultCount ++;
            if (exam(ps)) {
                ps->resultCount ++;
                ps->t_current = clock()- ps->t_start;
                sl_append( ps );
                print(*ps, false);
            }
        } else
        {
            msquare( ps, no + 1);
        }
        ps->remain[i] = 1;
        ps->i[no] = 0;
    }
    ps->rsum[r] = rsum;
    ps->csum[c] = csum;

}

void print(SQUARE s, int direct)
{
    int j;

    if (s.resultCount%interval==0 || (direct) ) {
        printf("%-6d[ ", s.resultCount);
        for (j = 0; j < NN; j++)
            printf("%2d ", s.i[j]);
        printf("] %ld\r\n", clock() - s.t_start);
    }
}

int exam(SQUARE *ps)
{
    int j;
    int sum1, sum2;
    
    
    sum1 = 0;
    sum2 = 0;

    
    for (j = 0; j < N; j++)
    {
        sum1 += ps->i[j * N + j];     
        sum2 += ps->i[j * N + N - 1 - j]; 
    }

    
    if (sum1 != ps->stdsum || ps->stdsum != sum2)
        return 0;
    else
        return 1;
}

